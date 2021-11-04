/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>
#include <terminal.h>
#include <output.h>
#include <list.h>
#include <scroll.h>
#include <stack.h>
#include <updates.h>
#include <sockets.h>
#include <prefs.h>

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <net.h>
#include <windows-aux.h>
#include <servers.h>

 MODULEID("$Id: servers.c,v 1.8 1998/04/06 06:13:44 ayman Beta $");

 List *servers;


 void InitServersList (void)

 {
  static List ServersList;

   ServersList.head=NULL;
   ServersList.tail=NULL;
   ServersList.nEntries=0;
   servers=&ServersList;

 }  /**/


 #define SERV_SERVERS 0x3E

 Server *AddtoServersList (List *ptr, char *server, int port)

 {
  register Server *vptr=NULL;
  register ListEntry *lptr;

   {
    int found=0;

     lptr=ptr->head;

       while ((lptr!=NULL)&&(!found))
        {
         (Server *)vptr=(Server *)lptr->whatever;

          if ((arethesame(vptr->server, server))&&(vptr->port==port))
           {
            found=1;
           }
          else
           {
            lptr=lptr->next;
           }
        }

       if (found)
        {
         return (Server *)vptr;
        }
   }

   xmalloc(vptr, (sizeof(Server)));
   memset (vptr, 0, sizeof(Server));
    
   strcpy (vptr->server, server);
   vptr->port=port;
   vptr->id=ptr->nEntries+1;

   lptr=AddtoList (ptr);

    if (lptr!=NULL)
     {
      (Server *)lptr->whatever=(Server *)vptr;
      (ListEntry *)vptr->link=(ListEntry *)lptr;

      UpdateServerTable (SERV_SERVERS, ptr->nEntries, NULL, '+');

       if (((ptr->glock>>0)&0x1)&&((ptr->glock>>1)&0x1))
        {
         RefreshListContents (ptr);
        }

      return (Server *)vptr;
     }
    else
     {
      say ("%$% Unable to add server %s:%d to Servers List: %s.\n", HOT,
           server, port, "possibly exhausted memory");

      return (Server *)NULL;
     }
        
 }  /**/


 #define vptr ((Server *)lptr->whatever)

 void ClearServersList (List *ptr, int how, ...)

 {
  register ListEntry *lptr;

    if (ptr->nEntries==0)
     {
      say ("%$% Servers List empty.\n", MILD);

      return;
     }

   switch (how)
          {
           case NUMBER:
                {
                 int i;
                 register int n=0;
                 va_list va;

                  va_start (va, how);
                  i=va_arg (va, int);
                  va_end (va);

                   if (i>ptr->nEntries)
                      {
                       say ("%$% You don't have that many servers.\n", MILD);

                       return;
                      }

                  lptr=ptr->head;

                   for ( ; lptr!=NULL; lptr=lptr->next)
                       {
                        n++;
                         if (n==i)
                            {
                             memset (vptr, 0, sizeof(Server));
                             free (vptr);
                             RemovefromList (ptr, lptr);
                             UpdateServerTable (SERV_SERVERS, ptr->nEntries, NULL, '-');
                             return;
                            }
                       }
                 }
                break;

           case NAME:
                {
                 char *srv;
                 unsigned long port;
                 va_list va;

                  va_start (va, how);
                  srv=va_arg (va, char *);
                  port=va_arg (va, unsigned long);
                  va_end (va);

                  lptr=ptr->head;

                   for ( ; lptr!=NULL; lptr=lptr->next)
                       {
                         if ((arethesame(srv,vptr->server))&&(port==vptr->port))
                            {
                             memset (vptr, 0, sizeof(Server));
                             free (vptr);
                             RemovefromList (ptr, lptr);
                             UpdateServerTable (SERV_SERVERS, ptr->nEntries, NULL, '-');

                             return;
                            }
                       }

                  say ("%$% %s isn't in your Servers List.\n", MILD, srv); 
                }
               break;

           case ALL:
                {
                 int i=0;
                 register ListEntry *aux;

                  lptr=ptr->head; //

                   while (lptr!=NULL)
                         {
                          i++;
                          aux=lptr->next;
                          memset (vptr, 0, sizeof(Server));
                          free (vptr);
                          memset (lptr, 0, sizeof(ListEntry));
                          free (lptr);
                          lptr=aux;
                         }

                  ptr->nEntries=0;
                  ptr->head=ptr->tail=NULL;

                  UpdateServerTable (SERV_SERVERS, ptr->nEntries, NULL, '-');
                 }

                break;
          }


 }  /**/


 Server *_GetServerfromList (List *ptr, int how, ...)

 {
    if (ptr->nEntries==0)
     {
      return (Server *)NULL;
     }

   switch (how)
          {
           case NUMBER:
                {
                 int i;
                 register int n=0;
                 va_list va;
                 register ListEntry *lptr;

                  va_start (va, how);
                  i=va_arg (va, int);
                  va_end (va);

                   if (i>ptr->nEntries)
                    {
                     say ("%$% You don't have that many entries.\n", MILD);

                     return (Server *)NULL;
                    }

                  lptr=ptr->head;

                   for ( ; lptr!=NULL; lptr=lptr->next)
                    {
                     if (++n==i)
                      {
                       return (Server *)vptr;
                      }
                    }

                  return (Server *)NULL;
                }
 
           case NAME:
                {
                 unsigned long port;
                 char *srv;
                 va_list va;
                 register ListEntry *lptr;

                  va_start (va, how);
                  srv=va_arg (va, char *);
                  port=va_arg (va, unsigned long);
                  va_end (va);

                  lptr=ptr->head;

                   for ( ; lptr!=NULL; lptr=lptr->next)
                    {
                     if ((arethesame(vptr->server,srv))&&(vptr->port==port))
                      {
                       return (Server *)vptr;
                      }
                    } 

                  return (Server *)NULL;
                }
          }

 }  /**/

 #undef vptr


 static char *UpdateServerEntry (char *, char **, unsigned);

 int _ConnectToServer (char *server, unsigned long port, int type)

 {
  int len,
      nsocket;
  register Socket *sptr;
  ResolvedAddress raddr;
  struct in_addr *saddr;
  struct sockaddr_in me;
  extern SocketsTable *const sockets;
  extern struct resolved *const r_ptr;

   memset (&raddr, 0, sizeof(ResolvedAddress));

   say ("%$% Resolving server %s...\n", INFO, server);

    if (!(ResolveAddress(server, &raddr)))  return 0;

   say ("%$% Negotiating connection to %s (%s) on port %d...\n", INFO,
         raddr.dns, raddr.dotted, port);

    if (!arethesame(server, raddr.dns))
     {
      UpdateServerEntry (raddr.dns, &server, port);
     }

    if (!(nsocket=Connect(raddr.inetaddr, port)))
     {
      return 0;
     }

    if (getsockname(abs(nsocket),(struct sockaddr*)&me,(len=sizeof(me),&len))<0)
     {
      say ("%$% 'getsockname' failed on socket %d\n", HOT, nsocket);

      close (abs(nsocket));

      return 0;
     }

    if (!(sptr=AddtoSocketsTable(sockets, abs(nsocket))))
     {
      say ("%$% Unable to complete connection to server %s - internal.\n",
            HOT, raddr.dns);

      close (abs(nsocket));
       
      return 0;
     }

   sptr->when=time (NULL);
   sptr->hport=port;
   sptr->port=ntohs (me.sin_port);
   sptr->sock=abs(nsocket);
   sptr->type=type;
   strcpy (sptr->haddress, server);
   strcpy (sptr->address, "localhost");
  
    if (nsocket<0)  sptr->flag|=BLOCKING;  /* in fact non-blocking...*/

   UpdateSockets (sptr->type, '+');

    if (type==SOCK_IRCSERVER)
     {
      sptr->read_func=IRC_ServerRead;
      sptr->close_func=IRCSERVER_Close;
      sptr->init_func=IRC_ServerInit;
     }
    else
    if (type==SOCK_OTHER)
     {
      sptr->read_func=OTHER_Read;
      sptr->close_func=OTHER_Close;
     }

   return nsocket;

 }  /**/


 int HowmanyServers (void)

 {
   return servers->nEntries;

 }  /**/


