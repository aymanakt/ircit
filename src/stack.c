/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>
#include <terminal.h>
#include <stack.h>

 MODULEID("$Id: stack.c,v 1.12 1998/04/06 06:13:44 ayman Beta $");

 StackEntry *PushStack (Stack *stk_ptr)

 {
  const size_t sizeofStackEntry=sizeof(StackEntry);
  register StackEntry *kptr;

   xmalloc(kptr, sizeofStackEntry);
   memset (kptr, 0, sizeofStackEntry);

   kptr->next=stk_ptr->top;
   stk_ptr->top=kptr;

   stk_ptr->nEntries++;

   return (StackEntry *)kptr;

 }  /**/


 StackEntry *PopStack (Stack *stk_ptr)

 {
  register StackEntry *kptr;

   kptr=stk_ptr->top;
   stk_ptr->top=stk_ptr->top->next;

   stk_ptr->nEntries--;
   
   return kptr;

 }  /**/


 StackEntry *TopoftheStack (Stack *stk_ptr)

 {
   return (stk_ptr->top);

 }  /**/


 StackEntry *DuplicateStack (Stack *s, Stack *d)

 {
  StackEntry *kptr,
             *aux;

   InitStack ();

   aux=s->top;

    for ( ; aux!=NULL; aux=aux->next)
     {
      kptr=PushStack (d);
      kptr->whatever=aux->whatever;
     }

   return (StackEntry *)d->top;

 }  /**/

