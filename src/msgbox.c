/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>
#include <terminal.h>
#include <output.h>
#include <colors.h>
#include <msgbox.h>
#include <windows-aux.h>

 MODULEID("$Id: msgbox.c,v 1.4 1998/06/25 15:06:17 ayman Beta $");

 static int DrawMsgBox (MsgBox *, Windows *);
 static void MsgBoxRight (MsgBox *);
 static void MsgBoxLeft (MsgBox *);
 static void MsgBoxEnter (MsgBox *);

 #define c_mxlines mxptr->geometry.nEntries
 #define c_mxwidth mxptr->geometry.width


 void DisplayMsgBox (const MsgBox *mxptr)

 {
  register int i;
  register Button *bptr;
  WINDOW *w=RetrieveCurrentWindow();

    if (mxptr->buttons)
     {
        for (i=0; i<=mxptr->buttons->nButtons-1; i++)
         {
          bptr=&mxptr->buttons->buttons[i];

          _wmove (w, w->_regbottom-1, bptr->caption_start);

           if (mxptr->c_hilited==i)  wattron (w, A_REVERSE);

           Fwprintf_nout (w, bptr->caption);

            if (mxptr->c_hilited==i)  wattroff (w, A_REVERSE);
         }
     }

   wnoutrefresh (w);

 }  /**/


 static __inline__ int DrawMsgBox (MsgBox *mxptr, Windows *wins)

 {
  register int *x, *y;
  register Windows *auxwins=wins;

   mxptr->starty?:(mxptr->starty=((MainWin->_maxy-c_mxlines)/2)+2);
   mxptr->startx?:(mxptr->startx=(MainWin->_maxx-c_mxwidth)/2);

    if (!(NewWindow(c_mxlines, c_mxwidth, &mxptr->starty, &mxptr->startx, 
         auxwins, 0)))
     {
      return 0;
     }

    if (!(NewSubWindow(c_mxlines-2, c_mxwidth-2, mxptr->starty+1, 
         mxptr->startx+1, auxwins)))
     {
      return 0;
     }

   wattrset (auxwins->w1, COLOR_PAIR(CLR_BOXBORDER));
   wattrset (auxwins->w2, COLOR_PAIR(CLR_BOX));
   wbkgd (auxwins->w2, COLOR_PAIR (CLR_BOX));

   ExplodeWindow (auxwins->w1, EXPLODE_OUT);
   box (auxwins->w1, ACS_VLINE, ACS_HLINE);
   wnoutrefresh (auxwins->w1);
   //borderI (auxwins->w1);

    {
     register int i=1, j=1;
     register char *s=mxptr->msg;

       while (*s)
        {
          while ((*s)&&(*s!='\n'))
           {
            wmove (auxwins->w2, i, j++);
            whline (auxwins->w2, *s++, 1);
           }

          if (*s=='\n')  s++;

         i++; j=1;
        }
    }

   DisplayMsgBox (mxptr);

   return 1;

 }  /**/


 void ClearMsgBox (void)

 {
   LoadPrevLevel ();

 }  /**/


 static __inline__ void MsgBoxLeft (MsgBox *mxptr)

 {
    if (--mxptr->c_hilited<0)
     {
      mxptr->c_hilited=(mxptr->buttons->nButtons-1);
     }

   DisplayMsgBox (mxptr);

 }  /**/


 static __inline__ void MsgBoxRight (MsgBox *mxptr)

 {
    if (++mxptr->c_hilited>(mxptr->buttons->nButtons-1))
     {
      mxptr->c_hilited=0;
     }

   DisplayMsgBox (mxptr);

 }  /**/


 static __inline__ void MsgBoxEnter (MsgBox *mxptr)

 {
  int n;
  register Button *bptr;

   bptr=&mxptr->buttons->buttons[(n=mxptr->c_hilited)];

   ClearMsgBox ();

    if (bptr->func)
     {
      (*bptr->func)((void *)&n);
     }

 }  /**/


 int GetMsgBoxInput (int key)

 {
  register MsgBox *mxptr=Hello();
  extern int Gets (int);

    if (key<KEY_MAX)
       {
        switch (key)
               {
                case 27:
                     ClearMsgBox ();
                     break;

                case KEY_UP:
                case KEY_LEFT:
                     MsgBoxLeft (mxptr);
                     return 0;

                case '\t':
                case KEY_DOWN:
                case KEY_RIGHT:
                     MsgBoxRight (mxptr);
                     break;

                case '\n':
                case '\r':
                     MsgBoxEnter (mxptr);
                     return 0;

                case 0x18:
                     NextPrev (); 
                     break;

                case KEY_HOME:
                     break;

                case KEY_END:
                     break;

                default:
                     Gets (key);
               }
        }

   return key;

 }  /**/


 void LoadMsgBox (MsgBox *mxptr)

 {
  register Windows *wins;

   switch (LoadStack(mxptr, NULL, GetMsgBoxInput, NULL, 
                     (int (*)(void *, Windows *))DrawMsgBox, 5, 0, FLGS_MSGBOX))
    {
     case 3:
          ClearMsgBox ();
    }

 }  /**/


 void TellMsg (void)

 {
  static Button buttons[]={
          {" Yes ", 2,  0, NULL},
          {" No  ", 10, 0, NULL}
         };

  static Buttons button={
         2, buttons
        };
         
  static MsgBox msgbox={
         {7, 20},
          0, 0, 
          0, 51, 
          "  Hello world",
          &button
         };

  LoadMsgBox (&msgbox);
 }

