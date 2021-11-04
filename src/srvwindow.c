/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>
#include <terminal.h>
#include <updates.h>
#include <list.h>
#include <scroll.h>
#include <queue.h>
#include <servers.h>
#include <lastlog.h>
#include <chanrec.h>
#include <notify.h>
#include <srvwindow.h>

 MODULEID("$Id: srvwindow.c,v 1.1 1998/04/06 06:13:44 ayman Beta $");


 static dList ServersWindow;
 dList *const srvwin=&ServersWindow;
 ServerWindow *c_srvwin;

 ServerWindow *InitNewServerWindow (dList *ptr)

 {
  ServerWindow *srvwptr;
  dListEntry *eptr;

   eptr=AddtodList (ptr);
 
   xmalloc(srvwptr, (sizeof(ServerWindow))); 
   memset (srvwptr, 0, sizeof(ServerWindow));

    /*if (!(srvwptr->w=newwin(19, 80, 1, 0)))
       {
        return (ServerWindow *)NULL;
       }*/

    if (!(srvwptr->w=newwin(17, 78, 1, 1)))
    //if (!(srvwptr->w2=newwin(17, 78, 1, 1)))
       {
        return (ServerWindow *)NULL;
       }

   //scrollok (srvwptr->w1, FALSE);
   scrollok (srvwptr->w, TRUE); 

   _InitChannelsTable (&srvwptr->channels);

   (ServerWindow *)eptr->whatever=srvwptr;
   (dListEntry *)srvwptr->link=eptr;

   return (ServerWindow *)srvwptr;

 }  /**/


 void InitServerWindows (void)

 {
  dListEntry *eptr;
  ServerWindow *srvwptr;
  extern ChannelsTable *channels;

   memset (srvwin, 0, sizeof(dList));

    if (!(srvwptr=InitNewServerWindow(srvwin)))
       {
        fprintf (stderr, "Unable to initiate Server Window.\n");

        exit (1);
       }

   //MmainWin=srvwptr->w1;
   MainWin=srvwptr->w;
   (WINDOW *)mt_ptr->c_window=MainWin;
   c_srvwin=srvwptr;

   channels=&srvwptr->channels;

 }  /**/

