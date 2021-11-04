/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: updates.h,v 1.7 1998/04/08 08:54:34 ayman Beta $")
**
*/

#define SESSION 1
#define SERVER  2
#define MAIL    4
#define TELNET  8
#define DCC     16
#define READING 32
#define MERGING 64
#define PENDING 128
#define L_TELNETUP 256
#define L_CHAT 512
#define L_SEND 1024
#define L_GET  2048

#define EVERYTHING 0xffff

#define MISC_T    0x01
#define SERVER_T  0x02
#define CHANNEL_T 0x03
#define HISTORY_T 0x04
#define STATS_T   0x05
#define USER_T    0x06
#define TIMERS_T  0x07
//1-20
#define M_MAIL       0x01
#define M_LIGHTS     0x02
#define M_DCC        0x03
#define M_TELNET     0x04
#define M_TELSOCK    0x05
#define M_SOCKETS    0x06
#define M_CPU        0x07
#define M_START      0x08
#define M_TIME       0x09
#define M_ONLINE     0x0a
#define M_UTMP_M     0x0b
#define M_MAIL_M     0x0c
#define M_UTMP_SZ    0x0d
#define M_MAIL_SZ    0x0e
#define M_CHATS      0x20
#define M_SENDS      0x21
#define M_GETS       0x22
//21-40
#define H_LINES      0x15
#define H_LINEID     0x16
#define H_TABS       0x17
#define H_TABID      0x18
#define H_AWAYMSGS   0x19
#define H_NOTIFY     0x1a
#define H_UTMP       0x1b
#define H_LINES_D    0x1c
#define H_TABS_D     0x1d	
//41-60
#define CH_CHANNELS  0x29
#define CH_CHANNELID 0x2A
#define CH_USERS     0x2B
#define CH_CHANNEL   0x2C
#define CH_CHANMODE  0x2D
#define CH_NUSERS    0x2E
//61-80
#define SERV_STATUS  0x3D
#define SERV_SERVERS 0x3E
#define SERV_ID      0x3F
#define SERV_LAG     0x40
#define SERV_PORT    0x41
#define SERV_SOCKET  0x42
#define SERV_NICK    0x43
#define SERV_SERVER  0x44
#define SERV_UMODE   0x45
//81-100
#define ST_MSGS      0x51
#define ST_RBYTES    0x52
#define ST_SBYTES    0x53
#define ST_TELNETS   0x54
#define ST_SOCKETS   0x55
#define ST_NOTICES   0x56
#define ST_DCCGETS   0x57
#define ST_DCCSENDS  0x58
#define ST_DCCCHATS  0x59

#define OFF     1
#define ON      2
#define FLASH   4


 struct Misc_t
        {
         int  nMail,
              lights,
              welcomed;
         int  nDccs,
              nOthers,
              nTelnets,
              nSockets,
              telnetsock,
              telnetport;
         int  idx;
         int  nGets,
              nSends,
              nChats;
         int  c_status,
              c_srvopt;  
         unsigned long c_ignore,
                       c_output,
                       s_attrs;
         char   myuserhost[165];
         time_t utmp_mt,
                mailbox_mt;
         off_t  utmp_sz,
                mailbox_sz;
         void *c_window;
        };
 typedef struct Misc_t MiscTable;

 struct Userdata_t
        {
         char *login,
              *shell,
              *homedir,
              *realname,
              *password;
         char *mailbox;
         uid_t userid,
               euid;
         gid_t usergid,
               egid;
         int   pid;

         unsigned long localip;
         char *localhost;
        };
 typedef struct Userdata_t UserdataTable;

 struct History_t 
        {
         int nLines,
             c_lineid;
         int nTabs,
             c_tabid;
         int nAwaymsgs;
         int nNotify,
             nUTMP;
         void *offered_chat;
        };
 typedef struct History_t HistoryTable;

 struct Channel_t
        {
         int  nChannels,
              c_channelid;
         int  nUsers;
         char c_channel[100],
              c_channelmode[100];
         void *c_chptr;
        };
 typedef struct Channel_t ChannelTable;

 struct Server_t
        {
         int  away,
              status,
              nServers,
              c_serverid,
              c_serverlag,
              c_serverport,
              c_usermode,
              c_serversocket;
         unsigned loglevel;
         char c_nick[MAXNICKLEN],
              c_server[MAXHOSTLEN],
              invited_to[MAXCHANLEN],
              msg_fromn[MAXNICKLEN],
              msg_fromh[MAXHOSTLEN];
        };
 typedef struct Server_t ServerTable;

 struct Statistics_t
        {
         int nMsgs,
             nRbytes,
             nSbytes,
             nTelnets,
             nSockets,
             nNotices,
             nDcc_gets,
             nDcc_chats,
             nDcc_sends;
        };
 typedef struct Statistics_t StatisticsTable;

 struct Timers_t
        {
         time_t aux,
                ping_replay,
                uping_replay,
                away_when,
                irc_idle,
                online,
                session_idle;
        };
 typedef struct Timers_t TimersTable;


 struct Session_t 
       {
         MiscTable       misc;
         UserdataTable   userdata;
         ServerTable     server;
         ChannelTable    channel;
         HistoryTable    history;
         TimersTable     timers;
         StatisticsTable stats;
       };
 typedef struct Session_t SessionTable;

 extern SessionTable t_master;

 #define misc_t    t_master.misc
 #define user_t    t_master.userdata
 #define server_t  t_master.server
 #define channel_t t_master.channel
 #define history_t t_master.history
 #define stats_t   t_master.stats
 #define timers_t  t_master.timers

 extern int status_busy;

 extern MiscTable *const mt_ptr;
 extern ServerTable *const st_ptr;
 extern ChannelTable *const cht_ptr;
 extern HistoryTable *const ht_ptr;
 extern TimersTable *const tt_ptr;
 extern UserdataTable *const ut_ptr;

 #define homedir() (ut_ptr->homedir)
 #define loginname() (ut_ptr->login)
 #define localhost() (ut_ptr->localhost)
 #define localip() (ut_ptr->localip)
 #define telnetport() (mt_ptr->telnetport)

