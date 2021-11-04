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
#include <list.h>
#include <scroll.h>
#include <colors.h>
#include <windows-aux.h>

 MODULEID("$Id: scroll.c,v 1.15 1998/04/09 14:45:39 ayman Beta $");


 static void FlashSelected (const ScrollingList *, unsigned);
 static int DrawScrollingList (ScrollingList *, Windows *);
 static int DrawOptionMenu (ScrollingList *, Windows *);

 static void ScrollingListUp (ScrollingList *);
 static void ScrollingListDown (ScrollingList *);
 static void _ScrollingListRight (ScrollingList *);
 static void _ScrollingListLeft (ScrollingList *);
 static void _ScrollingListEnter (ScrollingList *);
 static void ScrollingListPageup (ScrollingList *);
 static void ScrollingListPagedown (ScrollingList *);
 static void ScrollingListEnd (ScrollingList *);
 static void ScrollingListHome (ScrollingList *);


 int DrawScrollingList (ScrollingList *sptr, Windows *wins)

 {
  register int i=0;
  register Windows *auxwins=wins;

    if (!(NewWindow(sptr->hight, sptr->width, 
                    &sptr->starty, &sptr->startx, 
                    auxwins, 1)))
     {
      return 0;
     }

    if (!(NewSubWindow(sptr->hight-4, sptr->width-2, 
                       sptr->starty+3, sptr->startx+1,
                       auxwins)))
     {
      return 0;
     }

   wattrset (auxwins->w1, COLOR_PAIR(CLR_LISTBORDER));
   ExplodeWindow (auxwins->w1, EXPLODE_OUT);
   box (auxwins->w1, ACS_VLINE, ACS_HLINE);

   _wmove (auxwins->w1, 0, 2);
   waddch_ (auxwins->w1, ACS_RTEE);
   wattrset (auxwins->w1, COLOR_PAIR(CLR_LIST));
   i=Fwprintf_nout (auxwins->w1, "%s", sptr->caption);
   wattrset (auxwins->w1, COLOR_PAIR(CLR_LISTBORDER));
   _wmove (auxwins->w1, 0, i+3);
   waddch_ (auxwins->w1, ACS_LTEE);

    if (sptr->h)
     {
      register header *h=sptr->h;

       _wmove (auxwins->w1, 2, 2);

        for (i=0; i<=h->n-1; i++)
         {
          wprintw (auxwins->w1, "%-*s", h->cap_len[i], h->caption[i]);
           if (i!=h->n-1)
            {
             waddch_ (auxwins->w1, ACS_VLINE);
            }
         }
     }

    if (sptr->list)
     {
      _wmove (auxwins->w1, /*sptr->hight-1*/auxwins->w1->_maxy, 2);
      Fwprintf_nout (auxwins->w1,"[%$%]%$196[%$%]%$196[%d:1]",
                ACS_BULLET|COLOR_PAIR(RED),
                ACS_DARROW|COLOR_PAIR(CYAN), sptr->nEntries);
     }

   wattrset (auxwins->w2, COLOR_PAIR(CLR_LIST));
   wbkgd (auxwins->w2, COLOR_PAIR (CLR_LIST));

   sptr->listwin_start=0;
   sptr->hilited=0;
   sptr->page_size=/*sptr->hight-4*/auxwins->w1->_maxy-3;

   (*sptr->func)(sptr);

   return 1; 
            
 }  /**/


 static int DrawOptionMenu (ScrollingList *sptr, Windows *wins)

 {
  register Windows *auxwins=wins;
  register WINDOW *prvw=RetrieveParentWindow();
  #define pscr_ptr ((ScrollingList *)sptr->parent)

   sptr->starty=((prvw->_begy-1)+4)+
                (pscr_ptr->hilited-pscr_ptr->listwin_start);

    if (!(NewWindow(sptr->nEntries+2, sptr->width+2,
                    &sptr->starty, &sptr->startx,
                    auxwins, 1)))
     {
      return 0;
     }

    if (!(NewSubWindow(sptr->nEntries, sptr->width,
                       sptr->starty+1, sptr->startx+1,
                       auxwins)))
     {
      return 0;
     }

   wattrset (auxwins->w1, COLOR_PAIR(CLR_LISTMENUBORDER));
   ExplodeWindowGravityEast (auxwins->w1);
   box (auxwins->w1, ACS_VLINE, ACS_HLINE);

   wattrset (auxwins->w2, COLOR_PAIR(CLR_LISTMENU));

   sptr->listwin_start=0;
   sptr->hilited=0;
   sptr->page_size=sptr->hight;

   DisplayOptionMenuEntries (sptr);

   return 1;

 }  /**/


 void DisplayOptionMenuEntries (ScrollingList *sptr)

 {
  register int i;
  register header *h=sptr->h;
  WINDOW *w=RetrieveCurrentWindow();
  #define opt_sptr ((ScrollingList *)sptr->opt_menu)

    for (i=0; i<=(sptr->nEntries-1); i++)
     {
       if (!pscr_ptr->attrs[i])
        {
         wattron (w, COLOR_PAIR(CLR_LISTMENUINV));
        }

       if (i==sptr->hilited)
        {
         wattron (w, A_REVERSE);
        }

      _wmove (w, i, 0);
      Fwprintf_nout (w, h->caption[i]);

       if (!pscr_ptr->attrs[i])
        {
         wattron (w, COLOR_PAIR(CLR_LISTMENU));
        }

       if (i==sptr->hilited)
        {
         wattroff (w, A_REVERSE);
        }
     }  

 }  /**/


 void UpdatePosition (ScrollingList *sptr, int idx)

 {
  int oidx,
      last; 
  register WINDOW *w=RetrieveCurrentWindow(),
                  *w1=RetrieveCurrentParentWindow();

    if ((List *)sptr->list){
   oidx=sptr->hilited;

    if (((List *)sptr->list)->nEntries<sptr->page_size)
     {
      last=sptr->listwin_start+((List *)sptr->list)->nEntries-1;
     }
    else
     {
      last=sptr->listwin_start+sptr->page_size-1;
     }

    if (idx<sptr->listwin_start)
     {
       if (idx<0)  idx=0;

      sptr->hilited=sptr->listwin_start=idx;
     }
    else
    if (idx>last)
     {
       if (idx>(((List *)sptr->list)->nEntries-1))
        {
         idx=((List *)sptr->list)->nEntries-1;
        }

      sptr->hilited=idx;
      sptr->listwin_start=idx-(sptr->page_size-1);

       if (sptr->listwin_start<0)  sptr->listwin_start=0;
     }
    else
     {
      sptr->hilited=idx;
     }

   {
     if (sptr->ID!=SCR_OPTIONMENU)
      {
       _wmove (w1, sptr->hight-1, 7);

        if (oidx<idx)
         {
          Fwprintf_nout (w1, "%$25]%$196[%d:%d]%$196%$196", 
                    ((List *)sptr->list)->nEntries,
                    idx+1);
         }
        else
        if (oidx>idx)
         {
          Fwprintf_nout (w1, "%$24]%$196[%d:%d]%$196%$196", 
                    ((List *)sptr->list)->nEntries, 
                    idx+1);
         }
        else
         {
          Fwprintf_nout (w1,"%$%]%$196[%d:%d]%$196%$196",
                    idx==(((List *)sptr->list)->nEntries)-1?
                     (ACS_UARROW|A_BLINK):(ACS_DARROW|A_BLINK),  
                    ((List *)sptr->list)->nEntries,
                    idx+1);
         }
      }
   }
}
   (*sptr->func)(sptr);

   wnoutrefresh (w1);
   wnoutrefresh (w); 
   /* doupdate(); */

 }  /**/


 void ScrollingListLeft (ScrollingList *sptr)

 {
   LoadPrevLevel ();

 }  /**/


 static __inline__ void ScrollingListUp (ScrollingList *const sptr)

 {
   UpdatePosition (sptr, sptr->hilited-1);

 }  /**/


 static __inline__ void ScrollingListDown (ScrollingList *const sptr)

 {
   UpdatePosition (sptr, sptr->hilited+1);

 }  /**/


 static __inline__ void _ScrollingListRight (ScrollingList *const sptr)

 {
    if (sptr->func_init)
     {
      (*sptr->func_init)(sptr);
     }

    if (sptr->opt_menu)
     {
      FlashSelected (sptr, 50);
      LoadOptionMenu (sptr->opt_menu);
     }

 }  /**/


 static __inline__ void _ScrollingListLeft (ScrollingList *sptr)

 {
   ScrollingListLeft (sptr);

 }  /**/


 static __inline__ void _ScrollingListEnter (ScrollingList *sptr)

 {
   ScrollingListEnter (sptr);

 }  /**/


 static __inline__ void ScrollingListPageup (ScrollingList *const sptr)

 {
   UpdatePosition (sptr, (sptr->hilited-sptr->page_size));

 }  /**/


 static __inline__ void ScrollingListPagedown (ScrollingList *const sptr)

 {
   UpdatePosition (sptr, (sptr->hilited+sptr->page_size));

 }  /**/



 static __inline__ void ScrollingListHome (ScrollingList *const sptr)

 {
   UpdatePosition (sptr, 0);

 }  /**/


 static __inline__ void ScrollingListEnd (ScrollingList *const sptr)

 {
   UpdatePosition (sptr, ((List *)sptr->list)->nEntries);

 }  /**/


 #define IS_STICKYBITON() (mt_ptr->c_output&STICKY_LISTS)

 void ScrollingListEnter (ScrollingList *sptr)

 {
  int idx;

   if (sptr->ID==SCR_OPTIONMENU)
    {
      if (idx=sptr->hilited, !sptr->parent->attrs[idx])  return;

      if (IS_STICKYBITON())
       {
        FlashSelected (sptr, 100);
        _ScrollingListLeft (sptr);
    
        sptr=Hello ();
         if (sptr->enter_func)
          {
           (*sptr->enter_func)(sptr->c_item, idx);
          }
       }
      else
       {
        void *ptr;
        void (*func)(void *, int);

          if (sptr->parent->lock)  *(sptr->parent->lock)&=~(0x1<<1);

         _ScrollingListLeft (sptr); 
         sptr=Hello ();
         ptr=sptr->c_item;
         func=sptr->enter_func;

         _ScrollingListLeft (sptr);
             
          if (func)  (*func)(ptr, idx);
       }
    }
   else
   if (sptr->ID==SCR_SCROLLINGLIST)
    {
      if (IS_STICKYBITON())
       {
        FlashSelected (sptr, 200);
        (*sptr->enter_func)(sptr->c_item, 0);
       }
      else
       {
        int idx;
        void *ptr;
        void (*func)(void *, int);

          if (sptr->lock)  *(sptr->lock)&=~(0x1<<1);

         idx=0;
         ptr=sptr->c_item;
         func=sptr->enter_func;

         _ScrollingListLeft (sptr);

          if (func)  (*func)(ptr, idx);
       }
    }

 }  /**/


 int GetScrollingListInput (int key)

 {
  ScrollingList *sptr=Hello();

    if (key<KEY_MAX)
       {
        switch (key)
               {
                case KEY_UP:
                     ScrollingListUp (sptr);
                     break;

                case KEY_DOWN:
                     ScrollingListDown (sptr);
                     break;

                case KEY_LEFT:
                     _ScrollingListLeft (sptr);
                     break;

                case KEY_RIGHT:
                     _ScrollingListRight (sptr);
                     break;

                case '\n':
                case '\r':
                     _ScrollingListEnter (sptr);
                     break;

                case KEY_HOME:
                     ScrollingListHome (sptr);
                     break;

                case KEY_END:
                     ScrollingListEnd (sptr);
                     break;

                case KEY_PPAGE:
                     ScrollingListPageup (sptr);
                     break;

                case KEY_NPAGE:
                     ScrollingListPagedown (sptr);
                     break;

                case '\t':
                     break;

                case 27:
                     _ScrollingListLeft (sptr);
                     break;

                case 0x18:
                     NextPrev (); break;

                default:
                    Gets (key);
               }
        }

 }  /**/


 void CleanAllocatedList (ScrollingList *sptr)

 {
   CleanupListfromArray ((List *)sptr->list);

 }  /**/


 void AdjustScrollingList (void)

 {
#if 0
   if (stack->nEntries<=1) /* no GUI was posted */
      {
       return;
      }

   if (((InputProcessor *)(stack->top->whatever))->id==4)
      {
        if (scptr->ID==SCR_OPTIONMENU)
           {
            _ScrollingListLeft (); /* get rid of option menu */
           }

        /* cheap cheating...*/

       ScrollingListUp ();
       ScrollingListDown ();
      }
#endif
 }  /**/


 ScrollingList *CreateScrollingList (int optmenu, const void *ptr)

 {
  const size_t sizeofScrollingList=sizeof(ScrollingList);
  ScrollingList *auxscrptr;

    if (!((void *)auxscrptr=RetrievePosted(4, ptr)))
     {
      xmalloc(auxscrptr, sizeofScrollingList);
      memset (auxscrptr, 0, sizeofScrollingList);

      auxscrptr->ID=SCR_SCROLLINGLIST;
      auxscrptr->flags|=0x1; 

       if (optmenu)
        {
         xmalloc(auxscrptr->opt_menu, sizeofScrollingList);
         memset (auxscrptr->opt_menu, 0, sizeofScrollingList);

         auxscrptr->opt_menu->flags|=0x1;
         auxscrptr->opt_menu->func=DisplayOptionMenuEntries;

         ((ScrollingList *)(auxscrptr->opt_menu))->parent=auxscrptr;

         ((ScrollingList *)(auxscrptr->opt_menu))->ID=SCR_OPTIONMENU;
        }

      return (ScrollingList *)auxscrptr;
     }
    else
     {
      LoadScrollingList (auxscrptr);

      return NULL;
     }

 }  /**/


 void ScrollingListCleanup (void *scrptr)

 {
    if (((ScrollingList *)scrptr)->lock)
     {
      *((ScrollingList *)scrptr)->lock=0;
     }

    if (((ScrollingList *)scrptr)->flags&0x1)
     {
       if (((ScrollingList *)scrptr)->opt_menu)  
        {
         free (((ScrollingList *)scrptr)->opt_menu);
        }

       if (((ScrollingList *)scrptr)->attrs)  
        {
         free (((ScrollingList *)scrptr)->attrs);
        }

      free (scrptr);
     }

 }  /**/


 int isoptionmenu (void *ptr)

 {
    if (((ScrollingList *)ptr)->ID==3)  return 1;

   return 0;

 }  /**/


 void LoadOptionMenu (ScrollingList *sptr)

 {
  register Windows *wins;

   switch (LoadStack(sptr, NULL,
                     GetScrollingListInput, NULL, 
                     (int (*)(void *, Windows *))DrawOptionMenu,
                     4, 2, FLGS_SCROLLSUB))
    {
     case 3:
          LoadPrevLevel ();
    }

 }  /**/


 void LoadScrollingList (ScrollingList *sptr)

 {
  register Windows *wins;

    switch ((LoadStack(sptr, sptr->list,
                   GetScrollingListInput, ScrollingListCleanup,
                   (int (*)(void *, Windows *))DrawScrollingList,
                   4, 0, FLGS_SCROLLMAIN)))
     {
      case 2:
            if (sptr->lock)
             {
              *sptr->lock=0;
              *(sptr->lock)|=(0x1<<0);
              *(sptr->lock)|=(0x1<<1);
              *(sptr->lock)|=(0x1<<2);
             }

            if (((sptr->flags&(0x2<<0))||(IS_SHOWOPTMENU()))&&(sptr->opt_menu))
             {
              _ScrollingListRight (sptr);
             }

           break;

      case 3: /* error in paint */
           LoadPrevLevel ();
     }

 }  /**/


 void FlashSelected (const ScrollingList *sptr, unsigned time)

 {
  register int x, y;
  register chtype *chs;
  WINDOW *w=RetrieveCurrentWindow();

   y=(sptr->hilited-sptr->listwin_start);

    for (x=0; x<=w->_maxx-1; x++)  w->_line[y].text[x]|=A_BOLD;
   w->_line[y].firstchar=0; 
   w->_line[y].lastchar=w->_maxx; 

   wrefresh (w);
   napms (time);

    for (x=0; x<=w->_maxx; x++)  w->_line[y].text[x]&=~A_BOLD;
   w->_line[y].firstchar=0;
   w->_line[y].lastchar=w->_maxx;

   wrefresh (w);

 }  /**/

