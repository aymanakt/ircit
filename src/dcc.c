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
#include <stack.h>
#include <scroll.h>
#include <updates.h>
#include <u_status.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <net.h>
#include <serverio.h>
#include <sockets.h>
#include <misc.h>
#include <people.h>
#include <prefs.h>
#include <dcc.h>

 MODULEID("$Id: dcc.c,v 1.8 1998/04/06 06:13:44 ayman Beta $");

 static int InitDCCSlot_ (Someone *, Index *);
 static void AbortDCCOffer (Socket *);
 static Index *FindSomeoneDCCOffer (int, const Someone *);
 static Index *FindDCCOffer (int, ...);

 static void SelectDCCType (int, Someone *, Index *, int *);
 static void CloseAllDCCConnections (const int);
 static void CloseSomeoneDCCConnections (const int, Someone *);

#define tell_him_I_cant_send \
 nCTCPToServer (pptr->name, "DCC REJECT GET %s", dcc->file)
#define tell_him_I_cant_chat \
 nCTCPToServer (pptr->name, "DCC REJECT CHAT chat")

#define DCC 16


 List *GetDCCListptr (void)

 {
  static List DCCList;

    return (List *)&DCCList;

 }  /**/


  DCCParams *FillDCCEntry (Index *iptr)

 {
  register DCCParams *dcc;
  register ListEntry *eptr;
  List *lptr;

   xmalloc(dcc, (sizeof(DCCParams)));
   memset (dcc, 0, sizeof(DCCParams));

   eptr=AddtoList ((lptr=GetDCCListptr()));
   eptr->whatever=dcc;

   (Index *)dcc->index=iptr;
   (DCCParams *)iptr->pindex=dcc;

    if (((lptr->glock>>0)&0x1)&&((lptr->glock>>1)&0x1))
     {
      RefreshListContents (lptr);
     } 

   return (DCCParams *)dcc;

 }  /**/


 void RemoveDCCEntry (DCCParams *dcc)

 {
  int found=0;
  List *lptr=GetDCCListptr();
  register ListEntry *eptr,
                     *prev=NULL;

   eptr=lptr->head;

    while ((eptr!=NULL)&&(!found))
     {
      if (eptr->whatever==(void *)dcc)
       {
        found=1;
       }
      else
       {
        prev=eptr;
        eptr=eptr->next;
       }
     }

    if (!found)  return;

    if (prev==NULL)
     {
      lptr->head=eptr->next;
     }
    else
    if (eptr->next==NULL)
     {
      lptr->tail=prev;
      lptr->tail->next=NULL;
     }
    else
     {
      prev->next=eptr->next;
     }

   lptr->nEntries--;

    if (((lptr->glock>>0)&0x1)&&((lptr->glock>>1)&0x1))
     {
      if (lptr->nEntries==0)
       {
        DestroyWindow (4, lptr);
       }
      else
       {
        RefreshListContents (lptr);
       }
     }

   memset (eptr, 0, sizeof(ListEntry));
   free (eptr);

    if (lptr->nEntries==0)  
     {
      lptr->head=lptr->tail=NULL;

       if (mt_ptr->c_status==2)
        {
         DisplayUpperStatusII ();
        }
     }

 }  /**/


 #define sptr ((Socket *)iptr->index)

 void *InitDCCCHAT (Someone *pptr)

 {
  const size_t sizeofDCCParams=sizeof(DCCParams);
  int x;
  register Index *iptr;
  DCCParams *dcc;
  extern int AnswerDCCRequest_ (Socket *);


   iptr=AddtoSomeoneIndex (pptr);

    if (iptr==(Index *)NULL)
     {
      say ("%$% Unable to initiate 6DCC CHAT connection to %s -"
           " internal.\n", CTCPH, pptr->name);

      return NULL;
     }

   iptr->when=time (NULL);
   iptr->protocol=PROTOCOL_DCC;

   dcc=FillDCCEntry (iptr);
   dcc->type|=DCCCHATTOHIM;
   dcc->flags|=AWAITINGCONNECTION;

//HERE

   x=InitDCCSlot_ (pptr, iptr);

    if (x>0)
     {
      say ("%$% Initiating 6DCC CHAT connection to %s...\n", 
           CTCP, pptr->name);

      ToServer ("PRIVMSG %s :\01DCC CHAT chat %u %d\01\n",
                pptr->name,
                localip(),
                dcc->port);
       
      UpdateSockets (M_CHATS, '+'); 

      sptr->read_func=AnswerDCCRequest_;
      sptr->close_func=LISTENINGDCC_Close;
      iptr->idle=time (NULL);

      return (Index *)iptr;
     }
    else
     {
      say ("%$% Unable to initiate 6DCC CHAT connection to %s -"
           " network.\n", CTCPH, pptr->name);

      RemoveIndexbyptr (pptr, iptr);

      return NULL;
     }

 }  /**/


 int InitDCCSEND (Someone *const pptr, const char *file)

 {
  int x;
  const size_t sizeofDCCParams=sizeof(DCCParams);
  register Index *iptr;
  register DCCParams *dcc;
  extern int AnswerDCCRequest_ (Socket *);

    if (!(iptr=AddtoSomeoneIndex(pptr)))
     {
      say ("%$% Unable to initiate 6DCC SEND connection to %s -" 
           " internal.\n", CTCPH, pptr->name);

      return 0;
     }

   iptr->when=time (NULL);
   iptr->protocol=PROTOCOL_DCC;

   dcc=FillDCCEntry (iptr);
   dcc->file=strdup (file);
   dcc->type|=DCCGETTING;
   dcc->flags|=AWAITINGCONNECTION;

//HERE

   x=CheckFileBeforeSending (dcc);

     if (x==0)
      {
       say ("%$% 6DCC SEND (6%s %lu) to %s aborted.\n", 
            CTCPH, dcc->file, dcc->size, pptr->name);

       RemoveIndexbyptr (pptr, iptr); /* takes care of allocated dcc.. */

       return 0;
      }

   x=InitDCCSlot_ (pptr, iptr);

    if (x>0)
     {
      say ("%$% Initiating 6DCC SEND (6%s %lu) connection"
           " to %s...\n", CTCP, dcc->file, dcc->size, pptr->name);

      ToServer ("PRIVMSG %s :\01DCC SEND %s %u %d %d\01\n",
                pptr->name,
                dcc->file,
                localip(),
                dcc->port,
                dcc->size);
      
      UpdateSockets (M_SENDS, '+'); 

      sptr->read_func=AnswerDCCRequest_;
      sptr->close_func=LISTENINGDCC_Close;
      iptr->idle=time (NULL);

      return 1;
     }
    else
     {
      say ("%$% Unable to initiate 6DCC SEND connection to %s -"
           " network.\n", CTCPH, pptr->name);

      RemoveIndexbyptr (pptr, iptr);

      return 0;
     }

 }  /**/

