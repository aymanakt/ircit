/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: msgbox.h,v 1.1 1998/03/04 15:40:55 ayman Beta $")
**
*/

 struct Button {
        const char *caption;
        int caption_start;
        long attrs;
        void (*func)(void *);
       };
 typedef struct Button Button;

 struct Buttons {
        int nButtons;
        Button *buttons;
       };
 typedef struct Buttons Buttons;
      
 struct MsgBox {
        BoxGeometry geometry;
        int startx,
            starty;
        int c_hilited;
        int color;
        char *msg;
        Buttons *buttons;
       };
 typedef struct MsgBox MsgBox;

 const ObjectsAndColors *const _box;
 const ObjectsAndColors *const _boxborder;

 void LoadMsgBox (MsgBox *);
 void DisplayMsgBox (const MsgBox *);
 void ClearMsgBox (void);
 int GetMsgBoxInput (int);
 void TellMsg (void); 
                
