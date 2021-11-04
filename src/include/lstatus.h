/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: lstatus.h,v 1.2 1998/04/08 08:54:34 ayman Beta $")
**
*/

 enum Figures {BLOCK=254|A_ALTCHARSET, DOT=7|A_ALTCHARSET};
 typedef enum Figures Figures;

 enum State {on=A_BOLD, off=0, blink=A_BLINK};
 typedef enum State State;

 struct LowerStatusControls {
        const char *desc;
        int x;
        Colors color;
        Figures figure;
        State state;
        unsigned on_how;
       };
 typedef struct LowerStatusControls LowerStatusControls;

 void DisplayHistoryTable (const int, const int);

 void DisplayControlsPanel (void);

