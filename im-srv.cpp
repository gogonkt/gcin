#if UNIX
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <X11/Xatom.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#else
#include <winsock.h>
#endif

#include "gcin.h"
#include "gcin-protocol.h"
#include "im-srv.h"
#include <gdk/gdk.h>

#if UNIX
int im_sockfd, im_tcp_sockfd;
Atom get_gcin_sockpath_atom(Display *dpy);
Server_IP_port srv_ip_port;
static Window prop_win;
static Atom addr_atom;
#else
SOCKET im_tcp_sockfd;
#endif

#ifdef __cplusplus
extern "C" gint gdk_input_add	  (gint		     source,
			   GdkInputCondition condition,
			   GdkInputFunction  function,
			   gpointer	     data);
#endif
void gdk_input_remove	  (gint		     tag);


void get_gcin_im_srv_sock_path(char *outstr, int outstrN);
void process_client_req(int fd);

static gboolean cb_read_gcin_client_data(GIOChannel *source, GIOCondition condition, gpointer data)
{
  int fd=GPOINTER_TO_INT(data);

  process_client_req(fd);
  return TRUE;
}

#if UNIX
Atom get_gcin_addr_atom(Display *dpy);

static void gen_passwd_idx()
{
  srv_ip_port.passwd.seed = (rand() >> 1) % __GCIN_PASSWD_N_;

  Server_IP_port tsrv_ip_port = srv_ip_port;

  to_gcin_endian_4(&srv_ip_port.passwd.seed);
  XChangeProperty(dpy, prop_win , addr_atom, XA_STRING, 8,
     PropModeReplace, (unsigned char *)&tsrv_ip_port, sizeof(srv_ip_port));

  XSync(GDK_DISPLAY(), FALSE);
}
#endif

#if WIN32
typedef int socklen_t;
#endif

static gboolean cb_new_gcin_client(GIOChannel *source, GIOCondition condition, gpointer data)
{
  Connection_type type=(Connection_type) GPOINTER_TO_INT(data);
#if 0
  dbg("im-srv: cb_new_gcin_client %s\n", type==Connection_type_unix ? "unix":"tcp");
#endif
  unsigned int newsockfd;
  socklen_t clilen;

#if UNIX
  if (type==Connection_type_unix) {
    struct sockaddr_un cli_addr;

    bzero(&cli_addr, sizeof(cli_addr));
    clilen=0;
    newsockfd = accept(im_sockfd,(struct sockaddr *) & cli_addr, &clilen);
  } else
#endif
  {
    struct sockaddr_in cli_addr;

    bzero(&cli_addr, sizeof(cli_addr));
    clilen=sizeof(cli_addr);
    newsockfd = accept(im_tcp_sockfd,(struct sockaddr *) & cli_addr, &clilen);
  }

  if (newsockfd < 0) {
    perror("accept");
    return FALSE;
  }

//  dbg("newsockfd %d\n", newsockfd);

  if (newsockfd >= gcin_clientsN) {
    gcin_clients = trealloc(gcin_clients, GCIN_ENT, newsockfd+1);
    gcin_clientsN = newsockfd;
  }

  bzero(&gcin_clients[newsockfd], sizeof(gcin_clients[0]));

#if WIN32
  gcin_clients[newsockfd].tag = g_io_add_watch(g_io_channel_win32_new_socket(newsockfd), G_IO_IN, cb_read_gcin_client_data,
              GINT_TO_POINTER(newsockfd));
#else
  gcin_clients[newsockfd].tag = g_io_add_watch(g_io_channel_unix_new(newsockfd), G_IO_IN, cb_read_gcin_client_data,
              GINT_TO_POINTER(newsockfd));
#endif
#if UNIX
  if (type==Connection_type_tcp) {
    gcin_clients[newsockfd].seed = srv_ip_port.passwd.seed;
    gen_passwd_idx();
  }
#endif
  gcin_clients[newsockfd].type = type;
  return TRUE;
}

#if UNIX
static int get_ip_address(u_int *ip)
{
  char hostname[64];

  if (gethostname(hostname, sizeof(hostname)) < 0) {
    perror("cannot get hostname\n");
    return -1;
  }

  dbg("hostname %s\n", hostname);
  struct hostent *hent;

  if (!(hent=gethostbyname(hostname))) {
    dbg("cannot call gethostbyname to get IP address");
    return -1;
  }

  memcpy(ip, hent->h_addr_list[0], hent->h_length);
  return 0;
}
#endif

