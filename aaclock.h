/* Globals and Constants*/
#define MINIMIZED_SIZE_X 5	/*How many pixel are visible when clock is hiding */
#define MAXIMIZED_SIZE_X 95	/*You should change fonts and text alignment same time with this */
#define MINIMIZED_COLOR "grey" /**/
#define BACKGROUND_COLOR "grey"	/*I suggest MINIMIZED_COLOR=BACKGROUND_COLOR (may flickers) */
#define BORDER_COLOR "blue"	/*When clock is locked on the screen there are borders. */
int g_position_x;		/*current X-position, this is allways 0   */
int g_position_y;		/*current Y-position, this changing       */
int g_size_x;			/*width  of the window  */
int g_size_y;			/*height of the window  */
int g_dragging_y;		/*when user moves clock, we store the old position here */

/*Boolean-like 1=true 0=false*/
unsigned char g_force_maximized;
unsigned char g_button1_pressed;
unsigned char g_minimized;

Display *g_display;
Window g_root_win;
Window g_main_win;		/*actually only window */
int g_screen;

GC g_gc;
#ifndef XFT			/*we need two GC if we are without XFT */
GC g_gc2;
#endif

#ifdef XFT
#define XFT_FONT_TIME "system-16"
#define XFT_FONT_DATE "system-10"
#else
#define FONT_NAME_TIME "-*-lucida-medium-r-*-sans-12-*-*-*-*-*-*-*"
#define FONT_NAME_DATE "-*-system-medium-r-*-sans-10-*-*-*-*-*-*-*"
#endif

/* Selects color. */
unsigned long
select_color (char *color)
{
  Colormap cmap;
  XColor c0, c1;
  cmap = DefaultColormap (g_display, 0);
  XAllocNamedColor (g_display, cmap, color, &c1, &c0);
  return (c1.pixel);
}


/*these are from fspanel*/
#define MWM_HINTS_DECORATIONS         (1L << 1)
typedef struct _mwmhints
{
  unsigned long flags;
  unsigned long functions;
  unsigned long decorations;
  long inputMode;
  unsigned long status;
}
g_mwmhints;

#define WIN_STATE_STICKY          (1<<0)	/* everyone knows sticky */
#define WIN_STATE_MINIMIZED       (1<<1)	/* ??? */
#define WIN_STATE_MAXIMIZED_VERT  (1<<2)	/* window in maximized V state */
#define WIN_STATE_MAXIMIZED_HORIZ (1<<3)	/* window in maximized H state */
#define WIN_STATE_HIDDEN          (1<<4)	/* not on taskbar but window visible */
#define WIN_STATE_SHADED          (1<<5)	/* shaded (NeXT style) */
#define WIN_STATE_HID_WORKSPACE   (1<<6)	/* not on current desktop */
#define WIN_STATE_HID_TRANSIENT   (1<<7)	/* owner of transient is hidden */
#define WIN_STATE_FIXED_POSITION  (1<<8)	/* window is fixed in position even */
#define WIN_STATE_ARRANGE_IGNORE  (1<<9)	/* ignore for auto arranging */

#define WIN_HINTS_SKIP_FOCUS      (1<<0)	/* "alt-tab" skips this win */
#define WIN_HINTS_SKIP_WINLIST    (1<<1)	/* not in win list */
#define WIN_HINTS_SKIP_TASKBAR    (1<<2)	/* not on taskbar */
#define WIN_HINTS_GROUP_TRANSIENT (1<<3)	/* ??????? */
#define WIN_HINTS_FOCUS_ON_CLICK  (1<<4)	/* app only accepts focus when clicked */
#define WIN_HINTS_DO_NOT_COVER    (1<<5)	/* attempt to not cover this window */

char *atom_names[] = {
  /* clients */
  "KWM_WIN_ICON",
  "WM_STATE",
  "_MOTIF_WM_HINTS",
  "_NET_WM_STATE",
  "_NET_WM_STATE_SKIP_TASKBAR",
  "_NET_WM_STATE_SHADED",
  "_NET_WM_DESKTOP",
  "_NET_WM_WINDOW_TYPE",
  "_NET_WM_WINDOW_TYPE_DOCK",	/* 8 */
  "_NET_WM_STRUT",
  "_WIN_HINTS",
  /* root */
  "_NET_CLIENT_LIST",
  "_NET_NUMBER_OF_DESKTOPS",
  "_NET_CURRENT_DESKTOP",
  "_WIN_LAYER",
  "_WIN_STATE"
};

#define ATOM_COUNT (sizeof (atom_names) / sizeof (atom_names[0]))
Atom g_atoms[ATOM_COUNT];
#define atom_KWM_WIN_ICON g_atoms[0]
#define atom_WM_STATE g_atoms[1]
#define atom__MOTIF_WM_HINTS g_atoms[2]
#define atom__NET_WM_STATE g_atoms[3]
#define atom__NET_WM_STATE_SKIP_TASKBAR g_atoms[4]
#define atom__NET_WM_STATE_SHADED g_atoms[5]
#define atom__NET_WM_DESKTOP g_atoms[6]
#define atom__NET_WM_WINDOW_TYPE g_atoms[7]
#define atom__NET_WM_WINDOW_TYPE_DOCK g_atoms[8]
#define atom__NET_WM_STRUT g_atoms[9]
#define atom__WIN_HINTS g_atoms[10]
#define atom__NET_CLIENT_LIST g_atoms[11]
#define atom__NET_NUMBER_OF_DESKTOPS g_atoms[12]
#define atom__NET_CURRENT_DESKTOP g_atoms[13]
#define atom__WIN_LAYER g_atoms[14]
#define atom__WIN_STATE g_atoms[15]


#ifdef HELP_WITH_EVENTS
char *g_name_of_events[] = {
  "error",
  "reply last",
  "KeyPress               ",
  "KeyRelease             ",
  "ButtonPress            ",
  "ButtonRelease          ",
  "MotionNotify           ",
  "EnterNotify            ",
  "LeaveNotify            ",
  "FocusIn                ",
  "FocusOut               ",
  "KeymapNotify           ",
  "Expose                 ",
  "GraphicsExpose         ",
  "NoExpose               ",
  "VisibilityNotify       ",
  "CreateNotify           ",
  "DestroyNotify          ",
  "UnmapNotify            ",
  "MapNotify              ",
  "MapRequest             ",
  "ReparentNotify         ",
  "ConfigureNotify        ",
  "ConfigureRequest       ",
  "GravityNotify          ",
  "ResizeRequest          ",
  "CirculateNotify        ",
  "CirculateRequest       ",
  "PropertyNotify         ",
  "SelectionClear         ",
  "SelectionRequest       ",
  "SelectionNotify        ",
  "ColormapNotify         ",
  "ClientMessage          ",
  "MappingNotify          "
};
#endif
