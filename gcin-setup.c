#include "gcin.h"
#include "gtab.h"
#include "config.h"
#include <libintl.h>


static GtkWidget *check_button_root_style_use,
                 *check_button_gcin_pop_up_win,
                 *check_button_gcin_inner_frame,
                 *check_button_gcin_status_tray,
                 *check_button_gcin_win_color_use;

static GtkWidget *opt_spc_opts;

static GtkWidget *gcin_kbm_window = NULL, *gcin_appearance_conf_window;
static GtkClipboard *pclipboard, *opt_gcin_edit_display;
GtkWidget *main_window;
static GdkColor gcin_win_gcolor_fg, gcin_win_gcolor_bg, gcin_sel_key_gcolor;

typedef struct {
  GdkColor *color;
  char **color_str;
  GtkColorSelectionDialog *color_selector;
  char *title;
} COLORSEL;

COLORSEL colorsel[2] =
  { {&gcin_win_gcolor_fg, &gcin_win_color_fg, "前景顏色"},
    {&gcin_win_gcolor_bg, &gcin_win_color_bg, "背景顏色"}
  };

struct {
  char *keystr;
  int keynum;
} edit_disp[] = {
  {"gcin視窗", GCIN_EDIT_DISPLAY_OVER_THE_SPOT},
  {"應用程式編輯區", GCIN_EDIT_DISPLAY_ON_THE_SPOT},
  {"同時顯示",  GCIN_EDIT_DISPLAY_BOTH},
  { NULL, 0},
};

static gboolean close_application( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{
  exit(0);
}


void create_kbm_window();

static void cb_kbm()
{
  create_kbm_window();
}


static void cb_tslearn()
{
  system("tslearn &");
  exit(0);
}


static void cb_ret(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_destroy(user_data);
}


static void create_result_win(int res)
{
  char *restr = res ? N_("結果失敗"):N_("結果成功");
  main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  GtkWidget *button = gtk_button_new_with_label(_(restr));
  gtk_container_add (GTK_CONTAINER (main_window), button);
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (cb_ret), main_window);

  gtk_widget_show_all(main_window);
}



static void cb_file_ts_export(GtkWidget *widget, gpointer user_data)
{
   GtkWidget *file_selector = (GtkWidget *)user_data;
   const gchar *selected_filename;

   selected_filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector));
//   g_print ("Selected filename: %s\n", selected_filename);

   char gcin_dir[512];

   get_gcin_dir(gcin_dir);

   char cmd[256];
   snprintf(cmd, sizeof(cmd), GCIN_BIN_DIR"/tsd2a32 %s/tsin32 > %s", gcin_dir, selected_filename);
   int res = system(cmd);
   create_result_win(res);
}

static void cb_ts_export()
{
   /* Create the selector */

   GtkWidget *file_selector = gtk_file_selection_new (_("請輸入要匯出的檔案名稱"));

   g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
                     "clicked",
                     G_CALLBACK (cb_file_ts_export),
                     (gpointer) file_selector);

   g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) file_selector);

   g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->cancel_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) file_selector);

   gtk_widget_show(file_selector);
}

static void cb_file_ts_import(GtkWidget *widget, gpointer user_data)
{
   GtkWidget *file_selector = (GtkWidget *)user_data;
   const gchar *selected_filename;

   selected_filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector));
//   g_print ("Selected filename: %s\n", selected_filename);

   char cmd[256];
   snprintf(cmd, sizeof(cmd),
      "cd %s/.gcin && "GCIN_BIN_DIR"/tsd2a32 tsin32 > tmpfile && cat %s >> tmpfile && "GCIN_BIN_DIR"/tsa2d32 tmpfile",
      getenv("HOME"), selected_filename);
   int res = system(cmd);
   create_result_win(res);
}

static void cb_ts_import()
{
   /* Create the selector */

   GtkWidget *file_selector = gtk_file_selection_new (_("請輸入要匯入的檔案名稱"));

   g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
                     "clicked",
                     G_CALLBACK (cb_file_ts_import),
                     (gpointer) file_selector);

   g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) file_selector);

   g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->cancel_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) file_selector);

   gtk_widget_show(file_selector);
}

char utf8_edit[]=GCIN_SCRIPT_DIR"/utf8-edit";
char html_browse[]=GCIN_SCRIPT_DIR"/html-browser";

static void cb_ts_edit()
{
  char tt[512];

  sprintf(tt, "( cd ~/.gcin && "GCIN_BIN_DIR"/tsd2a32 tsin32 > tmpfile && %s tmpfile && "GCIN_BIN_DIR"/tsa2d32 tmpfile ) &", utf8_edit);
  dbg("exec %s\n", tt);
  system(tt);
}


