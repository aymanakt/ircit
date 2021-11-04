/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: colorstab.h,v 1.1 1998/03/12 12:33:52 ayman Beta $")
**
*/

 static const char *c_black="black";
 static const char *c_red="red";
 static const char *c_green="green";
 static const char *c_blue="blue";
 static const char *c_magenta="magenta";
 static const char *c_cyan="cyan";
 static const char *c_white="white";

 static const struct Foreground t1[]=/*black*/
 {{&c_red,1}, {&c_green,2}, {&c_magenta,5}, {&c_cyan,6}, {&c_white,7}};

static const struct Foreground t2[]=/*red*/
 {{&c_black,8}, {&c_green,10}, {&c_blue,12}, {&c_cyan,14}, {&c_white,15}};

static const struct Foreground t3[]= /*green*/
  {{&c_black,16}, {&c_red,17}, {&c_blue,20}, {&c_magenta,21}, {&c_white,23}};

static const struct Foreground t4[]= /*blue*/
  {{&c_red,33}, {&c_green,34}, {&c_magenta,37}, {&c_cyan,38}, {&c_white,39}};

static const struct Foreground t5[]= /*magenta*/
  {{&c_black,40}, {&c_green,42}, {&c_blue,44}, {&c_cyan,46}, {&c_white,47}};

static const struct Foreground t6[]= /*cyan*/
  {{&c_black,48}, {&c_red,49}, {&c_green,50}, {&c_blue,52}, {&c_magenta,53}};

static const struct Foreground t7[]= /*white*/
  {{&c_black,56}, {&c_red,57}, {&c_green,58}, {&c_blue,60}, {&c_magenta,61}};

static const struct ColorTable ColorsTable[]=
 {
  {&c_black,   t1, 4 },
  {&c_red,     t2, 13},
  {&c_green,   t3, 21},
  {&c_blue,    t4, 32},
  {&c_magenta, t5, 41}, 
  {&c_cyan,    t6, 55},
  {&c_white,   t7, 62}
 };

 ObjectsAndColors usr_tab[OBJECTS_CNT];

 ObjectsAndColors *const obj_ptr=usr_tab;

 const ObjectsAndColors *const _mbar=&usr_tab[0];
 const ObjectsAndColors *const _mbox=&usr_tab[1];
 const ObjectsAndColors *const _mborder=&usr_tab[2];
 
 const ObjectsAndColors *const _list=&usr_tab[3];
 const ObjectsAndColors *const _listborder=&usr_tab[4];
 const ObjectsAndColors *const _listmenu=&usr_tab[5];
 const ObjectsAndColors *const _listmenuborder=&usr_tab[6];

 const ObjectsAndColors *const _dialog=&usr_tab[7];
 const ObjectsAndColors *const _dialogborder=&usr_tab[8];

 const ObjectsAndColors *const _box=&usr_tab[9];
 const ObjectsAndColors *const _boxborder=&usr_tab[10];

 const ObjectsAndColors *const _lowestat=&usr_tab[11];
 const ObjectsAndColors *const _statborder=&usr_tab[12];
 const ObjectsAndColors *const _lowerstattxt=&usr_tab[13];

 const ObjectsAndColors *const _upperstat=&usr_tab[14];

 const ObjectsAndColors *const _mainborder=&usr_tab[15];
 const ObjectsAndColors *const _mainbordertxt=&usr_tab[16];

 const ObjectsAndColors *const _tables=&usr_tab[17]; 

