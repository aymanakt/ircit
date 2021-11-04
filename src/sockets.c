/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>
#include <terminal.h>
#include <output.h>
#include <sockets.h>
#include <windows-aux.h>

 MODULEID("$Id: sockets.c,v 1.10 1998/04/06 06:13:44 ayman Beta $");

 static void FinishoffQueue (Socket *);
 void uSOCKETS (char *);

 static SocketsTable SocketsMaster;
 SocketsTable *const sockets=&SocketsMaster;
 Socket *so_hash[]={[0 ... MAXSOCKSCOUNT]=(Socket *)NULL};
 
 extern char buf_q[];


 void InitSocketsTable (void)
 
 {
  SocketsMaster.head=NULL;
  SocketsMaster.tail=NULL;
  SocketsMaster.nEntries=0;
  SocketsMaster.glock=0;

 } /**/


 #include <signal.h>
 
 Socket *AddtoSocketsTable (SocketsTable *ptr, int socket)
 
 {
  register Socket *sptr;
  extern char *valueof (const char *);
  extern void UpdateSockets (int, const char);

    if (SocketsTableFull(ptr))
     {
      return (Socket *)NULL;
     }

   xmalloc(sptr, (sizeof(Socket)));
   memset (sptr, 0, sizeof(Socket));

   sptr->sock=socket;
   sptr->block_sz=atoi((char *)valueof("BLOCK_SIZE"));
   
    if (!SocketsTableEmpty(ptr))
     {
      ptr->tail->next=sptr;
      ptr->tail=sptr;
     }
    else
     {
      ptr->head=ptr->tail=sptr; 
     }

   ptr->nEntries++;
   so_hash[sptr->sock]=(Socket *)sptr;
   UpdateSockets (SOCK_SOCKSCOUNT, '+');

    if (((ptr->glock>>0)&0x1)&&((ptr->glock>>1)&0x1))
     {
      RefreshListContents (ptr);
     }

   return (Socket *)sptr;

 }  /**/
 

 static __inline__ void FinishoffQueue (Socket *sptr)

 {
    if (QUEUE_NOT_EMPTY(sptr))
     {
      register SocketQueue *qptr;

        while (sptr->nMsg--)
         {
          qptr=sptr->front;
          sptr->front=sptr->front->next;

           if (qptr->msg)
            {
             free (qptr->msg);
            }

          free (qptr);
         }
     }

 }  /**/


 int RemovefromSocketsTable (SocketsTable *ptr, int socket)

 {
  int found=0;
  register Socket *sptr=ptr->head, 
                  *prev=(Socket *)NULL;
  extern void UpdateSockets (int, const char);


    while ((sptr!=NULL)&&(!found))
     {
      if (sptr->sock==socket)
       {
        found=1;
       }
      else
       {
        prev=sptr;
        sptr=sptr->next;
       }
     }

    if (!found)  return 0;

    if (prev==NULL) 
     {
      ptr->head=sptr->next;
     }
    else
    if (sptr->next==NULL) 
     {
      ptr->tail=prev;
      ptr->tail->next=NULL;
     }
    else
     {
      prev->next=sptr->next;
     }

   ptr->nEntries--;

    if ((sptr->flag&CONNECTEDTELNET)||
        (sptr->type==SOCK_CONNECTEDDCC)||
        (sptr->type==SOCK_OTHER))
     {
      UpdateSockets (sptr->type, '-');
     }
   UpdateSockets (SOCK_SOCKSCOUNT, '-');

   so_hash[sptr->sock]=(Socket *)NULL;

   FinishoffQueue (sptr);

   memset (sptr, 0, sizeof(Socket));
   free (sptr);

    if (((ptr->glock>>0)&0x1)&&((ptr->glock>>1)&0x1))
     {
      if (ptr->nEntries==0)  
       {
        DestroyWindow (4, ptr);
       }
      else
       {
        RefreshListContents (ptr);
       } 
     }

   return 1;

 }  /**/


 void ConnectionLost (int type)

 {
#if 0
  register Socket *ptr;

   switch (type)
          {
           case SOCK_IRCSERVER:
                if (set("AUTO_RECONNECT"))
                   {
                    static int i;

                      if (!i)
                         {
                          

                      say ("%$% Attempting reconnection...\n", INFO);
                      i=Reconnect (NULL, 1|MODE1);
                       if (i<=0)
                          {
                           say ("%$% Servers list exhausted.\n", MILD);
                          }
                      break;
                   }
          }
#endif
 }  /**/


 void setsockflag (int socket, int flag, boolean bit_ON)

 {
  register Socket *sptr;
  extern Socket *so_hash[];

   sptr=so_hash[socket];

    if (sptr)
     {
      if (bit_ON)
       {
        sptr->flag|=flag;
       }
      else
       {
        sptr->flag&=~flag;
       }
     }
 
 }  /**/


 void TellSocketType (const Socket *sptr, char *buf)

 {
    if ((!buf)||(!sptr))  return;

   switch (sptr->type)
    {
     case SOCK_IRCSERVER:
          strcpy (buf, "IRC");
          break;

     case SOCK_LISTENINGTELNET:
          strcpy (buf, "L-TEL");
          break;

     case SOCK_CONNECTEDTELNET:
          strcpy (buf, "C-TEL");
          break;

     case SOCK_LISTENINGDCC:
          strcpy (buf, "L-DCC");
          break;

     case SOCK_CONNECTEDDCC :
          strcpy (buf, "C-DCC");
          break;

     case SOCK_IPC:
          strcpy (buf, "IPC");
          break;

     case SOCK_IPCCLIENT:
          strcpy (buf, "IPC-C");
          break;

     case SOCK_IRCIT:
          strcpy (buf, "IRCIT");
          break;

     case SOCK_OTHER:
          strcpy (buf, "OTHER");
          break;
    }

 }  /**/


 #define _HOURGLASS  0xE8|A_ALTCHARSET|COLOR_PAIR(CYAN)|A_BLINK
 #define BULLET     0xAE|A_ALTCHARSET|COLOR_PAIR(RED)|A_BLINK|A_BOLD

 #define TYPES \
  if ((ptr->type==SOCK_LISTENINGTELNET) || \
      (ptr->type==SOCK_LISTENINGDCC) || \
      (ptr->type==SOCK_IPC)) i=1
        
 #define SO_STAT \
  (ptr->flag&FIRING_SQUAD)? \
   (BULLET): \
   ((ptr->flag&(AWAITINGUSERNAME|AWAITINGCONNECTION)||i==1) ? \
    (_HOURGLASS):(' '))

 #define SO_ADDR \
  (((ptr->type==2)||(ptr->type==4))?("Not Applicable"):(ptr->haddress))


 #define SO_FORMAT \
  " %$179%-2.2d%$179%-5s %$%%$179%-5s%$179%-5d%$179%-20.20s%$179%-5d%$179%-3d%$179%-4u%$179%-4u%$179\n"

 #define SO_ARGS \
  ptr->sock, t, SO_STAT, s, ptr->port, SO_ADDR, ptr->hport, ptr->nMsg, \
  (ptr->rbytes>1024)?(ptr->rbytes/1024):(ptr->rbytes), \
  (ptr->sbytes>1024)?(ptr->sbytes/1024):(ptr->sbytes)

 void ShowSocketsTable (void)

 {
  int i=0;
  char t[10],
       s[10];
  register Socket *ptr=sockets->head;

    if (!sockets->nEntries)
     {
      say ("%$% Not a single open socket.\n", INFO);

      return;
     }

   say ("%$% Total of %d open socket%c\n", INFO, sockets->nEntries,
         sockets->nEntries>1?'s':' ');

   DisplayHeader (1, 9);

    for ( ; ptr!=NULL; ptr=ptr->next)
     {
      strftime (s, 10, "%H:%M", localtime(&ptr->when));

      TellSocketType (ptr, t);
      TYPES;

      say (SO_FORMAT, SO_ARGS);
 
      i=0; 

       if (!ptr->next)
        {
         finish2(1, 9);
        }
       else
        {
         seperator(1, 9);
        }
     }

 }  /**/


 void TellSocketInfo (const Socket *ptr)

 {
  int i=0;
  char t[10],
       s[10];

    if (!ptr)  return;

   DisplayHeader (1, 9);

   strftime (s, 10, "%H:%M", localtime(&ptr->when));

   TellSocketType (ptr, t);
   TYPES;

   say (SO_FORMAT, SO_ARGS);

   finish2(1, 9);

 }  /**/


 Socket *SocketByName (int socket)

 {
  register int found=0;
  register Socket *sptr=sockets->head;

    while ((sptr!=NULL)&&(!found))
     {
      if (sptr->sock==socket)
       {
        found=1;
       }
      else
       {
        sptr=sptr->next;
       }
     }

    if (!found)  return (Socket *)NULL;

   return (Socket *)sptr;

 }  /**/


 Socket *Socketbyaddress (const char *address, int port)

 {
  register Socket *sptr=sockets->head;

    for (; sptr!=NULL; sptr=sptr->next)
     {
      if ((!strcasecmp(sptr->haddress, address))&&(sptr->hport==port))
       {
        return (Socket *)sptr;
       }
     }

   return (Socket *)NULL;

 }  /**/


 #define RELEVANT ((sptr->type==SOCK_CONNECTEDTELNET)&& \
                   (sptr->flag&CONNECTEDTELNET)&& \
                   ((sptr->flag&FIRING_SQUAD)==0))

 int GetTelnetSockets (int *sockets_)

 {
  register int i=0;
           int *h;
  register Socket *sptr=sockets->head;

   h=sockets_;

    for ( ; sptr!=NULL; sptr=sptr->next)
     {
      if (RELEVANT)
       {
        *(h+i++)=sptr->sock;
       }
     }
   
   return i;

 }  /**/


 boolean SocketsTableEmpty (const SocketsTable *ptr)

 {
   return ((boolean)(ptr->nEntries==NO_SOCKETS));

 }  /**/                  


 boolean SocketsTableFull (const SocketsTable *ptr)

 {
   return ((boolean)(ptr->nEntries==MAXSOCKSCOUNT));

 }  /**/


 boolean SocketQueueEmpty (Socket *ptr)

 {
   return ((boolean)(ptr->nMsg==QUEUE_EMPTY));

 } /**/


 #include <list.h>
 #include <scroll.h>

 void ScrollingSocketsList (SocketsTable *const);
 void ScrollingSocketsEntries (ScrollingList *);
 void ScrollingSocketsEnter (void *, int);


 void ScrollingSockets (void *ptr, int idx)

 {
   ScrollingSocketsList (sockets);

 }  /**/


 #include <sockets-data.h>

 void ScrollingSocketsList (SocketsTable *const ptr)

 {
  extern ScrollingList so_list;
  ScrollingList *scrptr=&so_list;

    if (ptr->nEntries==0)
     {
      say ("%$% Not a single open connection, not even listening.\n", INFO);

      return;
     }

   ((ScrollingList *)(scrptr->opt_menu))->parent=scrptr;

   scrptr->nEntries=ptr->nEntries;
   (SocketsTable *)scrptr->list=ptr;

   so_list.lock=&ptr->glock; /* reset it to 0 when done with list! */

   LoadScrollingList (&so_list);

 }  /**/



 #define DETERMINE_TYPE() \
         if ((IS_LISTENINGTELNET(s_ptr))||(IS_CONNECTEDTELNET(s_ptr))) \
            sprintf (s, "%-6s ", "Telnet"); \
         else \
         if (IS_IRCSERVER(s_ptr)) \
            sprintf (s, "%-6s ", "IRC"); \
         else \
         if ((IS_LISTENINGDCC(s_ptr))||(IS_CONNECTEDDCC(s_ptr))) \
            sprintf (s, "%-6s ", "DCC"); \
         /*else \
         if (IS_FAULTY(s_ptr)) \
            sprintf (s, "%-6s ", "FAULTY"); \
         else \
         if (IS_FIRINGSQUAD(s_ptr)) \
            sprintf (s, "%-6s ", "FIRE");*/ \
         else \
         if (IS_IRCIT(s_ptr)) \
            sprintf (s, "%-6s ", "IRCIT"); \
         else \
         if (IS_OTHER(s_ptr)) \
            sprintf (s, "%-6s ", "OTHER"); \
         else \
         if (IS_EGGDROP(s_ptr)) \
            sprintf (s, "%-6s ", "EGGDRP"); \
         else \
            sprintf (s, "%-6s ", "HMMM..")

 #define IS_LISTENING() (IS_LISTENINGDCC(s_ptr)||IS_LISTENINGTELNET(s_ptr))
 #define ORA 0xE8|A_ALTCHARSET|A_BLINK

 void ScrollingSocketsEntries (ScrollingList *sptr)

 {
  register int j=0;
  int maxy,
      last;
  char s[sptr->width];
  register Socket *s_ptr;
  WINDOW *w=RetrieveCurrentWindow ();

   getmaxyx (w, maxy, last);

   last=maxy+sptr->listwin_start;

    for (s_ptr=((SocketsTable *)sptr->list)->head; 
         (s_ptr!=NULL)&&(j++!=sptr->listwin_start); 
         s_ptr=s_ptr->next)
        ;

    for (--j; 
         (j<last)&&(j<((SocketsTable *)sptr->list)->nEntries)&&(s_ptr!=NULL);
         j++, s_ptr=s_ptr->next)
     {
       if (j==sptr->hilited)
        {
         _wattron (w, A_REVERSE);
         (Socket *)sptr->c_item=(Socket *)s_ptr;
        }

      _wmove (w, j-sptr->listwin_start, 0);
      DETERMINE_TYPE();
      Fwprintf (w, " %-3d%$179%s%$%%$179%-25.25s%$179%-5d ", 
                s_ptr->sock, s, 
                IS_LISTENING()?ORA:' ', 
                s_ptr->haddress, s_ptr->hport);

       if (j==sptr->hilited)
        {
         _wattroff (w, A_REVERSE);
        }
     }

    for ( ; j<last; ++j) 
     {
      _wmove (w, j-sptr->listwin_start, 0);
      Fwprintf (w, " - ");
      _wmove (w, j-sptr->listwin_start, 2);
      wclrtoeol (w);
     }

 }  /**/


 void ScrollingSocketsEnter (void *sptr, int idx)

 {
  char s[MEDIUMBUF];
  extern void uSHOOT (char *);

   switch (idx)
    {
     case 0:
          sprintf (s, "%d", ((Socket *)sptr)->sock);
          uSHOOT (s);
          break;

     case 1:
          TellSocketInfo ((Socket *)sptr);
          break;
    }

 }  /**/


 STD_IRCIT_COMMAND(uSOCKETS)

 {
    if ((!args)||((args)&&(arethesame(args, "-LST"))))
     {
      ShowSocketsTable ();

      return;
     }

    if (arethesame(args, "-SCRL"))
     {
      ScrollingSockets (NULL, 0);
     }
    else
     {
      say ("%$% You need -h.\n", MILD);
     }

   return;

 }  /**/


