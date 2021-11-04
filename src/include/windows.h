/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
"$Id: windows.h,v 1.1 1998/03/09 01:53:39 ayman Beta $"
**
*/

#ifndef WINDOWS_H
# define WINDOWS_H

 struct InputProcessor {
         int id,
             idx;
         void *whatever;
         void *list;
         int (*func)(int);
         void (*cleanup_func)(void *);
         Windows wins;
        };
 typedef struct InputProcessor InputProcessor;

 #define c_inptr ((InputProcessor *)(stack->top->whatever))

 #define prv_inptr ((InputProcessor *)stack->top->next->whatever)

 #define c_inw1  (c_inptr->wins.w1)
 #define c_inw2  (c_inptr->wins.w2)
 #define c_inflags (c_inptr->wins.flags)
 #define c_instate (c_inptr->wins.state)
 #define c_inbuf (c_inptr->wins.buf)

 #define prv_inw1  (prv_inptr->wins.w1)
 #define prv_inw2  (prv_inptr->wins.w2)
 #define prv_inflags (prv_inptr->wins.flags)
 #define prv_inbuf (prv_inptr->wins.buf)


 #define ST_MOVING    (0x1<<0)
 #define ST_RESIZING  (0x1<<1)

 #define FLG_SAVEPREVIMAGE    (0x1<<0)
 #define FLG_LOADSPAREWIN     (0x1<<1)
 #define FLG_DONTOVERLAY      (0x1<<2)
 #define FLG_NOBENEATH        (0x1<<3)
 #define FLG_KEEPCOORDINATES  (0x1<<4)
 #define FLG_CURSORVISIBLE    (0x1<<5)
 #define FLG_OVERLAYED        (0x1<<6)
 #define FLG_ACTIVEBORDER     (0x1<<7)
 #define FLG_SHADOW           (0x1<<8)
 #define FLG_MOVEABLE         (0x1<<9)
 #define FLG_RESIZEABLE       (0x1<<10)

 #define FLGS_MENUBAR    (FLG_LOADSPAREWIN|FLG_NOBENEATH|FLG_KEEPCOORDINATES)
                      //(2|8|16)
 #define FLGS_MENUBOX    (1|2|16|FLG_SHADOW)
 #define FLGS_SCROLLMAIN (1|2|FLG_ACTIVEBORDER|FLG_SHADOW|FLG_MOVEABLE)
 #define FLGS_SCROLLSUB  (1|2|4|FLG_SHADOW|FLG_MOVEABLE)
 #define FLGS_MSGBOX     (1|2|FLG_ACTIVEBORDER|FLG_SHADOW|FLG_MOVEABLE)
 #define FLGS_DIALOGBOX  (1|2|32|FLG_ACTIVEBORDER|FLG_SHADOW|FLG_MOVEABLE)


/* extern __inline__ void Imageold (void)
 {
  register int x,
               y;
  extern Stack *const stack;

    if (prv_inflags&1)
     {
      for (y=prv_inw1->_begy; y<(prv_inw1->_begy+prv_inw1->_maxy+1); y++)
       {
        for (x=prv_inw1->_begx; x<prv_inw1->_maxx+prv_inw1->_begx+1; x++)
         {
          wmove (MainWin, y-2, x-1);
          //mwmove (y-2, x-1);
          whline (MainWin, curscr->_line[y].text[x], 1);
         }
       }
     }
 }
*/

  extern __inline__ void 
 Imagenew (WINDOW *dw, const WINDOW *sw, const WINDOW *bw, unsigned flag)
 {
  register int x,
               y;

    //if (flag&1)
     {                                        //+2
      for (y=bw->_begy; y<(bw->_begy+bw->_maxy+1); y++)
       {
        for (x=bw->_begx; x<bw->_maxx+bw->_begx+1; x++)
         {
          wmove (dw, (flag&2?y-2:y), (flag&2?x-1:x));
          whline (dw, sw->_line[y].text[x], 1);
         }
       }
     }
 }


