#include "gcin.h"
#include "pho.h"

void load_tab_pho_file();

#if UNIX
#include <signal.h>

int pho_play(phokey_t key)
{
  if (!phonetic_speak)
    return 0;

  static int pid;
  static time_t last_time;
  time_t t = time(NULL);
  if (!gcin_sound_play_overlap) {
    if (pid && t - last_time < 2)
      kill(pid, 9);
  }
  char *ph = phokey_to_str2(key, 1);
  char tt[512];

  last_time = t;
  sprintf(tt, GCIN_OGG_DIR"/%s/%s", ph, phonetic_speak_sel);

  if (access(tt, R_OK))
    return 0;

  if (pid = fork()) {
    if (pid < 0)
      dbg("cannot fork ?");
    return 1;
  }

  close(1);
  close(2);
  execlp("ogg123", "ogg123", tt, NULL);
}
#else
void ErrorExit(LPTSTR lpszFunction);
void pho_play(phokey_t key)
{
  if (!phonetic_speak)
    return;

  static PROCESS_INFORMATION procinfo;
  static time_t last_time;
  time_t t = time(NULL);
#if 0
  // the result is bad on windows
  if (!gcin_sound_play_overlap) {
    if (procinfo.hProcess && t - last_time < 2)
      TerminateProcess(procinfo.hProcess, 0);
  }
#endif
  char *ph = phokey_to_str2(key, 1);
  char tt[512];

  last_time = t;
  if (procinfo.hProcess) {
    CloseHandle(procinfo.hProcess);
    CloseHandle(procinfo.hThread);
  }

  sprintf(tt, "\"%s\\bin\\oggdec.exe\" -p \"%s\\ogg\\%s\\%s\"", gcin_program_files_path, gcin_program_files_path, ph, phonetic_speak_sel);
  wchar_t tt16[MAX_PATH*2];

  utf8_to_16(tt, tt16, sizeof(tt16));
#if 0
  char pro[64];
  wchar_t pro16[64];
  strcpy(pro, gcin_program_files_path);
  strcat(pro, "\\oggdec.exe");
  utf8_to_16(pro, pro16, sizeof(tt16));
#endif
  STARTUPINFOW si;
//  PROCESS_INFORMATION pi;

  ZeroMemory( &si, sizeof(si) );
  si.cb = sizeof(si);
  ZeroMemory( &procinfo, sizeof(procinfo));

  if(!CreateProcessW(NULL, tt16, NULL,NULL,FALSE, CREATE_NO_WINDOW, NULL,NULL, &si, &procinfo)) {
#if _DEBUG
    ErrorExit("CreateProcessW");
    dbg("cannot exec %s\n", tt);
#endif
  }
}
#endif

void char_play(char *utf8)
{
  if (!phonetic_speak || !(utf8[0]&128))
    return;

  if (!ch_pho)
    load_tab_pho_file();

  phokey_t phos[16];
  int phosN = utf8_pho_keys((char *)utf8, phos);

  if (!phosN)
    return;

  int i;
  for(i=0; i < phosN; i++)
    pho_play(phos[i]);
}
