/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: sockets.h,v 1.7 1998/04/08 08:54:34 ayman Beta $")
**
*/

#define NO_SOCKETS    0
#define MAXSOCKSCOUNT 50
                                
#define QUEUE_EMPTY 0
                                
#define SOCK_IRCSERVER       1
#define SOCK_LISTENINGTELNET 2
#define SOCK_CONNECTEDTELNET 3
#define SOCK_LISTENINGDCC    4
#define SOCK_CONNECTEDDCC    5
#define SOCK_IPC             6
#define SOCK_IPCCLIENT       7
#define SOCK_SOCKSCOUNT      8
#define SOCK_IRCIT           9
#define SOCK_EGGDROP         10
#define SOCK_OTHER           11
#define SOCK_SNDSERVER       12

#define IS_IRCSERVER(x) ((x)->type==SOCK_IRCSERVER)
#define IS_LISTENINGTELNET(x) ((x)->type==SOCK_LISTENINGTELNET)
#define IS_CONNECTEDTELNET(x) ((x)->type==SOCK_CONNECTEDTELNET)
#define IS_LISTENINGDCC(x) ((x)->type==SOCK_LISTENINGDCC)
#define IS_CONNECTEDDCC(x) ((x)->type==SOCK_CONNECTEDDCC)
/* #define IS_FAULTY(x) ((x)->type==SOCK_FAULTY) */
/* #define IS_FIRINGSQUAD(x) ((x)->type==SOCK_FIRINGSQUAD) */
#define IS_IRCIT(x) ((x)->type==SOCK_IRCIT)
#define IS_EGGDROP(x) ((x)->type==SOCK_EGGDROP)
#define IS_OTHER(x) ((x)->type==SOCK_OTHER)
#define IS_IPC(x) ((x)->type==SOCK_IPC)
#define IS_IPCCLIENT(x) ((x)->type==SOCK_IPCCLIENT)
#define IS_SNDSERVER(x) ((x)->type==SOCK_SNDSERVER)

#define DCCCHATTOHIM       (0x1<<0)
#define DCCCHATTOUS        (0x1<<1)
#define DCCSENDING         (0x1<<2)
#define DCCGETTING         (0x1<<3)
#define CONNECTEDTELNET    (0x1<<4)
#define AWAITINGUSERNAME   (0x1<<5)
#define AWAITINGCONNECTION (0x1<<6)
#define FIRING_SQUAD       (0x1<<7)
#define BLOCKING           (0x1<<8)
#define REPORTED           (0x1<<9)

 struct SocketQueue
                 {
                  time_t  when;
                  int     flag;
                  char   *msg;

                  struct SocketQueue *next;
                 };
 typedef struct SocketQueue SocketQueue;

 struct Socket {
                time_t   when;

                int      type,
                         gflags,
                         sock,
                         block_sz;
                unsigned flag;

                ssize_t  sbytes,
                         rbytes;

                long     hport,
                         port;

                char     address[MAXHOSTLEN],
                         haddress[MAXHOSTLEN];
                void    *index;

                int (*init_func)(struct Socket *);
                int (*read_func)(struct Socket *);
                int (*close_func)(struct Socket *, const int);

                struct Socket *next;

                long         nMsg;
                SocketQueue *front;
                SocketQueue *rear;
        };
 typedef struct Socket Socket;

 struct SocketsTable
                    {
                      int     glock,
                              nEntries;
                      Socket *head;
                      Socket *tail;
                    };
 typedef struct SocketsTable SocketsTable;

#define SOCKETTOINDEX(x) ((Index *)(x)->index)
#define QUEUE_NOT_EMPTY(x) ((x)->nMsg>=1)

 void InitSocketsTable (void);
 Socket *AddtoSocketsTable (SocketsTable *, int);
 int RemovefromSocketsTable (SocketsTable *, int);
 void TellSocketType (const Socket *, char *);
 void TellSocketInfo (const Socket *);
 void ShowSocketsTable (void);
 Socket *SocketByName (int);
 Socket *Socketbyaddress (const char *, int);
 int GetTelnetSockets (int *);
 void ConnectionLost (int);
 void setsockflag (int, int, boolean);
 boolean SocketsTableEmpty (const SocketsTable *);
 boolean SocketsTableFull (const SocketsTable *);
 boolean SocketQueueEmpty (Socket *);

 int IRC_ServerInit (Socket *);
 int DCCConnetInit (Socket *sptr);

 int IRC_ServerRead (Socket *);
 int OTHER_Read (Socket *);
 int IPCCLIENT_Read (Socket *);
 int DCCCHAT_Read (Socket *);
 int DCCGET_Read (Socket *);
 int DCCSEND_Read (Socket *);
 int TELNET_Read (Socket *);

 int IRCSERVER_Close (Socket *, const int);
 int TELNET_Close (Socket *, const int);
 int OTHER_Close (Socket *, const int);
 int LISTENINGTELNET_Close (Socket *, const int);
 int IPCCLIENT_Close (Socket *, const int);
 int IPCSERVER_Close (Socket *, const int);
 int CONNECTEDDCC_Close (Socket *, const int);
 int LISTENINGDCC_Close (Socket *, const int);

 void ScrollingSockets (void *, int);