static void cb_ts_import_sys()
{
  char tt[512];

  sprintf(tt, "cd ~/.gcin && "GCIN_BIN_DIR"/tsd2a32 tsin32 > tmpfile && "GCIN_BIN_DIR"/tsd2a32 %s/tsin32 >> tmpfile && "GCIN_BIN_DIR"/tsa2d32 tmpfile", GCIN_TABLE_DIR);
  dbg("exec %s\n", tt);
  system(tt);
}


static void cb_alt_shift()
{
  char tt[512];

  sprintf(tt, "( cd ~/.gcin && %s phrase.table ) &", utf8_edit);
  system(tt);
}


static void cb_symbol_table()
{
  char tt[512];

  sprintf(tt, "( cd ~/.gcin && %s symbol-table ) &", utf8_edit);
  system(tt);
}


int utf8_editor(char *fname)
{
  char tt[256];

  sprintf(tt, "%s %s", utf8_edit, fname);
  dbg("%s\n", tt);
  return system(tt);
}


int html_browser(char *fname)
{
  char tt[256];

  sprintf(tt, "%s %s", html_browse, fname);
  dbg("%s\n", tt);
  return system(tt);
}


static void cb_help()
{
  html_browser(DOC_DIR"/README.html");
}

static GtkWidget *spinner_gcin_font_size, *spinner_gcin_font_size_tsin_presel,
                 *spinner_gcin_font_size_symbol,*spinner_gcin_font_size_pho_near,
                 *spinner_gcin_font_size_tsin_pho_in, *spinner_gcin_font_size_gtab_in, *spinner_root_style_x,
                 *spinner_root_style_y, *font_sel;

static GtkWidget *label_win_color_test, *event_box_win_color_test;

static gboolean cb_appearance_conf_ok( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{
  int font_size = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_gcin_font_size));
  save_gcin_conf_int(GCIN_FONT_SIZE, font_size);

#if GTK_24
  char fname[128];
  strcpy(fname, gtk_font_button_get_font_name(GTK_FONT_BUTTON(font_sel)));
  int len = strlen(fname)-1;

  while (len > 0 && isdigit(fname[len])) {
       fname[len--]=0;
  }

  while (len > 0 && fname[len]==' ') {
       fname[len--]=0;
  }

  save_gcin_conf_str(GCIN_FONT_NAME, fname);
#endif

  int font_size_tsin_presel = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_gcin_font_size_tsin_presel));
  save_gcin_conf_int(GCIN_FONT_SIZE_TSIN_PRESEL, font_size_tsin_presel);

  int font_size_symbol = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_gcin_font_size_symbol));
  save_gcin_conf_int(GCIN_FONT_SIZE_SYMBOL, font_size_symbol);

  int font_size_tsin_pho_in = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_gcin_font_size_tsin_pho_in));
  save_gcin_conf_int(GCIN_FONT_SIZE_TSIN_PHO_IN, font_size_tsin_pho_in);

  int font_size_pho_near = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_gcin_font_size_pho_near));
  save_gcin_conf_int(GCIN_FONT_SIZE_PHO_NEAR, font_size_pho_near);

  int font_size_gtab_in = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_gcin_font_size_gtab_in));
  save_gcin_conf_int(GCIN_FONT_SIZE_GTAB_IN, font_size_gtab_in);

  int gcin_pop_up_win = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gcin_pop_up_win));
  save_gcin_conf_int(GCIN_POP_UP_WIN, gcin_pop_up_win);

  int gcin_root_x = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_root_style_x));
  save_gcin_conf_int(GCIN_ROOT_X, gcin_root_x);

  int gcin_root_y = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_root_style_y));
  save_gcin_conf_int(GCIN_ROOT_Y, gcin_root_y);

  int style = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_root_style_use)) ?
            InputStyleRoot : InputStyleOverSpot;
  save_gcin_conf_int(GCIN_INPUT_STYLE, style);

  save_gcin_conf_int(GCIN_INNER_FRAME, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gcin_inner_frame)));
#if TRAY_ENABLED
  save_gcin_conf_int(GCIN_STATUS_TRAY, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gcin_status_tray)));
