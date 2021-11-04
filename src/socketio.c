/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
  
#include <main.h>
#include <terminal.h>
#include <output.h>
#include <list.h>
#include <lastlog.h>
#include <scroll.h>
#include <updates.h>
#include <sockets.h>
#include <people.h>
#include <dcc.h>
#include <socketio.h>

#include <sys/socket.h>
#include <fcntl.h>

 MODULEID("$Id: socketio.c,v 1.8 1998/04/06 06:13:44 ayman Beta $");

#define READING 32
const int sizeofSocketQueue=sizeof(SocketQueue);
const int sizeofSocket=sizeof(Socket);

 static __inline__ int GetRegisteredSockets (int *sockets_)

 {
  register int i=0,
               *h;
  int max=0;
  register Socket *sptr;
  extern SocketsTable *const sockets;

   sptr=sockets->head;
   h=sockets_;

    for ( ; sptr!=NULL; sptr=sptr->next)
     {
       if (sptr->flag&FIRING_SQUAD)  continue;

      *(h+i++)=sptr->sock;

       if (sptr->sock>max)  max=sptr->sock;
     }

   *(h+i)=max;

   return i;

 }  /**/


 static int AddtoQueue (Socket *sptr)

 {
  register SocketQueue *qptr;
  extern SocketsTable *const sockets;

   Flash (9, 1);

   xmalloc(qptr, (sizeof(SocketQueue)));
   memset (qptr, 0, (sizeof(SocketQueue)));

   xmalloc(qptr->msg, sptr->block_sz);

   qptr->flag=read (sptr->sock, qptr->msg, sptr->block_sz-1);

    if (qptr->flag<=0)
     {
      memset (qptr->msg, 0, strlen(qptr->msg));
      free (qptr->msg);
      memset (qptr, 0, sizeofSocketQueue);
      free (qptr);

      (*sptr->close_func)(sptr, 1);

      Flash (9, 0);

      return 0;
     }

   qptr->msg[qptr->flag]=0;

   sptr->rbytes+=qptr->flag;

    if (sptr->nMsg)
     {
      sptr->rear->next=qptr;
      sptr->rear=qptr;
     }
    else
     {
      sptr->front=sptr->rear=qptr;
     }

   sptr->nMsg++;

   Flash (9, 0);

   return 1;

 }  /**/


 #define MERGING 64
 
 static __inline__ void RemovefromSocketQueue (Socket *ptr, int *comeback)

 {
  register char *nl;
  register SocketQueue *aux;
  extern char buf_q[];

    if (ptr->nMsg==0)
     {
      *comeback=0;

      return;
     }

   nl=strchr (ptr->front->msg, '\n');

    if (nl==(char *)NULL)
     {
      nl=strchr (ptr->front->msg, '\r');
     }

    if (nl!=(char *)NULL)
     {
      *nl=0;
      strcpy (buf_q, ptr->front->msg);

       if (buf_q[strlen(buf_q)-1]=='\r')
        {
         buf_q[strlen(buf_q)-1]=0;
        }

      ptr->front->msg[0]=0;
      strcpy (ptr->front->msg, nl+1);

       if (ptr->front->msg[0]/*&&strchr(ptr->front->msg, '\n')*/)
        {
         *comeback=1;

         return;
        }
       else
        {
         aux=ptr->front;
         ptr->front=ptr->front->next;
         ptr->nMsg--;

          if (!ptr->nMsg)
           {
            ptr->rear=NULL;
           }

         free (aux->msg);
         free (aux);

         /* buf_q[0]=0; */
         *comeback=0;

         return;
        }
     }

    else 

     {
      register SocketQueue *b=ptr->front;
#define BOTH (strlen(b->msg)+strlen(b->next->msg)+2)

       Flash (10, 1);

        if (!(ptr->nMsg>1))
         {
          *comeback=0;
          buf_q[0]=0;

          Flash (10, 0);

          return;
         }

       b->msg=(char *)realloc (b->msg, BOTH);
       strcat (b->msg, b->next->msg);

       memset (ptr->front->next->msg, 0, strlen(ptr->front->next->msg));
       free (ptr->front->next->msg);

       ptr->front->next->msg=ptr->front->msg;
       aux=ptr->front;
       ptr->front=ptr->front->next;

       ptr->nMsg--;

        if (!ptr->nMsg)
         {
          ptr->rear=NULL;
         }

       free (aux);
       buf_q[0]=0;

        if (strchr(ptr->front->msg, '\n'))
         {
          *comeback=1;
         }
        else
         {
          /*if (strlen(ptr->front->msg)>511)
             {
              ptr->front->msg[511]='\n';
              *comeback=1;
             }
            else
             {*/
              *comeback=0;
           //}
         }

       Flash (10, 0);

       return;
      }

 }  /**/


 int IRC_ServerInit (Socket *sptr)

 {
   sptr->flag&=~BLOCKING;

   say ("%$% Server %s accepted connection.\n", INFO, sptr->haddress);

    if (isup()<0)  /* close old */
     {
      extern SocketsTable *const sockets;

       ToSocket (abs(isup()), "QUIT :Disassociating...\n");

       RemovefromSocketsTable (sockets, abs(isup()));
       close (abs(isup()));
     }

   ServerUp (sptr->haddress, sptr->hport, sptr->sock);

 }  /**/

 #include <history.h>

 int DCCConnetInit (Socket *sptr)

 {
  #define iptr ((Index *)sptr->index)
  #define dcc  ((DCCParams *)iptr->pindex)
  #define pptr ((Someone *)iptr->sindex)
 
    if ((dcc->flags&AWAITINGCONNECTION)&&(sptr->flag&BLOCKING))
     {
      sptr->flag&=~BLOCKING;
      dcc->flags&=~AWAITINGCONNECTION;
      dcc->flags|=CONNECTED;

       if (IS_GET(dcc))
        {
         say ("%$% 6DCC GET %s from %s started.\n", CTCP,
              dcc->file, pptr->name);
        }
       else
       if (IS_CHATTOUS(dcc))
        {
         char s[LARGBUF];
         extern InputHistory *tab;

          say ("%$% 6DCC CHAT connection to %s established.\n", 
               CTCP, pptr->name);

          sprintf (s, "/DMSG %s ", pptr->name);
          AddtoUserInputHistory (tab, s, 0);
        }
     }

 }  /**/

 #undef iptr
 #undef dcc
 #undef pptr


 int IRC_ServerRead (Socket *sptr)

 {
  int comeback=0;
  extern char buf_q[];

   //st_ptr->c_serverlag=time (NULL);

    if (AddtoQueue(sptr))
     {
      do
       {
        RemovefromSocketQueue (sptr, &comeback);

         if (buf_q[0])
          {
           ParseServerMsg ();
          }
       }
      while (*&comeback)
            ; 
     }

 }  /**/


 int OTHER_Read (Socket *sptr)

 {
  int comeback=0;
  extern char buf_q[];

    if (AddtoQueue(sptr))
     {
      do
       {
        RemovefromSocketQueue (sptr, &comeback);

         if (buf_q[0])
          {
           say ("|%d| %s\n", sptr->sock, buf_q);
          }
       }
      while (*&comeback)
            ;
     }

 }  /**/


 int IPCCLIENT_Read (Socket *sptr)

 {
  int comeback=0;
  extern char buf_q[];

    if (AddtoQueue(sptr))
     {
      do
       {
        RemovefromSocketQueue (sptr, &comeback);

         if (buf_q[0])
          {
           ProcessExec (sptr, buf_q);
          }
       }
      while ((*&comeback)&&((sptr->flag&FIRING_SQUAD)==0))
            ;        
     }

 }  /**/


 int DCCCHAT_Read (Socket *sptr)

 {
  int comeback=0;
  extern char buf_q[];

    if (AddtoQueue(sptr))
     {
      do
       {
        RemovefromSocketQueue (sptr, &comeback);

         if (buf_q[0])
          {
           ParseDCC_ (sptr);
          }
       }
      while ((*&comeback)&&((sptr->flag&FIRING_SQUAD)==0))
            ;
     }

 }  /**/


 int DCCGET_Read (Socket *sptr)

 {
    if (AddtoQueue(sptr))
     {
      GetFilePacketfromSocket (sptr);
     }

 }  /**/


 int DCCSEND_Read (Socket *sptr)

 {
    if (AddtoQueue(sptr))
     {
      SendFilePackettoSocket (sptr);
     }

 }  /**/


 int TELNET_Read (Socket *sptr)

 {
  int comeback=0;
  extern char buf_q[];

    if (AddtoQueue(sptr))
     {
      do
       {
        RemovefromSocketQueue (sptr, &comeback);

         if (buf_q[0])
          {
           if (sptr->flag&AWAITINGUSERNAME)
            {
             TelnetLogin (sptr);
            }
           else
            {
             ParseTelnet (sptr);
            }
          }
       }
      while ((*&comeback)&&((sptr->flag&FIRING_SQUAD)==0))
            ;
     }

 }  /**/


 #define WHILE_CONNECTED (mode==1)
 #define SELECT_ERROR    (mode==2)
 #define FORCIBLE        (mode==3)
 #define WHILE_LOGGING   (mode==4)
 #define NONBLOCKING     (mode==5)

 int IRCSERVER_Close (Socket *sptr, const int mode)

 {
  extern SocketsTable *const sockets;

    if (WHILE_CONNECTED)
       {
        say ("%$% Connection to IRC server %s lost!\n", HOT, sptr->haddress);
       }
    else
    if (SELECT_ERROR)
       {
        say ("%$% IRC server %s's socket (%d) blown up!\n", 
             HOT, sptr->haddress, sptr->sock);
       }
    else
    if (NONBLOCKING)
       {
        say ("%$% Connection to IRC server %s refused - %s.\n", 
             MILD, sptr->haddress, strerror(errno));

        close (sptr->sock);
        RemovefromSocketsTable (sockets, sptr->sock);

         if (isup()<0) 
            {
             setsockflag (abs(isup()), FIRING_SQUAD, 0);
             setstatus (TRYING, 0);
             setstatus (CONNECTED, 1);
             st_ptr->c_serversocket=abs(st_ptr->c_serversocket);

             say ("%$% Connection to %s on port %lu resumed...\n", INFO,
                  curserver(), st_ptr->c_serverport);
            }
       
        return;
       }
    else
    if (FORCIBLE)
       {
        say ("%$% Closing connection to IRC server %s...\n", INFO,
             sptr->haddress);
       }

    if (sptr->sock==isup())
       {
        ServerDown (0, 0, 0);
        close (sptr->sock);
        RemovefromSocketsTable (sockets, sptr->sock);
        ConnectionLost (SOCK_IRCSERVER);
       }
    else
       {
        close (sptr->sock);
        RemovefromSocketsTable (sockets, sptr->sock);
        say ("**I thought i del'ed this server sock (%d)\n", sptr->sock);
       }    

 }  /**/


 #define pptr ((Someone *)((Index *)(sptr->index))->sindex)
 #define iptr ((Index *)sptr->index)

 int LISTENINGTELNET_Close (Socket *sptr, const int mode)

 {
  extern SocketsTable *const sockets;

    if (FORCIBLE)
     {
      say ("%$% Telnet service no longer available.\n", MILD);
     }
    else
    if (SELECT_ERROR)
     {
      say ("%$% Listening Telnet socket (%d) blown up!\n", HOT, sptr->sock);
     }

   close (sptr->sock);

   RemovefromSocketsTable (sockets, sptr->sock);

   Flash (2, 0);

 }  /**/


 int TELNET_Close (Socket *sptr, const int mode)

 {
  char s[LARGBUF];
  chtype *chs;
  extern SocketsTable *const sockets;

    if (WHILE_CONNECTED)
     {
       if (sptr->flag&AWAITINGUSERNAME)  goto not_connected;
            
      chs=PreSay ("%$% 3Telnet user 3%s [%s] dropped out.\n", 
                  TELH, pptr->name, sptr->haddress);

      sprintf (s, "\r\n-*- %s dropped out.", pptr->name);
      BroadcastMsg (s, sptr->sock);
     }
    else
    if (WHILE_LOGGING)
     {
      not_connected:

      chs=PreSay ("%$% 3Telnet user [3%s] aborted connection"
                  " before logging in.\n", TELH, sptr->haddress);
     }
    else
    if (FORCIBLE)
     {
      if (sptr->flag&AWAITINGUSERNAME)
       {
        chs=PreSay ("%$% Closed half-open 3Telnet connection"
                    " from [3%s]\n", TELH, sptr->haddress);
        ToSocket (sptr->sock, "\r\n It has been decided that your"
                              " connection should be terminated.\r\n");
       }
      else
       {
        ToSocket (sptr->sock, "\r\n-*- It has been decided that your"
                              " connection should be terminated.\r\n");
        chs=PreSay ("%$% Closed 3Telnet connection for %s.\n", 
                    TELH, pptr->name);

        sprintf (s, "\r\n-*- %s forcibly disconnected.", pptr->name);
        BroadcastMsg (s, sptr->sock);
       }
     }
    else
    if (SELECT_ERROR)
     {
      chs=PreSay ("%$% 3Telnet socket (%d) blown up!\n", HOT, sptr->sock);
     }

#ifdef LAST__LOG
    if (isLogLevel(LOG_TEL))
     {
      AddtoLastLog (LOG_TEL, NULL, chs);
     }
#endif

   nYelll (chs);

   close (sptr->sock);

    if (sptr->flag&AWAITINGUSERNAME)
     {
      RemovefromSocketsTable (sockets, sptr->sock);

      Flash (3, 0);
      
      return;
     }

   RemoveIndexbyptr (pptr, iptr);
   RemovefromSocketsTable (sockets, sptr->sock);

   Flash (3, 0);

 }  /**/


 int OTHER_Close (Socket *sptr, const int mode)

 {
  extern SocketsTable *const sockets;

   if (WHILE_CONNECTED)
    {
     say ("%$% Connection to server %s closed.\n", MILD, sptr->haddress);
    }
   else
   if (FORCIBLE)
    {
     say ("%$% Closed connection to server %s.\n", INFO, sptr->haddress);
    }
   else
   if (SELECT_ERROR)
    {
     say ("%$% Server %s's socket (%d) blown up!\n", HOT, 
          sptr->haddress, sptr->sock);       
    }
   else
   if (NONBLOCKING)
    {
     say ("%$% Connection to server %s refused. - %s\n", MILD, 
          sptr->haddress, strerror(errno));
    }

   close (sptr->sock);

   RemovefromSocketsTable (sockets, sptr->sock);

 }  /**/


 #define dcc ((DCCParams *)iptr->pindex)

 int CONNECTEDDCC_Close (Socket *sptr, const int mode)

 {
  extern SocketsTable *const sockets;

    if (WHILE_CONNECTED)
     {
     }
    else
    if (FORCIBLE)
     {
     }
    else
    if (SELECT_ERROR)
     {
      say ("%$% DCC Connection socket (%d) blown up!\n", HOT, sptr->sock);
     }
    else
    if (NONBLOCKING)
     {
      close (sptr->sock);

       if (IS_GET(dcc))
        {
         say ("%$% 6DCC GET (6%s %lu) from %s failed - %s.\n",
              CTCPH, dcc->file, dcc->size, pptr->name, strerror(errno));
        }
       else
       if (IS_CHATTOUS(dcc))
        {
         say ("%$% 6DCC CHAT connection to %s failed - %s.\n",CTCPH,
              pptr->name, strerror(errno));
        }

      RemoveIndexbyptr (pptr, iptr);
      RemovefromSocketsTable (sockets, sptr->sock);

      return 0;
     }

   close (sptr->sock);

   EOFDCCConnection (sptr, dcc);

 }  /**/


 int LISTENINGDCC_Close (Socket *sptr, const int mode)

 {
  extern SocketsTable *const sockets;

    if (WHILE_CONNECTED)
     {
     }
    else
    if (FORCIBLE)
     {
     }
    else
    if (SELECT_ERROR)
     {
      say ("%$% Listening DCC Connection socket (%d) blown up!\n", 
           HOT, sptr->sock);
     }

   close (sptr->sock);

   RemovefromSocketsTable (sockets, sptr->sock);

 }   /**/


 int IPCSERVER_Close (Socket *sptr, const int mode)

 {
  extern SocketsTable *const sockets;
  extern Someone *ipc_ptr;

    if (WHILE_CONNECTED)
     {
     }
    else
    if (FORCIBLE)
     {
      say ("%$% No longer accepting IPC requests.\n", MILD);
     }
    else
    if (SELECT_ERROR)
     {
      say ("%$% IPC Client's socket (%d) blown up!\n", HOT, sptr->sock);
     }

   close (sptr->sock);

   RemoveIndexbyptr (ipc_ptr, iptr); /* double-check */
   RemovefromSocketsTable (sockets, sptr->sock);

 }  /**/


 int IPCCLIENT_Close (Socket *sptr, const int mode)

 {
  extern SocketsTable *const sockets;

    if (WHILE_CONNECTED)
     {
     }
    else
    if (FORCIBLE)
     {
      say ("%$% IPC connection terminated.\n", INFO);
     }
    else
    if (SELECT_ERROR)
     {
      say ("%$% IPC Client's socket (%d) blown up!\n", HOT, sptr->sock);
     }

   close (sptr->sock);

   RemoveIndexbyptr (pptr, iptr);
   RemovefromSocketsTable (sockets, sptr->sock);

 }  /**/

 #undef iptr
 #undef pptr
 #undef dcc


