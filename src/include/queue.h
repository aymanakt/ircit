/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: queue.h,v 1.2 1998/02/20 14:58:44 ayman Beta $")
**
*/

  struct QueueEntry
                   {
                    void *whatever;
                    int len;
                    unsigned long tx[80];
                    struct QueueEntry *next;
                   };
 typedef struct QueueEntry QueueEntry;


 struct Queue
             {
              int nEntries;
              QueueEntry *front,
                          *rear;
             };
 typedef struct Queue Queue;

 QueueEntry *AddQueue (Queue *);
 QueueEntry *deQueue (Queue *);
 boolean QueueEmpty (const Queue *);

