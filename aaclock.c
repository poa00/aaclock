/*************************************************************
 *       aaclock                                             *
 *       v 1.0  (18. February 2008)                          *
 *       by Aapo Rantalainen                                 *
 *                                                           *
 *                                                           *
 * Digital clock:                                            *
 * Pops visible when mouse enters.                           *
 * Hides automatically when mouse leaves.                    *
 * Clicking it will lock it on the screen.                   *
 * When clocked, pressing button and dragging will change    *
 *  the position of it                                       *
 *  (It stays on the left side of the screen still)          *
 *                                                           *
 *                                                           *
 * about code:  GPL version 3 or any later version           *
 * Global variables and constants are in header-file         *
 * Indentions are python-like, no exceptions of it           *
 * All variables are lowercase, spaces replaced with _       *
 * CONSTANS are uppercase, spaces replaced with _            *
 *   (there are exceptions: atom_MWMHints etc.)              *
 * All global variables start with g_                        *
 * Names of functions contains only lowercase letters        *
 *  (and _ )                                                 *
 *                                                           *
 * todo: non-xft fonts do not work properly                  *
 * (Xft is a simple library designed to interface the        *
 *  FreeType rasterizer with the X Rendering Extension)      *
 *                                                           *
 ************************************************************/

#include <stdio.h>		/*debug messages */
#include <time.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <gcrypt.h>		/*fd_set etc. */
#include <strings.h>		/*bzero */
#include "aaclock.h"

/*make drawing little bit easier*/
#define draw_line(x,y,a,b) XDrawLine (g_display, g_main_win, g_gc, x, y, a, b)
#define fill_rect(x,y,a,b) XFillRectangle (g_display, g_main_win, g_gc, x, y, a, b)

#ifdef XFT
#include <X11/Xft/Xft.h>
XftDraw *xftdraw;
XftFont *xfs_time;
XftFont *xfs_date;
#else
XFontStruct *xfs_time;
XFontStruct *xfs_date;
#endif


/*draws the clock*/
void
draw_clock (void)
{
#ifdef XFT
  XftColor xft_color;
#endif

  unsigned char *time_string;	/*hh:mm:ss  */
  unsigned char *day_string;	/*Feb  9    */
  unsigned char *year_string;	/*2008      */

  time_t now;
  now = time (0);

  time_string = (unsigned char *) (ctime (&now) + 11);
  day_string = (unsigned char *) (ctime (&now) + 4);
  year_string = (unsigned char *) (ctime (&now) + 20);

  if (g_minimized)
    {
      XSetForeground (g_display, g_gc, select_color (MINIMIZED_COLOR));
      fill_rect (0, 0, g_size_x, g_size_y);
      return;
    }

  XSetForeground (g_display, g_gc, select_color (BACKGROUND_COLOR));
  fill_rect (0, 0, g_size_x, g_size_y);

  if (g_force_maximized)	/* do borders */
    {
      XSetForeground (g_display, g_gc, select_color (BORDER_COLOR));
      /* top */
      draw_line (0, 0, g_size_x, 0);
      draw_line (0, 1, g_size_x, 1);

      /* bottom */
      draw_line (0, g_size_y - 1, g_size_x, g_size_y - 1);
      draw_line (0, g_size_y - 2, g_size_x, g_size_y - 2);

      /* left - one pixel width is enough */
      draw_line (0, 0, 0, g_size_y - 1);

      /* right */
      draw_line (g_size_x - 1, 0, g_size_x - 1, g_size_y - 1);
      draw_line (g_size_x - 2, 0, g_size_x - 2, g_size_y - 1);
    }

#ifdef XFT
  xft_color.color.alpha = 0xffff;
  /*I don't know how this work, but now it is black */
  xft_color.color.red = 0;
  xft_color.color.green = 0;
  xft_color.color.blue = 0;

  XftDrawStringUtf8 (xftdraw, &xft_color, xfs_time, 1, 20, time_string, 8);
  /*fix, tasta tulee viela warning */
  /*XftDrawStringUtf8 (xftdraw, &xft_color, xfs_time, +1,  +20, "007007007", +8); */
  XftDrawStringUtf8 (xftdraw, &xft_color, xfs_date, 5, 36, day_string, 6);
  XftDrawStringUtf8 (xftdraw, &xft_color, xfs_date, 55, 36, year_string, 4);
#else
  XSetForeground (g_display, g_gc, select_color ("black"));
  XDrawString (g_display, g_main_win, g_gc, 1, 18, time_string, 8);
  XDrawString (g_display, g_main_win, g_gc, 1, 36, day_string, 6);
  XDrawString (g_display, g_main_win, g_gc, 55, 36, year_string, 4);
#endif
}