#endif

  gchar *cstr = gtk_color_selection_palette_to_string(&gcin_win_gcolor_fg, 1);
  dbg("color fg %s\n", cstr);
  save_gcin_conf_str(GCIN_WIN_COLOR_FG, cstr);
  g_free(cstr);

  cstr = gtk_color_selection_palette_to_string(&gcin_win_gcolor_bg, 1);
  dbg("color bg %s\n", cstr);
  save_gcin_conf_str(GCIN_WIN_COLOR_BG, cstr);
  g_free(cstr);

  save_gcin_conf_int(GCIN_WIN_COLOR_USE, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gcin_win_color_use)));

  cstr = gtk_color_selection_palette_to_string(&gcin_sel_key_gcolor, 1);
  dbg("selkey color %s\n", cstr);
  save_gcin_conf_str(GCIN_SEL_KEY_COLOR, cstr);

  int idx = gtk_option_menu_get_history (GTK_OPTION_MENU (opt_gcin_edit_display));
  save_gcin_conf_int(GCIN_EDIT_DISPLAY, edit_disp[idx].keynum);

  g_free(cstr);


  send_gcin_message(GDK_DISPLAY(), CHANGE_FONT_SIZE);
  gtk_widget_destroy(gcin_appearance_conf_window); gcin_appearance_conf_window = NULL;

  return TRUE;
}

static gboolean close_appearance_conf_window( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{
  gtk_widget_destroy(gcin_appearance_conf_window); gcin_appearance_conf_window = NULL;
  return TRUE;
}


static void cb_savecb_gcin_win_color_fg(GtkWidget *widget, gpointer user_data)
{
  COLORSEL *sel = user_data;
  GtkColorSelectionDialog *color_selector = sel->color_selector;
  GdkColor *col = sel->color;
  gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(color_selector->colorsel), col);

  if (sel->color == &gcin_win_gcolor_fg)
    gtk_widget_modify_fg(label_win_color_test, GTK_STATE_NORMAL, col);
  else {
    gtk_widget_modify_bg(event_box_win_color_test, GTK_STATE_NORMAL, col);
  }
}

static gboolean cb_gcin_win_color_fg( GtkWidget *widget,
                                   gpointer   data)
{
  COLORSEL *sel = data;
  GtkColorSelectionDialog *color_selector =
  (GtkColorSelectionDialog *)gtk_color_selection_dialog_new (sel->title);

  gdk_color_parse(*sel->color_str, sel->color);

  gtk_color_selection_set_current_color(
          GTK_COLOR_SELECTION(color_selector->colorsel),
          sel->color);

  g_signal_connect (GTK_OBJECT (color_selector->ok_button),
                    "clicked",
                    G_CALLBACK (cb_savecb_gcin_win_color_fg),
                    (gpointer) sel);

  sel->color_selector = color_selector;

  g_signal_connect_swapped (GTK_OBJECT (color_selector->ok_button),
                            "clicked",
                            G_CALLBACK (gtk_widget_destroy),
                            (gpointer) color_selector);

  g_signal_connect_swapped (GTK_OBJECT (color_selector->cancel_button),
                            "clicked",
                            G_CALLBACK (gtk_widget_destroy),
                            (gpointer) color_selector);

  gtk_widget_show((GtkWidget*)color_selector);
  return TRUE;
}

static GtkWidget *da_sel_key;

static void cb_save_gcin_sel_key_color(GtkWidget *widget, gpointer user_data)
{
  GtkColorSelectionDialog *color_selector = (GtkColorSelectionDialog *)user_data;
  gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(color_selector->colorsel), &gcin_sel_key_gcolor);

  gtk_widget_modify_bg(da_sel_key, GTK_STATE_NORMAL, &gcin_sel_key_gcolor);
}


static gboolean cb_gcin_sel_key_color( GtkWidget *widget, gpointer data)
{
   GtkColorSelectionDialog *color_selector = (GtkColorSelectionDialog *)gtk_color_selection_dialog_new (_("選擇鍵的顏色"));

   gtk_color_selection_set_current_color(
           GTK_COLOR_SELECTION(color_selector->colorsel),
           &gcin_sel_key_gcolor);

   g_signal_connect (GTK_OBJECT (color_selector->ok_button),
                     "clicked",
                     G_CALLBACK (cb_save_gcin_sel_key_color),
                     (gpointer) color_selector);
#if 1
   g_signal_connect_swapped (GTK_OBJECT (color_selector->ok_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) color_selector);
#endif
   g_signal_connect_swapped (GTK_OBJECT (color_selector->cancel_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) color_selector);

   gtk_widget_show((GtkWidget*)color_selector);
   return TRUE;
}

