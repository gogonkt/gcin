#include "gcin.h"
#include "pho.h"
#include "gtab.h"
#include "win-sym.h"
#include "eggtrayicon.h"
#include <string.h>
#if UNIX
#include <signal.h>
#else
#include <process.h>
#endif
#include "gst.h"
#include "pho-kbm-name.h"

gboolean tsin_pho_mode();
extern int tsin_half_full, gb_output;
extern int win32_tray_disabled;
GtkStatusIcon *icon_main, *icon_state;

void get_icon_path(char *iconame, char fname[]);

void cb_trad_sim_toggle();

#if WIN32
void cb_sim2trad(GtkCheckMenuItem *checkmenuitem, gpointer dat)
{
  win32exec("sim2trad");
}
void cb_trad2sim(GtkCheckMenuItem *checkmenuitem, gpointer dat)
{
  win32exec("trad2sim");
}
#else
void cb_sim2trad(GtkCheckMenuItem *checkmenuitem, gpointer dat)
{
  system(GCIN_BIN_DIR"/sim2trad &");
}

void cb_trad2sim(GtkCheckMenuItem *checkmenuitem, gpointer dat)
{
  system(GCIN_BIN_DIR"/trad2sim &");
}
#endif


void cb_tog_phospeak(GtkCheckMenuItem *checkmenuitem, gpointer dat)
{
  phonetic_speak= gtk_check_menu_item_get_active(checkmenuitem);
}

void show_inmd_menu();

void cb_inmd_menu(GtkCheckMenuItem *checkmenuitem, gpointer dat)
{
  show_inmd_menu();
}

void close_all_clients();
void do_exit();

void restart_gcin(GtkCheckMenuItem *checkmenuitem, gpointer dat)
{
  do_exit();
}

void gcb_main();
void cb_tog_gcb(GtkCheckMenuItem *checkmenuitem, gpointer dat)
{
#if USE_GCB
  gcb_enabled = gtk_check_menu_item_get_active(checkmenuitem);
//  dbg("gcb_enabled %d\n", gcb_enabled);
  gcb_main();
#endif
}


void kbm_toggle(), exec_gcin_setup(), restart_gcin(), cb_trad2sim(), cb_sim2trad();

void cb_trad2sim(GtkCheckMenuItem *checkmenuitem, gpointer dat);

void cb_trad_sim_toggle_(GtkCheckMenuItem *checkmenuitem, gpointer dat)
{
  cb_trad_sim_toggle();
//  dbg("checkmenuitem %x\n", checkmenuitem);
}

void exec_gcin_setup_(GtkCheckMenuItem *checkmenuitem, gpointer dat)
{
  exec_gcin_setup();
}

void kbm_toggle_(GtkCheckMenuItem *checkmenuitem, gpointer dat)
{
  kbm_toggle();
}

gint inmd_switch_popup_handler (GtkWidget *widget, GdkEvent *event);
extern gboolean win_kbm_inited;

#include "mitem.h"
extern int win_kbm_on;

static MITEM mitems_main[] = {
  {N_(_L("設定")), GTK_STOCK_PREFERENCES, exec_gcin_setup_},
#if USE_GCB
  {N_(_L("gcb(剪貼區暫存)")), NULL, cb_tog_gcb, &gcb_enabled},
#endif
  {N_(_L("重新執行gcin")), GTK_STOCK_QUIT, restart_gcin},
  {N_(_L("念出發音")), NULL, cb_tog_phospeak, &phonetic_speak},
  {N_(_L("小鍵盤")), NULL, kbm_toggle_, &win_kbm_on},
#if UNIX
  {N_(_L("選擇輸入法")), NULL, cb_inmd_menu, NULL},
#endif
  {NULL}
};


void set_output_buffer_bak_to_clipboard();
void set_output_buffer_bak_to_clipboard();
static void cb_set_output_buffer_bak_to_clipboard(GtkCheckMenuItem *checkmenuitem, gpointer dat)
{
  set_output_buffer_bak_to_clipboard();
}

void load_setttings(), load_tab_pho_file();;
void update_win_kbm();
void update_win_kbm_inited();
extern gboolean win_kbm_inited;
static void cb_fast_phonetic_kbd_switch(GtkCheckMenuItem *checkmenuitem, gpointer dat)
{
  char bak[128], cur[128];
  get_gcin_conf_fstr(PHONETIC_KEYBOARD, cur, "");
  get_gcin_conf_fstr(PHONETIC_KEYBOARD_BAK, bak, "");

  save_gcin_conf_str(PHONETIC_KEYBOARD, bak);
  save_gcin_conf_str(PHONETIC_KEYBOARD_BAK, cur);
  load_setttings();
  load_tab_pho_file();
  update_win_kbm_inited();
}