#if WIN32

static int my_port;
static HWND hwndNextViewer;
static UINT uFormat = (UINT)(-1);
void get_selection();
void disp_gcb_selection(const gchar *text);

LRESULT wndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
#if 0
// a very unreliable design in win32, cygwin/X sometimes flood WM_DRAWCLIPBOARD to gcin
		case WM_CREATE:
			hwndNextViewer = SetClipboardViewer(hwnd);
			break;
        case WM_CHANGECBCHAIN:
#if 1
//			dbg("WM_CHANGECBCHAIN\n");
            if ((HWND) wp == hwndNextViewer)
              hwndNextViewer = (HWND) lp;
            else if (hwndNextViewer != NULL)
              SendMessage(hwndNextViewer, msg, wp, lp);
			break;
#endif
		case WM_DRAWCLIPBOARD:
#if _DEBUG
			dbg("WM_DRAWCLIPBOARD\n");
#endif

			{
#if 1
				get_selection();
#else
				static UINT auPriorityList[] = {
					CF_UNICODETEXT
                };
                uFormat = GetPriorityClipboardFormat(auPriorityList, 1);
				if (OpenClipboard(hwnd)) {
                  HGLOBAL hglb = GetClipboardData(uFormat);
                  LPWSTR lpstr = (LPWSTR) GlobalLock(hglb);
				  char buf[4096];
				  utf16_to_8(lpstr, buf, sizeof(buf));
				  disp_gcb_selection(buf);
                  GlobalUnlock(hglb);
                  CloseClipboard();
                }
#endif
			}

			if (hwndNextViewer)
				SendMessage(hwndNextViewer, msg, wp, lp);

			break;
#endif
		case GCIN_PORT_MESSAGE:
//			dbg("wndProc %d\n", my_port);
			return my_port;
		default:
			return DefWindowProc(hwnd, msg, wp, lp);
	}
	return 0;
}

void ErrorExit(LPTSTR lpszFunction);
extern HINSTANCE g_inst;
void create_win32_svr_hwnd()
{
	WNDCLASSEX wc;
	wc.cbSize         = sizeof(WNDCLASSEX);
	wc.style          = 0;
	wc.lpfnWndProc    = (WNDPROC)wndProc;
	wc.cbClsExtra     = 0;
	wc.cbWndExtra     = 0;
	wc.hInstance      = (HINSTANCE)GetModuleHandleA(NULL);
	wc.hCursor        = NULL;
	wc.hIcon          = NULL;
	wc.lpszMenuName   = (LPTSTR)NULL;
	wc.lpszClassName  = GCIN_WIN_NAME;
	wc.hbrBackground  = NULL;
	wc.hIconSm        = NULL;

	if( !RegisterClassEx( (LPWNDCLASSEX)&wc ) )
		return;

	HWND hwnd = CreateWindowExA(0, GCIN_WIN_NAME, NULL, 0,
		0, 0, 0, 0, HWND_DESKTOP, NULL, wc.hInstance, NULL);

	if (!hwnd)
		ErrorExit("CreateWindowEx");

//	dbg("svr_hwnd %x\n", hwnd);
}
#endif

void init_gcin_im_serv(Window win)
{
  int servlen;

  dbg("init_gcin_im_serv\n");

#if UNIX
  prop_win = win;
  struct sockadd_un;
  struct sockaddr_un serv_addr;

  // unix socket
  bzero(&serv_addr,sizeof(serv_addr));
  serv_addr.sun_family = AF_UNIX;
  char sock_path[128];
  get_gcin_im_srv_sock_path(sock_path, sizeof(sock_path));
  strcpy(serv_addr.sun_path, sock_path);

#ifdef SUN_LEN
  servlen = SUN_LEN (&serv_addr);
#else
  servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);