static GtkWidget *create_gcin_edit_display()
{

  GtkWidget *hbox = gtk_hbox_new (FALSE, 1);
  GtkWidget *label = gtk_label_new(_("編輯區顯示"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  opt_gcin_edit_display = gtk_option_menu_new ();
  gtk_box_pack_start (GTK_BOX (hbox), opt_gcin_edit_display, FALSE, FALSE, 0);
  GtkWidget *menu = gtk_menu_new ();

  int i, current_idx=0;

  for(i=0; edit_disp[i].keystr; i++) {
    GtkWidget *item = gtk_menu_item_new_with_label (edit_disp[i].keystr);

    if (edit_disp[i].keynum == gcin_edit_display)
      current_idx = i;

    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
  }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (opt_gcin_edit_display), menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (opt_gcin_edit_display), current_idx);

  return hbox;
}


void create_appearance_conf_window()
{
  if (gcin_appearance_conf_window) {
    gtk_window_present(GTK_WINDOW(gcin_appearance_conf_window));
    return;
  }

  load_setttings();

  gcin_appearance_conf_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  g_signal_connect (G_OBJECT (gcin_appearance_conf_window), "delete_event",
                    G_CALLBACK (close_appearance_conf_window),
                    NULL);

  gtk_window_set_title (GTK_WINDOW (gcin_appearance_conf_window), _("輸入視窗外觀設定"));
  gtk_container_set_border_width (GTK_CONTAINER (gcin_appearance_conf_window), 3);

  GtkWidget *vbox_top = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (gcin_appearance_conf_window), vbox_top);

  GtkWidget *hbox_gcin_font_size = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_gcin_font_size, FALSE, FALSE, 0);
  GtkWidget *label_gcin_font_size = gtk_label_new(_("字型大小"));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size), label_gcin_font_size, FALSE, FALSE, 0);
  GtkAdjustment *adj_gcin_font_size =
   (GtkAdjustment *) gtk_adjustment_new (gcin_font_size, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_gcin_font_size = gtk_spin_button_new (adj_gcin_font_size, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size), spinner_gcin_font_size, FALSE, FALSE, 0);


  GtkWidget *hbox_gcin_font_size_symbol = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_gcin_font_size_symbol, FALSE, FALSE, 0);
  GtkWidget *label_gcin_font_size_symbol = gtk_label_new(_("符號選擇視窗字型大小"));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_symbol), label_gcin_font_size_symbol, FALSE, FALSE, 0);
  GtkAdjustment *adj_gcin_font_size_symbol =
   (GtkAdjustment *) gtk_adjustment_new (gcin_font_size_symbol, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_gcin_font_size_symbol = gtk_spin_button_new (adj_gcin_font_size_symbol, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_symbol), spinner_gcin_font_size_symbol, FALSE, FALSE, 0);


  GtkWidget *hbox_gcin_font_size_tsin_presel = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_gcin_font_size_tsin_presel, FALSE, FALSE, 0);
  GtkWidget *label_gcin_font_size_tsin_presel = gtk_label_new(_("詞音預選詞視窗字型大小"));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_tsin_presel), label_gcin_font_size_tsin_presel, FALSE, FALSE, 0);
  GtkAdjustment *adj_gcin_font_size_tsin_presel =
   (GtkAdjustment *) gtk_adjustment_new (gcin_font_size_tsin_presel, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_gcin_font_size_tsin_presel = gtk_spin_button_new (adj_gcin_font_size_tsin_presel, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_tsin_presel), spinner_gcin_font_size_tsin_presel, FALSE, FALSE, 0);


  GtkWidget *hbox_gcin_font_size_tsin_pho_in = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_gcin_font_size_tsin_pho_in, FALSE, FALSE, 0);
  GtkWidget *label_gcin_font_size_tsin_pho_in = gtk_label_new(_("詞音注音輸入區字型大小"));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_tsin_pho_in), label_gcin_font_size_tsin_pho_in, FALSE, FALSE, 0);
  GtkAdjustment *adj_gcin_font_size_tsin_pho_in =
   (GtkAdjustment *) gtk_adjustment_new (gcin_font_size_tsin_pho_in, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_gcin_font_size_tsin_pho_in = gtk_spin_button_new (adj_gcin_font_size_tsin_pho_in, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_tsin_pho_in), spinner_gcin_font_size_tsin_pho_in, FALSE, FALSE, 0);


  GtkWidget *hbox_gcin_font_size_pho_near = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_gcin_font_size_pho_near, FALSE, FALSE, 0);
  GtkWidget *label_gcin_font_size_pho_near = gtk_label_new(_("詞音近似音顯示字型大小"));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_pho_near), label_gcin_font_size_pho_near, FALSE, FALSE, 0);
  GtkAdjustment *adj_gcin_font_size_pho_near =
   (GtkAdjustment *) gtk_adjustment_new (gcin_font_size_pho_near, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_gcin_font_size_pho_near = gtk_spin_button_new (adj_gcin_font_size_pho_near, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_pho_near), spinner_gcin_font_size_pho_near, FALSE, FALSE, 0);


  GtkWidget *hbox_gcin_font_size_gtab_in = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_gcin_font_size_gtab_in, FALSE, FALSE, 0);
  GtkWidget *label_gcin_font_size_gtab_in = gtk_label_new(_("gtab(倉頡…)輸入區字型大小"));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_gtab_in), label_gcin_font_size_gtab_in, FALSE, FALSE, 0);
  GtkAdjustment *adj_gcin_font_size_gtab_in =
   (GtkAdjustment *) gtk_adjustment_new (gcin_font_size_gtab_in, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_gcin_font_size_gtab_in = gtk_spin_button_new (adj_gcin_font_size_gtab_in, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_gtab_in), spinner_gcin_font_size_gtab_in, FALSE, FALSE, 0);