int serverid (char *server, unsigned long port)

 {
  register int n=0;
  register ListEntry *lptr=servers->head;
  #define vptr ((Server *)lptr->whatever)

    while (lptr!=NULL)
     {
      n++;
       if ((arethesame(vptr->server, server))&&(vptr->port==port))
        {
         return n;
        }
       else
        {
         lptr=lptr->next;
        }
     }

   return 0;

 }  /**/

 #undef vptr


 char *UpdateServerEntry (char *r, char **c, unsigned p)

 {
  register ListEntry *eptr;
  register Server *vptr;

    if (vptr=_GetServerfromList(servers, NAME, *c, p))
     {
       if (_GetServerfromList(servers, NAME, r, p))
        {
         ClearServersList (servers, NAME, vptr->server, vptr->port);

         say ("%$% Redundant server entry removed...\n", INFO);
        }
       else
        {
         mstrncpy (vptr->server, r, MAXHOSTLEN);

         say ("%$% Server entry updated...\n", INFO);
        }

      return (*c=r);
     }

   return "";

 }  /**/


 /*
 ** rewrite!
 */
 int IRCServerConnect (char *server, unsigned long port)

 {
  int j;
  register Socket *sptr;
  extern SocketsTable *const sockets;

   sptr=Socketbyaddress (server, port);

    if (!sptr)
     {
      j=_ConnectToServer (server, port, SOCK_IRCSERVER);

       if (j)
        {
         if (j>0)
          {
            if (isstatus(CONNECTED)||isstatus(TRYING))
             {
              say ("%$% Disconnecting from current server...\n",INFO);

               if (isstatus(CONNECTED))
                {
                 WriteToSocket(isup(), "QUIT :Disassociating...\n");
                }

              close (abs(isup()));
              RemovefromSocketsTable (sockets, isup());

              ServerUp (server, port, j);

              return 1;
             }
          }
         else
          {
            if (isstatus(CONNECTED))  
             {
              setsockflag (isup(), FIRING_SQUAD, 1);
              setstatus (CONNECTED, 0);
              st_ptr->c_serversocket=-(isup());
              setstatus (TRYING, 1);
             }
          }

        say ("%$% Waiting for response from server...\n",INFO);

        return 1;
       }
      else
       {
        say ("%$% Connection to server %s resumed...\n", INFO,
             curserver());

        return 0;
       }
     }
    else
     {
      if (sptr->flag&BLOCKING)
       {
        say ("%$% Connection to IRC server %s in progress...\n", MILD,
             sptr->haddress);

        return 0;
       }
      else
       {
        say ("%$% You are already connected to IRC server %s on port %u.\n",
             MILD, sptr->haddress, sptr->hport);

        return 0;
       }
     }

 }  /**/


 void PostConnection (void)

 {
  char *n=NULL,
       *u=NULL,
       *r=NULL;

    if (!*st_ptr->c_nick)
     {
       if (!(n=valueof("DEFAULT_NICK")))
        {
         if (!(n=valueof("ALT_NICK")))
          {
           if (!(n=ut_ptr->login))
            {
             n="_IDI-OTA_";
            }
          }
        }

      mstrncpy (st_ptr->c_nick, n, MAXNICKLEN);
     }

   u=(char *)valueof ("USERNAME");
   r=(char *)valueof ("REALNAME");

   ToServer ("NICK %s\n" "USER %s * * :%s\n",
             st_ptr->c_nick, u?u:ut_ptr->login,
             r?r:"IRCIT - Not for the faint-hearted!");
   
 }  /**/


