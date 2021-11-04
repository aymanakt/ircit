/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>
#include <terminal.h>
#include <output.h>
#include <updates.h>
#include <stack.h>
#include <menu.h>
#include <scroll.h>
#include <msgbox.h>
#include <input.h>
#include <windows.h>


 MODULEID("$Id: windows.c,v 1.5 1998/06/25 14:57:14 ayman Beta $");

/* MainWin (sub of MmainWin) offsets from curscr */
#define Y_OFFSET  2
#define X_OFFSET  1

 static void SaveUnders (Windows *, WINDOW *, unsigned);
 static void ImprintPreviousWindow (WINDOW *, WINDOW *, Windows *, unsigned);

 static void WindowWandering (WINDOW *, int);
 static InputProcessor *OutSomewhere (const void *, const void *);
 static void BringForwardMenubar (const InputProcessor *);
 static void BringForward (InputProcessor *);
 static void CheckHold (void);

 static void ShutdownWindowGravityZero (Windows *, unsigned);
 static void ShutdownWindowGravitySouth (Windows *, unsigned);
 static void ShutdownWindowGravityNorth (Windows *, unsigned);
 static void ShutdownWindowGravityWest (Windows *, unsigned);

 Stack sc_stack;
 Stack *const stack=&sc_stack;

 static int cx, cy;

  void (*Shutdown[])(Windows *, unsigned)={ShutdownWindowGravityZero,
                                           ShutdownWindowGravityNorth,
                                           ShutdownWindowGravityWest,
                                           ShutdownWindowGravitySouth};

 void InitStack (void)

 {
   ReallyInitStack (stack);

 }  /**/


 void ReallyInitStack (Stack *stk_ptr)

 {
  register InputProcessor *ptr;
  register StackEntry *kptr;

   memset (stk_ptr, 0, sizeof(Stack));

   xmalloc(ptr, (sizeof(InputProcessor)));
   memset (ptr, 0, sizeof(InputProcessor));

   ptr->id=1;
   ptr->whatever=NULL;
   ptr->func=Gets;

   kptr=PushStack (stk_ptr);
   (InputProcessor *)kptr->whatever=ptr;

 }  /**/


 void wUpdate (WINDOW *w)

 {
    if (stack->nEntries>1)  
     {
      int i=0;
      InputProcessor *tmp[stack->nEntries];
      InputProcessor *inptr;
      register StackEntry *kptr=stack->top;

       while (kptr->next!=NULL&&
               ((InputProcessor *)kptr->whatever)->id!=1)     
        {
         tmp[i++]=(InputProcessor *)kptr->whatever;
         kptr=kptr->next;
        }

       i--;
       while (i>=0)
        {
         inptr=tmp[i--];

          if (inptr->id==2)  continue;

         touchwin (inptr->wins.w1);
         wnoutrefresh (inptr->wins.w1);
        }
     }

   wnoutrefresh (w);
   doupdate ();

 }  /**/


  static __inline__ 
 InputProcessor *OutSomewhere (const void *xptr, const void *yptr)

 {
  register StackEntry *kptr;

    for (kptr=stack->top; kptr!=NULL; kptr=kptr->next)
     {
      if (((InputProcessor *)kptr->whatever)->whatever==xptr)
       {
        return (InputProcessor *)kptr->whatever;
       }
     }

   return (InputProcessor *)NULL;

 }  /**/


 int RetrieveType (const void *xptr)

 {
  register InputProcessor *inptr;

    return (inptr=OutSomewhere(xptr, NULL))?(inptr->id):0;

 }  /**/


 void LoadPrevLevel (void)

 {
  StackEntry *kptr;
  register InputProcessor *inptr;

    if (stack->nEntries>1)
     {
      kptr=PopStack (stack);
      inptr=(InputProcessor *)kptr->whatever;

      (*Shutdown[inptr->idx])(&inptr->wins, 0x1<<0);
      //ShutdownWindowGravityWest/*North*/ (&inptr->wins, 0x1<<0);

       if (inptr->cleanup_func)  (*inptr->cleanup_func)(inptr->whatever);

       if (inptr->wins.w2)
        {
         delwin (inptr->wins.w2);
         inptr->wins.w2=NULL;
        }

       if (inptr->wins.w1)
        {
          if ((inptr->wins.flags&FLG_KEEPCOORDINATES)==0)
           {
            UpdateGeometry (inptr->wins.w1->_begy, inptr->wins.w1->_begx);
           }

         delwin (inptr->wins.w1);
         inptr->wins.w1=NULL;
        }

      free (kptr->whatever);
      free (kptr);
      
      wnoutrefresh (MainWin);

       if ((c_inptr->id==4)&&(c_inflags&(0x1<<6)))
        {
         c_inflags&=~(0x1<<6);
         RefreshListContents (c_inptr->list);
        }

       if (c_inflags&FLG_ACTIVEBORDER)  
        {
         BorderOn (c_inw1); 
         wnoutrefresh (c_inw1);
        }
       
      doupdate ();

       if (stack->nEntries==1)
        {
         MainWin->_cury=cy;
         MainWin->_curx=cx;
         LOADMAINWIN();
        }
     }

    if (stack->nEntries==1)
     {
      if (((InputProcessor *)stack->top->whatever)->func!=Gets)
       {
        beep ();
        InitStack ();
       }
     }

 }  /**/


 void MoveWindow (int _y, int _x)

 {
  int x, y, flgs;
  size_t linesize;
  register WINDOW *ow1=c_inw1, 
                  *ow2=c_inw2;

   ShutdownWindowGravityZero (&c_inptr->wins, 0);

   x=ow1->_begx+(_x); 
   y=ow1->_begy+(_y);

   /* 
   ** can be further optimized by directly changing WINDOW struct to reflect
   ** the new settings.
   */

   /*
   ** set FLG_NOBENEATH so that a call to SaveUnders() in 
   ** NewWindow() will fail.
   */
   flgs=c_inflags;
   c_inflags|=FLG_NOBENEATH;
   NewWindow (ow1->_maxy+1, ow1->_maxx+1, &y, &x, 
              &((InputProcessor *)stack->top->whatever)->wins, 0);
   c_inflags&=~FLG_NOBENEATH;
   c_inflags=flgs;
   wnoutrefresh (MainWin);

   /* dont allocate buf.  Use MainWindow offsets */
   SaveUnders (&c_inptr->wins, newscr, 0);//MainWin, 0x1<<1);

   c_inw1->_attrs=ow1->_attrs;
   c_inw1->_bkgd=ow1->_bkgd;

    if (ow2) 
     {
      NewSubWindow (ow2->_maxy+1, ow2->_maxx+1, /*y, x,*/
                    (ow2->_begy-ow1->_begy)+y,
                    (ow2->_begx-ow1->_begx)+x,
                    &((InputProcessor *)stack->top->whatever)->wins);

      c_inw2->_attrs=ow2->_attrs;
      c_inw2->_bkgd=ow2->_bkgd;
     }

   linesize=(ow1->_maxx+1)*sizeof(chtype);
    for (x=0; x<=c_inw1->_maxy; x++) 
     {
      memcpy (c_inw1->_line[x].text, ow1->_line[x].text, linesize);
     }

   touchwin (c_inw1); 
   wnoutrefresh (c_inw1);

    if (ow2)  delwin (ow2); 
   delwin (ow1);

 }  /**/


 void FlashMenu (void)

 {
  register int x;

    for (x=0; x<=c_inw1->_maxx-1; x++)  c_inw1->_line[0].text[x]|=A_BLINK;
   wrefresh (c_inw1); 

   napms (500);

    for (x=0; x<=c_inw1->_maxx-1; x++)  c_inw1->_line[0].text[x]&=~A_BLINK;
   c_inw1->_line[0].firstchar=0, c_inw1->_line[0].lastchar=c_inw1->_maxx;
   wnoutrefresh (c_inw1);
   
 }  /**/


 static  void BringForwardMenubar (const InputProcessor *inptr)

 {
  register int i=0;
  register StackEntry *aux,
                      *aux2;
  InputProcessor *tmp[stack->nEntries];


    if (c_inflags&FLG_ACTIVEBORDER) 
     {
      BorderOff (c_inw1);
      wrefresh (c_inw1);  /* we need this on curscr */
     }
  
    while (aux=PopStack(stack),((aux)&&((InputProcessor*)aux->whatever!=inptr)))
     {
      tmp[i++]=(InputProcessor *)aux->whatever;
      free (aux);
     }

    if ((aux)&&((InputProcessor *)aux->whatever==inptr))
     {
       while (--i!=(-1))
        {
         aux2=PushStack (stack);
         aux2->whatever=tmp[i];
        }

       if (aux->whatever==inptr)
        {
         aux2=PushStack (stack);
         (InputProcessor *) aux2->whatever=inptr;
        }

      free (aux);
     }
    else
     {
      beep();
     }

   FlashMenu ();

 }  /**/


 void KillMenu (void *mptr)

 {
  register InputProcessor *aux;

    if (aux=OutSomewhere(mptr, NULL))
     {
       if (c_inptr!=aux)  /* if menu isn't the top-most... */
        {
         BringForwardMenubar (aux);
        }
       else
        {
         FlashMenu ();
        }

      LoadPrevLevel ();

      DoUpperStatus ();
     }

 }  /**/


 static void BringForward (InputProcessor *inptr)

 {
  register int i=0;
  register StackEntry *aux,
                      *aux2;
  InputProcessor *tmp[stack->nEntries];
  extern Menu *menu_ptr;

    if (inptr->whatever==menu_ptr)  /* wrong...*/
     {
      BringForwardMenubar (inptr);

      return;
     }

    if (c_inflags&FLG_ACTIVEBORDER)  BorderOff (c_inw1);

    while (aux=PopStack(stack),((aux)&&((InputProcessor*)aux->whatever!=inptr)))
     {
      ShutdownWindowGravityZero (&((InputProcessor *)aux->whatever)->wins, 1);

      tmp[i++]=(InputProcessor *)aux->whatever;
      free (aux);
      napms (1);
     }

   ShutdownWindowGravityZero (&((InputProcessor *)aux->whatever)->wins, 1); 

   wnoutrefresh (MainWin);
   doupdate ();

    if ((aux)&&((InputProcessor *)aux->whatever==inptr))
     {
       while (--i!=(-1))
        {
         aux2=PushStack (stack);
         aux2->whatever=tmp[i];

         SaveUnders (&((InputProcessor *)tmp[i])->wins, curscr, 0x1<<0);
         
         touchwin (c_inw1);
         wrefresh (c_inw1);
         napms (1);
        }

       if (aux->whatever==inptr)
        {
         aux2=PushStack (stack);
         aux2->whatever=inptr;

          if (c_inflags&FLG_ACTIVEBORDER)  BorderOn (c_inw1);

         SaveUnders (&inptr->wins, curscr, 0x1<<0);

         touchwin (c_inw1);
         wrefresh (c_inw1);
         napms (1);
        }

      free (aux);
     }
 
 }  /**/


 #define ENOUGH_ENTRIES (stack->nEntries>=3)

 void NextPrev (void)

 {
  register StackEntry *tmp,
                      *tmp2,
                      *aux;
  extern Menu *menu_ptr;

    if (c_inflags&FLG_DONTOVERLAY/*4*/)  LoadPrevLevel ();
    
    if (ENOUGH_ENTRIES)
     {
      ShutdownWindowGravityZero (&c_inptr->wins, 0);
      tmp=PopStack (stack);

      ShutdownWindowGravityZero (&c_inptr->wins, 0); 
      tmp2=PopStack (stack);

      wnoutrefresh (MainWin);

      aux=PushStack (stack);
      aux->whatever=tmp->whatever;
      SaveUnders (&c_inptr->wins, newscr, 0);//MainWin, 0x1<<1);

       if (c_inflags&FLG_ACTIVEBORDER)  BorderOff (c_inw1);

      touchwin (c_inw1);
      wnoutrefresh (c_inw1);

      aux=PushStack (stack);
      aux->whatever=tmp2->whatever;

      /* don't malloc buffer. Use MainWindow offsets */
      SaveUnders (&c_inptr->wins, newscr, 0);//MainWin, 0x1<<1);

       if (c_inflags&FLG_ACTIVEBORDER)  BorderOn (c_inw1);
       if (c_inptr->whatever==menu_ptr)  FlashMenu ();  /* wrong...*/
      touchwin (c_inw1);
      wnoutrefresh (c_inw1); 

      /* doupdate (); */
     }

 }  /**/


 void DestroyWindow (int id, void *yptr)

 {
  void *xptr;
  InputProcessor *inptr;
  extern int isoptionmenu (void *);

    /* wrong... */
    if ((((InputProcessor *)stack->top->whatever)->id==4)&&
        (isoptionmenu(c_inptr->whatever)))
     {
      LoadPrevLevel ();
     }
        
    if ((xptr=RetrievePosted(id, yptr)))
     {
      if ((inptr=OutSomewhere(xptr, NULL)))
       {
         if (inptr==stack->top->whatever)
          { 
           LoadPrevLevel ();
          }
         else
          {
           BringForward (inptr);
           LoadPrevLevel ();
          }
       }
     }

 }  /**/


 static void ShutdownWindowGravityZero (Windows *wptr, unsigned auxflgs)

 {
    if ((wptr->flags&FLG_NOBENEATH)||(!wptr->buf))  return;

   {
    register int x, y;
    register WINDOW *w=wptr->w1;
    register chtype *chs=wptr->buf;

      for (y=w->_begy-2; y<(w->_begy+w->_maxy)-1; y++)
       {
         for (x=w->_begx-1; x<w->_maxx+w->_begx; x++, chs++)
          {
           mwmove (y, x);
           whline (MainWin, *chs, 1);
          }
       }

      if (auxflgs&(0x1<<0))  free (wptr->buf);
   }

 }  /**/


 static void ShutdownWindowGravitySouth (Windows *wptr, unsigned auxflgs)

 {
    if ((wptr->flags&FLG_NOBENEATH)||(!wptr->buf))  return;
  
   {
    register int x, y;
    register WINDOW *w=wptr->w1;
    register chtype *chs=wptr->buf;

      for (y=w->_begy-2; y<(w->_begy+w->_maxy)-1; y++)
       {
         for (x=w->_begx-1; x<w->_maxx+w->_begx; x++, chs++)
          {
           mwmove (y, x);
           whline (MainWin, *chs, 1);
          }

        napms (20);
        wrefresh (MainWin);
       }

      if (auxflgs&(0x1<<0))  free (wptr->buf);
   }

 }  /**/


 static void ShutdownWindowGravityWest (Windows *wptr, unsigned auxflgs)

 {
    if ((wptr->flags&FLG_NOBENEATH)||(!wptr->buf))  return;
  
   {
    register WINDOW *w=wptr->w1;
    struct fTemp {
                  chtype *s,
                         *e;
            }; 
    struct fTemp tmp[w->_maxy+1];

     {
      register int i;

        for (i=0; i<=w->_maxy; i++)
         {
          tmp[i].s=(wptr->buf+(i*(w->_maxx)));
          tmp[i].e=tmp[i].s+(w->_maxx+i);
         }
     }


     {
      register int x, y;

        for (x=(w->_maxx+w->_begx)-1; x>=w->_begx-1; x--)
         {
           for (y=w->_begy-2; y<(w->_begy+w->_maxy)-1; y++)
            {
             mwmove (y, x);
             whline (MainWin, *tmp[y-(w->_begy-2)].e--, 1);
            }

          napms (10);
          wrefresh (MainWin);
         }
     }

      if (auxflgs&(0x1<<0))  free (wptr->buf);
   }

 }  /**/


 static void ShutdownWindowGravityNorth (Windows *wptr, unsigned auxflgs)

 {
    if ((wptr->flags&FLG_NOBENEATH)||(!wptr->buf))  return;
  
   {
    register int x, y;
    register WINDOW *w=wptr->w1;
    register chtype *chs;

     chs=wptr->buf+ (((w->_maxy+1)*(w->_maxx+1))-1);

      for (y=(w->_begy+w->_maxy)-2; y>=w->_begy-2; y--)
       {
         for (x=(w->_maxx+w->_begx)-1; x>=w->_begx-1; x--, chs--)
          {
           mwmove (y, x);
           whline (MainWin, *chs, 1);
          }

        napms (10);
        wrefresh (MainWin);
       }

      if (auxflgs&(0x1<<0))  free (wptr->buf);
   }

 }  /**/


 void ExplodeWindow (WINDOW *win, int how)

 {
  register int i,
               j;
  int nlines,
      ncols;
  int nline,
      ncol;
  register WINDOW *w=win,
                  *ptr;
  typedef struct coordinates {
                  int lines,
                      cols;
                  int y,
                      x;
                 } coordinates;
  coordinates wins[13];

   memset (wins, 0, 13*sizeof(coordinates));

    /* what's going on here? */
   nlines=((w->_maxy+1)-((w->_maxy+1)%2))/2;
   nline=(w->_maxy+1)+2;
   ncols=((w->_maxx+1)-((w->_maxx+1)%2))/2;
   ncol=(w->_maxx+1)+2;

    for (i=1; (((ncol-=2)>1)&&(i<=nlines)); i++)
     {
      wins[i].lines=nline-=2;
      wins[i].cols=ncol;
      wins[i].y=(i-1)+w->_begy;
      wins[i].x=(i-1)+w->_begx;
     }

    if (how&EXPLODE_IN)
    for (j=1; j<=i-1; j++)
     {
      if ((ptr=newwin(wins[j].lines, wins[j].cols, wins[j].y, wins[j].x)))
       {
        box (ptr, ACS_VLINE, ACS_HLINE);
        wrefresh (ptr);
        delwin (ptr);
        napms(10);
       }
     }

    if (how&EXPLODE_OUT)
    for (j=i-1; j>=1; j--)
     {
/*
      Box (ptr, wins[j].lines, wins[j].cols, wins[j].y, wins[j].x);
      wrefresh (ptr);
      napms (10);
*/

      if ((ptr=newwin(wins[j].lines, wins[j].cols, wins[j].y, wins[j].x)))
       {
        box (ptr, ACS_VLINE, ACS_HLINE);
        wrefresh (ptr);
        delwin (ptr);
        napms(10);
       }

     }

 }  /**/


 void ExplodeWindowGravityEast (WINDOW *w)

 {
  register int i;
  register WINDOW *ptr;
  int ncols=w->_maxx+1;
  int nlines=w->_maxy+1;

    for (i=1; i<ncols+1; i++)
     {
      if ((ptr=newwin(nlines, i, w->_begy, w->_begx)))
       {
        ptr->_attrs=w->_attrs;
        box (ptr, ACS_VLINE, ACS_HLINE);
        wrefresh (ptr);
        delwin (ptr);
       }
     }

 }  /**/


 void ExplodeWindowGravitySouth (WINDOW *w)

 {
  register int i;
  register WINDOW *ptr;
  int ncols=w->_maxx+1;
  int nlines=w->_maxy+1;

    for (i=1; i<nlines+1; i++)
     {
#if 0
      Box (w, i, ncols, w->_begy, w->_begx);
      wrefresh (w);
      napms (1000);
#endif
#if 1
       if ((ptr=newwin(i, ncols, w->_begy, w->_begx)))
        {
         ptr->_attrs=w->_attrs;
         box (ptr, ACS_VLINE, ACS_HLINE);
         wrefresh (ptr);
         delwin (ptr);
         napms (10);
        }
#endif
     }

 }  /**/


 #define mwmvaddch(y, x, z) \
         wmove (win, (y), (x)); \
         whline (win, (z), 1)

 void Box (WINDOW *win, int h, int w, int y, int x)

 {
  int i;
  register int _x,
               _y;

   y=y-win->_begy;
   x=x-win->_begx;

    if ((w>=2)&&(h>=2))
     {
      win->_line[y].text[x]=ACS_ULCORNER;
      win->_line[y].text[(x+w)-1]=ACS_URCORNER;
      win->_line[(y+h)-1].text[x]=ACS_LLCORNER;
      win->_line[(y+h)-1].text[(x+w)-1]=ACS_LRCORNER;

       for (_x=x+1; _x<x+w-1; _x++)
        {
         win->_line[y].text[_x]=ACS_HLINE;
         win->_line[(y+h)-1].text[_x]=ACS_HLINE;
        }

       for (_y=y+1; _y<y+h-1; _y++)
        {
         win->_line[_y].text[x]=ACS_VLINE;
         win->_line[_y].text[(x+w)-1]=ACS_VLINE;
        }

      touchwin (win);
     }

 }  /**/


 void BorderOn (WINDOW *w1)

 {

   w1->_line[0].text[0]=214|A_ALTCHARSET;
   w1->_line[0].text[w1->_maxx]=183|A_ALTCHARSET;
   w1->_line[w1->_maxy].text[0]=211|A_ALTCHARSET;
   w1->_line[w1->_maxy].text[w1->_maxx]=189|A_ALTCHARSET;

   w1->_line[0].firstchar=0, w1->_line[0].lastchar=w1->_maxx;
   w1->_line[w1->_maxy].firstchar=0, w1->_line[w1->_maxy].lastchar=w1->_maxx;

 }  /**/


 void BorderOff (WINDOW *w1)

 {
   w1->_line[0].text[0]=ACS_ULCORNER;
   w1->_line[0].text[w1->_maxx]=ACS_URCORNER;
   w1->_line[w1->_maxy].text[0]=ACS_LLCORNER;
   w1->_line[w1->_maxy].text[w1->_maxx]=ACS_LRCORNER;

   w1->_line[0].firstchar=0, w1->_line[0].lastchar=w1->_maxx;
   w1->_line[w1->_maxy].firstchar=0, w1->_line[w1->_maxy].lastchar=w1->_maxx;

 }  /**/


 static void WindowWandering (WINDOW *w1, int h)

 {
   h?(w1->_line[0].text[0]|=A_BLINK):(w1->_line[0].text[0]&=~A_BLINK);
   h?(w1->_line[0].text[w1->_maxx]|=A_BLINK):(w1->_line[0].text[w1->_maxx]&=~A_BLINK);
   h?(w1->_line[w1->_maxy].text[0]|=A_BLINK):(w1->_line[w1->_maxy].text[0]&=~A_BLINK);
   h?(w1->_line[w1->_maxy].text[w1->_maxx]|=A_BLINK):(w1->_line[w1->_maxy].text[w1->_maxx]&=~A_BLINK);

   w1->_line[0].firstchar=0, w1->_line[0].lastchar=w1->_maxx;
   w1->_line[w1->_maxy].firstchar=0, w1->_line[w1->_maxy].lastchar=w1->_maxx;

   wnoutrefresh (w1);

 }  /**/


 int LoadStack (void *xptr, void *yptr, 
                int (*func)(int), void (*cleanup)(void *),
                int (*paint_func)(void *, Windows *),
                int id, int close, int flags)

 {
  InputProcessor *ptr, 
                 *aux=NULL;
  StackEntry *kptr;
  extern Stack *const stack;


    while (c_inflags&FLG_DONTOVERLAY)  LoadPrevLevel ();

    if (((InputProcessor *)(stack->top->whatever))->whatever==xptr)
     {
      return 0;
     }

    if ((aux=OutSomewhere(xptr, yptr)))
     {
      BringForward (aux);

      return 0;
     }

   xmalloc(ptr, (sizeof(InputProcessor)));
   memset (ptr, 0, sizeof(InputProcessor));

   ptr->id=id;
   ptr->idx=close;
   ptr->whatever=xptr;
   ptr->list=yptr;
   ptr->func=func;
   ptr->cleanup_func=cleanup;
   ptr->wins.flags|=flags;

   kptr=PushStack (stack);
   (InputProcessor *)kptr->whatever=ptr;

    if (stack->nEntries==2)
     {
      CheckHold ();
      cx=MainWin->_curx;
      cy=MainWin->_cury;
      LOADSPAREWIN();
     }

    if (prv_inflags&FLG_ACTIVEBORDER)
     {
      BorderOff (prv_inw1); /*WWW*/

      wrefresh (prv_inw1);
     }

    if (paint_func)
     {
      int what;
     
        if ((*paint_func)(xptr, &ptr->wins))
         {
           if (c_inflags&FLG_ACTIVEBORDER)  BorderOn (ptr->wins.w1);
          wnoutrefresh (ptr->wins.w1);
           if (ptr->wins.w2)  wnoutrefresh (ptr->wins.w2);
          doupdate ();

          return 2;
         }
        else
         {
          return 3;
         }
     }
    else
     return 1;

 }  /**/


 void *RetrievePosted (int id, const void *yptr)

 {
  StackEntry *kptr;
  extern Stack *const stack;
 
    for (kptr=stack->top; kptr!=NULL; kptr=kptr->next)
     {
      if (((InputProcessor *)kptr->whatever)->id==id)
       {
        if (((InputProcessor *)kptr->whatever)->list==yptr)
         {
          return ((InputProcessor *)kptr->whatever)->whatever;
         }
       }
     }

   return NULL;

 }  /**/


 void *RetrieveCurrentWindow (void)

 {
    if (stack->nEntries>1)
     {
      if (!((InputProcessor *)(stack->top->whatever))->wins.w2)
       {
        return ((InputProcessor *)(stack->top->whatever))->wins.w1;
       }
      else
       {
        return ((InputProcessor *)(stack->top->whatever))->wins.w2;
       }
     }

   return NULL;

 }  /**/


 void *RetrieveCurrentParentWindow (void)

 {
    if (stack->nEntries>1)
     {
      return ((InputProcessor *)(stack->top->whatever))->wins.w1;
     }

   return NULL;

 }  /**/


 void RefreshListContents (void *yptr)

 {
    if (stack->nEntries>1)
     {
       if (((InputProcessor *)(stack->top->whatever))->id==4)
        {
         register ScrollingList *scptr=(ScrollingList *)c_inptr->whatever;

           if ((scptr->ID==SCR_OPTIONMENU)&&(scptr->parent->list==yptr))
            {
             ScrollingListLeft (scptr);

             UpdatePosition ((ScrollingList *)c_inptr->whatever,
                             ((ScrollingList *)c_inptr->whatever)->hilited);

             return;
            }
           else
           if (scptr->list==yptr)
            {
             UpdatePosition ((ScrollingList *)c_inptr->whatever,
                             ((ScrollingList *)c_inptr->whatever)->hilited);

             return;
            }
        }
      {
       register StackEntry *kptr;

        for (kptr=stack->top; kptr!=NULL; kptr=kptr->next)
         {
          if (((InputProcessor *)kptr->whatever)->id==4)
           {
            if (((InputProcessor *)kptr->whatever)->list==yptr)
             {
              ((InputProcessor *)kptr->whatever)->wins.flags|=(0x1<<6);
             }
           }
         }
      }
     }

 }  /**/


 void *RetrieveParentWindow (void)

 {
    if (stack->nEntries>2)
     {
      return ((InputProcessor *)stack->top->next->whatever)->wins.w1;
     }

   return NULL;

 }  /**/


 void *Hello (void)

 {
    if (stack->nEntries>1)  
     {
      return ((InputProcessor *)stack->top->whatever)->whatever;
     }

   return NULL;

 }  /**/


 int CursorStatus (void)

 {
   return ((c_inflags&32)?1:0);

 }  /**/ 


 /* static int CellAt[5][77]; */

 /*
 ** 24x79 is assumed...
 */
 void CheckGeometry (int h, int w, int *y, int *x)

 {
  register int m=0,
               n=0;

    if ((*x)-1<1)  *x=1;
    if ((*x)+w>79)  *x=79-w;

    if ((*y)-1<2)  *y=2;
    if (*y+h>18)   *y=19-h;

#if 0
    if (CellAt[*y][*x]==1)
       {
         while (((m=((*x)+(++n)))<76)&&(CellAt[*y][m]==1))
               ;
        *x=m;
       }

    CellAt[*y][*x]=1;
#endif
 }  /**/


 void UpdateGeometry (int y, int x)

 {
   /* CellAt[y][x]=0; */

 }  /**/


 void DrawShadow (const WINDOW *w, int how)

 {
  register int x, y;

    for (y=w->_begy+1; y<w->_begy+w->_maxy+2; y++)
     {
      x=(w->_begx+w->_maxx+1);
      how?(newscr->_line[y].text[x]|=A_DIM):(newscr->_line[y].text[x]&=~A_DIM);
      how?
       (newscr->_line[y].text[x]&=~A_COLOR):
       (newscr->_line[y].text[x]|=A_COLOR);
     }

    for (y--, x=w->_begx+1; x<w->_begx+w->_maxx+1; x++)
     {
      how?(newscr->_line[y].text[x]|=A_DIM):(newscr->_line[y].text[x]&=~A_DIM);
      how?
       (newscr->_line[y].text[x]&=~A_COLOR):
       (newscr->_line[y].text[x]|=A_COLOR);
     }

/*
    for (y=w->_begy+1; y<w->_begy+w->_maxy+2; y++)
     {
      for (x=w->_begx+1; x<w->_maxx+w->_begx+2; x++)
       {
        newscr->_line[y].text[x]|=A_DIM;
        newscr->_line[y].text[x]&=~A_COLOR;
       }
     }
*/

 }  /**/


 /* 
 ** copy contents within the coordinates of bw from curscr(sw) to 
 ** MainWin(dw)
 */
  void static
 ImprintPreviousWindow (WINDOW *dw, WINDOW *sw, Windows *bw, unsigned flags)
 {
  register int x,
               y;

    if ((bw->flags&FLG_SAVEPREVIMAGE)==0)  return;
 
    for (y=bw->w1->_begy; y<(bw->w1->_begy+bw->w1->_maxy+1); y++)
     {
       for (x=bw->w1->_begx; x<bw->w1->_maxx+bw->w1->_begx+1; x++)
        {
         wmove (dw, (flags&2?y-Y_OFFSET:y), (flags&2?x-X_OFFSET:x));
         whline (dw, sw->_line[y].text[x], 1);
        }
     }

 }  /**/


 /*
 ** flags&(0x1<<0)   malloc buffer
 ** flags&(0x1<<1)   add MainWindow offsets
 */
 static void SaveUnders (Windows *wptr, WINDOW *bw, unsigned flags)

 {
  register int x, y;
  register chtype *b;

    if (wptr->flags&FLG_NOBENEATH)  return;

    if (flags&(0x1<<0))
     {
      xmalloc(wptr->buf, 
              ((wptr->w1->_maxy+2)*(wptr->w1->_maxx+2)*sizeof(chtype)));
     }

   b=wptr->buf;

    for (y=wptr->w1->_begy; y<wptr->w1->_begy+wptr->w1->_maxy+1; y++)
     {
       for (x=wptr->w1->_begx; x<wptr->w1->_maxx+wptr->w1->_begx+1; x++)
        {
         *b++=mvwinch (bw, (flags&2?y-Y_OFFSET:y), (flags&2?x-X_OFFSET:x));
        }
     }

 }  /**/


 int NewWindow (int h, int w, int *y, int *x, Windows *wins, int flag)

 {
    if ((wins->flags&FLG_KEEPCOORDINATES)==0)  
     {
      CheckGeometry (h, w, y, x);
     }

    if (!(wins->w1=newwin(h, w, *y, *x)))
     {
      say ("%$% Error allocationg window structure!\n", HOT);

      return 0;
     }

   SaveUnders (wins, curscr, 0x1<<0);
   /*if ((wins->flags&8)==0)  GrabitII (wins);*/

    //if (wins->flags&FLG_SHADOW)  DrawShadow (wins->w1, 1);

   return 1; 

 }  /**/


 int NewSubWindow (int h, int w, int y, int x, Windows *wins)

 {
    if (!(wins->w2=subwin(wins->w1, h, w, y, x)))
     {
      say ("%$% Error allocationg subwindow structure!\n", HOT);

      return 0;
     }

   return 1;
 
 }  /**/


 void ReadKeyboardInput (void)

 {
  int key;

   switch (key=wgetch(InputWin))
    {
     case 0x16:
          {
            if (c_inflags&FLG_MOVEABLE)
             {
               if (c_instate&ST_MOVING)
                {
                 c_instate&=~ST_MOVING;

                 WindowWandering (c_inw1, 0);
                }
               else
                {
                 c_instate|=ST_MOVING;

                 WindowWandering (c_inw1, 1);
                }

              goto label_update;
             }

           break;
          }

     case 27:
           if (c_inflags&FLG_MOVEABLE)
             {
               if (c_instate&ST_MOVING)
                {
                 c_instate&=~ST_MOVING;

                 WindowWandering (c_inw1, 0);

                 goto label_update;
                }
             }

          break;

     case 0x0C:
          touchwin (curscr);
          wrefresh (curscr);

          return;

     case 'k':
     case KEY_UP:
           if (c_inflags&FLG_MOVEABLE)
            {
              if (c_instate&ST_MOVING)
                {
                 MoveWindow (-1, 0);
                 
                 goto label_update;
                }
            }

          break;

     case 'j':
     case KEY_DOWN:
           if (c_inflags&FLG_MOVEABLE)
            {
              if (c_instate&ST_MOVING)
                {
                 MoveWindow (1, 0);

                 goto label_update;
                }
            }

          break;

     case 'h':
     case KEY_LEFT:
           if (c_inflags&FLG_MOVEABLE)
            {
              if (c_instate&ST_MOVING)
                {
                 MoveWindow (0, -1);

                 goto label_update;
                }
            }

          break;

     case 'l':
     case KEY_RIGHT:
           if (c_inflags&FLG_MOVEABLE)
            {
              if (c_instate&ST_MOVING)
                {
                 MoveWindow (0, 1);

                 goto label_update;
                }
            }
    }

    if (c_inflags&FLG_MOVEABLE)
     {
       if (c_instate&ST_MOVING)
        {
         c_instate&=~ST_MOVING;

         WindowWandering (c_inw1, 0);
        }
     }
   (*((InputProcessor *)(stack->top->whatever))->func)(key);
   //(*((InputProcessor *)(stack->top->whatever))->func)(wgetch(InputWin));

   label_update:
   doupdate ();

 }  /**/

 
 static void CheckHold (void)

 {
  extern int hold_screen;

    if (set("AUTO_HOLD_MODE"))
     {
       if (hold_screen)  return;

       if (set("HOLD_MODE"))
        {
         hold_screen=1;

         return;
        }
 
      {
       char s[15];

        strcpy (s, "HOLD_MODE ON");
        hold_screen=1;
        uSET (s);
      }
     } 

 }  /**/