#if GTK_24
  char tt[128];
  sprintf(tt, "%s %d", gcin_font_name, gcin_font_size);
  font_sel = gtk_font_button_new_with_font (tt);
  gtk_box_pack_start (GTK_BOX (vbox_top), font_sel, FALSE, FALSE, 0);
#endif

  GtkWidget *hbox_gcin_pop_up_win = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX(vbox_top), hbox_gcin_pop_up_win, FALSE, FALSE, 0);
  GtkWidget *label_gcin_pop_up_win = gtk_label_new(_("彈出式輸入視窗"));
  gtk_box_pack_start (GTK_BOX(hbox_gcin_pop_up_win), label_gcin_pop_up_win, FALSE, FALSE, 0);
  check_button_gcin_pop_up_win = gtk_check_button_new ();
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_pop_up_win),
       gcin_pop_up_win);
  gtk_box_pack_start (GTK_BOX(hbox_gcin_pop_up_win), check_button_gcin_pop_up_win, FALSE, FALSE, 0);

  GtkWidget *frame_root_style = gtk_frame_new(_("固定 gcin 視窗位置"));
  gtk_box_pack_start (GTK_BOX (vbox_top), frame_root_style, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_root_style), 3);
  GtkWidget *vbox_root_style = gtk_vbox_new (FALSE, 10);
  gtk_container_add (GTK_CONTAINER (frame_root_style), vbox_root_style);

  GtkWidget *hbox_root_style_use = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX(vbox_root_style), hbox_root_style_use, FALSE, FALSE, 0);
  GtkWidget *label_root_style_use = gtk_label_new(_("使用"));
  gtk_box_pack_start (GTK_BOX(hbox_root_style_use), label_root_style_use, FALSE, FALSE, 0);
  check_button_root_style_use = gtk_check_button_new ();
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_root_style_use),
       gcin_input_style == InputStyleRoot);
  gtk_box_pack_start (GTK_BOX(hbox_root_style_use), check_button_root_style_use, FALSE, FALSE, 0);


  GtkWidget *hbox_root_style = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX(vbox_root_style), hbox_root_style, FALSE, FALSE, 0);

  GtkAdjustment *adj_root_style_x =
   (GtkAdjustment *) gtk_adjustment_new (gcin_root_x, 0.0, 1600.0, 1.0, 1.0, 0.0);
  spinner_root_style_x = gtk_spin_button_new (adj_root_style_x, 0, 0);
  gtk_container_add (GTK_CONTAINER (hbox_root_style), spinner_root_style_x);

  GtkAdjustment *adj_root_style_y =
   (GtkAdjustment *) gtk_adjustment_new (gcin_root_y, 0.0, 1200.0, 1.0, 1.0, 0.0);
  spinner_root_style_y = gtk_spin_button_new (adj_root_style_y, 0, 0);
  gtk_container_add (GTK_CONTAINER (hbox_root_style), spinner_root_style_y);


  gtk_box_pack_start (GTK_BOX(vbox_top), create_gcin_edit_display(), FALSE, FALSE, 0);

  GtkWidget *hbox_gcin_inner_frame = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX(vbox_top), hbox_gcin_inner_frame, FALSE, FALSE, 0);
  GtkWidget *label_gcin_inner_frame = gtk_label_new(_("顯示內框"));
  gtk_box_pack_start (GTK_BOX(hbox_gcin_inner_frame), label_gcin_inner_frame, FALSE, FALSE, 0);
  check_button_gcin_inner_frame = gtk_check_button_new ();
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_inner_frame),
       gcin_inner_frame);
  gtk_box_pack_start (GTK_BOX(hbox_gcin_inner_frame), check_button_gcin_inner_frame, FALSE, FALSE, 0);

