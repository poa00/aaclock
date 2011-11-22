CC=cc
CFLAGS=-O2 -I/usr/include -I/usr/include/freetype2 -DXFT 
#-DHELP_WITH_EVENTS
#HELP_WITH_EVENTS: debugging and learning event-handling
#-DCOMMANDLINE_PARAMETERS
#COMMANDLINE_PARAMETERS: not any

LDFLAGS= -L/usr/lib -lX11 -lXft -Wall -pedantic -ansi
PROGNAME=aaclock

$(PROGNAME): Makefile aaclock.c aaclock.h
	$(CC)  aaclock.c -o $(PROGNAME) $(CFLAGS) $(LDFLAGS)
	@ls -l $(PROGNAME)
	strip $(PROGNAME)
	@ls -l $(PROGNAME)