/*If size or position of clock is changed, we must call this.*/
void
resize_clock (void)
{
  if (g_minimized)
    {
      if (g_force_maximized)
	return;
      g_size_x = MINIMIZED_SIZE_X;
    }
  else
    {
      g_size_x = MAXIMIZED_SIZE_X;
    }
  XMoveResizeWindow (g_display, g_main_win, g_position_x, g_position_y,
		     g_size_x, g_size_y);
  draw_clock ();
}


/*This helps little bit.*/
void
set_prop (Window win, Atom at, Atom type, long val)
{
  XChangeProperty (g_display, win, at, type, 32, PropModeReplace,
		   (unsigned char *) &val, 1);
}


/* Creates the window
   g_main_win = create_window   */
Window
create_window (void)
{
  Window win;
  g_mwmhints mwm;
  XSizeHints size_hints;
  XWMHints wmhints;
  XSetWindowAttributes att;
  att.background_pixel = select_color (MINIMIZED_COLOR);

  /*We set that this new window will take this kind on events */
  att.event_mask =
    ExposureMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask |
    LeaveWindowMask;
  /* List of all event_masks and explanation:
     NoEventMask               No events
     KeyPressMask              Keyboard down events
     KeyReleaseMask            Keyboard up events
     ButtonPressMask           Pointer button down events
     ButtonReleaseMask         Pointer button up events
     EnterWindowMask           Pointer window entry events
     LeaveWindowMask           Pointer window leave events
     PointerMotionMask         All pointer motion events
     PointerMotionHintMask     Fewer pointer motion events
     Button1MotionMask         Pointer motion while button 1 down
     Button2MotionMask         Pointer motion while button 2 down
     Button3MotionMask         Pointer motion while button 3 down
     Button4MotionMask         Pointer motion while button 4 down
     Button5MotionMask         Pointer motion while button 5 down
     ButtonMotionMask          Pointer motion while any button down
     KeymapStateMask           Any keyboard state change on EnterNotify
     LeaveNotify, FocusIn or FocusOut
     ExposureMask              Any exposure
     (except GraphicsExpose and NoExpose )
     VisibilityChangeMask      Any change in visibility
     StructureNotifyMask       Any change in window configuration.
     ResizeRedirectMask        Redirect resize of this window
     SubstructureNotifyMask    Notify about reconfiguration of children
     SubstructureRedirectMask  Redirect reconfiguration of children
     FocusChangeMask           Any change in keyboard focus
     PropertyChangeMask        Any change in property
     ColormapChangeMask        Any change in colormap
     OwnerGrabButtonMask       Modifies handling of pointer events
   */

  win = XCreateWindow ( /*  display     */ g_display,
		       /*  parent      */ g_root_win,
		       /*  x           */ g_position_x,
		       /*  y           */ g_position_y,
		       /*  width       */ g_size_x,
		       /*  height      */ g_size_y,
		       /*  border      */ 0,
		       /*  depth       */ CopyFromParent,
		       /*  class       */ InputOutput,
		       /*  visual      */ CopyFromParent,
		       /*  value mask  */ CWBackPixel | CWEventMask,
		       /*  attribs     */ &att);

  /*clock in ALL desktops */
  set_prop (win, atom__NET_WM_DESKTOP, XA_CARDINAL, 0xFFFFFFFF);

  /* docked = can't be moved (even by pressing alt) */
  set_prop (win, atom__NET_WM_WINDOW_TYPE, XA_ATOM,
	    atom__NET_WM_WINDOW_TYPE_DOCK);

  set_prop (win, atom__WIN_HINTS, XA_CARDINAL,
	    WIN_HINTS_SKIP_FOCUS | WIN_HINTS_SKIP_WINLIST |
	    WIN_HINTS_SKIP_TASKBAR | WIN_HINTS_DO_NOT_COVER);


  /* borderless motif hint */
  bzero (&mwm, sizeof (mwm));
  mwm.flags = MWM_HINTS_DECORATIONS;
  XChangeProperty (g_display, win, atom__MOTIF_WM_HINTS, atom__MOTIF_WM_HINTS,
		   32, PropModeReplace, (unsigned char *) &mwm,
		   sizeof (g_mwmhints) / 4);


  /* make sure the WM obays window position */
  size_hints.flags = PPosition;
  size_hints.x = 0;
  XChangeProperty (g_display, win, XA_WM_NORMAL_HINTS, XA_WM_SIZE_HINTS, 32,
		   PropModeReplace, (unsigned char *) &size_hints,
		   sizeof (XSizeHints) / 4);

  /* make window unfocusable */
  wmhints.flags = InputHint;
  wmhints.input = False;
  XSetWMHints (g_display, win, &wmhints);
  XChangeProperty (g_display, win, XA_WM_HINTS, XA_WM_HINTS, 32,
		   PropModeReplace, (unsigned char *) &wmhints,
		   sizeof (XWMHints) / 4);
  /*done */
  XMapWindow (g_display, win);

#ifdef XFT
  /*initialize XFT-font */
  xftdraw =
    XftDrawCreate (g_display, win, DefaultVisual (g_display, g_screen),
		   DefaultColormap (g_display, g_screen));
#endif

  return win;
}


