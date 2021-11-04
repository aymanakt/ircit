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
#include <stack.h>
#include <scroll.h>
#include <windows.h>
#include <colors.h>

#include <menu.h>
#include <menu-data.h>

 MODULEID("$Id: menu.c,v 1.15 1998/04/09 14:45:39 ayman Beta $");

 static void MarkMenubarUnderline (WINDOW *, const MenuItem *);
 static void MarkMenuboxUnderline (WINDOW *, const MenuboxEntry *, int);

 static int DrawMenuBar (Menu *, Windows *);
 static void DisplayMenuBarEntries (struct Menu *);

 static int DrawMenuBox (MenuItem *, Windows *);
 static int DrawSubMenuBox (MenuItem *, Windows *);
 static void DisplayMenuBoxEntries (MenuItem *);

 static void MenuLeft (struct Menu *);
 static void MenuRight (struct Menu *);
 static void MenuDown (struct Menu *);
 static void MenuHome (struct Menu *);
 static void MenuEnd (struct Menu *);
 static void MenuEnter (struct Menu *);

 static void MenuboxDown (MenuItem *);
 static void MenuboxUp (MenuItem *);
 static void MenuboxPageup (MenuItem *);
 static void MenuboxPagedown (MenuItem *);
 static void MenuboxRight (MenuItem *);
 static void MenuboxLeft (MenuItem *);
 static void MenuboxEnter (MenuItem *);