#if TRAY_ENABLED
  GtkWidget *hbox_gcin_status_tray = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX(vbox_top), hbox_gcin_status_tray, FALSE, FALSE, 0);
  GtkWidget *label_gcin_status_tray = gtk_label_new(_("面板狀態(tray)"));
  gtk_box_pack_start (GTK_BOX(hbox_gcin_status_tray), label_gcin_status_tray, FALSE, FALSE, 0);
  check_button_gcin_status_tray = gtk_check_button_new ();
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_status_tray),
       gcin_status_tray);
  gtk_box_pack_start (GTK_BOX(hbox_gcin_status_tray), check_button_gcin_status_tray, FALSE, FALSE, 0);
#endif

  GtkWidget *frame_win_color = gtk_frame_new(_("顏色選擇"));
  gtk_box_pack_start (GTK_BOX (vbox_top), frame_win_color, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_win_color), 1);
  GtkWidget *vbox_win_color = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame_win_color), vbox_win_color);

  GtkWidget *hbox_win_color_use = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX(vbox_win_color), hbox_win_color_use, FALSE, FALSE, 0);
  GtkWidget *label_win_color_use = gtk_label_new(_("使用"));
  gtk_box_pack_start (GTK_BOX(hbox_win_color_use), label_win_color_use, FALSE, FALSE, 0);
  check_button_gcin_win_color_use = gtk_check_button_new ();
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_win_color_use),
       gcin_win_color_use);
  gtk_box_pack_start (GTK_BOX(hbox_win_color_use), check_button_gcin_win_color_use, FALSE, FALSE, 0);
  event_box_win_color_test = gtk_event_box_new();
  gtk_box_pack_start (GTK_BOX(vbox_win_color), event_box_win_color_test, FALSE, FALSE, 0);
  label_win_color_test = gtk_label_new(_("測試目前狀態"));
  gtk_container_add (GTK_CONTAINER(event_box_win_color_test), label_win_color_test);
  GtkWidget *hbox_win_color_fbg = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX(vbox_win_color), hbox_win_color_fbg, FALSE, FALSE, 0);
  GtkWidget *button_fg = gtk_button_new_with_label(_("前景顏色"));
  gtk_box_pack_start (GTK_BOX(hbox_win_color_fbg), button_fg, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_fg), "clicked",
                    G_CALLBACK (cb_gcin_win_color_fg), &colorsel[0]);
  GdkColor color;
  gdk_color_parse(gcin_win_color_fg, &gcin_win_gcolor_fg);
  gtk_widget_modify_fg(label_win_color_test, GTK_STATE_NORMAL, &gcin_win_gcolor_fg);
  gdk_color_parse(gcin_win_color_bg, &gcin_win_gcolor_bg);
  gtk_widget_modify_bg(event_box_win_color_test, GTK_STATE_NORMAL, &gcin_win_gcolor_bg);

  GtkWidget *button_bg = gtk_button_new_with_label(_("背景顏色"));
  gtk_box_pack_start (GTK_BOX(hbox_win_color_fbg), button_bg, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_bg), "clicked",
                    G_CALLBACK (cb_gcin_win_color_fg), &colorsel[1]);

  GtkWidget *frame_gcin_sel_key_color = gtk_frame_new(_("選擇鍵的顏色"));
  gtk_box_pack_start (GTK_BOX (vbox_win_color), frame_gcin_sel_key_color, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_gcin_sel_key_color), 1);
  GtkWidget *button_gcin_sel_key_color = gtk_button_new();
  g_signal_connect (G_OBJECT (button_gcin_sel_key_color), "clicked",
                    G_CALLBACK (cb_gcin_sel_key_color), G_OBJECT (gcin_kbm_window));
  da_sel_key =  gtk_drawing_area_new();
  gtk_container_add (GTK_CONTAINER (button_gcin_sel_key_color), da_sel_key);
  gdk_color_parse(gcin_sel_key_color, &gcin_sel_key_gcolor);
  gtk_widget_modify_bg(da_sel_key, GTK_STATE_NORMAL, &gcin_sel_key_gcolor);
  gtk_widget_set_size_request(da_sel_key, 16, 2);
  gtk_container_add (GTK_CONTAINER (frame_gcin_sel_key_color), button_gcin_sel_key_color);


  GtkWidget *hbox_cancel_ok = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_cancel_ok, FALSE, FALSE, 0);

  GtkWidget *button_cancel = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
  gtk_box_pack_start (GTK_BOX (hbox_cancel_ok), button_cancel, FALSE, FALSE, 0);

  g_signal_connect (G_OBJECT (button_cancel), "clicked",
                            G_CALLBACK (close_appearance_conf_window),
                            G_OBJECT (gcin_appearance_conf_window));

  GtkWidget *button_close = gtk_button_new_from_stock (GTK_STOCK_OK);
  gtk_box_pack_start (GTK_BOX (hbox_cancel_ok), button_close, TRUE, TRUE, 0);

  g_signal_connect_swapped (G_OBJECT (button_close), "clicked",
                            G_CALLBACK (cb_appearance_conf_ok),
                            G_OBJECT (gcin_kbm_window));

  GTK_WIDGET_SET_FLAGS (button_close, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (button_close);

  gtk_widget_show_all (gcin_appearance_conf_window);

  return;
}