#endif

  dbg("-- %s\n",serv_addr.sun_path);
  struct stat st;

  if (!stat(serv_addr.sun_path, &st)) {
    if (unlink(serv_addr.sun_path) < 0) {
      char tt[512];
      snprintf(tt, sizeof(tt), "unlink error %s", serv_addr.sun_path);
      perror(tt);
    }
  }


  if ((im_sockfd = socket(AF_UNIX,SOCK_STREAM,0)) < 0) {
    perror("cannot open unix socket");
    exit(-1);
  }

  if (bind(im_sockfd, (struct sockaddr *) &serv_addr, servlen) < 0) {
    perror("cannot bind");
    exit(-1);
  }

  listen(im_sockfd,2);

  dbg("im_sockfd:%d\n", im_sockfd);

#if 0
  gdk_input_add(im_sockfd, GDK_INPUT_READ, cb_new_gcin_client,
                GINT_TO_POINTER(Connection_type_unix));
#else
  g_io_add_watch(g_io_channel_unix_new(im_sockfd), G_IO_IN, cb_new_gcin_client,
                GINT_TO_POINTER(Connection_type_unix));
#endif

  Display *dpy = GDK_DISPLAY();

  Server_sock_path srv_sockpath;
  strcpy(srv_sockpath.sock_path, sock_path);
  Atom sockpath_atom = get_gcin_sockpath_atom(dpy);
  XChangeProperty(dpy, prop_win , sockpath_atom, XA_STRING, 8,
     PropModeReplace, (unsigned char *)&srv_sockpath, sizeof(srv_sockpath));

  addr_atom = get_gcin_addr_atom(dpy);
  XSetSelectionOwner(dpy, addr_atom, win, CurrentTime);

  if (!gcin_remote_client) {
    dbg("connection via TCP is disabled\n");
    return;
  }
#endif

  // tcp socket
#if UNIX
  if (get_ip_address(&srv_ip_port.ip) < 0)
    return;
#else
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        dbg("WSAStartup failed with error: %d\n", err);
        exit(1);
    }
#endif

  if ((im_tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("cannot open tcp socket");
    exit(-1);
  }

//  dbg("socket succ\n");

  struct sockaddr_in serv_addr_tcp;
  u_short port;

  for(port=9999; port < 20000; port++)
  {
    // tcp socket
    bzero(&serv_addr_tcp, sizeof(serv_addr_tcp));
    serv_addr_tcp.sin_family = AF_INET;
#if WIN32
    serv_addr_tcp.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr_tcp.sin_port = htons(port);
	if (bind(im_tcp_sockfd, (struct sockaddr *) &serv_addr_tcp, sizeof(serv_addr_tcp))!=SOCKET_ERROR) {
		break;
	}

	dbg("after bind\n");
#else
    serv_addr_tcp.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr_tcp.sin_port = htons(port);
    if (bind(im_tcp_sockfd, (struct sockaddr *) &serv_addr_tcp, sizeof(serv_addr_tcp)) == 0)
      break;
#endif
  }

#if WIN32
  if (port == 20000)
	  p_err("Cannot bind port");
  my_port = port;

  dbg("----- my_port %d\n", port);
#endif


#if 0
  u_char *myip = (char *)&srv_ip_port.ip;
#endif
#if UNIX
  srv_ip_port.port = serv_addr_tcp.sin_port;
  dbg("server port bind to %s:%d\n", inet_ntoa(serv_addr_tcp.sin_addr), port);
  time_t t;
  srand(time(&t));

  int i;
  for(i=0; i < __GCIN_PASSWD_N_; i++) {
    srv_ip_port.passwd.passwd[i] = (rand()>>2) & 0xff;
  }
#endif

  if (listen(im_tcp_sockfd, 5) < 0) {
    perror("cannot listen: ");
	exit(1);
  }

  dbg("after listen\n");


#if UNIX
  gen_passwd_idx();
#endif

#if WIN32
  g_io_add_watch(g_io_channel_win32_new_socket(im_tcp_sockfd), G_IO_IN, cb_new_gcin_client,
                GINT_TO_POINTER(Connection_type_tcp));
#else
  g_io_add_watch(g_io_channel_unix_new(im_tcp_sockfd), G_IO_IN, cb_new_gcin_client,
                GINT_TO_POINTER(Connection_type_tcp));
#endif

//  printf("im_sockfd: %d\n",im_sockfd);
#if WIN32
	create_win32_svr_hwnd();
#endif
}
