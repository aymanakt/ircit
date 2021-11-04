/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: scroll.h,v 1.6 1998/04/08 08:54:34 ayman Beta $")
**
*/

 #define SCR_CHANNELS      1
 #define SCR_CHANUSERS     2
 #define SCR_OPTIONMENU    3
 #define SCR_SCROLLINGLIST 4

 /* this can be shared, because its text only */
 struct header {
         int n;
         const int *cap_len;
         const char **caption;
        };
 typedef struct header header;

 struct ScrollingListEntry {
        int attrs;
        void *link;
       };
 typedef struct ScrollingListEntry ScrollingListEntry;


 struct ScrollingList {
         int ID;
         int nEntries;
         int *attrs;
         int startx,
             starty;
         int width,
             hight;
         int cnt,
             hilited,
             page_size,
             listwin_start;
             int flags;
         char caption[20];
         int  *lock;
         void *c_item;
         header *h;
         void *list;
         struct ScrollingList *parent,
                              *opt_menu;
         void (*func_cleanup) (struct ScrollingList *);
         void (*func)(struct ScrollingList *);
         void (*func_init)(struct ScrollingList *);
         void (*enter_func)(void *, int);
        };
 typedef struct ScrollingList ScrollingList;


 extern const ObjectsAndColors *const _list;
 extern const ObjectsAndColors *const _listborder;
 extern const ObjectsAndColors *const _listmenu;
 extern const ObjectsAndColors *const _listmenuborder;

 #define sc_up_blink(x) \
  _wmove (w1, (x)->hight-1, 7); \
  whline (w1, ACS_UARROW|A_BLINK, 1)

 #define sc_down_blink(x) \
  _wmove (w1, (x)->hight-1, 7); \
  whline (w1, ACS_DARROW|A_BLINK, 1)

 void LoadScrollingList (ScrollingList *);
 void LoadOptionMenu (ScrollingList *); 

 void DisplayScrollingListEntries (ScrollingList *);

 void DisplayOptionMenuEntries (ScrollingList *);

 void UpdatePosition (ScrollingList *, int);

 void RestoreMainWin (ScrollingList *);
 void ScrollingListEnter (ScrollingList *);
 void ScrollingListLeft (ScrollingList *);

 int GetScrollingListInput (int);

 void CleanAllocatedList (ScrollingList *);
 void AdjustScrollingList (void);
 ScrollingList *CreateScrollingList (int, const void *);
 void ScrollingListCleanup (void *);
 int isoptionmenu (void *);