static void cb_appearance_conf()
{
  create_appearance_conf_window();
}


static void cb_gtab_conf()
{
  create_gtab_conf_window();
}



static void cb_gb_output_toggle()
{
  send_gcin_message(GDK_DISPLAY(), GB_OUTPUT_TOGGLE);
  exit(0);
}


static void cb_gb_translate_toggle()
{
  system(GCIN_BIN_DIR"/sim2trad &");
  exit(0);
}


static void cb_juying_learn()
{
  system(GCIN_BIN_DIR"/juyin-learn &");
  exit(0);
}


int gcin_pid;

static void cb_gcin_exit()
{
  kill(gcin_pid, 9);
}


void create_gtablist_window();
static void cb_default_input_method()
{
  create_gtablist_window();
}

void create_about_window();

static void create_main_win()
{
  main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  g_signal_connect (G_OBJECT (main_window), "delete_event",
                     G_CALLBACK (close_application),
                     NULL);

  gtk_window_set_icon_from_file(GTK_WINDOW(main_window), SYS_ICON_DIR"/gcin.png", NULL);


  GtkWidget *vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (main_window), vbox);

  GtkWidget *button_kbm = gtk_button_new_with_label(_("gcin 注音/詞音設定"));
  gtk_box_pack_start (GTK_BOX (vbox), button_kbm, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_kbm), "clicked",
                    G_CALLBACK (cb_kbm), NULL);

  GtkWidget *button_appearance_conf = gtk_button_new_with_label(_("外觀設定"));
  gtk_box_pack_start (GTK_BOX (vbox), button_appearance_conf, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_appearance_conf), "clicked",
                    G_CALLBACK (cb_appearance_conf), NULL);

  GtkWidget *button_gtab_conf = gtk_button_new_with_label(_("倉頡/行列/嘸蝦米/大易設定"));
  gtk_box_pack_start (GTK_BOX (vbox), button_gtab_conf, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_gtab_conf), "clicked",
                    G_CALLBACK (cb_gtab_conf), NULL);


  GtkWidget *button_default_input_method = gtk_button_new_with_label(_("內定輸入法 & 開啟/關閉"));
  gtk_box_pack_start (GTK_BOX (vbox), button_default_input_method, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_default_input_method), "clicked",
                    G_CALLBACK (cb_default_input_method), NULL);



  GtkWidget *button_alt_shift = gtk_button_new_with_label(_("alt-shift 片語編輯"));
  gtk_box_pack_start (GTK_BOX (vbox), button_alt_shift, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_alt_shift), "clicked",
                    G_CALLBACK (cb_alt_shift), NULL);

  GtkWidget *button_symbol_table = gtk_button_new_with_label(_("符號表編輯"));
  gtk_box_pack_start (GTK_BOX (vbox), button_symbol_table, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_symbol_table), "clicked",
                    G_CALLBACK (cb_symbol_table), NULL);

  GtkWidget *button_gb_output_toggle = gtk_button_new_with_label(_("簡體字輸出切換"));
  gtk_box_pack_start (GTK_BOX (vbox), button_gb_output_toggle, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_gb_output_toggle), "clicked",
                    G_CALLBACK (cb_gb_output_toggle), NULL);

  GtkWidget *button_gb_translate_toggle = gtk_button_new_with_label(_("剪貼區 簡體字->正體字"));
  gtk_box_pack_start (GTK_BOX (vbox), button_gb_translate_toggle, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_gb_translate_toggle), "clicked",
                    G_CALLBACK (cb_gb_translate_toggle), NULL);

  GtkWidget *button_juying_learn_toggle = gtk_button_new_with_label(_("剪貼區 注音查詢"));
  gtk_box_pack_start (GTK_BOX (vbox), button_juying_learn_toggle, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_juying_learn_toggle), "clicked",
                    G_CALLBACK (cb_juying_learn), NULL);

