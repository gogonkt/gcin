#include <stdio.h>
#include "gcin.h"
#include "pho.h"

main()
{
  FILE *fp;
  char fnamein[]="pin-juyin.src";
  PIN_JUYIN pinju[1024];
  short pinjuN=0;

  if ((fp=fopen(fnamein, "r"))==NULL)
    p_err("cannot open %s", fnamein);

  while (!feof(fp)) {
    char tt[128];

    tt[0]=0;
    fgets(tt, sizeof(tt), fp);
    if (strlen(tt) < 3)
      break;

    char pin[7], ju[64];
    bzero(pin, sizeof(pin));
    sscanf(tt, "%s %s",pin, ju);

    phokey_t kk=0;
    int len = strlen(ju);
    int i=0;
    while (i<len) {
      kk |= lookup(&ju[i]);
      i+=utf8_sz(&ju[i]);
    }

    memcpy(pinju[pinjuN].pinyin, pin, sizeof(pinju[0].pinyin));
    pinju[pinjuN].key = kk;
    pinjuN++;
  }

  fclose(fp);
  dbg("pinjuN:%d\n", pinjuN);

  char fnameout[]="pin-juyin.xlt";

  if ((fp=fopen(fnameout, "w"))==NULL)
    p_err("cannot create %s", fnameout);

  fwrite(&pinjuN, sizeof(pinjuN), 1, fp);
  fwrite(pinju, sizeof(PIN_JUYIN), pinjuN, fp);
  fclose(fp);

  return 0;
}
