/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: stack.h,v 1.6 1998/04/08 08:54:34 ayman Beta $")
**
*/

 struct StackEntry {
                    void *whatever;
                    struct StackEntry *next;
                   };
 typedef struct StackEntry StackEntry;

 struct Stack {
               int nEntries;
               StackEntry *top;
              };
 typedef struct Stack Stack;

 #define TOP(x) ((x)->top)

 StackEntry *PushStack (Stack *);
 StackEntry *PopStack (Stack *);
 StackEntry *DuplicateStack (Stack *, Stack *);
 StackEntry *TopoftheStack (Stack *);

//#include <windows.h>