#if GTK_MAJOR_VERSION >=2 && GTK_MINOR_VERSION >= 4
  GtkWidget *expander_ts = gtk_expander_new (_("詞庫選項"));
  gtk_box_pack_start (GTK_BOX (vbox), expander_ts, FALSE, FALSE, 0);
  GtkWidget *vbox_ts = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (expander_ts), vbox_ts);
#else
  GtkWidget *vbox_ts = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), vbox_ts, FALSE, FALSE, 0);
#endif

  GtkWidget *button_ts_export = gtk_button_new_with_label(_("詞庫匯出"));
  gtk_box_pack_start (GTK_BOX (vbox_ts), button_ts_export, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_ts_export), "clicked",
                    G_CALLBACK (cb_ts_export), NULL);

  GtkWidget *button_ts_import = gtk_button_new_with_label(_("詞庫匯入"));
  gtk_box_pack_start (GTK_BOX (vbox_ts), button_ts_import, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_ts_import), "clicked",
                    G_CALLBACK (cb_ts_import), NULL);

  GtkWidget *button_ts_edit = gtk_button_new_with_label(_("詞庫編輯"));
  gtk_box_pack_start (GTK_BOX (vbox_ts), button_ts_edit, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_ts_edit), "clicked",
                    G_CALLBACK (cb_ts_edit), NULL);

  GtkWidget *button_tslearn = gtk_button_new_with_label(_("讓詞音從文章學習詞"));
  gtk_box_pack_start (GTK_BOX (vbox_ts), button_tslearn, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_tslearn), "clicked",
                    G_CALLBACK (cb_tslearn), NULL);

  GtkWidget *button_ts_import_sys = gtk_button_new_with_label(_("匯入系統的詞庫"));
  gtk_box_pack_start (GTK_BOX (vbox_ts), button_ts_import_sys, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_ts_import_sys), "clicked",
                    G_CALLBACK (cb_ts_import_sys), NULL);

  GtkWidget *button_about = gtk_button_new_with_label(_("關於 gcin"));
  gtk_box_pack_start (GTK_BOX (vbox), button_about, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_about), "clicked",
                    G_CALLBACK (create_about_window),  NULL);


  GtkWidget *button_help = gtk_button_new_from_stock (GTK_STOCK_HELP);
  gtk_box_pack_start (GTK_BOX (vbox), button_help, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_help), "clicked",
                    G_CALLBACK (cb_help), NULL);

  char *pid = getenv("GCIN_PID");

  if (pid && (gcin_pid = atoi(pid))) {
    GtkWidget *button_gcin_exit = gtk_button_new_with_label (_("結束 gcin"));
    gtk_box_pack_start (GTK_BOX (vbox), button_gcin_exit, TRUE, TRUE, 0);
    g_signal_connect (G_OBJECT (button_gcin_exit), "clicked",
                      G_CALLBACK (cb_gcin_exit), NULL);
  }


  GtkWidget *button_quit = gtk_button_new_from_stock (GTK_STOCK_QUIT);
  gtk_box_pack_start (GTK_BOX (vbox), button_quit, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_quit), "clicked",
                    G_CALLBACK (close_application), NULL);

  gtk_widget_show_all(main_window);
}


void init_TableDir(), exec_setup_scripts();

int main(int argc, char **argv)
{
//  char *messages=getenv("LC_MESSAGES");
#if 0
  char *ctype=getenv("LC_CTYPE");
  if (!(ctype && strstr(ctype, "zh_CN")))
    putenv("LANGUAGE=zh_TW.UTF-8");
#endif
  exec_setup_scripts();

  init_TableDir();

  load_setttings();

#if GCIN_i18n_message
  gtk_set_locale();
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);
#endif

  gtk_init(&argc, &argv);

  create_main_win();

  // once you invoke gcin-setup, the left-right buton tips is disabled
  save_gcin_conf_int(LEFT_RIGHT_BUTTON_TIPS, 0);

  pclipboard = gtk_clipboard_get(GDK_SELECTION_PRIMARY);

  gtk_main();

  return 0;
}
