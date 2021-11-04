/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: colors.h,v 1.1 1998/03/12 12:33:47 ayman Beta $")
**
*/

 struct ColorMap {
        int cx, 
            cy,
            cattr;

        int sattr;
        void *ptr;
       };
 typedef struct ColorMap ColorMap;


#define CLR_MENU       _mbar->color
#define CLR_MENUINV    _mbar->inv
#define CLR_MENUBOX    _mbox->color
#define CLR_MENUBOXINV _mbox->inv
#define CLR_MENUBORDER _mborder->color

#define CLR_LIST           _list->color
#define CLR_LISTINV        _list->inv
#define CLR_LISTBORDER     _listborder->color
#define CLR_LISTMENU       _listmenu->color
#define CLR_LISTMENUINV    _listmenu->inv
#define CLR_LISTMENUBORDER _listmenuborder->color

#define CLR_DIALOG       _dialog->color
#define CLR_DIALOGINV    _dialog->inv
#define CLR_DIALOGBORDER _dialogborder->color

#define CLR_BOX	      _box->color
#define CLR_BOXINV    _box->inv
#define CLR_BOXBORDER _boxborder->color

#define CLR_LOWERSTAT          _lowerstat->color
#define CLR_LOWERSTATINV       _lowerstat->inv
#define CLR_LOWERSTATBORDER    _lowerstatborder->color
#define CLR_LOWERSTATBORDERTXT _lowerstattxt->color

extern  const ObjectsAndColors *const _upperstat;
#define CLR_UPPERSTAT     _upperstat->color
#define CLR_UPPERSTATINV  _upperstat->inv

#define CLR_MAINBORDER    _mainborder->color
#define CLR_MAINBORDERTXT _mainbordertxt->color

#define CLR_TABLES   _tables->color


 int LocateColorDefinition (const char *, const char *, int *);
 void SetupColorScheme (void *, int);
 void ColorsListEnter (void *, int);
 void LoadColorMap (ColorMap *);
 void ColorMapEnter (ColorMap *);
 int GetColorMapInput (int);
 void SetupColorScheme (void *, int);

