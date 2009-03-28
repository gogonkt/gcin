#include "gcin.h"
#include "pho.h"
#include <sys/stat.h>
#include <stdlib.h>


char phofname[128]="";
extern char *TableDir;
u_short idxnum_pho;
PHO_IDX idx_pho[1403];
int ch_pho_ofs;
PHO_ITEM *ch_pho;
int ch_phoN;
static char pho_normal_tab[]="pho.tab";
static char pho_huge_tab[]="pho-huge.tab";
void update_table_file(char *name, int version);

void pho_load()
{
  char *pho_tab = phonetic_huge_tab ? pho_huge_tab:pho_normal_tab;

  if (!getenv("GCIN_TABLE_DIR") && phonetic_char_dynamic_sequence) {
    get_gcin_user_fname(pho_tab, phofname);

    if (access(phofname, W_OK) < 0){
      char sys_file[256], vv[256];

      get_sys_table_file_name(sys_file, pho_tab);
      sprintf(vv,"cp %s %s\n", sys_file, phofname);
      system(vv);
    }
  } else {
    get_sys_table_file_name(pho_tab, phofname);
    dbg("use system's pho, no dynamic adj\n");
  }

  update_table_file(pho_tab, 3);

  FILE *fr;

  if ((fr=fopen(phofname,"r"))==NULL)
    p_err("err %s\n", phofname);

  struct stat st;
  fstat(fileno(fr), &st);

  int count = st.st_size / sizeof(PHO_ITEM);

  fread(&idxnum_pho,sizeof(u_short),1,fr);
  fread(&idxnum_pho,sizeof(u_short),1,fr);
  fread(idx_pho, sizeof(PHO_IDX), idxnum_pho, fr);

  ch_pho_ofs = ftell(fr);

  if (ch_pho)
    free(ch_pho);

  if (!(ch_pho=(PHO_ITEM *)malloc(sizeof(PHO_ITEM) * count))) {
    p_err("malloc error");
  }

  ch_phoN=fread(ch_pho,sizeof(PHO_ITEM), count, fr);
//  dbg("ch_phoN:%d  %d\n", ch_phoN, idxnum_pho);
  fclose(fr);

  idx_pho[idxnum_pho].key=0xffff;
  idx_pho[idxnum_pho].start=ch_phoN;

#if 0
  int i;
  for(i=0; i <ch_phoN; i++) {
    char tt[5];

    utf8cpy(tt, ch_pho[i].ch);
    dbg("oooo %s\n", tt);
  }
#endif
}


void free_pho_mem()
{
  if (ch_pho)
    free(ch_pho);
}

typedef struct {
  phokey_t key;
  short count;
} PH_COUNT;

static int qcmp_pho_count(const void *aa, const void *bb)
{
  PH_COUNT *a = (PH_COUNT *)aa;
  PH_COUNT *b = (PH_COUNT *)bb;

  return b->count - a->count;
}


int utf8_pho_keys(char *utf8, phokey_t *phkeys)
{
  int i;
  int ofs=0;
  int phkeysN=0;
  PH_COUNT phcou[256];

  do {
    for(; ofs < ch_phoN; ofs++)
      if (utf8_eq(utf8, ch_pho[ofs].ch))
        break;

    if (ofs==ch_phoN)
      goto ret;

    for(i=0; i < idxnum_pho; i++) {
      if (idx_pho[i].start<= ofs && ofs < idx_pho[i+1].start) {
        phcou[phkeysN].count = ch_pho[ofs].count;
        phcou[phkeysN++].key = idx_pho[i].key;
        break;
      }
    }

    ofs++;
  } while (ofs < ch_phoN);

ret:

//  dbg("%s %d\n", utf8, phkeysN);
  qsort(phcou, phkeysN, sizeof(PH_COUNT), qcmp_pho_count);

  for(i=0; i < phkeysN; i++)
    phkeys[i] = phcou[i].key;

  return phkeysN;
}

char *phokey_to_str(phokey_t kk)
{
  u_int k1,k2,k3,k4;
  static u_char phchars[PHO_CHAR_LEN * 4 + 1];
  int phcharsN=0;

  k4=(kk&7);
  kk>>=3;
  k3=(kk&15) * PHO_CHAR_LEN;
  kk>>=4;
  k2=(kk&3) * PHO_CHAR_LEN;
  kk>>=2;
  k1=(kk&31) * PHO_CHAR_LEN;

  if (k1) {
    bchcpy(phchars, &pho_chars[0][k1]);
    phcharsN+=PHO_CHAR_LEN;
  }

  if (k2) {
    bchcpy(&phchars[phcharsN], &pho_chars[1][k2]);
    phcharsN+=PHO_CHAR_LEN;
  }

  if (k3)  {
    bchcpy(&phchars[phcharsN], &pho_chars[2][k3]);
    phcharsN+=PHO_CHAR_LEN;
  }

  if (k4)
    phchars[phcharsN++] = k4 + '0';

  phchars[phcharsN] = 0;

  return phchars;
}

void str_to_all_phokey_chars(char *b5_str, char *out)
{
  out[0]=0;

  while (*b5_str) {
    phokey_t phos[32];

    int n=utf8_pho_keys(b5_str, phos);

//    utf8_putchar(b5_str);
//    dbg("n %d\n", n);

    int i;
    for(i=0; i < n; i++) {
      char *pstr = phokey_to_str(phos[i]);
      strcat(out, pstr);
      if (i < n -1)
        strcat(out, " ");
    }

    b5_str+=utf8_sz(b5_str);

    if (*b5_str)
      strcat(out, " | ");
  }
}