/* static __inline__ void ReadKeyboardInput (void)

 {
  extern Stack *const stack;

   (*((InputProcessor *)(stack->top->whatever))->func)(wgetch(InputWin));

 }*/  /**/


#include <fcntl.h>


 #define KEYBOARD_READABLE() \
   ((mt_ptr->c_output&BACKGROUND)==0)&&(FD_ISSET(STDIN_FILENO, &fd))

 /*
                    -:::::::- The Engine Room -:::::::-
 */
 int ProbeSockets (void)

 {
  register int i;
  int x,
      k,
      max,
      j[]={[0 ... MAXSOCKSCOUNT]=0};
  fd_set fd,
         wfd;
  register Socket *sptr;
  extern Socket *so_hash[];
  extern SocketsTable *const sockets;

   again:
   max=i=0;
   FD_ZERO (&fd);
   FD_ZERO (&wfd);

    if ((mt_ptr->c_output&BACKGROUND)==0)  FD_SET(STDIN_FILENO, &fd);

    for (sptr=sockets->head; sptr!=NULL; sptr=sptr->next)
     {
       if (sptr->flag&FIRING_SQUAD)
        {
          if (((sptr->flag&REPORTED)==0))
           {
            sptr->flag|=REPORTED;

            say ("%$% Connection corresponding to socket %d placed"
                 " before firing squad...\n", INFO, sptr->sock);
           }

         continue;
        }

       if (sptr->flag&BLOCKING)  FD_SET(sptr->sock, &wfd);

      FD_SET((*(j+i++)=sptr->sock), &fd);

       if (sptr->sock>max)  max=sptr->sock;
     }

   *(j+i)=max;

   x=select ((*(j+i))+1, &fd, &wfd, NULL, NULL);

    if (x>0) 
     {
      BlockSignals ();

       if (KEYBOARD_READABLE())
        {
         ReadKeyboardInput (); 
        }

       for (k=0; k<i; k++)
        {
          if (!(sptr=so_hash[(j[k])]))  continue;

          if (sptr->flag&BLOCKING)
           {
             if ((FD_ISSET(*(j+k), &wfd))||(FD_ISSET(*(j+k), &fd)))
              {
               int err=0, 
               len=sizeof(len);

                 if (getsockopt(*(j+k), SOL_SOCKET, SO_ERROR, &err, &len)<0)
                  {
                   (*sptr->close_func)(sptr, 5);

                   continue;
                  }

                 if (err!=0)
                  {
                   errno=err;
                   (*sptr->close_func)(sptr, 5);
                  }
                 else
                  {
                   SetSocketFlags (sptr->sock, 0, O_NONBLOCK);

                    if (sptr->init_func)
                     {
                      (*sptr->init_func)(sptr);
                     }
                    else
                     {
                      sptr->flag&=~BLOCKING;
                     }
                  }
              }
           }
          else   
          if (FD_ISSET(*(j+k), &fd))
           {
            sptr->read_func(sptr);
           }

          if (sptr->flag&FIRING_SQUAD)
           {
            if (set("AUTO_SHOOT"))
             {
              sptr->close_func(sptr, 3);
             }
            else
             {
              sptr->flag|=REPORTED;
              say ("%$% Connection corresponding to socket %d placed"
                   " before firing squad...\n", INFO, sptr->sock);
             }
           }
        }

      UnblockSignals ();

      goto again;
     } 

    else
    if ((x==-1)&&(errno!=EINTR))  /* select error */
     {
      int l;

       say ("select: %s\n", strerror(errno));
        for (k=0; k<i; k++)
         {
          if ((l=fcntl(j[k], F_GETFD, 0))<0)
           {
            sptr=so_hash[(j[k])];

            (*sptr->close_func)(sptr, 2);
           }
         }

       return; /* start afresh */
     }

   goto again;

 }  /**/


 __inline__ int WriteToSocket (int socket, const char *str)

 {
  register Socket *sptr;
  extern Socket *so_hash[];

    if ((sptr=so_hash[abs(socket)]))
     {
       if ((write(socket, str, strlen(str))<0)&&(errno!=EAGAIN))
        {
         sptr->flag|=FIRING_SQUAD;

          if (isup()==socket)  {Flash(1, 1); isup()=0;}

         return 0;
        }

      sptr->sbytes+=strlen (str);

      return 1;
     }

   return 0;

 }  /**/


 int ToSocket (int socket, const char *format, ...)

 {
  static char buffer [600];
  va_list va;

   va_start (va, format);
   vsnprintf (buffer, 512, format, va);
   va_end (va);

   return (WriteToSocket(socket, buffer));

 }  /**/