#define S_UP 17|A_ALTCHARSET|A_BLINK|COLOR_PAIR(CYAN)

 static void ShowServersEntry (const ListEntry *, int);


 static __inline__ void ShowServersEntry (const ListEntry *eptr, int i) 

 {
  char t[10];
  #define vptr ((Server *)eptr->whatever)

    if (vptr->when)  strftime (t, 10, "%H:%M", localtime(&vptr->when));
    else  strcpy (t, "--:--");

   say (" %$179%-2d%$179%-30.30s %$% %$179%-5.5d%$179%-5.5s%$179\n",
        i, vptr->server,
        (isup()&&iscurserver(vptr->server)&&iscurport(vptr->port))?S_UP:' ',
        vptr->port, t);


 }  /**/


 void ShowServersList (const List *lptr, int x, int y)

 {
  register int i=0;
  register ListEntry *eptr=lptr->head;

    if (lptr->nEntries==0)
     {
      say ("%$% Servers List empty - add some...\n", MILD);

      return;
     }

    while ((++i)<=x)
     {
      eptr=eptr->next;
     }
   i--;

   DisplayHeader (3, 4);

    for ( ; eptr!=NULL, ((++i)<=(y)); eptr=eptr->next)
     {
      ShowServersEntry (eptr, i);
     }

   finish2 (3, 4);

 }  /**/

 #undef vptr


 STD_IRCIT_COMMAND(uSERVER)

 {
  int add=1;
  register int j=0;
  char *server;
  extern char tok[][80];
  extern void uIRCIT (char *);

    if (!args)
     {
      ShowServersList (servers, (1-1), servers->nEntries);

      return;
     }

   splitargs2 (args, ' ');

    if ((*tok[j]=='+')||(*tok[j]=='-'))
     {
      {
       unsigned long port=0L;

         if (*tok[j]=='-')
          {
           add=0;
     
           {
            if ((arethesame(tok[j], "-SCRL"))&&(!*tok[j+1]))
             {
              ScrollingServers (servers, 0);
              return;
             }
            else
            if ((arethesame(tok[j], "-LST"))&&(!*tok[j+1]))
             {
              ShowServersList (servers, (1-1), servers->nEntries);
              return;
             } 
           }

            if (*(tok[j]+1)=='#')
             {
              unsigned n=atoul(tok[j]+2);

                if (!n)
                 {
                  say ("%$% You really need -h.\n");

                  return;
                 }

               ClearServersList (servers, NUMBER, n);

               return;
             }
          }

        server=tok[j++];
 
        (!*tok[j])||((*tok[j])&&(!(port=atoul(tok[j]))))?
         (port=atoul(valueof("DEFAULT_SERVER_PORT"))):1;
               
        switch (add)
         {
          case 1:
               AddtoServersList (servers, server+1, port);
               break;

          case 0:
               ClearServersList (servers, NAME, server+1, port);
               break;
         } 
      }
     }
    else
     {
      char s[strlen(args)+1];

       sprintf (s, "-IRC %s", args);

       uIRCIT (s);
     }

 }  /**/


 void ScrollingServersList (List *const);
 void ScrollingServersEntries (ScrollingList *);
 void ScrollingServersInit (ScrollingList *);
 void ScrollingServersEnter (void *, int);


 void ScrollingServers (void *ptr, int idx)

 {
   ScrollingServersList (servers);

 }  /**/


 #include <servers-data.h>

 void DialogNewServerEnter (DialogBox *);


 void ScrollingServersList (List *ptr)

 {
  ScrollingList *scrptr=&srv_list;
          
   if (ptr->nEntries==0)
    {
     say ("%$% Servers List empty - add some...\n", MILD);

     return;
    }

   ((ScrollingList *)(scrptr->opt_menu))->parent=scrptr;

   scrptr->nEntries=ptr->nEntries;
   (List *)scrptr->list=ptr;

   srv_list.lock=&ptr->glock;

   LoadScrollingList (&srv_list);

 }  /**/


 void ScrollingServersEntries (ScrollingList *sptr)

 {
  register int j=0;
  int maxy, last;
  char t[6];
  register ListEntry *lptr;
  WINDOW *w=RetrieveCurrentWindow();
  #define vptr ((Server *)lptr->whatever)

   getmaxyx (w, maxy, last);

   last=maxy+sptr->listwin_start;

    for (lptr=((List *)sptr->list)->head; 
         (lptr!=NULL)&&(j++!=sptr->listwin_start); 
         lptr=lptr->next)
        ;

    for (--j;
         (j<last)&&(j<((List *)sptr->list)->nEntries)&&(lptr!=NULL);
         j++, lptr=lptr->next)
     {
       if (j==sptr->hilited)
        {
         _wattron (w, A_REVERSE);
         (ListEntry *)sptr->c_item=(ListEntry *)lptr;
        }

       if (vptr->when)
        {
         strftime (t, 10, "%H:%M", localtime(&vptr->when));
        }
       else
        {
         strcpy (t, "--:--");
        }

      wmove (w, j-sptr->listwin_start, 0);
      Fwprintf_nout (w, " %-20.20s%$179%-5d%$179%s %$%",
                vptr->server, vptr->port, t, 
                (iscurserver(vptr->server)&&isup()&&(iscurport(vptr->port)))?
                 (17|A_BLINK):(' '));

       if (j==sptr->hilited)
        {
         _wattroff (w, A_REVERSE);
        }
     }

    for ( ; j<last; ++j)
     {
      _wmove (w, j-sptr->listwin_start, 0);
      Fwprintf_nout (w, " - ");
      _wmove (w, j-sptr->listwin_start, 2);
      wclrtoeol (w);
     }

 }  /**/

 #undef vptr


 void ScrollingServersInit (ScrollingList *sptr)

 {
  Server *vptr=((ListEntry *)sptr->c_item)->whatever;
  register Socket *ptr;

   ptr=Socketbyaddress (vptr->server, vptr->port);

    if (ptr==(Socket *)NULL)
       {
        sptr->attrs[2]=0;
       }
    else
       {
        sptr->attrs[2]=1;
       }

 }  /**/


 #define vptr ((Server *)((ListEntry *)sptr)->whatever)

 void ScrollingServersEnter (void *sptr, int idx)

 {
  char s[MEDIUMBUF];
  extern void uIRCIT (char *);

    if (!((ListEntry *)sptr)->whatever)
     {
      return;
     }

   switch (idx)
    {
     case 0:
          sprintf (s, "-IRC %s %lu", vptr->server, vptr->port);
          uIRCIT (s);
          break;

     case 1:
          sprintf (s, "-OTHR %s %lu", vptr->server, vptr->port);
          uIRCIT (s);
          break;

     case 2:
          {
           register Socket *ptr;

            ptr=Socketbyaddress (vptr->server, vptr->port);

             if (ptr==(Socket *)NULL)
              {
               say ("%$% You are not connected to %s\n", MILD,
                    vptr->server);
              }
             else
              {
               sprintf (s, "%d", ptr->sock);
               uSHOOT (s);
              }
          }
         break;

     case 3:
          sprintf (s, "-%s %lu", vptr->server, vptr->port);
          uSERVER (s);
          break;

     case 4:
          LoadDialogBox (&dlg_srvnew);
    }

 }  /**/


 void DialogNewServerEnter (DialogBox *dptr)

 {
  register Field *fptr;
  char s[]={[0 ... LARGBUF]=0};

    if ((!dptr)||(fptr=&dptr->fptr[dptr->c_field], !fptr))  return;

    if (fptr->flags&(0x1<<4))
       {
        return;
       }

    if (!*((dptr->fptr+0)->edit.buffer))  return;  /* server omitted */

    if (!*((dptr->fptr+1)->edit.buffer))
       {
        strcpy ((dptr->fptr+1)->edit.buffer, valueof("DEFAULT_SERVER_PORT"));
       } 

    if (dptr->c_field==3)
       {
        sprintf (s, "%s %s", (dptr->fptr+0)->edit.buffer, 
                             (dptr->fptr+1)->edit.buffer);
       }
    else
       {
        sprintf (s, "+%s %s", (dptr->fptr+0)->edit.buffer,
                              (dptr->fptr+1)->edit.buffer);
       }

   uSERVER (s);

 }  /**/

