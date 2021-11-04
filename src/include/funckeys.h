/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: funckeys.h,v 1.1 1998/02/17 13:18:48 ayman Beta $")
**
*/

 extern void F1 (void);
 extern void F2 (void);
 extern void F3 (void);
 extern void F4 (void);

 extern void F5 (void);
 extern void F6 (void);
 extern void F7 (void);
 extern void F8 (void);
 
 struct CtrlKeys {
         unsigned int key;
         void (*func)(chtype);
        };

 typedef struct CtrlKeys CtrlKeys;


 struct FunctionKeys {
         const char *name;
         char *desc,
              *fplexpr;
         /*void (*func)(void);*/
        };

 typedef struct FunctionKeys FunctionKeys;


 /* default is 23. You have to manually add entries for MAX_F_KEYS>22 */
 FunctionKeys FuncKeys[MAX_F_KEYS]=
 {
  {"F0", NULL, NULL},
  {"F1", NULL, NULL}, //265
  {"F2", NULL, NULL},
  {"F3", NULL, NULL},
  {"F4", NULL, NULL},
  {"F5", NULL, NULL},
  {"F6", NULL, NULL},
  {"F7", NULL, NULL},
  {"F8", NULL, NULL},
  {"F9", NULL, NULL},
  {"F10", NULL, NULL},
  {"F11", NULL, NULL},
  {"F12", NULL, NULL},
  {"F13", NULL, NULL},
  {"F14", NULL, NULL},
  {"F15", NULL, NULL},
  {"F16", NULL, NULL},
  {"F17", NULL, NULL},
  {"F18", NULL, NULL},
  {"F19", NULL, NULL},
  {"F20", NULL, NULL},
  {"F21", NULL, NULL},
  {"F22", NULL, NULL}
#if 0
  {NULL},
  {F1}, 
  {F2},
  {F3},
  {F4},
  {F5},
  {NULL},
  {NULL},
  {NULL},
  {NULL},
  {NULL},
  {NULL},
  {NULL}
#endif
 };