static MITEM mitems_state[] = {
  {NULL, NULL, cb_fast_phonetic_kbd_switch},
  {N_(_L("正→簡體")), NULL, cb_trad2sim},
  {N_(_L("簡→正體")), NULL, cb_sim2trad},
  {N_(_L("简体输出")), NULL, cb_trad_sim_toggle_, &gb_output},
  {N_(_L("送字到剪貼區")), NULL, cb_set_output_buffer_bak_to_clipboard},
  {NULL}
};


static GtkWidget *tray_menu, *tray_menu_state;


void toggle_im_enabled();
GtkWidget *create_tray_menu(MITEM *mitems)
{
  GtkWidget *menu = gtk_menu_new ();

  int i;
  for(i=0; mitems[i].cb; i++) {
    GtkWidget *item;

    if (!mitems[i].name)
      continue;

    if (mitems[i].stock_id) {
      item = gtk_image_menu_item_new_with_label (_(mitems[i].name));
      gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), gtk_image_new_from_stock(mitems[i].stock_id, GTK_ICON_SIZE_MENU));
    }
    else
    if (mitems[i].check_dat) {
      item = gtk_check_menu_item_new_with_label (_(mitems[i].name));
      gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), *mitems[i].check_dat);
    } else
      item = gtk_menu_item_new_with_label (_(mitems[i].name));

    mitems[i].handler = g_signal_connect (G_OBJECT (item), "activate",
                      G_CALLBACK (mitems[i].cb), NULL);

    gtk_widget_show(item);
    mitems[i].item = item;

    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
  }

  return menu;
}

void _null_cb(GtkCheckMenuItem *checkmenuitem, gpointer dat)
{
}

void update_item_active(MITEM *mitems)
{
  int i;
  for(i=0; mitems[i].name; i++)
    if (mitems[i].check_dat) {
      GtkWidget *item = mitems[i].item;
      if (!item)
        continue;

      g_signal_handler_block(item, mitems[i].handler);
      gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), *mitems[i].check_dat);
      g_signal_handler_unblock(item, mitems[i].handler);
    }
}

void update_item_active_unix();

void update_item_active_all()
{
  if (gcin_win32_icon) {
    update_item_active(mitems_main);
    update_item_active(mitems_state);
  }
#if UNIX
  else
    update_item_active_unix();
#endif
}


void inmd_popup_tray();

static void cb_activate(GtkStatusIcon *status_icon, gpointer user_data)
{
#if UNIX
//  dbg("cb_activate\n");
  toggle_im_enabled();

  GdkRectangle rect;
  bzero(&rect, sizeof(rect));
  GtkOrientation ori;
  gtk_status_icon_get_geometry(status_icon, NULL, &rect, &ori);
#else
  inmd_popup_tray();
#endif
}

static void cb_popup(GtkStatusIcon *status_icon, guint button, guint activate_time, gpointer user_data)
{
//  dbg("cb_popup\n");
  switch (button) {
#if UNIX
    case 1:
      toggle_im_enabled();

      break;
    case 2:
      kbm_toggle();
      break;
#endif
    case 3:
      if (!tray_menu)
        tray_menu = create_tray_menu(mitems_main);
#if 0
      gtk_menu_popup(GTK_MENU(tray_menu), NULL, NULL, gtk_status_icon_position_menu, status_icon, button, activate_time);
#else
	  gtk_menu_popup(GTK_MENU(tray_menu), NULL, NULL, NULL, NULL, button, activate_time);
#endif
      break;
  }
}

void toggle_half_full_char();
static void cb_activate_state(GtkStatusIcon *status_icon, gpointer user_data)
{
//  dbg("cb_activate\n");
  toggle_half_full_char();
}


static void cb_popup_state(GtkStatusIcon *status_icon, guint button, guint activate_time, gpointer user_data)
{
//  dbg("cb_popup_state\n");
  switch (button) {

    case 1:
#if UNIX
      toggle_im_enabled();
      break;
    case 2:
#endif
      kbm_toggle();
      break;
    case 3:
    {
      char bak[128], cur[128];
      get_gcin_conf_fstr(PHONETIC_KEYBOARD, cur, "");
      get_gcin_conf_fstr(PHONETIC_KEYBOARD_BAK, bak, "");
      if (bak[0] && strcmp(bak, cur)) {
        char kbm[128];

        strcpy(kbm, bak);
        char *p=strchr(kbm, ' ');

        if (p) {
          *(p++)=0;
          int i;
          for(i=0;kbm_sel[i].name;i++)
            if (!strcmp(kbm_sel[i].kbm, kbm)) {
              break;
            }

          if (kbm_sel[i].kbm) {
            static unich_t tt[32];
#if UNIX
            sprintf(tt, "注音換 %s %s", kbm_sel[i].name, p);
#else
            swprintf(tt, L"注音換 %s %S", kbm_sel[i].name, p);
#endif
            mitems_state[0].name = tt;
          }
        }

        gtk_widget_destroy(tray_menu_state);
        tray_menu_state = NULL;
      }

      if (!tray_menu_state)
        tray_menu_state = create_tray_menu(mitems_state);

      gtk_menu_popup(GTK_MENU(tray_menu_state), NULL, NULL, NULL, NULL, button, activate_time);
      break;
    }
  }
}


