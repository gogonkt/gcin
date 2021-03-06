#include "gcin.h"
#include "intcode.h"

GtkWidget *gwin_int;
extern int current_intcode;

static GtkWidget *button_int;
static GtkWidget *labels_int[MAX_INTCODE];

static struct {
  char *name;
} int_sel[]={
  {"Big5"},
  {"UTF-32(U+)"},
};
static int int_selN = sizeof(int_sel)/sizeof(int_sel[0]);

static GtkWidget *opt_int_opts;

static void minimize_win()
{
  gtk_window_resize(GTK_WINDOW(gwin_int), 10, 10);
}

static void adj_intcode_buttons()
{
  int i;

  if (current_intcode==INTCODE_UTF32) {
    for(i=4;i < MAX_INTCODE; i++)
      gtk_widget_show(labels_int[i]);
  } else {
    for(i=4;i < MAX_INTCODE; i++)
      gtk_widget_hide(labels_int[i]);
  }

  minimize_win();
}


static void cb_select( GtkWidget *widget, gpointer data)
{
#if GTK_CHECK_VERSION(2,4,0)
  current_intcode = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
#else
  current_intcode = gtk_option_menu_get_history (GTK_OPTION_MENU (widget));
#endif
  adj_intcode_buttons();
}

static GtkWidget *create_int_opts()
{

  GtkWidget *hbox = gtk_hbox_new (FALSE, 1);

#if GTK_CHECK_VERSION(2,4,0)
  opt_int_opts = gtk_combo_box_new_text ();
#else
  opt_int_opts = gtk_option_menu_new ();
  GtkWidget *menu_int_opts = gtk_menu_new ();
#endif
  gtk_box_pack_start (GTK_BOX (hbox), opt_int_opts, FALSE, FALSE, 0);

  int i;
  for(i=0; i < int_selN; i++) {
#if GTK_CHECK_VERSION(2,4,0)
    gtk_combo_box_append_text (GTK_COMBO_BOX_TEXT (opt_int_opts), int_sel[i].name);
#else
    GtkWidget *item = gtk_menu_item_new_with_label (int_sel[i].name);

    gtk_menu_shell_append (GTK_MENU_SHELL (menu_int_opts), item);
#endif
  }

#if GTK_CHECK_VERSION(2,4,0)
  gtk_combo_box_set_active (GTK_COMBO_BOX (opt_int_opts), current_intcode);
#else
  gtk_option_menu_set_menu (GTK_OPTION_MENU (opt_int_opts), menu_int_opts);
  gtk_option_menu_set_history (GTK_OPTION_MENU (opt_int_opts), current_intcode);
#endif
  g_signal_connect (G_OBJECT (opt_int_opts), "changed", G_CALLBACK (cb_select), NULL);

  return hbox;
}


void disp_int(int index, char *intcode)
{
  gtk_label_set_text(GTK_LABEL(labels_int[index]), intcode);
}

static unich_t full_space[]=_L("　");

void clear_int_code(int index)
{
  gtk_label_set_text(GTK_LABEL(labels_int[index]), _(full_space));
}


#if 0
static void switch_intcode()
{
  current_intcode ^= 1;
  adj_intcode_buttons();
}
#endif


void clear_int_code_all()
{
  int i;

  for(i=0; i < MAX_INTCODE; i++)
    clear_int_code(i);
}

void hide_win_int()
{
  if (!gwin_int)
    return;
  gtk_widget_hide(gwin_int);
}

extern gboolean force_show;
extern int intcode_cin;

void move_win_int(int x, int y)
{
  if (!gwin_int)
    return;

  dbg("move_win_int %d %d\n",x,y);

  gtk_window_get_size(GTK_WINDOW(gwin_int), &win_xl, &win_yl);

  if (x + win_xl > dpy_xl)
    x = dpy_xl - win_xl;
  if (x < 0)
    x = 0;

  if (y + win_yl > dpy_yl)
    y = dpy_yl - win_yl;
  if (y < 0)
    y = 0;

  win_x = x;  win_y = y;
  gtk_window_move(GTK_WINDOW(gwin_int), x, y);
}

void show_win_int();

void create_win_intcode()
{
  if (gwin_int) {
    show_win_int();
    return;
  }

  gwin_int = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_has_resize_grip(GTK_WINDOW(gwin_int), FALSE);
#if WIN32
  set_no_focus(gwin_int);
#endif

//  gtk_window_set_default_size(GTK_WINDOW (gwin_int), 1, 1);
  gtk_container_set_border_width (GTK_CONTAINER (gwin_int), 0);

  GdkWindow *gdkwin = gtk_widget_get_window(gwin_int);

  GtkWidget *frame = gtk_frame_new(NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 0);
  gtk_container_add (GTK_CONTAINER(gwin_int), frame);


  GtkWidget *hbox_top = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), hbox_top);

  GtkWidget *button_intcode = gtk_button_new_with_label(_(_L("內碼")));
  g_signal_connect_swapped (GTK_OBJECT (button_intcode), "button_press_event",
        G_CALLBACK (inmd_switch_popup_handler), NULL);
  gtk_box_pack_start (GTK_BOX (hbox_top), button_intcode, FALSE, FALSE, 0);

  button_int = gtk_button_new();
  gtk_container_set_border_width (GTK_CONTAINER (button_int), 0);
  gtk_box_pack_start (GTK_BOX (hbox_top), button_int, FALSE, FALSE, 0);
  GtkWidget *hbox_int = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (button_int), hbox_int);

  int i;
  for(i=0; i < MAX_INTCODE;i ++) {
    GtkWidget *label = gtk_label_new(_(full_space));
    labels_int[i] = label;
    gtk_box_pack_start (GTK_BOX (hbox_int), label, FALSE, FALSE, 0);
    set_label_font_size(label, gcin_font_size);
  }

  GtkWidget *intsel = create_int_opts();
  gtk_box_pack_start (GTK_BOX (hbox_top), intsel, FALSE, FALSE, 0);

  gtk_widget_show_all (gwin_int);

  gtk_widget_realize (gwin_int);
#if WIN32
  win32_init_win(gwin_int);
#else
  set_no_focus(gwin_int);
#endif

  adj_intcode_buttons();
  minimize_win();
}

void show_win_int()
{
  if (!gwin_int)
    return;

#if 0
  if (gcin_pop_up_win && !intcode_cin && !force_show)
    return;
#endif

#if UNIX
  if (!GTK_WIDGET_VISIBLE(gwin_int))
#endif
  {
    gtk_widget_show(gwin_int);
    move_win_int(win_x, win_y);
  }

  gtk_widget_show(gwin_int);
}


void get_win_int_geom()
{
  if (!gwin_int)
    return;

  gtk_window_get_position(GTK_WINDOW(gwin_int), &win_x, &win_y);

  get_win_size(gwin_int, &win_xl, &win_yl);
}
