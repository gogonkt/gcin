#include "gcin.h"
#include "pho.h"
#include "tsin.h"
#include "gst.h"
#include "pho-status.h"

extern void key_typ_pho(phokey_t phokey, u_char rtyp_pho[]);

gboolean pin2juyin()
{
  int i;

  char pin[8];
  bzero(poo.typ_pho, sizeof(poo.typ_pho));
  if (poo.inph[0]=='`') {
    poo.typ_pho[0]=BACK_QUOTE_NO;
    poo.typ_pho[1]=poo.inph[1];
    return TRUE;
  }

  int inphN = strlen(poo.inph);
  for(i=0; i < pin_juyinN; i++) {
    bzero(pin, sizeof(pin));
    memcpy(pin,  pin_juyin[i].pinyin, sizeof(pin_juyin[0].pinyin));

    int pinN = strlen(pin);
    if (pinN < inphN)
      continue;

    if (!memcmp(pin, poo.inph, inphN)) {
//      dbg("pin %s %s\n", pin, poo.inph);
//      prph(pin_juyin[i].key);
      break;
    }
  }

  if (i==pin_juyinN) {
    dbg("pin2juyin not found ..\n");
    return FALSE;
  }

  bzero(poo.typ_pho, sizeof(poo.typ_pho));
//  prph(pin_juyin[i].key); dbg(" %x ph\n", pin_juyin[i].key);
  key_typ_pho(pin_juyin[i].key, (u_char *)poo.typ_pho);

//  dbg("pin2juyin found %d\n", poo.typ_pho[0]);

  return TRUE;
}


gboolean inph_typ_pho_pinyin(int newkey)
{
  int i;

//  dbg("inph_typ_pho_pinyin '%c'\n", newkey);

  if (newkey != ' ') {
    char num = phkbm.phokbm[newkey][0].num;
    int typ = phkbm.phokbm[newkey][0].typ;

//    dbg("cccc num %d typ:%d\n", num, typ);

    if (typ==3) {
      pin2juyin();
      poo.typ_pho[typ] = num;
//      tss.chpho[tss.c_idx].flag |=FLAG_CHPHO_PINYIN_TONE;
//      dbg("set %d\n",tss.c_idx);
//      if (!num)
//        poo.typ_pho[typ]= PHO_PINYIN_TONE1;
      return PHO_STATUS_OK_NEW | PHO_STATUS_TONE;
    }

//    dbg("'%c' %d\n", newkey, typ);

    for(i=0; i < 7; i++)
      if (!poo.inph[i])
        break;
    if (i==7)
      return FALSE;
    poo.inph[i] = newkey;
  }

  if (pin2juyin()) {
//    dbg("zzzz\n");
    if (newkey==' ')
      return PHO_STATUS_OK_NEW;
    if (poo.typ_pho[0]==BACK_QUOTE_NO && poo.typ_pho[1])
      return PHO_STATUS_OK_NEW | PHO_STATUS_TONE;
//    dbg("ok %d\n", poo.typ_pho[0]);
    return PHO_STATUS_OK;
  }

//  dbg("yyy %d\n", i);

  poo.inph[i]=0;
  if (!i)
    return PHO_STATUS_REJECT;

  // v is not used as the first key
  int j;
  for(j=0; j < pin_juyinN; j++)
    if (pin_juyin[j].pinyin[0]==newkey)
      break;

  int r = pin2juyin();
  if (j==pin_juyinN)
    return PHO_STATUS_REJECT;

  bzero(poo.inph, sizeof(poo.inph));
  poo.inph[0]=newkey;
  return PHO_STATUS_OK_NEW|PHO_STATUS_PINYIN_LEFT;
}

extern int text_pho_N;

void load_pin_juyin()
{
  text_pho_N = 6;
  char pinfname[128];

  get_sys_table_file_name("pin-juyin.xlt", pinfname);
  dbg("pinyin kbm %s\n", pinfname);

  FILE *fr;
  if ((fr=fopen(pinfname,"rb"))==NULL)
     p_err("Cannot open %s", pinfname);

  fread(&pin_juyinN, sizeof(short), 1, fr);
  pin_juyin = tmalloc(PIN_JUYIN, pin_juyinN);
  fread(pin_juyin, sizeof(PIN_JUYIN), pin_juyinN, fr);
  fclose(fr);
}