#undef sptr


 int InitDCCSlot_ (Someone *pptr, Index *iptr)

 {
  int len,
      reuse=1,
      dcc_sock;
  register Socket *sptr;
  register DCCParams *dptr;
  struct sockaddr_in dcc;
  extern SocketsTable *const sockets;

   memset (&dcc, 0, sizeof(dcc));
   dcc.sin_family=AF_INET;
   dcc.sin_port=htons (0);
   dcc.sin_addr.s_addr=htonl (INADDR_ANY);

    if (!(dcc_sock=RequestSocket()))  return 0;

   setsockopt (dcc_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    if (bind(dcc_sock, (struct sockaddr *)&dcc, sizeof(dcc))<0)
     {
      say ("%$% Unable to bind listening DCC socket %d - %s.\n", HOT, 
           dcc_sock, strerror(errno));

      close (dcc_sock);

      return 0;
     }

    if (listen(dcc_sock, 1)<0)
     {
      say ("%$% Unabel to get DCC socket %d to listen - %s.\n", HOT, 
           dcc_sock, strerror(errno));

      close (dcc_sock);
        
      return 0;
     }

   len=sizeof (dcc);
    if (getsockname(dcc_sock, (struct sockaddr *)&dcc, &len)<0)
     {
      say ("%$% 'getsockname()' failed on DCC socket %d - %s.\n", HOT, 
           dcc_sock, strerror(errno));

      close (dcc_sock);

      return 0;
     }

    if (!(sptr=AddtoSocketsTable(sockets, dcc_sock)))
     {
      close (dcc_sock);

      return 0;
     }

   sptr->when=time (NULL);
   sptr->port=ntohs (dcc.sin_port);
   sptr->hport=0;
   strcpy (sptr->address, "localhost");
   strcpy (sptr->haddress, "Yet to be known");
   sptr->type=SOCK_LISTENINGDCC;

   (Index *)sptr->index=(Index *)iptr;
   (Socket *)iptr->index=(Socket *)sptr;

   (DCCParams *)dptr=(DCCParams *)iptr->pindex;
   dptr->port=sptr->port;

   /*dcc_timeout_timer++;
    if (dcc_timeout_timer==1)
       {
        ActivateTimer (5, NULL);
       }*/

   return dcc_sock;

 }  /**/


 static __inline__ void AbortDCCOffer (Socket *sptr)

 {
  extern SocketsTable *const sockets;

  #define iptr ((Index *)sptr->index)
  #define pptr ((Someone *)iptr->sindex)
  #define dcc ((DCCParams *)iptr->pindex)

    if (IS_SEND(dcc))
     { 
      say ("%$% %s (6%s %lu) to %s - %s.\n", CTCPH,
           "Unable to accept on previously initiated 6DCC SEND",
           dcc->file, dcc->size, pptr->name, strerror(errno));
      say ("  6DCC SEND file %s to %s aborted!\n", 
           dcc->file, pptr->name);

      nCTCPToServer (pptr->name, "DCC REJECT GET %s", dcc->file);

      UpdateSockets (M_SENDS, '-');
     }
    else
    if (IS_CHATTOHIM(dcc))
     {
      say ("%$% %s offer to %s - %s.\n", CTCPH,
           "Unable to accept on previously initiated 6DCC CHAT",
           pptr->name, strerror(errno));
      say ("  6DCC CHAT offer to %s aborted!\n", pptr->name);

      nCTCPToServer (pptr->name, "DCC REJECT CHAT chat");

      UpdateSockets (M_CHATS, '-');
     }

   RemoveIndexbyptr (pptr, iptr);
   RemovefromSocketsTable (sockets, sptr->sock);

 }  /**/

 #undef iptr
 #undef pptr
 #undef dcc


 int AnswerDCCRequest_ (Socket *ptr)

 {
  int len,
      nsocket,
      sin_size=sizeof(struct sockaddr_in),
      opt=1;
  register Someone *pptr;
  register Index *iptr;
  DCCParams *dcc;
  struct sockaddr_in me,
                     hisaddr;
  extern SocketsTable *const sockets;
  extern Socket *so_hash[];
  extern int DCCCHAT_Read (Socket *);
  extern int DCCSEND_Read (Socket *);

   /* sin_size=sizeof (struct sockaddr_in); */
    if ((nsocket=accept(ptr->sock, (struct sockaddr *)&hisaddr, &sin_size))<=0)
     {
      close (ptr->sock);
      /* so_hash[ptr->sock]=(Socket *)NULL; */
      AbortDCCOffer (ptr);

      return 0;
     }

   close (ptr->sock);  /* listening socket */
   
   setsockopt (nsocket, SOL_SOCKET, SO_KEEPALIVE, (void *)&opt, sizeof(int));

   len=sizeof (me);
    if (getsockname(nsocket, (struct sockaddr *)&me, &len)<0)
       {
        close (nsocket);
        //so_hash[ptr->sock]=(Socket *)NULL;
        AbortDCCOffer (ptr);
        
        return 0;
       }

   if (getpeername(nsocket, (struct sockaddr *)&hisaddr, &len)<0)
      {
       close (nsocket);
       //so_hash[ptr->sock]=(Socket *)NULL;
       AbortDCCOffer (ptr);

       return 0;
      } 

   strcpy (ptr->haddress, (char *)inet_ntoa(hisaddr.sin_addr));
   ptr->port=ntohs (me.sin_port);
   strcpy (ptr->address, "localhost");
   ptr->hport=ntohs (hisaddr.sin_port);

   /* is this necessary? */
   (Index *)iptr=(Index *)ptr->index;  /* this yes, see 2 lines bellow  */
   (Socket *)iptr->index=(Socket *)ptr;

   (Someone*)pptr=INDEXTOSOMEONE(iptr);
   (DCCParams *)dcc=INDEXTODCC(iptr);
   
   dcc->flags&=~AWAITINGCONNECTION;
   dcc->flags|=CONNECTED;

    if (IS_SEND(dcc))
     {
      ssize_t n;
      char buf[ptr->block_sz+1];

       n=read (dcc->fd, (char *)buf, ptr->block_sz-1);

        if (n<0)
         {
          say ("%$% Error reading file %s - %s.\n", HOT, dcc->file, 
               strerror(errno));

          close (nsocket);
          /* so_hash[ptr->sock]=(Socket *)NULL; */
          AbortDCCOffer (ptr);

          return 0;
         }

       dcc->xfer+=n;
       /* dcc->ack=ntohl (n); */
       ptr->sbytes=dcc->xfer;
       ptr->rbytes=0;

       ptr->read_func=DCCSEND_Read;
       ptr->close_func=CONNECTEDDCC_Close;

        if ((write(nsocket, buf, n))<n) //oopsz
         {
          say ("%$% Error sending first packet to DCC socket - %s.\n", HOT,
               strerror(errno));

          close (nsocket);
          /* so_hash[ptr->sock]=(Socket *)NULL; */
          AbortDCCOffer (ptr);

          return 0;
         }

       say ("%$% 6DCC SEND (6%s %lu) to %s started."
            " Waited %s seconds.\n", CTCP, dcc->file, dcc->size, 
            pptr->name, secstostr((time(NULL)-iptr->when), 1));

        if (mt_ptr->c_status==2)
         {
          DisplayUpperStatusII ();
         }
     }
    else
    if (IS_CHATTOHIM(dcc))
     {
      ptr->read_func=DCCCHAT_Read;
      ptr->close_func=CONNECTEDDCC_Close;
      ptr->rbytes=ptr->sbytes=0;

      say ("%$% 6DCC CHAT connection to %s established."
           " waited %s secs.\n", 
           CTCP, pptr->name, secstostr((time(NULL)-iptr->when), 1));
     }    
   
   iptr->idle=iptr->when=time (NULL);

   so_hash[ptr->sock]=(Socket *)NULL;
   ptr->sock=nsocket;
   so_hash[ptr->sock]=(Socket *)ptr;
   
   ptr->type=SOCK_CONNECTEDDCC;

   //UpdateSockets (ptr->type, '+'); //STATS

   Flash (6, 1);

   /*dcc_timeout_timer--;
    if (dcc_timeout_timer==0)
       {
        DeActivateTimer (5, NULL);
       }*/
 
   return nsocket;

 }  /**/


 int AttemptDCCConnection_ (Someone *ptr, DCCParams *dcc)

 {
  int len,
      socket;
  register Socket *sptr;
  register Index *iptr;
  struct in_addr *addr;
  ResolvedAddress raddr;
  struct sockaddr_in me;
  extern SocketsTable *const sockets;

    if (!(ResolveAddress(RawIPToDotted(dcc->ip), &raddr)))  goto there;

    if (!(socket=Connect (raddr.inetaddr, dcc->port)))  goto there;

   len=sizeof (me);
    if (getsockname(abs(socket), (struct sockaddr *)&me, &len)<0)
     {
      say ("%$% 'getsockname' failed on DCC socket %d.\n", HOT, socket);

      close (abs(socket));

      goto there;
     }

    if ((sptr=AddtoSocketsTable(sockets, abs(socket))))
       {
         sptr->when=time (NULL);
         sptr->hport=dcc->port;
         sptr->port=ntohs (me.sin_port);
         strcpy (sptr->haddress, raddr.dns);
         strcpy (sptr->address, "localhost");
         sptr->type=SOCK_CONNECTEDDCC;

          if (IS_CHATTOUS(dcc))
             {
              sptr->read_func=DCCCHAT_Read;
             }
          else
          if (IS_GET(dcc))
             {
              sptr->read_func=DCCGET_Read;
             }

         sptr->close_func=CONNECTEDDCC_Close;
         sptr->init_func=DCCConnetInit;

          if (socket<0)  /* non-blocking */
             {
              sptr->flag|=BLOCKING;
              dcc->flags|=AWAITINGCONNECTION;
             }
          else
             {
              dcc->flags&=~AWAITINGCONNECTION;
              dcc->flags|=CONNECTED;
             }

         dcc->socket=sptr->sock; /* hmmm.... */

         (Index *)iptr=(Index *)dcc->index;

         (Socket *)iptr->index=(Socket *)sptr;
         (Index *)sptr->index=(Index *)iptr;

         iptr->when=time (NULL);

         /* UpdateSockets (sptr->type, '+'); */

         Flash (6, 1);
 
         return socket;
        }
    else
       {
        close (abs(socket));
  there:
         {
          IS_CHATTOUS(dcc)?
           (UpdateSockets(M_CHATS, '-')):
           (IS_GET(dcc)?(UpdateSockets(M_GETS, '-')):0);
         }

        return 0;
       }

 }  /**/


 #define iptr ((Index *)ptr->index)
 #define pptr ((Someone *)iptr->sindex)
 #define dcc  ((DCCParams *)iptr->pindex)

 int ParseDCC_ (Socket *ptr)

 {
  extern char buf_q[];

   dcc->recv=ptr->rbytes;
   iptr->idle=time (NULL);

   say ("6=%s= %s\n", pptr->name, buf_q);

   buf_q[0]=0;

 }  /**/

 #undef iptr
 #undef pptr
 #undef dcc


#define HAVE_SOMETHING_TO_KILL() (n>0)

 int GetAllPendingFiles (Someone *ptr)

 {
  int i=0,
      n=0,
      x;
  Socket *sptr;
  register Someone *pptr=NULL;
  register Index *iptr;
  DCCParams *dcc;
  extern People *const people;

    if (ptr==(Someone *)NULL)
       {
         for (pptr=people->head; pptr!=NULL; pptr=pptr->next)
             {
               if (GOT_INDICES(pptr))
                  {
                    for (iptr=pptr->head; iptr!=NULL; iptr=iptr->next)
                        {
                          if (IS_DCC(iptr))
                             {
                              (DCCParams *)dcc=INDEXTOPROTOCOL(iptr);
                              //sptr=(Socket *)iptr->index;
               
                                if ((dcc->type&DCCSENDING)&&(!iptr->index)&&
                                    (dcc->flags&AWAITINGCONNECTION))
                                   {
                                     if (CheckFileBeforeGetting(dcc))
                                        {
                                          if ((x=AttemptDCCConnection_(pptr, dcc)))
                                             {
                                              i++;
                                               if (x>0)
                                              say ( 
                                               "%$% 6DCC GET %s from %s started.\n", CTCP, dcc->file, pptr->name);
                                              }
                                           else
                                              {
                                               say ("%$% Unable to start 6DCC GET (6%s %lu) from %s.\n", CTCPH, dcc->file, dcc->size, pptr->name);

                                               n++;
                                               dcc->flags|=FIRING_SQUAD;
                                              }
                                        }
                                   }
                             }
                        }
                  }
             }

         if (HAVE_SOMETHING_TO_KILL())
            {
             CallFiringSquad (NULL);
            }
       }
    else
       {
         if (GOT_INDICES(ptr))
            {
              for (iptr=ptr->head; iptr!=NULL; iptr=iptr->next)
                  {
                    if (IS_DCC(iptr))
                       {
                        (DCCParams *)dcc=INDEXTOPROTOCOL(iptr);
                        sptr=(Socket *)iptr->index;

                         if ((dcc->type&DCCSENDING)&&(!iptr->index)&&
                             (dcc->flags&AWAITINGCONNECTION))
                            {
                              if (CheckFileBeforeGetting(dcc))
                                 {
                                   if ((x=AttemptDCCConnection_(ptr, dcc)))
                                      {
                                       i++;
                                        if (x>0)
                                       say ("%$% 6DCC GET (6%s %lu) from %s started.\n", CTCP, dcc->file, dcc->size, ptr->name);
                                       }
                                    else
                                       {
                                        say ("%$% 6DCC GET (6%s %lu) from %s failed.\n", CTCPH, dcc->file, dcc->size, ptr->name);

                                        n++;
                                        dcc->flags|=FIRING_SQUAD;
                                       }
                                 }
                            }
                       }
                  }
            
              if (HAVE_SOMETHING_TO_KILL())
                 {
                  CallFiringSquad (ptr);
                 }
            }
       }

  return i;

 }  /**/


 #define dcc ((DCCParams *)((Index *)(sptr->index))->pindex)
 #define iptr ((Index *)sptr->index)
 #define pptr ((Someone *)(iptr->sindex))

 int GetFilePacketfromSocket (Socket *sptr)

 {
  unsigned long rbytes;
  register SocketQueue *qptr;

   qptr=sptr->front;

   dcc->recv+=qptr->flag;

    if ((write(dcc->fd, qptr->msg, qptr->flag))<qptr->flag)
     {
      say ("%$% Error while processing 6DCC GET (6%s %lu)"
           " from %s: %s.\n", 
           CTCPH, dcc->file, dcc->size, pptr->name, strerror(errno));

      EOFDCCConnection (sptr, dcc);

      return;
     }   

   qptr=sptr->front;
   sptr->front=sptr->front->next;
   sptr->nMsg--;

    if (!sptr->nMsg)
     {
      sptr->rear=NULL;
     }
   free (qptr->msg);
   free (qptr); 
  
   rbytes=htonl (dcc->recv);
   write (sptr->sock, (char *)&rbytes, sizeof(unsigned long));

   iptr->idle=time (NULL);   
  
    if (mt_ptr->c_status==2)
     {
      DisplayUpperStatusII ();
     }

 }  /**/  


 int SendFilePackettoSocket (Socket *sptr)

 {
  char buf[]={[0 ... 2048]=0};
  ssize_t n;
  unsigned long xfer=0L;
  register SocketQueue *qptr;
  extern SocketsTable *const sockets;
   
   qptr=sptr->front;
   sptr->front=sptr->front->next;
   sptr->nMsg--;

    if (qptr->flag<sizeof(unsigned long))
       {
        say ("%$% Aborted 6DCC SEND (6%s %lu) to %s"
             " - wrong handshake.\n", CTCPH, dcc->file, dcc->size, pptr->name);
       
        DisplayDCCSummary (sptr, dcc); 
        close (sptr->sock);
        RemoveIndexbyptr (pptr, iptr);
        RemovefromSocketsTable (sockets, sptr->sock);

        UpdateSockets (M_SENDS, '-');

        Flash (6, 0);
        
         if (mt_ptr->c_status==2)
            {
             DisplayUpperStatusII ();
            }

        return;
       }

    xfer=(qptr->msg[0]<<24)+(qptr->msg[1]<<16)+
         (qptr->msg[2]<<8)+qptr->msg[3];

      /*if (xfer<dcc->xfer)
         {
          free (qptr->msg);
          free (qptr);

          return;
         }*/

   free (qptr->msg);
   free (qptr);

   n=read (dcc->fd, (char *)buf, sptr->block_sz-1);
   write (sptr->sock, buf, n);

   dcc->xfer+=n;
   sptr->sbytes=dcc->xfer;

    if (dcc->xfer==dcc->size)
       {
        say ("%$% 6DCC SEND (6%s %lu) to %s completed in"
               " %d seconds.\n", CTCP, 
               dcc->file, dcc->size, pptr->name, (time(NULL)-iptr->when));

        DisplayDCCSummary (sptr, dcc);
        close (sptr->sock);
        RemoveIndexbyptr (pptr, iptr);
        RemovefromSocketsTable (sockets, sptr->sock);

        UpdateSockets (M_SENDS, '-');

        Flash (6, 0);

         if (mt_ptr->c_status==2)
            {
             DisplayUpperStatusII ();
            }

        return;
       }

   iptr->idle=time (NULL);

    if (mt_ptr->c_status==2)
     {
      DisplayUpperStatusII ();
     }
    
 }  /**/ 
        
 #undef dcc
 #undef pptr
 #undef iptr


 /*
 ** already connected dcc connection
 */
 void EOFDCCConnection (Socket *sptr, DCCParams *dcc)

 {
  register Someone *pptr;
  register Index *iptr;
  extern SocketsTable *const sockets;

   (Index *)iptr=SOCKETTOINDEX(sptr);
   (Someone *)pptr=INDEXTOSOMEONE(iptr);

    if ((IS_CHATTOUS(dcc))||(IS_CHATTOHIM(dcc)))
       {
        say ("%$% 6DCC CHAT to %s lost.\n", CTCPH, pptr->name);

        DisplayDCCSummary (sptr, dcc);        
        RemoveIndexbyptr (pptr, iptr);
        RemovefromSocketsTable (sockets, sptr->sock);

        UpdateSockets (M_CHATS, '-');

        Flash (6, 0);

        return;
       }

   if (IS_GET(dcc))
      {
        if (dcc->size==dcc->recv)
           {
            char s[SMALLBUF];	
            float xfertime;

             xfertime=(float)dcc->size/((float)time(NULL)-(float)iptr->when);
              if (xfertime<1024.0)
                 {
                  sprintf (s, "%.1f bytes/s", xfertime);
                 }
              else
                 {
                  sprintf (s, "%.1f kb/s", xfertime/1024.0);
                 }

             say ("%$% 6DCC GET (6%s %lu) from %s completed"
                    " in %d seconds (%s).\n", CTCP,
                    dcc->file, dcc->size, pptr->name, time(NULL)-iptr->when, s);

             DisplayDCCSummary (sptr, dcc);
             RemoveIndexbyptr (pptr, iptr);
             RemovefromSocketsTable (sockets, sptr->sock);

             UpdateSockets (M_GETS, '-');

             Flash (6, 0);

             return;
           }
        else
           {
            char s[SMALLBUF];

             sprintf (s, "(%.1f%%)", ((float)dcc->recv/(float)dcc->size)*100.0);

             say ("%$% 6DCC GET (6%s %lu) from %s ended"
                    " prematurely!\n  %$192%$196%$16 %lu bytes %s made"
                    " it to your HD.\n", CTCPH, 
                    dcc->file, dcc->size, pptr->name, dcc->recv, s);

            DisplayDCCSummary (sptr, dcc);
            RemoveIndexbyptr (pptr, iptr);
            RemovefromSocketsTable (sockets, sptr->sock);

            UpdateSockets (M_GETS, '-');

            Flash (6, 0);

            return;
           } 

        if (mt_ptr->c_status==2)
           {
            DisplayUpperStatusII ();
           }
      }

   if (IS_SEND(dcc))
      {
        if (dcc->size==dcc->xfer)
           {
            char s[SMALLBUF];
            float xfertime;

             xfertime=(float)dcc->size/((float)time(NULL)-(float)iptr->when);
              if (xfertime<1024.0)
                 {
                  sprintf (s, "%.1f bytes/s", xfertime);
                 }
              else
                 {
                  sprintf (s, "%.1f kb/s", xfertime/1024.0);
                 }

             say ("%$% 6DCC SEND (6%s %lu) to %s completed"
                  " in %d seconds (%s).\n", CTCPH, dcc->file, 
                   dcc->size, dcc->file, dcc->size, pptr->name, 
                   time(NULL)-iptr->when, s);

             DisplayDCCSummary (sptr, dcc);
             RemoveIndexbyptr (pptr, iptr);
             RemovefromSocketsTable (sockets, sptr->sock);

             UpdateSockets (M_SENDS, '-');

             Flash (6, 0);

             return;
           }
        else
           {
            char s[MINIBUF];

             sprintf (s, "(%.1f%%)", ((float)dcc->xfer/(float)dcc->size)*100.0);

             say ("%$% 6DCC SEND (6%s %lu) to %s ended"
                  " prematurely!\n  %$192%$196%$16 %lu bytes %s made"
                  " it to him.\n", CTCPH, dcc->file, dcc->size, 
                   pptr->name, dcc->xfer, s);

            DisplayDCCSummary (sptr, dcc);
            RemoveIndexbyptr (pptr, iptr);
            RemovefromSocketsTable (sockets, sptr->sock);

            UpdateSockets (M_SENDS, '-');

            Flash (6, 0);

            return;
           }

        if (mt_ptr->c_status==2)
           {
            DisplayUpperStatusII ();
           }
      }

 }  /**/


 char *PrintDCCState (Index *iptr)

 {
  register DCCParams *dcc;
  static char s[]={[0 ... MEDIUMBUF]=0};

    dcc=INDEXTODCC(iptr); 

     if ((IS_CHATTOUS(dcc))||(IS_CHATTOHIM(dcc)))
        {
          if (IS_DCCWAITING(dcc))
             {
              sprintf (s, 
               "%s seconds lapsed since offer %s", 
               secstostr(time(NULL)-iptr->when, 1),
               dcc->type&DCCCHATTOUS?"recieved":"sent");
             }
          else
          if (IS_DCCCONNECTED(dcc))
             {
              sprintf (s, 
               "Running for %s seconds, %d bytes worth of traffic.\n      %s %s seconds ago", 
               secstostr(time(NULL)-iptr->when, 1), dcc->xfer+dcc->recv,
               "Last msg recieved", secstostr(time(NULL)-iptr->idle, 1));
             }
       
         return (char *)s;
        }
     else
     if (IS_GET(dcc))
        {
          if (IS_DCCWAITING(dcc))
             {
              sprintf (s, "%s seconds lapsed since offer recieved", 
               secstostr(time(NULL)-iptr->when, 1));
             }
          else
          if (IS_DCCCONNECTED(dcc))
             {
              float xfertime;

               xfertime=(float)dcc->size/((float)time(NULL)-(float)iptr->when);
               sprintf (s, 
                "Running for %s seconds, %d bytes transfered to us.\n      %s %s seconds ago at %.1f bytes/s.",
                secstostr(time(NULL)-iptr->when, 1), dcc->recv,
                "Last packet recieved", secstostr(time(NULL)-iptr->idle, 1), xfertime);
             }

         return (char *)s;
        }
     else
     if (IS_SEND(dcc))
        {
          if (IS_DCCWAITING(dcc))
             {
              sprintf (s, "%s seconds lapsed since offer sent",
                          secstostr(time(NULL)-iptr->when, 1));
             }
          else
          if (IS_DCCCONNECTED(dcc))
             {
              float xfertime;

               xfertime=(float)dcc->size/((float)time(NULL)-(float)iptr->when);

               sprintf (s, "Running for %s seconds, %d bytes transfered to"
                           " him.\n       Last packet ack'ed %s seconds ago"
                           " at %.1f bytes/s.",
                           secstostr(time(NULL)-iptr->when, 1), dcc->recv,
                           secstostr(time(NULL)-iptr->idle, 1), xfertime);
             }

         return (char *)s;
        }

   return "";

 }  /**/

 
 int CheckFileBeforeSending (DCCParams *dcc)

 {
  char buf[LARGBUF];	
  char *p;
  struct stat st;

    if (*dcc->file=='/')
     {
      mstrncpy (buf, dcc->file, LARGBUF);
     }
    else
     {
      getcwd (buf, LARGBUF);
      strcat (buf, "/");
      strcat (buf, dcc->file);
     }

    if (access(buf, R_OK)) 
     {
      say ("%$% Unable to access %s - %s.\n", MILD, buf, strerror(errno));

      return 0;
     }

    if (stat(buf, &st))
     {
      say ("%$% Unable to get stat structure on file %s - %s.\n", 
           MILD, buf, strerror(errno));

      return 0;
     }

    if (st.st_size==0)
     {
      say ("%$% Zero length file.\n", MILD);

      return 0;
     }

    if (S_ISDIR(st.st_mode))
     {
      say ("%$% Directory file.\n", MILD);

      return 0;
     }

    if (S_ISCHR(st.st_mode))
     {
      say ("%$% Character special file.\n", MILD);

      return 0;
     }

    if (S_ISBLK(st.st_mode))
     {
      say ("%$% Block special file.\n", MILD);

      return 0;
     }
 
   dcc->fd=open (buf, O_RDONLY);
    if (dcc->fd<0)
     {
      say ("%$% Couldn't open %s for reading: %s.\n", MILD, 
           dcc->file, strerror(errno));

      return 0;
     } 

   p=strrchr (buf, '/');
   p++;
   strcpy (dcc->file, p);
   dcc->size=st.st_size;

   return 1;

 }  /**/
       

 int CheckFileBeforeGetting (DCCParams *dcc)

 {
  //fetch valueof("upload_dir");
  char f[strlen(dcc->file)+strlen("/tmp/")+2];
  int fd;
  struct stat st;

    if (strpbrk(dcc->file, "\t\n\b\\/?*"))
     {
      say ("%$% File has invalid format: ``%s''\n", MILD, dcc->file);
      
      return 0;
     }

   sprintf (f, "%s%s", "/tmp/", dcc->file);
    
   fd=open (f, O_RDONLY);

    if (fd<=0)
     {
      dcc->fd=open (f, O_WRONLY|O_TRUNC|O_CREAT, 0666);      
   
       if (dcc->fd<=0)
        {
         say ("%$% Unable to setup file for getting - %s.\n", 
              MILD, strerror(errno));

         return 0;
        }
       else
        {
         return 1;
        }
     }
    else
     {
      stat (f, &st);

       if ((st.st_size)==(dcc->size))
        {
         say("%$% You already have this file - Truncating to zero!\n",MILD);
        }
       else   
       if ((st.st_size)<(dcc->size)) 
        {
         say ("%$% You already have this file less %d bytes.\n", MILD, 
              dcc->size-st.st_size);
        } 
       else
        {
         say ("%$% You already have this file %d bytes larger.\n", MILD, 
              st.st_size-dcc->size);         
        }
     }

   dcc->fd=open (f, O_WRONLY|O_TRUNC|O_CREAT, 0666);

    if (dcc->fd<=0)
     {
      say("%$% Unable to setup file for getting - %s.\n",MILD,strerror(errno));

      return 0;
     }
    else
     {
      return 1;
     }

 }  /**/


#define NOT_FOUND()  (i==0)
#define FOUND_MORE() (i>1)
#define FOUND_ONE()  (i==1)

 static __inline__ Index *FindSomeoneDCCOffer (int type, const Someone *pptr)

 {
  const register Index *iptr;
  extern People *const people;
  #define dccptr ((DCCParams *)iptr->pindex)

   switch (type)
          {
           case DCC_CHAT:
                {
                 if (GOT_INDICES(pptr))
                    {
                     for (iptr=pptr->head; iptr!=NULL; iptr=iptr->next)
                         {
                          if ((IS_DCC(iptr))&&
                              (IS_CHATTOUS(dccptr))&&
                              (IS_DCCWAITING(dccptr))&&
                              (!iptr->index)) /* no socket established */
                             {
                              return (Index *)iptr;
                             }
                         }
                    }
  
                 break;
                }

           case DCC_GET:
                {
                 if (GOT_INDICES(pptr))
                    {
                     for (iptr=pptr->head; iptr!=NULL; iptr=iptr->next)
                         {
                          if ((IS_DCC(iptr))&&
                              (IS_GET(dccptr))&&
                              (IS_DCCWAITING(dccptr))&&
                              (!iptr->index))
                             {
                              return (Index *)iptr;
                             }
                         }
                    }

                 break;
                }

           case DCC_SEND:
                {
                 if (GOT_INDICES(pptr))
                    {
                     for (iptr=pptr->head; iptr!=NULL; iptr=iptr->next)
                         {
                          if ((IS_DCC(iptr))&&
                              (IS_SEND(dccptr))&&
                              (IS_DCCWAITING(dccptr))&&
                              (!iptr->index))
                             {
                              return (Index *)iptr;
                             }
                         }
                    }

                 break;
                }
          }

   return (Index *)NULL;

 }  /**/


 static Index *FindDCCOffer (int type, ...)

 {
  register Index *iptr;
  register Someone *pptr;
  extern People *const people;

   switch (type)
          {
           case DCC_CHAT:
                {
                  for (pptr=people->head; pptr!=NULL; pptr=pptr->next)
                      {
                       if (GOT_INDICES(pptr))
                          {
                           for (iptr=pptr->head; iptr!=NULL; iptr=iptr->next)
                               {
                                if (IS_DCC(iptr))
                                   {
                                    if ((IS_CHATTOUS(dccptr))&&
                                        (IS_DCCWAITING(dccptr))&&
                                        (!iptr->index))
                                       {
                                        return (Index *)iptr;
                                       }
                                   }
                               }
                          }
                      }

                 break;
                }

           case DCC_GET:
                {
                 for (pptr=people->head; pptr!=NULL; pptr=pptr->next)
                      {
                       if (GOT_INDICES(pptr))
                          {
                           for (iptr=pptr->head; iptr!=NULL; iptr=iptr->next)
                               {
                                if (IS_DCC(iptr))
                                   {
                                    if ((IS_GET(dccptr))&&
                                        (IS_DCCWAITING(dccptr))&&
                                        (!iptr->index))
                                       {
                                        return (Index *)iptr;
                                       }
                                   }
                               }
                          }
                      } 
                 break;
                }
          }

   return (Index *)NULL;

 }  /**/


 Index *FindDCCIndex (int type, Someone *pptr, ...)

 {
  int i=0;
  va_list va;
  register Index *iptr;
  Index *aux;
  DCCParams *dcc;

   switch (type)
          {
           case DCC_CHAT:
                {
                  if (!pptr)
                     {
                      return (FindDCCOffer(DCC_CHAT)); /* first pending offer */
                     }

                 iptr=pptr->head;

                  for ( ; iptr!=NULL; iptr=iptr->next)
                      {
                        if (IS_DCC(iptr))
                           {
                            dcc=INDEXTODCC(iptr);

                             if ((IS_CHATTOUS(dcc))||(IS_CHATTOHIM(dcc)))
                                {
                                 i++;
                                 aux=iptr;
                                }
                           }
                      }

                  if (FOUND_ONE())
                     {
                      return (Index *)aux;
                     }

                  if (FOUND_MORE())
                     {
                      say ("%$% Multible DCC CHAT to %s!\n", HOT, pptr->name);

                      return (Index *)aux;
                     }
                 
                   if (NOT_FOUND()) 
                     {
                      return (Index *)NULL;
                     }
                }

           case DCC_SEND:
                {
                 char *file;

                  va_start (va, pptr);
                  file=va_arg (va, char *);

                  iptr=pptr->head;

                   for ( ; iptr!=NULL; iptr=iptr->next)
                       {
                         if (IS_DCC(iptr))
                            {
                             dcc=INDEXTODCC(iptr);
 
                              if ((dcc->type&DCCGETTING)&&
                                  (!strcmp(file, dcc->file)))
                                 {
                                  i++;
                                  aux=iptr;
                                 }
                            }
                       }
                
                  if (FOUND_ONE())
                     {
                      return (Index *)aux;
                     }

                  if (FOUND_MORE())
                     {
                      say ("%$% %d DCC SEND connections found to %s with"
                           " the same file name! (%s)\n", 
                            HOT, i, pptr->name, dcc->file);

                      return (Index *)aux;
                     }

                  if (NOT_FOUND())
                     {
                      return (Index *)NULL;
                     }
                }

           case DCC_GET:   
                {
                 char *file=NULL;

                   if (!pptr)
                      {
                       return (FindDCCOffer(DCC_GET)); /*first pending offer*/
                      }

                  va_start (va, pptr);
                  file=va_arg (va, char *);

                   if (!file)
                      {
                       return (FindSomeoneDCCOffer(DCC_GET, pptr));
                      }

                  iptr=pptr->head;

                   for ( ; iptr!=NULL; iptr=iptr->next)
                       {
                         if (IS_DCC(iptr))
                            {
                             dcc=INDEXTODCC(iptr);

                              if ((IS_GET(dcc))&&
                                  (!strcmp(file, dcc->file)))
                                 {
                                  i++;
                                  aux=iptr;
                                 }
                            }
                       }

                  if (FOUND_ONE())
                     {
                      return (Index *)aux;
                     }

                  if (FOUND_MORE())
                     {
                      say ("%$% %d DCC GET connections found to %s with"
                           " the same file name! (%s)\n", 
                            HOT, i, pptr->name, dcc->file);

                      return (Index *)aux;
                     }

                  if (NOT_FOUND())
                     {
                      return (Index *)NULL;
                     }
                }

           default:
                 say ("%$% Unrecognized DCC type specified ``%d''.\n", 
                       MILD, type);
          }

   return (Index *)NULL;

 }  /**/


#define CLOSE_NOTCONNECTEDCHATTOUS() \
 say ("%$% Rejecting 6DCC CHAT offer from %s...\n  %$192%$196%$16 %s\n", CTCPH, pptr->name, PrintDCCState(iptr)); \
 nCTCPToServer (pptr->name, "DCC REJECT CHAT chat"); \
 UpdateSockets (M_CHATS, '-')

#define CLOSE_NOTCONNECTEDCHATTOHIM() \
 say ("%$% Closing 6DCC CHAT offer to %s...\n  %$192%$196%$16 %s\n", CTCPH, pptr->name, PrintDCCState(iptr)); \
 nCTCPToServer (pptr->name, "DCC REJECT CHAT chat"); \
 sptr=INDEXTOSOCKET(iptr); \
 close (sptr->sock); \
 RemovefromSocketsTable (sockets, sptr->sock); \
 UpdateSockets (M_CHATS, '-')

#define CLOSE_CONNECTED_CHAT() \
 say ("%$% Closing 6DCC CHAT connection to %s...\n", CTCPH, pptr->name); \
 sptr=INDEXTOSOCKET(iptr); \
 close (sptr->sock); \
 DisplayDCCSummary (sptr, dcc); \
 RemovefromSocketsTable (sockets, sptr->sock); \
 UpdateSockets (M_CHATS, '-')

#define CLOSE_NOT_CONNECTED_GET() \
 say ("%$% Rejecting 6DCC SEND (6%s %lu) offer from %s...\n  %$192%$196%$16 %s\n", CTCPH, dcc->file, dcc->size, pptr->name,PrintDCCState(iptr)); \
 nCTCPToServer (pptr->name, "DCC REJECT SEND %s", dcc->file); \
 UpdateSockets (M_GETS, '-')

#define CLOSE_CONNECTED_GET() \
 say ("%$% Closing 6DCC GET (6%s %lu) from %s...\n", CTCP, \
      dcc->file, dcc->size, pptr->name); \
 sptr=INDEXTOSOCKET(iptr); \
 close (sptr->sock); \
 DisplayDCCSummary (sptr, dcc); \
 RemovefromSocketsTable (sockets, sptr->sock); \
 UpdateSockets (M_GETS, '-')

#define CLOSE_NOT_CONNECTED_SEND() \
 say ("%$% Closing 6DCC SEND (6%s %lu) to %s...\n", CTCP, dcc->file, dcc->size, pptr->name); \
 nCTCPToServer (pptr->name, "DCC REJECT GET %s", dcc->file); \
 sptr=INDEXTOSOCKET(iptr); \
 close (sptr->sock); \
 DisplayDCCSummary (sptr, dcc); \
 RemovefromSocketsTable (sockets, sptr->sock); \
 UpdateSockets (M_SENDS, '-')

#define CLOSE_CONNECTED_SEND() \
 say ("%$% Closing ^C6DCC SEND (6%s %lu) to %s...\n", CTCP, dcc->file, dcc->size, pptr->name); \
 sptr=INDEXTOSOCKET(iptr); \
 close (sptr->sock); \
 DisplayDCCSummary (sptr, dcc); \
 RemovefromSocketsTable (sockets, sptr->sock); \
 UpdateSockets (M_SENDS, '-')

#define TELL_DCC_TYPE() \
         if (dcc->type&DCCCHATTOUS) \
            { \
              if (dcc->flags&CONNECTED) \
                 { \
                  CLOSE_CONNECTED_CHAT(); \
                 } \
              else \
              if (dcc->flags&AWAITINGCONNECTION) \
                 { \
                  CLOSE_NOTCONNECTEDCHATTOUS(); \
                 } \
            } \
         else \
         if (dcc->type&DCCCHATTOHIM) \
            { \
              if (dcc->flags&CONNECTED) \
                 { \
                  CLOSE_CONNECTED_CHAT(); \
                 } \
              else \
              if (dcc->flags&AWAITINGCONNECTION) \
                 { \
                  CLOSE_NOTCONNECTEDCHATTOHIM(); \
                 } \
            } \
         else \
         if (dcc->type&DCCGETTING) \
            { \
              if (dcc->flags&CONNECTED) \
                 { \
                  CLOSE_CONNECTED_SEND(); \
                 } \
              else \
              if (dcc->flags&AWAITINGCONNECTION) \
                 { \
                  CLOSE_NOT_CONNECTED_SEND(); \
                 } \
            } \
         else \
         if (dcc->type&DCCSENDING) \
            { \
              if (dcc->flags&CONNECTED) \
                 { \
                  CLOSE_CONNECTED_GET(); \
                 } \
              else \
              if (dcc->flags&AWAITINGCONNECTION) \
                 { \
                  CLOSE_NOT_CONNECTED_GET(); \
                 } \
            }

 
  static __inline__ void 
 SelectDCCType (int what, Someone *pptr, Index *iptr, int *n)

 {
  register Socket *sptr;
  extern SocketsTable *const sockets;
  #define dcc  ((DCCParams *)iptr->pindex)

   switch (what)
    {
     case EVERYTHING:
           {
            (*n)++;
            TELL_DCC_TYPE();
            RemoveIndexbyptr (pptr, iptr);

            break;
           }

     case DCC_CHAT:
           {
             if (IS_DCCCONNECTED(dcc))
              {
                if ((IS_CHATTOUS(dcc))||(IS_CHATTOHIM(dcc)))
                 {
                  (*n)++;
                  CLOSE_CONNECTED_CHAT();
                  RemoveIndexbyptr (pptr,iptr);
                 }
              }
             else
             if (IS_DCCWAITING(dcc))
              {
                if (IS_CHATTOUS(dcc))
                 {
                  (*n)++;
                  CLOSE_NOTCONNECTEDCHATTOUS();                                                   RemoveIndexbyptr (pptr,iptr);                                                  }
                else
                if (IS_CHATTOHIM(dcc))
                 {
                  (*n)++;
                  CLOSE_NOTCONNECTEDCHATTOHIM();
                  RemoveIndexbyptr (pptr,iptr);
                 }
              }

            break;
           }

     case DCC_GET:
           {
             if (IS_DCCCONNECTED(dcc))
              {
               (*n)++;
               CLOSE_CONNECTED_GET();
               RemoveIndexbyptr (pptr,iptr);
              }
             else
             if (IS_DCCWAITING(dcc))
              {
               (*n)++;
               CLOSE_NOT_CONNECTED_GET();
               RemoveIndexbyptr (pptr,iptr);
              }

            break;
           }

     case DCC_SEND:
           {
             if (IS_DCCCONNECTED(dcc))
              {
               (*n)++;
               CLOSE_CONNECTED_SEND();
               RemoveIndexbyptr (pptr,iptr);
              }
             else
             if (IS_DCCWAITING(dcc))
              {
               (*n)++;
               CLOSE_NOT_CONNECTED_SEND();
               RemoveIndexbyptr (pptr,iptr);
              }

            break;
           }
    }

 }  /**/


 static __inline__ void CloseAllDCCConnections (const int what)

 {
  int n=0;
  Socket *sptr;
  static Someone *x_pptr;
  register Someone *pptr;
  static Index *x_iptr;
  register Index *iptr;
  extern People *const people;
  extern SocketsTable *const sockets;

   pptr=people->head;

    while (pptr!=NULL)
     {
      x_pptr=pptr->next;

       if (GOT_INDICES(pptr))
        {
         iptr=pptr->head;

          while (iptr!=NULL)
           {
            x_iptr=iptr->next;

             if (IS_DCC(iptr))
              {
               SelectDCCType (what, pptr, iptr, &n);
              }

            iptr=x_iptr;
           }
        }

      pptr=x_pptr;
     }

   say ("%$% Purged %d DCC connection%s\n", CTCP, n, n>1?"s.":".");

 }  /**/


  static __inline__ void
 CloseSomeoneDCCConnections (const int type, Someone *pptr)
 
 {
  int cnt=0;
  register Index *iptr,
                 *x_iptr;

    if (GOT_INDICES(pptr))
     {
      iptr=pptr->head;

       while (iptr!=NULL)
        {
         x_iptr=iptr->next;

          if (IS_DCC(iptr))
           {
            SelectDCCType (type, pptr, iptr, &cnt);
           }

         iptr=x_iptr;
        }
     }

   say ("%$% Purged %d DCC connection%s\n", CTCP, cnt, cnt>1?"s.":".");

 }   /**/


 void CloseDCCConnection (int type, Someone *pptr, ...)

 {
  extern SocketsTable *const sockets;

   switch (type)
          {
           case EVERYTHING:
                {
                  if (!pptr)  
                     {
                      CloseAllDCCConnections (EVERYTHING);
                     }
                  else
                     {
                      CloseSomeoneDCCConnections (EVERYTHING, pptr);
                     }
               
                 break;
                }

           case DCC_CHAT:
                {
                  if (!pptr)  
                     {
                      CloseAllDCCConnections (DCC_CHAT);
                     }
                  else
                     {
                      va_list va;
                      int n=0;
                      Index *iptr=NULL;

                       va_start (va, pptr);
                       iptr=va_arg (va, Index *);
        
                       SelectDCCType (DCC_CHAT, pptr, iptr, &n);
                     }

                 break;
                }

           case DCC_GET:
                {
                  if (!pptr)  
                     {
                      CloseAllDCCConnections (DCC_GET);
                     }
                  else
                     {
                      va_list va;
                      int n=0;
                      Index *iptr=NULL;

                       va_start (va, pptr);
                       iptr=va_arg (va, Index *);

                        if (!iptr)
                           {
                            CloseSomeoneDCCConnections (DCC_GET, pptr);
                           }
                        else
                           {
                            SelectDCCType (DCC_GET, pptr, iptr, &n);
                           }
                     }

                 break;
                }

           case DCC_SEND:
                {
                  if (!pptr)  
                     {
                      CloseAllDCCConnections (DCC_SEND);
                     }
                  else
                     {
                      va_list va;
                      int n=0;
                      Index *iptr=NULL;

                       va_start (va, pptr);
                       iptr=va_arg (va, Index *);

                        if (!iptr)
                           {
                            CloseSomeoneDCCConnections (DCC_SEND, pptr);
                           }
                        else
                           {
                            SelectDCCType (DCC_SEND, pptr, iptr, &n);
                           }
                     }

                 break;
                }
          }

 }  /**/

 #undef dcc


 int CallFiringSquad (Someone *ptr)

 {
  int n=0;
  register Someone *pptr;
  register Index *iptr;
  DCCParams *dcc;
  Someone *p_next;
  Index *i_next;
  extern People *const people;

   if (ptr==(Someone *)NULL)  
      {
       pptr=people->head;

        while (pptr!=NULL)
              {
               p_next=pptr->next;

               iptr=pptr->head;

                while (iptr!=NULL)
                      {
                       i_next=iptr->next;

                        if (IS_DCC(iptr))
                           {
                            dcc=INDEXTODCC(iptr);
                            
                             if (dcc->flags&FIRING_SQUAD)
                                {
                                 FinishoffIndex (iptr);
                                 free (iptr);
                                 pptr->nEntries--;
                                }
                           }

                       iptr=i_next;
                      } 

                if (NOMORE_INDICES(pptr))
                   {
                    RemovefromPeople (pptr->name);
                   }

               pptr=p_next;
              }

        if (people->nEntries==0)
           {
            people->head=people->tail=(Someone *)NULL;
           }
      }
   else
      {
       iptr=ptr->head;

        while (iptr!=NULL)
              {
               i_next=iptr->next;

                if (IS_DCC(iptr))
                   {
                    dcc=INDEXTODCC(iptr);

                     if (dcc->flags&FIRING_SQUAD)
                        {
                         FinishoffIndex (iptr);
                         free (iptr);
                         ptr->nEntries--;
                        }
                   }

               iptr=i_next;
              }

        if (NOMORE_INDICES(ptr))
           {
            RemovefromPeople (ptr->name);

             if (people->nEntries==0)
                {
                 people->head=people->tail=(Someone *)NULL;
                }
           } 
      }

 }  /**/


 void CloseDCCSocket (Socket *sptr, DCCParams *dcc)

 {
  char s[MINIBUF]={0},
       p[MINIBUF]={0};
  register Someone *pptr;
  register Index *iptr;
  extern SocketsTable *const sockets;

   (Index *)iptr=SOCKETTOINDEX(sptr);
   (Someone *)pptr=INDEXTOSOMEONE(iptr);

    if (dcc->type&(DCCCHATTOUS|DCCCHATTOHIM))
       {
         if (dcc->flags&CONNECTED)
            {
             strcpy (s, "ACTIVE DCC CHAT");
            }
         else
         if (dcc->flags&AWAITINGCONNECTION)
            {
             strcpy (s, "INACTIVE DCC CHAT");
            }

        strcpy (p, "DCC REJECT CHAT chat");

        UpdateSockets (M_CHATS, '-');
       }
    else
    if (dcc->type&DCCGETTING)
       {
         if (dcc->flags&CONNECTED)
            {
             strcpy (s, "ACTIVE DCC SEND");
            }
         else
         if (dcc->flags&AWAITINGCONNECTION)
            {
             strcpy (s, "INACTIVE DCC SEND");
            }

        sprintf (p, "DCC REJECT GET %s", dcc->file);

        UpdateSockets (M_SENDS, '-');
       }
    else
    if (dcc->type&DCCSENDING)
       {
         if (dcc->flags&CONNECTED)
            {
             strcpy (s, "ACTIVE DCC GET");
            }
         else
         if (dcc->flags&AWAITINGCONNECTION)
            {
             strcpy (s, "INACTIVE DCC GET");
            }

        sprintf (p, "DCC REJECT SEND %s", dcc->file);

        UpdateSockets (M_GETS, '-');
       }

  say ("%$% Now closing socket %d of type %s...\n", 
         INFO, sptr->sock, s);

  close (sptr->sock);
   if (p[0])
      {
       nCTCPToServer (pptr->name, "%s", p);
      }

  DisplayDCCSummary (sptr, dcc);

  RemoveIndexbyptr (pptr, iptr);
  RemovefromSocketsTable (sockets, sptr->sock);
  
  Flash (6, 0);

 }  /**/


 void CheckIdle_TimeoutDCC (void)

 {
  int x,
      i=0;
  register Someone *pptr;
  register Index *iptr;
  DCCParams *dcc;
  extern People *const people;

   x=atoi((char *)valueof("DCC_TIMEOUT"));
    if (x==0)
       {
        return;
       }

   pptr=people->head;

    for ( ; pptr!=NULL; pptr=pptr->next)
        {
         i++;
          if (pptr->nEntries>=1)
             {
              iptr=pptr->head;

                for ( ; iptr!=NULL; iptr=iptr->next)
                    {
                      if (iptr->protocol==PROTOCOL_DCC)
                         {
                          (DCCParams *)dcc=INDEXTODCC(iptr);

                           if (dcc->flags&AWAITINGCONNECTION)
                              {
                                if (dcc->type&DCCCHATTOHIM)
                                   {
                                     if ((time(NULL)-iptr->when)>=x)
                                        {
                                         say ("%$% %s %s\n", INFO, "AUTO-CLOSING unhonoured DCC CHAT offer to", pptr->name);
                                         ToServer ("NOTICE %s :%s\n", 
                                          pptr->name,
                                          "DCC CHAT offer withdrawn.");
                                         nCTCPToServer (pptr->name, "%s",
                                          "DCC CHAT REJECT chat"); 
                                        }
                                   }
                                else
                                if ((dcc->type&DCCGETTING)&&
                                    (time(NULL)-iptr->when>=x))
                                   {
                                    say ("%$% %s %s to %s\n", INFO, 
                                     "AUTO-CLOSING unhonoured DCC SEND",
                                     dcc->file, pptr->name);
                                    ToServer ("NOTICE %s :%s\n", pptr->name,
                                     "DCC SEND offer withdrawn.");
                                    nCTCPToServer (pptr->name, "%s %s",
                                     "DCC REJECT GET", dcc->file);
                                   }
                                else
                                if ((dcc->type&DCCCHATTOHIM)&&
                                    ((time(NULL)-iptr->when)>=60))
                                   {
                                    say ("%s %s's DCC CHAT offer.\n", INFO,
                                     "You may want to answer", pptr->name); 
                                   }
                              }
                         }
                    }
             }
        }

 }  /**/

 
 void DisplayDCCSummary (Socket *sptr, DCCParams *dcc)

 {
 /*
  char t[10]={0},
       s[10]={0},
       file[MEDIUMBUF]={0};
  register Someone *pptr;
  register Index *iptr;

  (Index *)iptr=SOCKETTOINDEX(sptr);
  (Someone *)pptr=INDEXTOSOMEONE(iptr);
  
  DisplayHeader (10, 10);
 
  say (" %$179%-3.3d%$179%-13.13s%$179%-3.2d%$179 
  space; vertical;
  mwprintf ("%-3.3d", sptr->sock); vertical;
  mwprintf ("%-13.13s", pptr->name); vertical;
  mwprintf ("%-3.2d", pptr->nEntries-1); vertical;

   {
     if (dcc->type&DCCCHATTOHIM)
        {
         strcpy (s, "CHAT");
         file[0]=-1;
        }
     else
     if (dcc->type&DCCCHATTOUS)
        {
         strcpy (s, "CHAT");
         file[0]=-2;
        }
     else
     if (dcc->type&DCCSENDING)
        {
         strcpy (s, "GET ");
         strcpy (file, dcc->file);
        }
     else
     if (dcc->type&DCCGETTING)
        {
         strcpy (s, "SEND");
         strcpy (file, dcc->file);
        } 
   }
  mwprintf (s);
  (file[0]==-1)?({rhead;1;}):((file[0]==-2)?
  ({lhead;1;}):({space;1;})); vertical;

  strftime (t, 10, "%H:%M", localtime(&iptr->when));
  mwprintf (t); vertical;

  mwprintf ("%-8.8s", secstostr(time(NULL)-iptr->when, 1)); vertical;

  mwprintf ("%-4.4d", dcc->xfer/1024); vertical;
  mwprintf ("%-4.4d", dcc->recv/1024); vertical;

  (file[0]>0)?
  ({mwprintf("%-11.11s", file);vertical;1;}):
  ({space4;space;null;space4;space;vertical;1;});

   if (dcc->type&(DCCGETTING|DCCSENDING))
      {
        if (dcc->size>1024)
           {
            mwprintf ("%-5dkb", dcc->size/1024); 
           }
        else
           {
            mwprintf ("%-5db ", dcc->size);
           }
       vertical;
      }
   else
      {
       space2;space; null;
       space2; space; vertical;
      }

  newline;

  finish2 (10, 10);

  */
 }  /**/ 
  

 #define PROGRESS_ if ((n>0.0)&&(n<=20.0)) \
                        z=DCC_PRO10; \
                     else \
                     if ((n>20.0)&&(n<=40.4)) \
                        z=DCC_PRO20; \
                     else \
                     if ((n>40.0)&&(n<=60.0)) \
                        z=DCC_PRO30; \
                     else \
                     if ((n>60.0)&&(n<=80.0)) \
                        z=DCC_PRO40; \
                     else \
                     if ((n>80.0)&&(n<=100.0)) \
                        z=DCC_PRO50; \
                     else \
                        z=DCC_PRO00
                    
 #define indicator waddch_ (MainWin, 0xAE|A_ALTCHARSET|COLOR_PAIR(GREEN)|A_BOLD)

 int ShowDCCConnections (void)

 {
  register int i=0;
  char t[10]={0},
       p[10]={0},
       s[10]={0},
       file[100]={0};
  unsigned long DCC_PRO00[]={[0 ... 4]=TIN, [5]=0},
                DCC_PRO10[]={[0]=TINB|A_BLINK, [1 ... 4]=TIN, [5]=0},
                DCC_PRO20[]={[0]=TINB, [1]=TINB|A_BLINK, [2 ... 4]=TIN, [5]=0},
                DCC_PRO30[]={[0 ... 1]=TINB, [2]=TINB|A_BLINK, [3 ... 4]=TIN, [5]=0},
                DCC_PRO40[]={[0 ... 2]=TINB, [3]=TINB|A_BLINK, [4]=TIN, [5]=0},
                DCC_PRO50[]={[0 ... 3]=TINB, [4]=TINB|A_BLINK, [5]=0};
  register unsigned long *z=0;
  register Someone *pptr;
  register Index *iptr;
  DCCParams *dcc;
  #define TOUS 17|A_ALTCHARSET|COLOR_PAIR(CYAN)|A_BLINK
  #define TOHIM 16|A_ALTCHARSET|COLOR_PAIR(CYAN)|A_BLINK
  #define sptr ((Socket *)(iptr->index))
  extern People *const people;

  #define CHAT_CON \
   " %$179%-3.3d%$179%-3.3d %$179%-13.13s%$179%-4.4s %$%%$179%-5.5s%$179%-6.6s%$179     %$237     %$179     %$237     %$179\n"
  #define CHAT_CONARGS \
   i, sptr->sock, pptr->name, "CHAT", (dcc->type&DCCCHATTOUS)?(TOUS):(TOHIM), t, p
  #define CHAT_NOTCONARGS \
   i, pptr->name, "CHAT", (dcc->type&DCCCHATTOUS)?(TOUS):(TOHIM), t, p
  #define CHAT_NOTCON \
  " %$179%-3.3d%$179 %$237  %$179%-13.13s%$179%-4.4s %$%%$179%-5.5s%$179%-6.6s%$179     %$237     %$179     %$237     %$179\n"

  #define SEND_ \
" %$179%-3.3d%$179%-3.3d %$179%-13.13s%$179%-4.4s  %$179%-5.5s%$179%-6.6s%$179%-11.11s%$179%z %-5.5s%$179\n"
  #define SEND_ARGS \
 i, sptr->sock, pptr->name, "SEND", t, p, dcc->file, z, d

  #define GET_CON \
 " %$179%-3.3d%$179%-3.3d %$179%-13.13s%$179%-4.4s  %$179%-5.5s%$179%-6.6s%$179%-11.11s%$179%z %-5.5s%$179\n"
  #define GET_ARGS \
 i, sptr->sock, pptr->name, "GET", t, p, dcc->file, z, d

 #define GET_NOTCON \
 " %$179%-3.3d%$179 %$237  %$179%-13.13s%$179%-4.4s  %$179%-5.5s%$179%-6.6s%$179%-11.11s%$179%z %-5.5s%$179\n"
  #define GETNOTCON_ARGS \
 i,  pptr->name, "GET", t, p, dcc->file, z, d



    if ((!mt_ptr->nChats)&&(!mt_ptr->nSends)&&(!mt_ptr->nGets))
       {
        say ("%$% Not a single DCC connection.\n", INFO);

        return 0; 
       }

   DisplayHeader (7, 8);

   pptr=people->head;

    for ( ; pptr!=NULL; pptr=pptr->next)
        {
          if (pptr->nEntries>=1)
             {
              iptr=pptr->head;

               for ( ; iptr!=NULL; iptr=iptr->next)
                   {
                     if (iptr->protocol==PROTOCOL_DCC)
                        {
                         dcc=iptr->pindex;

                         i++;
                         strftime (t, 10, "%H:%M", localtime(&iptr->when));

                          if (dcc->flags&AWAITINGCONNECTION)
                             strcpy (p, "WAIT  ");
                          else
                          if (dcc->flags&CONNECTED)
                             strcpy (p, "ACTIVE");

                          if ((dcc->type&DCCCHATTOHIM)||(dcc->type&DCCCHATTOUS))
                             {
                               if (sptr)
                                  {
                                   say (CHAT_CON, CHAT_CONARGS);
                                  }
                               else
                                  {
                                   say (CHAT_NOTCON, CHAT_NOTCONARGS);
                                  }
                             }
                          else
                          if (dcc->type&DCCGETTING)
                             {
                              double n=0;
                              char d[10];

                               n=((double)dcc->xfer/(double)dcc->size)*100.0;
                               sprintf (d, "%.1f%%", n);
                               PROGRESS_;
                               say (SEND_, SEND_ARGS);
                             }
                          else
                          if (dcc->type&DCCSENDING)
                             {
                              double n=0;
                              char d[10];

                               n=((double)dcc->recv/(double)dcc->size)*100.0;
                               sprintf (d, "%.1f%%", n);
                               PROGRESS_;

                                if (dcc->flags&AWAITINGCONNECTION)
                                   {
                                    say (GET_NOTCON, GETNOTCON_ARGS);
                                   } 
                                else
                                if (dcc->flags&CONNECTED)
                                   {
                                    say (GET_CON, GET_ARGS);
                                   }
                              }

                          if ((!pptr->next)&&(!iptr->next))
                             {
                              finish2(7, 8);
                             }
                          else
                             {
                              seperator(7, 8);
                             }
                        }
                   }
             }
        }

   return i;

 }  /**/

 #undef sptr


 static void ScrollingDCCEntries (ScrollingList *);
 static void ScrollingDCCEnter (void *, int);
 static void ScrollingDCCInit (ScrollingList *);

 #include <dcc-data.h>

 void ScrollingDCCConnections (void *ptr, int idx)

 {
  List *lptr;
  ScrollingList *scrptr=&dcc_list;

    if ((mt_ptr->nChats+mt_ptr->nGets+mt_ptr->nSends)==0)
     {
      say ("%$% Not a single DCC connection!\n", MILD);

      return;
     }

   lptr=GetDCCListptr ();

   scrptr->nEntries=lptr->nEntries;
   (List *)scrptr->list=lptr;
   ((ScrollingList *)(scrptr->opt_menu))->parent=scrptr;

   lptr->glock=0;
   scrptr->lock=&lptr->glock;

   LoadScrollingList (scrptr);

 }  /**/


 static void ScrollingDCCEntries (ScrollingList *sptr)

 {
  register int j=0;
  int maxy,
      last;
  register ListEntry *eptr;
  char s[sptr->width],
       t[6],
       file[LARGBUF],
       u[10];
  register Someone *pptr;
  register Index *iptr;
  WINDOW *w=(WINDOW *)RetrieveCurrentWindow();
  #define dcc ((DCCParams *)eptr->whatever)

   getmaxyx (w, maxy, last);
   last=maxy+sptr->listwin_start;

    for (eptr=((List *)sptr->list)->head; 
         (eptr!=NULL)&&(j++!=sptr->listwin_start); 
         eptr=eptr->next)
        ;
        
    for (--j; 
         (j<last)&&(j<((List *)sptr->list)->nEntries);
         j++, eptr=eptr->next)
        {
          if (j==sptr->hilited)
             {
              _wattron (w, A_REVERSE);
              sptr->c_item=eptr->whatever;
             }

         _wmove (w, j-sptr->listwin_start, 0);

         iptr=PROTOCOLTOINDEX((DCCParams *)eptr->whatever);
         pptr=INDEXTOSOMEONE(iptr);

          if (dcc->flags&AWAITINGCONNECTION)
              strcpy (u, "WAIT");
          else
          if (dcc->flags&CONNECTED)
             strcpy (u, "ACTIVE");

          if (dcc->type&DCCCHATTOHIM)
             {
              strcpy (t, "CHAT");
              file[0]=-1;
             }
          else
          if (dcc->type&DCCCHATTOUS)
             {
              strcpy (t, "CHAT");
              file[0]=-2;
             }
          else
          if (dcc->type&DCCSENDING)
             {
              strcpy (t, "GET ");
              strcpy (file, dcc->file);
             }
          else
          if (dcc->type&DCCGETTING)
             {
              strcpy (t, "SEND");
              strcpy (file, dcc->file);
             }

         wmove (w, j-sptr->listwin_start, 0);
         Fwprintf (w, " %-10.10s%$179%-4.4s%$179%-6.6s%$179%-20.20s%$179%-9s",
                   pptr->name, t, u,
                   file[0]>0?file:"        -         ", " ");

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

#undef dcc


 void ScrollingDCCInit (ScrollingList *scrptr)

 {
  #define dcc ((DCCParams *)scrptr->c_item)
  #define iptr ((Index *)dcc->index)
  #define sptr ((Socket *)iptr->index)

    if (IS_DCCWAITING(dcc)&&(IS_CHATTOUS(dcc)||IS_GET(dcc))) scrptr->attrs[0]=1;
    else scrptr->attrs[0]=0;

 }  /**/

#undef sptr
#undef iptr
#undef dcc


 static void ScrollingDCCEnter (void *_dccptr, int idx)

 {
  char s[LARGBUF];
  register Index *iptr;
  register Someone *pptr;
  extern uDCC (char *);
  #define dcc ((DCCParams *)_dccptr)

   iptr=PROTOCOLTOINDEX(dcc);
   pptr=INDEXTOSOMEONE(iptr);

    switch (idx)
           {
            case 0:
                  if (IS_CHATTOUS(dcc))
                     {
                      sprintf (s, "CHAT %s", pptr->name);
                     }
                  else
                  if (IS_GET(dcc))
                     {
                      sprintf (s, "GET %s %s", pptr->name, dcc->file);
                     }

                 uDCC (s);
                 break;

            case 1:
                  if (IS_CHAT(dcc))
                     {
                      sprintf (s, "CLOSE CHAT %s", pptr->name);
                     }
                  else
                  if (IS_GET(dcc))
                     {
                      sprintf (s, "CLOSE GET %s %s", pptr->name, dcc->file);
                     }
                  else
                  if (IS_SEND(dcc))
                     {
                      sprintf (s, "CLOSE SEND %s %s", pptr->name, dcc->file);
                     }

                 uDCC (s);
                 break;

            case 2:
                 break;
           }  

 }  /**/