#define c_iptr \
     ((MenuItem *)((InputProcessor *)(stack->top->whatever))->whatever)

 #define c_lines c_iptr->geometry->nEntries
 #define c_width c_iptr->geometry->width

 #define l_lines l_iptr->geometry->nEntries
 #define l_width l_iptr->geometry->width


 Menu *menu_ptr=&menu;
 List MenuSystem;
 extern Stack *const stack;


 void LoadMenuBar (Menu *mptr)

 {
  register Windows *wins;

    switch (LoadStack(mptr, NULL, 
                      GetMenuInput, MenuBarCleanup, 
                      (int (*)(void *, Windows *))DrawMenuBar,
                      2, 0, FLGS_MENUBAR))
     {
      case 2:
           LOADMENUBAR();
           break;

      case 3:
           LoadPrevLevel ();
           DoUpperStatus ();
           menu_ptr=NULL;
           break;
     }

 }  /**/


 void LoadMenuBox (MenuItem *miptr)

 {
  register Windows *wins;

    switch (LoadStack(miptr, NULL, GetMenuBoxInput, NULL, 
                       (int (*)(void *, Windows *))DrawMenuBox,
                       3, 1, FLGS_MENUBOX))
     {
      case 3:
           ClearMenubox ();
           break;
     }

 }  /**/


 void LoadSubMenuBox (MenuItem *miptr)

 {
  register Windows *wins;

    switch (LoadStack((void *)miptr, NULL, GetMenuBoxInput, NULL, 
                      (int (*)(void *, Windows *))DrawSubMenuBox,
                      3, 2, FLGS_MENUBOX))
     {
      case 3:
           ClearMenubox ();
           break;
     }

 }  /**/


  static __inline__  void
 MarkMenubarUnderline (WINDOW *w, const MenuItem *miptr)

 {
    if ((miptr->underline>=0)&&(miptr->underline<=strlen(miptr->caption)))
     {
      w->_line[0].text[miptr->caption_start+miptr->underline]|=A_BOLD;
     }
    
 }  /**/


  static __inline__ void 
 MarkMenuboxUnderline (WINDOW *w, const MenuboxEntry *mbxptr, int y)

 {
    if ((mbxptr->item.underline>=0)&&
        (mbxptr->item.underline<=strlen(mbxptr->item.name)))
     {
      w->_line[y].text[mbxptr->item.underline]|=A_BOLD;
     }

 }  /**/


 static __inline__ void DisplayMenuBarEntries (Menu *mptr)

 {
  register int i;
  register MenuItem *miptr;
  WINDOW *w=RetrieveCurrentWindow();

    for (i=0; i<=mptr->nItems-1; i++)
     {
      miptr=&mptr->items[i];

       if (!mptr->attrs[i])
        {
         _wattron (w, COLOR_PAIR(CLR_MENUINV));
        }

       if (i==mptr->c_hilited)
        {
         _wattron (w, A_REVERSE);
        }

      _wmove (w, 0, miptr->caption_start);
      Fwprintf_nout (w, miptr->caption);

      MarkMenubarUnderline (w, miptr);

       if (!mptr->attrs[i])
        {
         _wattroff (w, COLOR_PAIR(CLR_MENUINV));
        }

       if (i==mptr->c_hilited)
        {
         _wattroff (w, A_REVERSE);
        }
     }

   wnoutrefresh (w);
   doupdate ();

 }  /**/


 static int DrawMenuBar (Menu *mptr, Windows *wins)

 {
  register Windows *auxwins=wins;

    if (!(NewWindow(1, mptr->cols, &mptr->starty, &mptr->startx, auxwins, 0)))
     {
      return 0;
     }

   wattrset (auxwins->w1, COLOR_PAIR(CLR_MENU));
   wbkgd (auxwins->w1, COLOR_PAIR(CLR_MENU));

   {
    chtype chs[80];

     chsprintf (chs, "    IRCIT  Status   List         Help");
     FancyOutput2 (RetrieveCurrentWindow(), chs, 0, 79, 0);
   }

   DisplayMenuBarEntries (mptr);

   return 1;

 }  /**/


 static __inline__ void DisplayMenuBoxEntries (MenuItem *miptr)

 {
  register int i;
  register MenuboxEntry *xptr;
  WINDOW *w=RetrieveCurrentWindow();
  WINDOW *w1=RetrieveCurrentParentWindow();

    for (i=0; i<=(c_lines-1); i++)
     {
      xptr=&miptr->entries[i];
       if (!*xptr->item.name)  xptr->item.attrs=0;

       if (!xptr->item.attrs)
        {
         wattrset (w, COLOR_PAIR(CLR_MENUBOXINV));
        }

       if (i==miptr->c_hilited)
        {
         _wattron (w, A_REVERSE);
        }

      _wmove (w, i, 0);

       if (*xptr->item.name<0x4)
        {
         switch (*xptr->item.name)
          {
           case 0x1:
                Fwprintf_nout (w, " %$196,* ", c_width-2);
                break;
           case 0x2:
                Fwprintf_nout (w, "%$196,*", c_width);
                break;
           case 0x3:
                Fwprintf_nout (w, " %$32,*  ", c_width);
          }
        }
       else
        {
         Fwprintf_nout (w, "%s%$% ", xptr->item.name, xptr->smenu?16:' ');

         MarkMenuboxUnderline (w, xptr, i);
        }

       if (!xptr->item.attrs)
        {
         wattrset (w, COLOR_PAIR(CLR_MENUBOX));
        }

       if (i==miptr->c_hilited)
        {
         _wattroff (w, A_REVERSE);
        }
     }

   wnoutrefresh (w1);
   wnoutrefresh (w);
   doupdate();

 }  /**/


 static int DrawMenuBox (MenuItem *miptr, Windows *wins)

 {
  int y=2;
  register Windows *auxwins=wins;

    if (!menu_ptr->attrs[menu_ptr->c_hilited])  return 0;

    if (!(NewWindow(c_lines+2, c_width+2,&y, &miptr->caption_start, auxwins,0)))
     {
      return 0;
     }

    if (!(NewSubWindow(c_lines, c_width, 3, miptr->caption_start+1, auxwins)))
     {
      return 0;
     }

   wattrset (auxwins->w1, COLOR_PAIR(CLR_MENUBORDER));
   wattrset (auxwins->w2, COLOR_PAIR(CLR_MENUBOX));

   ExplodeWindowGravitySouth (auxwins->w1);
   box (auxwins->w1, ACS_VLINE, ACS_HLINE);

   {
     while (*(miptr->entries[miptr->c_hilited].item.name)<0x4)
      {
        if (miptr->c_hilited==(c_lines)-1)  break;

       miptr->c_hilited++;
      }
   }

   DisplayMenuBoxEntries (miptr);

   return 1;

 }  /**/


 static int DrawSubMenuBox (MenuItem *miptr, Windows *wins)

 {
  register Windows *auxwins=wins;
  int x, y;

   y=(prv_inw1->_begy)+miptr->caption_start;
   x=((prv_inw1->_begx)+(miptr->geometry->width))-2;

    if (!(NewWindow(c_lines+2, c_width+2, &y, &x, auxwins, 0)))
     {
      return 0;
     }

    if (!(NewSubWindow(c_lines, c_width, y+1, x+1, auxwins)))
     {
      return 0;
     }

   wattrset (auxwins->w1, COLOR_PAIR(CLR_MENUBORDER));
   wattrset (auxwins->w2, COLOR_PAIR(CLR_MENUBOX));

   ExplodeWindowGravityEast (auxwins->w1);
   box (auxwins->w1, ACS_VLINE, ACS_HLINE);
   DisplayMenuBoxEntries (miptr);

   return 1;

 }  /**/


 void ClearMenubox (void)

 {
   LoadPrevLevel ();

 }  /**/


 void MenuBoxTerminator (void *ptr)

 {
   /* NOTHING! */

 }  /**/


 static __inline__ void MenuEnter (struct Menu *mptr)

 {
    if (mptr->enter_func)
     {
      (*mptr->enter_func)(mptr);
     }
    else
     {
      MenuItem *iptr;

       iptr=&mptr->items[mptr->c_hilited];

       LoadMenuBox (iptr);
     }

 }  /**/


 static __inline__ void MenuLeft (struct Menu *mptr)

 {
    if (--mptr->c_hilited<0)
     {
      mptr->c_hilited=(mptr->nItems-1);
     }

   DisplayMenuBarEntries (mptr);

 }  /**/


 static __inline__ void MenuRight (struct Menu *mptr)

 {
    if ((++mptr->c_hilited)>(mptr->nItems-1))
     {
      mptr->c_hilited=0;
     }

   DisplayMenuBarEntries (mptr);

 }  /**/


 static __inline__ void MenuHome (struct Menu *mptr)

 {
   mptr->c_hilited=0;
   DisplayMenuBarEntries (mptr);

 }  /**/


 static __inline__ void MenuEnd (struct Menu *mptr)

 {
   mptr->c_hilited=mptr->nItems-1;
   DisplayMenuBarEntries (mptr);

 }  /**/


 static __inline__ void MenuDown (struct Menu *mptr)

 {
   MenuEnter (mptr);

 }  /**/


 static void ActivateMenubarUnderline (Menu *, int);
 static void MenubarNumericSelection (Menu *, int);

 static void MenubarNumericSelection (Menu *mptr, int key)

 {
  int number=key-'0';
  int cur=mptr->c_hilited+1;
  int des;

    if ((des=number+cur)>mptr->nItems)  return;

   mptr->c_hilited=des-1;
   DisplayMenuBarEntries (mptr);

 }  /**/


 static void ActivateMenubarUnderline (Menu *mptr, int key)

 {
  int c1, c2;
  register int n;
  register MenuItem *miptr;

   c1=toupper (key);
   
    for (n=0; n<=mptr->nItems-1; n++)
     {
      miptr=&mptr->items[n];

       if ((miptr->underline>=0)&&
           ((c2=toupper(*(miptr->caption+miptr->underline)))==c1))
        {
         mptr->c_hilited=n;

         DisplayMenuBarEntries (mptr);
         MenuEnter (mptr);

         return;
        }
     }

    if (isdigit(key))
     {
       if (key=='0')
        {
         MenuHome (mptr);

         return;
        }

      MenubarNumericSelection (mptr, key);
     }

   return;

 }  /**/


 int GetMenuInput (int key)

 {
  register Menu *mptr=Hello();

    if (key<KEY_MIN)
       {
         if (key<=31)
            {
             switch (key)
                    {
                     case '\n':
                     case '\r':
                          MenuEnter (mptr);
                          break;

                     case 27:
                          BlowMenu (mptr);
                          break;
                    }
            } 
         else
            {
             switch (key)
              {
               case '>':
               case '+':
                     MenuRight (mptr);
                     break;

               case '<':
               case '-':
                     MenuLeft (mptr);
                     break;

               default:
                     ActivateMenubarUnderline (mptr, key);
                     break;
              }
            }
       }
    else
    if (key<KEY_MAX)
       {
        switch (key)
               {
                case KEY_LEFT:
                     MenuLeft (mptr);
                     break;

                case KEY_RIGHT:
                     MenuRight (mptr);
                     break;

                case KEY_UP:
                     break;

                case KEY_DOWN:
                     MenuDown (mptr);
                     break;

                case KEY_PPAGE:
                case KEY_HOME:
                     MenuHome (mptr);
                     break;

                case KEY_NPAGE:
                case KEY_END:
                     MenuEnd (mptr);
                     break;

                case KEY_ENTER:
                     MenuEnter (mptr);
                     break;

                case 0413:
                     BlowMenu (mptr);
               }
       }

 }  /**/


 static __inline__ void MenuboxDown (MenuItem *miptr)

 {
  register int i=0;

    if (++miptr->c_hilited>(c_lines)-1) miptr->c_hilited=0;

    while (*(miptr->entries[miptr->c_hilited].item.name)<0x4)
     {
       if (++i==c_lines)  break;

       if (++miptr->c_hilited>(c_lines)-1)
        {
         miptr->c_hilited=0;
        }
     }
    
   DisplayMenuBoxEntries (miptr);

 }  /**/


 static __inline__ void MenuboxUp (MenuItem *miptr)

 {
  register int i=0;

    if (--miptr->c_hilited<0)  miptr->c_hilited=(c_lines)-1;

    while (*(miptr->entries[miptr->c_hilited].item.name)<0x4)
     {
       if (++i==c_lines)  break;

       if (--miptr->c_hilited<0)
        {
         miptr->c_hilited=(c_lines)-1;
        }
     }
    
   DisplayMenuBoxEntries (miptr);

 }  /**/


 static __inline__ void MenuboxPageup (MenuItem *miptr)

 {
   miptr->c_hilited=0;

    while (*(miptr->entries[miptr->c_hilited].item.name)<0x4)
     {
       if (miptr->c_hilited==(c_lines)-1)  break;

      miptr->c_hilited++;
     }
    
   DisplayMenuBoxEntries (miptr);

 }  /**/


 static __inline__ void MenuboxPagedown (MenuItem *miptr)

 {
   miptr->c_hilited=(c_lines)-1;

    while (*(miptr->entries[miptr->c_hilited].item.name)<0x4)
     {
       if (miptr->c_hilited==0)  break;

      miptr->c_hilited--;
     }

   DisplayMenuBoxEntries (miptr);

 }  /**/


 static __inline__ void MenuboxLeft (MenuItem *miptr)

 {
  MenuboxEntry *xptr;

   xptr=&miptr->entries[miptr->c_hilited];

   ClearMenubox ();

    if (c_inptr->id==2) 
     {
      MenuLeft (menu_ptr);
     }

 }  /**/


 static __inline__ void MenuboxRight (MenuItem *miptr)

 {
  MenuboxEntry *xptr;

   xptr=&c_iptr->entries[miptr->c_hilited];

    if (xptr->smenu)
     {
      ((MenuItem *)(xptr->smenu))->caption_start=miptr->c_hilited;
      LoadSubMenuBox ((MenuItem *)(xptr->smenu));
     }
    else
    if (xptr->right_func)  /* normally terminator */
     {
      (*xptr->right_func)((void *)miptr);
     }
    else
     {
      ClearMenubox ();

      MenuRight (&menu);
     }

#if 0
  MenuboxEntry *xptr;

   xptr=&miptr->entries[miptr->c_hilited];

    if (xptr->item.attrs==0)  return;  /* entry disabled */

    if (xptr->smenu)
     {
       if (xptr->item.attrs==0)  return;  /* entry disabled */

      ((MenuItem *)(xptr->smenu))->caption_start=miptr->c_hilited;
      LoadSubMenuBox ((MenuItem *)(xptr->smenu));
     }
    else
     {
       if (!set("MENUBOX_TERMINATOR"))
        {
         BlowupMenuBoxes ();
         MenuRight (menu_ptr);
        }
     }
#endif
 }  /**/


 static __inline__ void MenuboxEnter (MenuItem *miptr)

 {
  register int idx;
  register MenuboxEntry *xptr;

   xptr=&c_iptr->entries[(idx=c_iptr->c_hilited)];

    if (xptr->smenu)
     {
      MenuboxRight (/*(MenuItem *)(xptr->smenu)*/c_iptr);
     }
    else
    if (xptr->enter_func)
     {
       if (mt_ptr->c_output&STICKY_MENU)
        {
         BlowupMenuBoxes();
        }
       else
        {
         BlowMenu (&menu);
        }

      (*xptr->enter_func)(NULL, idx);
     }

 }  /**/


 static void MenuboxNumericSelection (MenuItem *, int);
 static void ActivateMenuboxUnderline (MenuItem *, int);


 static void MenuboxNumericSelection (MenuItem *miptr, int key)

 {
  int number=key-'0';
  int cur=miptr->c_hilited+1;
  int des;

    if ((des=number+cur)>miptr->geometry->nEntries)  return;

    if (*(miptr->entries[des-1].item.name)<0x4)
     {
      MenuboxNumericSelection (miptr, key+1);
     }
    else
     {
      miptr->c_hilited=des-1;
      DisplayMenuBoxEntries (miptr);
     }

 }  /**/


 static void ActivateMenuboxUnderline (MenuItem *miptr, int key)

 {
  int c1, c2;
  register int n;
  register MenuboxEntry *mbxptr;

   c1=toupper (key);

    for (n=0; n<=miptr->geometry->nEntries-1; n++)
     {
      mbxptr=&miptr->entries[n];

       if ((mbxptr->item.underline>=0)&&
           ((c2=toupper(*(mbxptr->item.name+mbxptr->item.underline)))==c1))
        {
         miptr->c_hilited=n;

         DisplayMenuBoxEntries (miptr);
         MenuboxEnter (miptr);

         return;
        }
     }

    if (isdigit(key))
     {
       if (key=='0')
        {
         MenuboxPageup (miptr);

         return;
        }

      MenuboxNumericSelection (miptr, key);
     }

   return;

 }  /**/


 int GetMenuBoxInput (int key)

 {
  register MenuItem *miptr=Hello();

    if (key<KEY_MAX)
       {
        switch (key)
               {
                case '\n':
                case '\r':
                     MenuboxEnter (miptr);
                     break;

                case 27:
                     ClearMenubox ();
                     break;

                case KEY_LEFT:
                case '<':
                     MenuboxLeft (miptr);
                     break;

                case KEY_RIGHT:
                case '>':
                     MenuboxRight (miptr);
                     break;

                case '-':
                case KEY_UP:
                     MenuboxUp (miptr);
                     break;

                case '+':
                case KEY_DOWN:
                     MenuboxDown (miptr);
                     break;

                case KEY_HOME:
                case KEY_PPAGE:
                     MenuboxPageup (miptr);
                     break;

                case KEY_END:
                case KEY_NPAGE:
                     MenuboxPagedown (miptr);
                     break;

                case 0413:
                     BlowMenu (menu_ptr);

                default:
                      if (isalnum(key))
                       {
                        ActivateMenuboxUnderline (miptr, key);
                       }
                     /* Gets (key); not ready yet */
               }
        }

 }  /**/


 void BlowMenu (Menu *mptr)

 {
    while (((InputProcessor *)(stack->top->whatever))->id==3)
     {
      ClearMenubox ();
     }  

    if (((InputProcessor *)(stack->top->whatever))->id==2)
     {
      LoadPrevLevel ();
      DoUpperStatus ();
     }
    else
     {
      beep ();
      say ("%$% where's the menubar gone?\n", HOT);  
     }  

 }  /**/


 void BlowupMenuBoxes (void)

 {
    while (Hello()!=menu_ptr)
     {
      ClearMenubox ();
     }

 }  /**/


 void MenuBarCleanup (void *mptr)

 {
   UNLOADMENUBAR();

 }  /**/


 void F3 (void)

 {
   LoadMenuBar (menu_ptr);

 }  /**/

