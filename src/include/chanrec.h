/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: chanrec.h,v 1.5 1998/04/08 08:54:34 ayman Beta $") 
** 
*/
 
#define NO_CHANNEL       0
#define NO_MORE_CHANNELS 9
#define NO_USERS         0

#define DONE_WITH_PROCESSING 2

#define PROCESSINGNAMESLIST 1
#define PROCESSINGWHOREPLAY 2
#define PROCESSINGCHANMODE  4
#define PROCESSINGBANLIST   8
#define FRESHLYJOINED       16

#define PROCESSING  (PROCESSINGNAMESLIST|PROCESSINGWHOREPLAY|PROCESSINGWHOREPLAY|PROCESSINGCHANMODE|PROCESSINGBANLIST)  

#define CHAN_ANON     1
#define CHAN_MODERATE 2
#define CHAN_INVITE   4
#define CHAN_TOPIC    8
#define CHAN_NO_MSG   16
#define CHAN_PRIVATE  32
#define CHAN_SECRET   64
#define CHAN_QUIET    128

#define USER_CHANOP     1
#define USER_CHANVOICE  2
#define USER_RESTRICTED 4
#define USER_INVISIBLE  8
#define USER_WALLOP     16
#define USER_HERE       32
#define USER_GONE       64
#define USER_IRCOP      128

#define KICK   1
#define IGNORE 2
#define BAN    4

#define BAN_ENFORCE  1
#define BAN_ENFORCED 2
#define BAN_OTHERS   4
#define BAN_ME       8

#define FRMT_HOST     1
#define FRMT_NICK     2
#define FRMT_USERNAME 4

#define NICKLEN 32 
#define HASHSIZE 79

#define CHAN_FLOODS  7

#define CH_DEOPFLOOD   0
#define CH_JOINFLOOD   1
#define CH_KICKFLOOD   2
#define CH_NICKFLOOD   3
#define CH_NOTICEFLOOD 4
#define CH_PUBMSGFLOOD 5

 struct ChanPrefs {
                   int floodprotection,
                       deopflood_thr,
                       deopflood_lps,
                       ondeopflood;
                   int joinflood_thr,
                       joinflood_lps,
                       onjoinflood;
                   int pubmsgflood_thr,
                       pubmsgfllod_lps,
                       onpubmsgflood;
                   int kickflood_thr,
                       kickflood_lps,
                       onkickflood;
                   int nickflood_thr,
                       nickflood_lps,
                       onnickflood;
                   int noticeflood_thr,
                       noticeflood_lps;
                   int autojoin;
                  };
 typedef struct ChanPrefs ChanPrefs;

 struct ChannelFlood {
                      char   nick[MAXNICKLEN],
                             host[MAXHOSTLEN];
                      int    type,
                             counter;
                      time_t when;
                     };
 typedef struct ChannelFlood ChannelFlood;
                       

 struct ChanUser {
                  time_t  when,
                          idle;
                  char    nick[MAXNICKLEN],
                          user[MAXNICKLEN],
                          host[MAXHOSTLEN],
                          realname[MINIBUF],
                          server[MAXHOSTLEN];
                  int     split,
                          status,
                          flags,
                          usermodes;
                  struct  Channel *chptr;
                  struct  ChanUser *next;
                  struct  ChanUser *hnext;
                 };
 typedef struct ChanUser ChanUser;

 struct HashTable {
                   ChanUser *chuptr;
                   ChanUser *next;
                  };
 typedef struct HashTable HashTable;

 struct Ban {
             time_t when;
             char   who[MAXNICKLEN],
                    mask[MAXHOSTLEN];
             int    flag,
                    how;
             void  *link;
            };
 typedef struct Ban Ban;
 

 struct Channel {
                 int glock, /*this and the one bellow correspond to usr*/
                     nEntries;
                 time_t when;
                 char key[MINIBUF], 
                      channel[MAXCHANLEN];
                 char *topic;    
                 int id,
                     limit,
                     flags,
                     status,
                     chanop,
                     nsplit,
                     nchanop,
                     nchanvoice,
                     chanmode;
                 ChanPrefs *prefs;   
                 ChannelFlood chfloods[CHAN_FLOODS-1];
#ifdef LAST__LOG
                 LastLogParams lastlogs[6];
#endif
                 List BanList;

                 struct Channel *next;

                 ChanUser *Hash[HASHSIZE];

                 ChanUser *front,
                          *rear;
                };
 typedef struct Channel Channel;

 struct ChannelsTable {
                       int glock,
                           nEntries;
                       Channel *id_hash[11];
                       Channel *head,
                               *tail;
                      };
 typedef struct ChannelsTable ChannelsTable;

 #define NOCHANNELS(x) ((x)->nEntries==0)

 void InitChannelsTable (void);
 void _InitChannelsTable (ChannelsTable *);
 Channel *ChannelbyName (const char *);
 int id (const char *);
 Channel *AddtoChannelsTable (ChannelsTable *, const char *);
 Channel *AddChannel (const char *);
 int RemovefromChannelsTable (ChannelsTable *, const char *);
 int ResetChannelsTable (void);
 void PrintChannelsTable (ChannelsTable *);

 Ban *AddBan (Channel *, char *);
 int ResetBanList (Channel *);
 int FlushBans (Channel *);
 void ShowBanList (Channel *, int);
 
 void ShowChannelsTable (const ChannelsTable *, int, int);
 void ShowChLastLogs (const Channel *);
 boolean ChannelsTableEmpty (const ChannelsTable *);
 boolean ChannelsTableFull  (const ChannelsTable *);
 char *SwitchtoChannel (const char *);
 void AdjustChannels (ChannelsTable *, char *, int);
 void RecordChanMode (/*const char **/Channel *, char *);
 void ResetChanmode (char *);
 void ParseChanmode (const char *, char *);
 void Postjoin (const char *);
 int LeaveAllChannels (void);
 int MassOV (const Channel *, const char *);
 int MassOVWithMask (const Channel *, const char *, char *);
 int MassBan (const Channel *, char *);
 int MassKick (const Channel *, char *);
 void ScrollingChannels (void *, int);