#define GCIN_TRAY_PNG "gcin-tray.png"


void load_tray_icon_win32()
{
#if UNIX
  if (!gcin_win32_icon)
    return;
#endif

#if WIN32
  // when login, creating icon too early may cause block in gtk_status_icon_new_from_file
  if (win32_tray_disabled)
	  return;
#endif

//  dbg("load_tray_icon_win32\n");
#if UNIX
  char *tip;
  tip="";
#else
  wchar_t *tip;
  tip=L"";
#endif

  char *iconame;
  if (!current_CS || current_CS->im_state == GCIN_STATE_DISABLED||current_CS->im_state == GCIN_STATE_ENG_FULL) {
    iconame=GCIN_TRAY_PNG;
  } else {
    iconame=inmd[current_CS->in_method].icon;
  }

  char tt[32];
  if (current_CS && (current_method_type()==method_type_TSIN || current_method_type()==method_type_ANTHY) &&current_CS->im_state == GCIN_STATE_CHINESE && !tsin_pho_mode()) {
    strcpy(tt, "en-");
    strcat(tt, iconame);
    iconame = tt;
  }

//  dbg("iconame %s\n", iconame);
  char fname[128];
  fname[0]=0;
  if (iconame)
    get_icon_path(iconame, fname);


  char *icon_st=NULL;
  char fname_state[128];

//  dbg("%d %d\n",current_CS->im_state,current_CS->b_half_full_char);

  if (current_CS && (current_CS->im_state == GCIN_STATE_ENG_FULL ||
      current_CS->im_state != GCIN_STATE_DISABLED && current_CS->b_half_full_char ||
      current_method_type()==method_type_TSIN && tss.tsin_half_full)) {
		if (gb_output) {
          icon_st="full-simp.png";
		   tip = _L("全形/簡體輸出");
		}
		else {
          icon_st="full-trad.png";
		  tip = _L("全形/正體輸出");
		}
  } else {
	  if (gb_output) {
        icon_st="half-simp.png";
		tip= _L("半形/簡體輸出");
	  }
	  else {
        icon_st="half-trad.png";
		tip = _L("半形/正體輸出");
	  }
  }

  get_icon_path(icon_st, fname_state);
//  dbg("wwwwwwww %s\n", fname_state);


  if (icon_main) {
//    dbg("set %s %s\n", fname, fname_state);
    gtk_status_icon_set_from_file(icon_main, fname);
    gtk_status_icon_set_from_file(icon_state, fname_state);
  }
  else {
//    dbg("gtk_status_icon_new_from_file a\n");
    icon_main = gtk_status_icon_new_from_file(fname);
    g_signal_connect(G_OBJECT(icon_main),"activate", G_CALLBACK (cb_activate), NULL);
    g_signal_connect(G_OBJECT(icon_main),"popup-menu", G_CALLBACK (cb_popup), NULL);

//	dbg("gtk_status_icon_new_from_file %s b\n", fname_state);
    icon_state = gtk_status_icon_new_from_file(fname_state);
    g_signal_connect(G_OBJECT(icon_state),"activate", G_CALLBACK (cb_activate_state), NULL);
    g_signal_connect(G_OBJECT(icon_state),"popup-menu", G_CALLBACK (cb_popup_state), NULL);

//	dbg("icon %s %s\n", fname, fname_state);
  }

#if GTK_CHECK_VERSION(2,16,0)
  if (icon_state)
    gtk_status_icon_set_tooltip_text(icon_state, _(tip));
#endif

  if (icon_main) {
    char tt[64];
    if (current_CS && current_CS->in_method && inmd[current_CS->in_method].cname[0])
      strcpy(tt, inmd[current_CS->in_method].cname);

    if (!iconame || !strcmp(iconame, GCIN_TRAY_PNG) || !tsin_pho_mode())
      strcpy(tt, "English");
#if GTK_CHECK_VERSION(2,16,0)
    gtk_status_icon_set_tooltip_text(icon_main, tt);
#endif
  }

  return;
}

void init_tray_win32()
{
  load_tray_icon_win32();
}

void destroy_tray_win32()
{
  g_object_unref(icon_main); icon_main = NULL;
  g_object_unref(icon_state); icon_state = NULL;
}