/*
This function handles mouse buttons pressing. (not releasing)
*/
void
handle_press (int x, int y, int button)
{
  if (button == 1)
    {
      if (g_force_maximized)	/*can dragged */
	{
	  g_dragging_y = y;
	  g_button1_pressed = 1;
	  return;
	}
      else			/*can't be dragged yet */
	{
	  g_button1_pressed = 0;
	  g_force_maximized = 1;
	  draw_clock ();
	  return;
	}
      return;
    }

  if (button == 2)		/* middle-click */
    {
      return;
    }

  if (button == 3)		/* right-click */
    {
      return;
    }
}

/*
This function handles mouse buttons releasing.
*/
void
handle_release (int x, int y, int button)
{
  int distance;
  if (button == 1)
    {
      if (!g_button1_pressed)
	{
	  return;
	}
      distance = y - g_dragging_y;
      /*if distance if too low, do not drag */
      if (distance == 0 || distance == 1 || distance == -1)
	{
	  g_force_maximized = 0;
	  draw_clock ();
	  return;
	}
      g_position_y += distance;
      resize_clock ();
      return;
    }
}


/*Empty*/
void
handle_error (Display * d, XErrorEvent * ev)
{
}



/*
a) commandline-parameters
b) initialization
c) mainloop
*/
int
main (int argc, char *argv[])
{
  /*initalization */
  XEvent event;			/*events, like mouse clicking */
  fd_set fd;			/* http://www.delorie.com/gnu/docs/glibc/libc_248.html */
  int xfd;

  /*time */
  struct timeval tv;
  time_t now;
  /*time */

  XGCValues gcv;

  g_position_x = 0;
  g_position_y = 300;
  g_size_x = MINIMIZED_SIZE_X;
  g_size_y = 40;
  g_minimized = 1;
  g_button1_pressed = 0;

#ifndef XFT
  char *fontname;
#endif

#ifdef COMMANDLINE_PARAMETERS
  /*commanline parameters */
  int i;
  for (i = 1; i < argc; ++i)
    {
      if (!strcmp (argv[i], "--version"))
	{
	  printf ("aaclock v:1.0\n");
	  return 0;
	}
      else if (!strcmp (argv[i], "--help"))
	{
	  printf ("aaclock v:1.0\n");
	  printf
	    ("There are no commandline parameters. All features are used in compile time.\n");
	  return 0;
	}
    }
#endif

  /*initalization */
  if (!(g_display = XOpenDisplay (NULL)))
    {				/*quit if there are now Display */
      printf ("This is X-Window application\n");
      return 0;
    }

  XSelectInput (g_display, g_root_win,
		PropertyChangeMask | VisibilityChangeMask);
  XSetErrorHandler ((XErrorHandler) handle_error);

  XInternAtoms (g_display, atom_names, ATOM_COUNT, False, g_atoms);

  g_screen = DefaultScreen (g_display);
  g_root_win = RootWindow (g_display, g_screen);



#ifdef XFT
  xfs_time = XftFontOpenName (g_display, g_screen, XFT_FONT_TIME);
  xfs_date = XftFontOpenName (g_display, g_screen, XFT_FONT_DATE);
  g_gc = XCreateGC (g_display, g_root_win, GCGraphicsExposures, &gcv);
#else
  fontname = FONT_NAME_TIME;
  do
    {
      xfs_time = XLoadQueryFont (g_display, fontname);
      fontname = "fixed";
    }
  while (!xfs_time);
  /* printf("fontname= %s\n",fontname); */
  gcv.font = xfs_time->fid;
  g_gc =
    XCreateGC (g_display, g_root_win, GCFont | GCGraphicsExposures, &gcv);

  XGCValues gcv_date;
  fontname = FONT_NAME_DATE;
  do
    {
      xfs_date = XLoadQueryFont (g_display, fontname);
      fontname = "fixed";
    }
  while (!xfs_date);
  gcv_date.font = xfs_time->fid;
  g_gc2 =
    XCreateGC (g_display, g_root_win, GCFont | GCGraphicsExposures,
	       &gcv_date);
  gcv_date.graphics_exposures = False;
#endif

  gcv.graphics_exposures = False;
  g_main_win = create_window ();
  xfd = ConnectionNumber (g_display);
  XSync (g_display, False);

  resize_clock ();
  draw_clock ();

  /*main loop */
  while (1)
    {
      FD_ZERO (&fd);		/*clearing socket */
      FD_SET (xfd, &fd);	/*connect xfd to fd */
      now = time (0);		/*time */
      gmtime (&now);	/*gmtime */

      /*How often we update the clock */
      tv.tv_usec = 100000;	/*microseconds, 1 second = 1 million microseconds */
      tv.tv_sec = 0;

      if (g_minimized)		/*practically do not update, if user dont see clock */
	tv.tv_sec = 100000;

      if (select (xfd + 1, &fd, 0, 0, &tv) == 0)
	draw_clock ();

      while (XPending (g_display))	/*XPending = how many events on the queue */
	{
	  XNextEvent (g_display, &event);
#ifdef HELP_WITH_EVENTS
	  printf ("event occurs: %s\n", g_name_of_events[event.type]);
#endif
	  switch (event.type)
	    {
	    case EnterNotify:
	      g_minimized = 0;
	      resize_clock ();
	      break;
	    case LeaveNotify:
	      if (!(g_force_maximized))
		{
		  g_minimized = 1;
		  resize_clock ();
		}
	      break;
	    case ButtonPress:
	      handle_press (event.xbutton.x, event.xbutton.y,
			    event.xbutton.button);
	      break;
	    case ButtonRelease:
	      handle_release (event.xbutton.x, event.xbutton.y,
			      event.xbutton.button);
	      break;
	    case Expose:	/*anything cover or uncover */
	      draw_clock ();
	      break;
	    }
/* What to catch?
  Event Mask                Event Type          Structure
KeyPressMask                KeyPress            XKeyPressedEvent
KeyReleaseMask              KeyRelease          XKeyReleasedEvent
ButtonPressMask             ButtonPress         XButtonPressedEvent
ButtonReleaseMask           ButtonRelease       XButtonReleasedEvent
OwnerGrabButtonMask         n/a                 n/a
KeymapStateMask             KeymapNotify        XKeymapEvent
PointerMotionMask           MotionNotify        XPointerMovedEvent
PointerMotionHintMask       MotionNotify        XPointerMovedEvent
ButtonMotionMask            MotionNotify        XPointerMovedEvent
Button1MotionMask           MotionNotify        XPointerMovedEvent
Button2MotionMask           MotionNotify        XPointerMovedEvent
Button3MotionMask           MotionNotify        XPointerMovedEvent
Button4MotionMask           MotionNotify        XPointerMovedEvent
Button5MotionMask           MotionNotify        XPointerMovedEvent
EnterWindowMask             EnterNotify         XEnterWindowEvent
LeaveWindowMask             LeaveNotify         XLeaveWindowEvent
FocusChangeMask             XFocusInEvent       XFocusOutEvent
FocusIn                     XFocusInEvent       XFocusOutEvent
FocusOut                    XFocusInEvent       XFocusOutEvent
ExposureMask                Expose              XExposeEvent
ColormapChangeMask          ColormapNotify      XColormapEvent
PropertyChangeMask          PropertyNotify      XPropertyEvent
VisibilityChangeMask        VisibilityNotify    XVisibilityEvent
ResizeRedirectMask          ResizeRequest       XResizeRequestEvent

StructureNotifyMask         CirculateNotify     XCirculateEvent
 or                         ConfigureNotify     XConfigureEvent
SubstructureNotifyMask      DestroyNotify       XDestroyWindowEvent
 or                         GravityNotify       XGravityEvent
SubstructureRedirectMask    MapNotify           XMapEvent
                            ReparentNotify      XReparentEvent
                            UnmapNotify         XUnmapEvent

(Always selected)           MappingNotify       XMappingEvent
(Always selected)           ClientMessage       XClientMessageEvent
(Always selected)           SelectionClear      XSetSelectClearEvent
(Always selected)           SelectionNotify     XSelectionEvent
(Always selected)           SelectionRequest    XSelectionRequestEvent

*/
	}
    }
}
