/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>
#include <queue.h>

 MODULEID("$Id: queue.c,v 1.5 1998/02/15 01:49:16 ayman Beta $")

 QueueEntry *AddQueue (Queue *ptr)

 {
  register QueueEntry *qptr;

   qptr=(QueueEntry *)malloc (sizeof(QueueEntry));

    if (qptr==NULL)
       {
        return (QueueEntry *)NULL;
       }

   memset (qptr, 0, sizeof(QueueEntry));

    if (ptr->nEntries)
       {
        ptr->rear->next=qptr;
        ptr->rear=qptr;
       }
    else
       {
        ptr->front=ptr->rear=qptr;
       }

   ptr->nEntries++;

   return (QueueEntry *)qptr;

 }  /**/


 QueueEntry *deQueue (Queue *ptr)

 {
  QueueEntry *qptr;

   qptr=ptr->front;
   ptr->front=ptr->front->next;

   ptr->nEntries--;

   return qptr;

 }  /**/


 boolean QueueEmpty (const Queue *ptr)

 {
   return ((boolean) (ptr->nEntries==0));

 }  /**/

