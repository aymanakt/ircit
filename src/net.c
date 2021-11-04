/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <main.h>
#include <terminal.h>
#include <list.h>
#include <scroll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netdb.h>
#ifdef _HAVE_PTHREAD
 #include <pthread.h>
#endif
#include <misc.h>
#include <sockets.h>
#include <people.h>
#include <updates.h>
#include <prefs.h>
#include <thread.h>
#include <net.h>

 MODULEID("$Id: net.c,v 1.8 1998/04/06 06:13:44 ayman Beta $");

 struct resolved result;
 struct resolved *const r_ptr=&result;
 static f=1;

 void InitNet (void)

 {
   FetchLocalhost ();
   InitIPC ();
   InitTelnet ();

 }  /**/


#include <sys/stat.h>

 void InitIPC (void)

 {
  int len,
      n,
      m;
  char *ipc_path;
  struct sockaddr_un ircit_addr;
  struct Socket *sptr;
  extern Someone *ipc_ptr;
  extern SocketsTable *const sockets;
  extern int IPCAccept (Socket *);

    if (!(ipc_path=valueof("IPC_PATH")))
       {
        say ("%$% IPC_PATH is not set.\n", HOT);
        mt_ptr->s_attrs|=S_NOIPC;

        return;
       }

    if ((n=socket(AF_UNIX, SOCK_STREAM, 0))<0)
       {
        say ("%$% Unable to obtain socket for IPC server - %s.\n", 
             HOT, strerror(errno));
        mt_ptr->s_attrs|=S_NOIPC;

        return;
       }

   SetSocketFlags (n, 1, O_NONBLOCK);

   memset (&ircit_addr, 0, sizeof(ircit_addr));

   sprintf (ircit_addr.sun_path, "%s-%d", ipc_path, 
            (mt_ptr->idx=AllRightsReserved()));

   unlink (ircit_addr.sun_path);
   ircit_addr.sun_family=AF_UNIX;
   len=strlen(ircit_addr.sun_path)+sizeof(ircit_addr.sun_family);

    if (bind(n, (struct sockaddr *)&ircit_addr, len)<0)
       {
        say ("%$% Unable to bind IPC server - %s.\n", HOT, strerror(errno));

        close (n);
        mt_ptr->s_attrs|=S_NOIPC;

        return;
       }

    if (chmod(ircit_addr.sun_path, S_IRWXU)<0)
       {
        say ("%$% Unable to set up IPC server - %s.\n", HOT, strerror(errno));

        close (n);
        unlink (ircit_addr.sun_path);
        mt_ptr->s_attrs|=S_NOIPC;

        return;
       }

    if (listen (n, 5)<0)
       {
        say ("%$% IPC server wont listen - %s.\n", HOT, strerror(errno));

        close (n);
        unlink (ircit_addr.sun_path);
        mt_ptr->s_attrs|=S_NOIPC;

        return;
       }

   sptr=AddtoSocketsTable (sockets, n);

   sptr->when=time (NULL);
   sptr->port=0;
   sptr->type=SOCK_IPC;
   sptr->read_func=IPCAccept;
   sptr->close_func=IPCSERVER_Close;
   strcpy (sptr->address, ircit_addr.sun_path);

    if ((ipc_ptr=AddtoPeople ("__IPC__")))
       {
        Index *iptr;

         ipc_ptr->when=time (NULL);

         iptr=AddtoSomeoneIndex (ipc_ptr); //dummy link for server

         iptr->protocol=PROTOCOL_IPC;

         say ("%$% Accepting IPC requests on %s...\n",INFO,ircit_addr.sun_path);
       }

 }  /**/


 int IPCAccept (Socket *ptr)

 {
  int fd,
      len,
      found=0;
  Socket *sptr;
  register Index *iptr;
  struct stat st;
  struct sockaddr_un addr;
  extern Someone *ipc_ptr;
  extern SocketsTable *const sockets;
  #define lpath ((struct thread_params *)iptr->pindex)->lpath

   /* log (1,  "incoming..ipc"); */
   say ("%$% Proccesing incoming IPC connection...\n", INFO);

   len=sizeof(addr);

    if ((fd=accept(ptr->sock, (struct sockaddr *)&addr, &len))<0)
     {
      /* log (1, "unable to accept %s", strerror(errno)); */
      say ("%$% Unable to accept IPC connection - %s.\n ", HOT, 
           strerror(errno));

      return;
     }

   len-=sizeof(addr.sun_family);
   addr.sun_path[len]=0;

   //sprintf (path, "%s/.ircit/knock-knock", ut_ptr->homedir);

//#ifdef 0
     if (arethesame(strrchr(addr.sun_path, '/'), "/knock-knock"))//used to work
     //if (arethesame(path, addr.sun_path))
        {
         int n, fd2;
         char s[20];
         struct param {
                pid_t pgrp,
                      pid;
                char tty[20];
              };
         struct param x;

           if ((n=read(fd, &x, sizeof(struct param))))
              {
               s[n]=0;

               close (1);
               close (2);

               if  ((fd2=open(x.tty, O_RDWR))<0)
                {
                 /*log (1, "unable to open term dev - %s.", strerror(errno));*/

                 close (fd);

                 return;
                }

                if (fd2!=0)
                   {
                    dup2 (fd2, 0);
                    }

                cbreak();
                curs_set (0);


                dup2 (0, 1);
                dup2 (0, 2);
                mt_ptr->c_output&=~BACKGROUND;

                touchwin (curscr);
                wrefresh (curscr);

                x.pid=getpid();
                x.pgrp=getpgrp();
                strcpy (x.tty, ttyname(1));

                n=write (fd, &x, sizeof(struct param));

                say ("%$% Thanks for that!\n", INFO);

                return;
              }
           else
              {
               return;
              }
        }

     for (iptr=ipc_ptr->head; iptr!=NULL; iptr=iptr->next)
         {
           if (!iptr->pindex) continue;

           if (arethesame(lpath, addr.sun_path))
              {
               found=1;
               break;
              }
         }

    if (!found)
       {
        say ("%$% Unauthorized IPC connection %s\n", HOT, addr.sun_path);

        close (fd);
        return;
       }

    if (stat(addr.sun_path, &st)<0)
       {
        say ("%$% Unable to proceed with accepting IPC connection %s - %s.\n",
              HOT, addr.sun_path, strerror(errno));

        close (fd);
        RemoveIndexbyptr (ipc_ptr, iptr);

        return;
       }

    if (S_ISSOCK(st.st_mode)==0)
       {
        say ("%$% IPC connection by %s wasn't configured properly.\n", HOT,
             addr.sun_path);

        close (fd);

        RemoveIndexbyptr (ipc_ptr, iptr);

        return;
       }

   sptr=AddtoSocketsTable (sockets, fd);

   sptr->when=time (NULL);
   sptr->port=0;
   sptr->type=SOCK_IPCCLIENT;
   sptr->read_func=IPCCLIENT_Read;
   sptr->close_func=IPCCLIENT_Close;
   strcpy (sptr->haddress, addr.sun_path);
   strcpy (sptr->address, valueof("IPC_PATH"));
   sptr->index=iptr;
   iptr->index=sptr;

   unlink (addr.sun_path);

   return fd;

 }  /**/


 void FetchLocalhost (void)

 {
  int x;
  char p[MAXHOSTLEN]={0},
       *s;
  UserdataTable *uptr;
  struct hostent *hp;
  struct in_addr in;

   x=gethostname (p, sizeof(p));

    if (x==-1)
       {
        s=(char *)getenv ("HOSTNAME");

         if (!s)
            {
             say ("%$% %s %s\n", HOT,
                  "Unable to determine localhost, even tried $HOSTNAME!",
                  "Trying $IRCHOST...");
             
             s=(char *)getenv ("IRCHOST");
              if (!s)
                 {
                  say ("%$% $IRCHOST failed! trying $IRC_HOST...\n", HOT);

                  s=(char *)getenv ("IRC_HOST");
                   if (!s)
                      {
                       say ("%$% %s %s\n", HOT,
                        "Someting seriously wrong with this machine or",
                        "with you or with both of you!");
                       say ("%$% %s %s\n", HOT,
                        "Host self-lookup failed. It's highly unlikely",
                        "that you'll be able to connect to sensible server.");
                       say ("  %s %s\n",
                        "Provided that you know what you up to, use /lookup <",
                        "interface> to choose different network device.");
                       
                       ut_ptr->localhost=strcpyd("stuffed-machine.org");
                       ut_ptr->localip=(unsigned long)0;
                      }
                 }
            }
       }
    else
       {
        if (hp=gethostbyname(p))
           {
            memcpy (&in, hp->h_addr, sizeof(in));
            ut_ptr->localhost=strdup((char *)hp->h_name);
            ut_ptr->localip=ntohl (in.s_addr);
           }
        else
           {
            say ("%$% %s %s\n", HOT,
                   "Someting seriously wrong with this machine or",
                   "with you or with both of you!");

            ut_ptr->localhost=strdup("stuffed-machine.org");
            ut_ptr->localip=(unsigned long)0;
           }
       }

   say ("%$% Local net %s %s (%lu)\n", INFO,
         localhost(), RawIPToDotted(localip()), localip());

 }  /**/


 int PlaySound (int idx)

 {
  int fd,
      len;
  char req[100];
  struct sockaddr_un addr;

    if ((fd=socket(AF_UNIX, SOCK_STREAM, 0))<0)
       {
        return;
       }

   memset (&addr, 0, sizeof(addr));

   addr.sun_family=AF_UNIX;
   strcpy (addr.sun_path, "/home/ayman/.ircit/sound.req");
   len=strlen(addr.sun_path)+sizeof(addr.sun_family);

   unlink (addr.sun_path);

    if (bind(fd, (struct sockaddr *)&addr, len)<0)
       {
        close (fd);

        return;
       }

    if (chmod(addr.sun_path, S_IRWXU)<0)
       {
        close (fd);

        return;
       }

   memset (&addr, 0, sizeof(addr));

   addr.sun_family=AF_UNIX;
   strcpy (addr.sun_path, "/home/ayman/.ircit/sound");
   len=strlen(addr.sun_path)+sizeof(addr.sun_family);

    if (connect(fd, (struct sockaddr *)&addr, len)<0)
       {
        close (fd);

        return;
       }

   sprintf (req, "%d", 1);

    if (write(fd, req, strlen(req))<0)
       {
        say ("%$% sound request failed.\n", HOT);
       }
    
   close (fd); 

 }  /**/


 int LookupInterface (const char *interface)

 {
  int fd;
  struct ifconf ifc;
  struct ifreq ifr;
  struct sockaddr sa;

    if ((fd=socket(AF_INET, SOCK_DGRAM, 0))<0)
       {
        say ("%$% Interface look-up failed - fd<0\n", MILD);

        return 0;
       }

   strcpy (ifr.ifr_name, interface);

    if (ioctl(fd, SIOCGIFADDR, &ifr)<0)
       {
        say ("%$% Unable to lookup interface %s - %s.\n", MILD,
              interface, strerror(errno));
        close (fd);

        return 0;
       }
    else
       {
        memset (&sa, 0, sizeof(sa));
        sa=ifr.ifr_addr;
       }

    if (&sa)
       {
         if (sa.sa_family==AF_INET)
            {
             unsigned long ad;
             struct sockaddr_in *sin;

              sin=(struct sockaddr_in *)&sa;
              ut_ptr->localip=ntohl (sin->sin_addr.s_addr);
              ad=(unsigned long)sin->sin_addr.s_addr; /* network-byte-order */

              say ("%$% %s is registered with IP address of %d.%d.%d.%d\n",
                    INFO, interface,
                    (int)(ad&0xFF),
                    (int)((ad>>8)&0xFF),
                    (int)((ad>>16)&0xFF),
                    (int)((ad>>24)&0xFF));
             close (fd);
            }
         else
            {
             say ("%$% %s doesn't talk TCP/IP.\n", MILD, interface);

             close (fd);
             return 0;
            }
         }

 }  /**/


 int InitTelnet (void)

 {
  extern struct UserData data;

    if (!set("ALLOW_TELNET"))
       {
        say ("%$% Telnet service is currently disabled.\n", MILD);
       }
    else
       {
        int telnet_sock,
            reuse=1;
        unsigned long port=2500;
        time_t now;
        struct Socket *s;
        struct sockaddr_in telnet;
        extern SocketsTable *const sockets;
        extern int AnswerTelnetRequest (Socket *);

          if (!(port=atoi(valueof("DEFAULT_TELNET_PORT"))))
             {
              say ("%$% Unable to initiate Telnet service - port undefined.\n",
                   HOT);
              return;
             }
 
         memset ((char *)&telnet, 0, sizeof(telnet));
         telnet.sin_family=AF_INET;
         telnet.sin_port=htons (port); 
         telnet.sin_addr.s_addr=htonl (INADDR_ANY);

         telnet_sock=RequestSocket ();

         setsockopt(telnet_sock, SOL_SOCKET, SO_REUSEADDR,&reuse,sizeof(reuse));

          while (bind(telnet_sock, (struct sockaddr *)&telnet,sizeof(telnet))<0)
                {
                  if (errno==EADDRINUSE)
                     {
                      say ("%$% Port %u in use - Incrementing port number...\n",                            MILD,  port);

                      port+=10;
                      telnet.sin_port=htons (port);
                     }
                  else
                     {
                      say ("%$% Unable to setup Telnet service - %s.\n", HOT, 
                           strerror(errno));

                      close (telnet_sock);

                      return 0;
                     }
                }  

          if ((listen(telnet_sock, 1)<0)&&(errno!=EINTR))
             {
              say ("%$% Unable to setup telnet service - %s.\n", HOT, 
                   strerror(errno));

              close (telnet_sock);

              return 0;
             }

         s=AddtoSocketsTable(sockets, telnet_sock);
         mt_ptr->telnetsock=telnet_sock;
         s->when=time (NULL);
         s->port=ntohs(telnet.sin_port);
         s->type=SOCK_LISTENINGTELNET;
         s->read_func=AnswerTelnetRequest;
         s->close_func=LISTENINGTELNET_Close;
         strcpy (s->address, "localhost");

         mt_ptr->telnetport=data.local.telnet_port=ntohs (telnet.sin_port);
         say ("%$% Now listening on port %u\n", INFO, telnetport());

         Flash (2, 1);

         return telnet_sock;
        }

 }  /**/


 void InitUDP (void)

 {
  int udp_sock;
  time_t now;
  struct Socket *s;
  struct sockaddr_in udp;

  memset ((char *)&udp, 0, sizeof(udp));
  udp.sin_family=AF_INET;
  udp.sin_port=htons (8123);
  udp.sin_addr.s_addr=htonl (INADDR_ANY);

   if ((udp_sock=socket(AF_INET, SOCK_DGRAM, 0))<0)
      {
       say ("%$% Oopsz...socket failed on udp...", HOT);

       return;
      }

   if (bind(udp_sock, (struct sockaddr *)&udp, sizeof(udp))<0)
      {
       close (udp_sock);

       say ("%$% Oopsz...bind failed on udp.\n", HOT);

       return;
      }

 }  /**/


 int Reconnect (char *server, int flag)

 {
  register int i=1;
           int j;
  #if 0
    if (!server)
       {
         if (flag&MODE1)
            {
             j=AttemptConnection (NULL, ptr->c_serverid, 0);
              if (j>0)
                 return j;
              else
                  {
                    for (i=1; i<=HowmanyServers(); i++)
                        {
                         j=AttemptConnection (NULL, i, 0);
                          if (j>0)
                             {
                              return j;
                             }
                        }
                   return j;
                  }
            }
       } 
  #endif
 }  /**/  


 #include <dnscache.h>

 int ResolveAddress (const char *address, ResolvedAddress *raddr)

 {
  register DNSCacheEntry *dnsptr;
  unsigned long raw;
  struct hostent *hp;
  static struct in_addr saddr;
  struct in_addr *saddrr=NULL; 
  extern int h_error;

    if (!(dnsptr=FindDNSEntry(address, NULL)))
       {
         if (isdottedquad(address))
            {
              if (inet_aton(address, &saddr)==0)
                 {
                  say ("%$% Unable to resolve %s - %s.\n", MILD,
                         address, h_strerror(h_errno));

                  return 0;
                 }

             hp=gethostbyaddr ((char *)&saddr, sizeof(saddr), AF_INET);

              if (hp==(struct hostent *)NULL)
                 {
                  say ("%$% Unable to resolve %s - %s.\n", MILD,
                         address, h_strerror(h_errno));

                  return 0;
                 }
            }
         else 
            {
             hp=gethostbyname (address);

              if (hp==(struct hostent *)NULL)
                 {
                  say ("%$% Unable to resolve %s - %s.\n", MILD,
                         address, h_strerror(h_errno));

                  return 0;
                 }
            }

        memcpy ((void *)&saddr, (void *)hp->h_addr, sizeof(saddr)); /*use own*/
        raw=ntohl (saddr.s_addr);

        dnsptr=AddDNSCacheEntry (saddr, 
                                 hp->h_name,
                                 RawIPToDotted(raw));
       }

   raddr->dns=dnsptr->dns;
   raddr->dotted=dnsptr->dotted;
   raddr->inetaddr=&dnsptr->saddr;

   return 1;

 }  /**/


 void nslookup (char *addr)

 {
  extern int h_errno;

    if (isdottedquad(addr))
       {
        struct sockaddr_in saddr;
        struct hostent *hp;

         saddr.sin_addr.s_addr=inet_addr (addr);

         hp=gethostbyaddr ((char *)&saddr.sin_addr, sizeof(saddr.sin_addr), AF_INET);
          if (!hp)
             {
              say ("%$% Unable to resolve address %s - %s.\n", MILD,
                    addr, h_strerror(h_errno));
             }
          else
             {
              say ("%$% Address lookup result:\n"
                   "  %$192%$196%$16 DOTTED IP: %s\n"
                   "  %$192%$196%$16 DNS      : %s\n"
                   "  %$192%$196%$16 RAW IP   : %lu\n", INFO, 
                   addr, hp->h_name, ntohl(saddr.sin_addr.s_addr));
             }
       }
    else
       {
        unsigned long ip;
        struct hostent *hp;
        struct in_addr saddr;

          if ((hp=gethostbyname(addr))!=NULL)
             {
              memcpy ((void *)&saddr, (void *)hp->h_addr, sizeof(saddr));

              ip=ntohl (saddr.s_addr);

              say ("%$% Address lookup result:\n"
                   "  %$192%$196%$16 DOTTED IP: %s\n"
                   "  %$192%$196%$16 DNS      : %s\n"
                   "  %$192%$196%$16 RAW IP   : %lu\n", INFO, 
                   RawIPToDotted(ip), hp->h_name, ip);
             }
          else
             {
              say ("%$% Unable to resolve address %s - %s.\n", MILD,
                    addr, h_strerror(h_errno));
             }
       }

 }  /**/


 int RequestSocket (void)

 {
  int nsocket;
  
    if ((nsocket=socket(AF_INET, SOCK_STREAM, 0))<0)
       {
        say ("%$% Unable to obtain socket -  %s.\n", HOT, strerror(errno));

        return 0;
       }

    return nsocket;

 }  /**/


 int SetSocketFlags (int socket, int ON_OFF, int flags)

 {
  int cur;

     if ((cur=fcntl(socket, F_GETFL, 0))<0)
        {
         say ("%$% Unable to F_GETFL socket - %s.\n", HOT, strerror(errno));

         return 0;
        }

   switch (ON_OFF)
          {
           case 1:
                cur|=flags;
                break;

           case 0:
                cur&=~flags;
                break;
          }
    
    if ((fcntl(socket, F_SETFL, cur))<0)
       {
        say ("%$% Unable to set socket O_NONBLOCK - %s.\n",HOT,strerror(errno));

        return 0;
       }

   return 1;

 }  /**/


 int Connect (struct in_addr *addr, int port) 

 {
  int sock, 
      connected;
  struct sockaddr_in address;

   memset ((char *)&address, 0, sizeof(address));

   address.sin_family=AF_INET;
   address.sin_port=htons (port);
   address.sin_addr.s_addr=addr->s_addr;
   
    if (!(sock=RequestSocket()))  return 0;

    if (!(SetSocketFlags(sock, 1, O_NONBLOCK)))  return 0;
       
   connected=connect (sock, (struct sockaddr *)&address, sizeof(address));

   switch (connected)
          {
           case 0:
                SetSocketFlags(sock, 0, O_NONBLOCK);
                return sock;

          default:
                 if (errno!=EINPROGRESS) 
                    {
                     say ("%$% Connection refused - %s.\n",HOT,strerror(errno));
                     close (sock);

                     return 0;
                    }
                 else
                    {
                     return (-sock);
                    }
         }

 }  /**/


 char *RawIPToDotted (unsigned long raw)

 {
  static char rv[16];
  unsigned long ip;
 
   ip=htonl (raw);
 
   sprintf (rv, "%d.%d.%d.%d",
            (int)((ip>>0)&0xFF),
            (int)((ip>>8)&0xFF),
            (int)((ip>>16)&0xFF),
            (int)((ip>>24)&0xFF));

   return rv;

 }  /**/


 char *HostToDotted (char *host)
 
 {
  unsigned long ip;
  struct hostent *hp;
  struct in_addr addr;
   
    if (isdottedquad(host))
       {
        return host;
       } 

   hp=gethostbyname (host);

    if (hp==(struct hostent *)NULL)
       {
        return ((char *)NULL);
       }

  memcpy ((void *)&addr, (void *)hp->h_addr, sizeof(addr));

  ip=ntohl (addr.s_addr);

   if (!ip)
      {
       return ((char *)NULL);
      }

  return (RawIPToDotted(ip));

 }  /**/


 char *DottedToHost (char *dip) 
 {
  struct sockaddr_in addr;
  struct hostent *hp;

   addr.sin_addr.s_addr=inet_addr (dip);

   hp=gethostbyaddr ((char *)&addr.sin_addr, sizeof(addr.sin_addr), AF_INET);
    if (hp==(struct hostent *)NULL)
       {
        return ((char *)dip);
       }

   return ((char *)hp->h_name);

 }  /**/


 int isdottedquad (const char *address)

 {
  register int n,
           numbered=1;

   n=strlen(address)-1;

    while ((address[n]!='.')&&(n))
    {
      if ((address[n]<'0')||(address[n]>'9'))
        {
         numbered=0;
         break;
        }
     n--;
    }

   return numbered;

 }  /**/


 static char *_h_errlist[]={
    "You should be fine",
    "Host not found",
    "Host name lookup failure",
    "Unknown server error",
    "No address associated with name",
    "Service unavailable",
};

static int _h_nerr = sizeof(_h_errlist)/sizeof(_h_errlist[0]);

 char *h_strerror (int error)

 {
  static char aux[35];

    if ((error<0)||(error>_h_nerr)) 
       {
        sprintf (aux, "Unknown resolver error");

        return (char *)aux;
       }

   return (char *)_h_errlist[error];

 }  /**/