/* extern __inline__ void GrabitI (void)

 {
  register int x, y;
  register chtype *b;
  extern Stack *const stack;

    if (c_inptr->id==2)  return;

   b=c_inbuf=malloc((c_inw1->_maxy+2)*(c_inw1->_maxx+2)*sizeof(chtype));
                                                //2
    for (y=c_inw1->_begy; y<c_inw1->_begy+c_inw1->_maxy+1; y++)
     {                                             //2
      for (x=c_inw1->_begx; x<c_inw1->_maxx+c_inw1->_begx+1; x++)
       {
        *b++=mvwinch (curscr, y, x);
       }
     }
 }
*/

/*
 extern __inline__ 
  void GrabitII (Windows *auxw)

 {
  register int x, y;
  register chtype *b;

   b=auxw->buf=malloc((auxw->w1->_maxy+2)*(auxw->w1->_maxx+2)*sizeof(chtype));
                                                             //+2
    for (y=auxw->w1->_begy; y<auxw->w1->_begy+auxw->w1->_maxy+1; y++)
     {
      for (x=auxw->w1->_begx; x<auxw->w1->_maxx+auxw->w1->_begx+1; x++)
       {
        *b++=mvwinch (curscr, y, x);
       }
     }

 }
*/


/*  extern __inline__ void
Grabit (WINDOW *bw, const WINDOW *w, chtype *buf, unsigned flag)

 {
  register int x, y;
  register chtype *b;

    if (flag&1)
     {
      buf=malloc ((w->_maxy+2)*(w->_maxx+2)*sizeof(chtype));
     }

   b=buf;

    for (y=w->_begy; y<w->_begy+w->_maxy+1; y++)
     {
      for (x=w->_begx; x<w->_maxx+w->_begx+1; x++)
       {
        *b++=mvwinch (bw, (flag&2?y-Y_OFFSET:y), (flag&2?x-X_OFFSET:x));
       }
     }
 }
*/

/*  extern __inline__ void
 NotAnymore (const WINDOW *sw, chtype *buf, unsigned flags)

 {
  register int x, y;
  register chtype *auxb;

    if (!buf)  return;

   auxb=buf;
                                            //no -1
    for (y=sw->_begy-2; y<(sw->_begy+sw->_maxy)-1; y++)
     {
      for (x=sw->_begx-1; x<sw->_maxx+sw->_begx; x++, auxb++)
       {                                       // +1
        mwmove (y, x);
        whline (MainWin, *auxb, 1);
       }
     }

    if (flags&1)  free (buf);
 }
*/
 
 void InitStack (void);
 void ReallyInitStack (Stack *);

 int RetrieveType (const void *);

 void KillMenu (void *);

 int LoadStack (void *, void *, int (*)(int), void (*)(void *), 
                int (*)(void *, Windows *), int, int, int);
 void LoadPrevLevel (void);
 void *RetrievePosted (int, const void *);
 void *RetrieveCurrentWindow (void);
 void *RetrieveCurrentParentWindow (void);
 void *RetrieveParentWindow (void);
 int CursorStatus (void);
 void RefreshListContents (void *);
 void *Hello (void);
 void CheckGeometry (int, int, int *, int *);
 void UpdateGeometry (int, int);
 int NewWindow (int, int, int *, int *, Windows *, int);
 int NewSubWindow (int, int, int, int, Windows *);
 void DestroyWindow (int, void *);
 void MoveWindow (int, int);
 void NextPrev (void);
 void ReadKeyboardInput (void);

 void ExplodeWindow (WINDOW *, int);
 void ExplodeWindowGravityEast (WINDOW *);
 void ExplodeWindowGravitySouth (WINDOW *);
 void Box (WINDOW *, int, int, int, int);
 void BorderOn (WINDOW *);
 void BorderOff (WINDOW *);
#endif

