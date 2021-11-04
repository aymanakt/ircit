/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: menu.h,v 1.7 1998/03/21 09:42:29 ayman Beta $")
**
*/

 struct Item {
         char *name;
         int underline;
         int attrs;
        };
 typedef struct Item Item;

 struct MenuboxEntry {
         Item item;
         void (*right_func)(void *);
         void (*left_func)(void *);
         void (*enter_func)(void *, int);

         void *smenu;
        };
 typedef struct MenuboxEntry MenuboxEntry;

 
 struct MenuItem {
         char *caption;
         int underline,
             c_hilited,
             caption_start;
         BoxGeometry *geometry;
         MenuboxEntry *entries;
        };
 typedef struct MenuItem MenuItem;

 struct Menu {
         int startx,
             starty;
         int cols;
         int nItems;
         int *attrs;
         int c_hilited;

         MenuItem *items;
        
         void (*down_func)(struct Menu *);
         void (*enter_func)(struct Menu *);
        };
 typedef struct Menu Menu;

 /* ------------------------------------------------------------------- */


 extern const ObjectsAndColors *const _mbar;
 extern const ObjectsAndColors *const _mbox;
 extern const ObjectsAndColors *const _mborder;

 void LoadMenuBar (Menu *); 
 void LoadMenuBox (MenuItem *); 
 void LoadSubMenuBox (MenuItem *);

 void ClearMenubox (void);
 void MenuBoxTerminator (void *);
 void SubMenuBoxLeft (MenuItem *);

 int GetMenuInput (int);

 int GetMenuBoxInput (int);

 void BlowMenu (Menu *);
 void BlowupMenuBoxes (void);
 void MenuBarCleanup (void *);

 void DeleteMenuSystem (Menu *);

#if 0
 int intrin_menubar_define (struct fplArgument *);
 int intrin_menuentry_set (struct fplArgument *);

 int intrin_menubarbutton_add (struct fplArgument *);
 int intrin_menubaritem_add (struct fplArgument *);
 int intrin_menubarentry_set (struct fplArgument *);

 int intrin_menuboxitem_add (struct fplArgument *);
 int intrin_menuboxbutton_add (struct fplArgument *);
 int intrin_menuboxsep_add  (struct fplArgument *);
 int intrin_menuentryselection_get (struct fplArgument *);
#endif