#define USER_CHANOP     1
#define USER_CHANVOICE  2
#define USER_RESTRICTED 4
#define USER_INVISIBLE  8
#define USER_WALLOP     16

 /* session attributes */
 #define S_MULTIBLE  0x1 
 #define S_NOCONFIG  0x2
 #define S_NOIPC     0x4 
 
 #define isup() (st_ptr->c_serversocket!=0?st_ptr->c_serversocket:0)
 #define iscurport(x) (((x)==st_ptr->c_serverport)?st_ptr->c_serverport:0)
 #define curport() (st_ptr->c_serverport)
 #define iscurserver(x) (!strcasecmp((x), st_ptr->c_server))
 #define iscurserverid(x) \
 (((x)==0)?st_ptr->c_serverid:(((x)==st_ptr->c_serverid)?st_ptr->c_serverid:0))

 #define isme(x) (!strcasecmp((x), st_ptr->c_nick))
 #define isop() (st_ptr->c_usermode&USER_CHANOP)
 #define ischanop(x) ((x)->chanop?1:0)
 #define isvoice() (st_ptr->c_usermode&USER_CHANVOICE)
 #define isrestricted() (st_ptr->c_usermode&USER_RESTRICTED)
 #define isinvisible() (st_ptr->c_usermode&USER_INVISIBLE)
 #define iswallop() (st_ptr->c_usermode&USER_WALLOP)
 #define curnick() (st_ptr->c_nick)
 #define curmode() (st_ptr->c_usermode)
 #define curserver() (st_ptr->c_server)
 #define lastmsg_n() (st_ptr->msg_fromn)
 #define lastmsg_h() (st_ptr->msg_fromh)

 #define me() (mt_ptr->myuserhost)
 #define welcomed() (mt_ptr->welcomed>0?1:0)
 #define ndccs() (mt_ptr->nDccs)
 #define ntelnets() (mt_ptr->nTelnets)
 #define nsockets() (mt_ptr->nSockets)

 #define LOADMENUBAR() (mt_ptr->c_output|=MENUBAR)
 #define LOADMENUBOX() (mt_ptr->c_output|=MENUBOX)
 #define LOADSCROLLINGLIST() (mt_ptr->c_output|=SCROLLINGLIST)
 #define UNLOADMENUBAR() (mt_ptr->c_output&=~MENUBAR)
 #define UNLOADMENUBOX() (mt_ptr->c_output&=~MENUBOX)
 #define UNLOADSCROLLINGLIST() (mt_ptr->c_output&=~SCROLLINGLIST)
 #define IS_MENUBAR() (mt_ptr->c_output&MENUBAR)
 #define IS_MENUBOX() (mt_ptr->c_output&MENUBOX)
 #define IS_SCROLLINGLIST() (mt_ptr->c_output&SCROLLINGLIST)
 #define RESETOUTPUTMEDIUM() (mt_ptr->c_output=0)
 #define IS_SHOWOPTMENU() (mt_ptr->c_output&SHOWOPTMENU)
 #define IS_BEEPONMSG() (mt_ptr->c_output&BEEPONMSG)

 #define RESET_LOGLEVEL(x) (st_ptr->loglevel&=~(x))
 #define ADDTO_LOGLEVEL(x) (st_ptr->loglevel|=(x))
 #define IS_LOGLEVEL(x) ((st_ptr->loglevel&(x))?1:0)

 #define LOADMAINWIN() \
       { \
        status_busy=0; \
        (WINDOW *)mt_ptr->c_window=MainWin; \
       }

 #define LOADSPAREWIN() \
       { \
        status_busy=1; \
        (WINDOW *)mt_ptr->c_window=SpareWin; \
       }

 #define iscurchan(x) (!strcasecmp((x), cht_ptr->c_channel))
 #define curchan() (cht_ptr->c_channel)
 #define curchptr() ((Channel *)cht_ptr->c_chptr)
 #define nchannels() (cht_ptr->nChannels)

 void InitSessionTable (void);
 void ResetMiscTable (int);
 void ResetUserdataTable (int);
 void ResetHistoryTable (int);
 void ResetChannelTable (int);
 void ResetServerTable (int);
 void SessionUp (void);
 void *GetSessionTable (int);
 void *GetTable (int);
 void UpdateMisc (void);
 void ShowMiscTable (int);
 void UpdateHistoryTable (int, int, const char);
 void UpdateServerTable (int, int, char *, const char);
 void DisplayServerTable (int);
 void DisplayLowerStatus (void);
 void UpdateUpperBorder (int , int , char *, const char);
 void DisplayUpperBorder (int);
 void UpdateUpper (void);
 void UpdateLower (int);
 void UpdateServerLag (void);
 void PingServer (void);
 void Flash (int, unsigned long);
 void CheckLights (void);
 void ResetUpdates (void);
 void ServerUp (char *, int, int);
 void ServerDown (char *, int, int);
 void setstatus (int, int);
 boolean isstatus (int);
 void Away (int);

