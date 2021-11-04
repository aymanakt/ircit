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
#include <scroll.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <utmp.h>
#ifdef _HAVE_PTHREAD
# include <pthread.h>
#endif
#include <sockets.h>
#include <misc.h>
#include <servers.h>
#include <lastlog.h>
#include <chanrec.h> 
#include <notify.h>
#include <prefs.h>
#include <u_status.h>
#include <lstatus.h>
#include <events.h>
#include <updates.h>

 MODULEID("$Id: updates.c,v 1.10 1998/04/09 14:45:39 ayman Beta $");

 const char *c_version="IRCIT 0.3.1";
 SessionTable t_master;
 volatile int firsttime=1;
 time_t aux,
        online;

 UserdataTable *const ut_ptr=&user_t;
 HistoryTable *const ht_ptr=&history_t;
 ServerTable *const st_ptr=&server_t;
 MiscTable *const mt_ptr=&misc_t;
 ChannelTable *const cht_ptr=&channel_t;
 TimersTable *const tt_ptr=&timers_t;

 extern int away;
 extern time_t away_when;
 extern struct UserData data;


 void InitSessionTable (void)

 {
   ResetMiscTable (0xffff);
   ResetUserdataTable (0xffff);
   ResetHistoryTable (0xffff);
   ResetChannelTable (0xffff);
   ResetServerTable (0xffff);

 }  /**/


 void SessionUp (void)

 {
  char str[80];
  chtype chs[80];
  time_t now;
  struct stat st;
  extern char *t_compiled;
  extern char *u_compiled;
  extern List *utmp_notify;
  extern List *servers;

   tt_ptr->online=online=now=time (NULL);

   strftime (str, 79, " %H:%M%p %Z on %a, %d %b '%y", localtime(&now));
   chsprintf (chs, "%s      (%s (c) 1998 Ayman Akt)", str, c_version);

   mt_ptr->c_status=1;

    if (stat(UTMP_FILE, &st)<0)
     {
      say ("%$% UTMP Notification turned off: %s.\n", HOT, strerror(errno));
      mt_ptr->utmp_mt=now;
      mt_ptr->utmp_sz=0;
     }
    else
     {
      mt_ptr->utmp_mt=st.st_mtime;
      mt_ptr->utmp_sz=st.st_size;
     }

   memset (&st, 0, sizeof(struct stat));

    if (stat(data.mailbox, &st)<0) 
     {
      say ("%$% Unable to open mailbox: %s.\n", HOT, strerror(errno));
      mt_ptr->mailbox_mt=now;
      mt_ptr->mailbox_sz=0;
     }
    else
     {
      mt_ptr->nMail=CountMail ();
      mt_ptr->mailbox_mt=st.st_mtime;
      mt_ptr->mailbox_sz=st.st_size;

      RegisterWithEvery30Seconds (CheckMail);
     } 
   
   FancyOutput2 (UpperStatusWin, chs, 0, 79, 0);

   st_ptr->nServers=servers->nEntries;
   st_ptr->status|=NOTCONNECTED;

   UpdateLower (0);

   DisplayControlsPanel ();
   wnoutrefresh (UpperStatusWin);
   wnoutrefresh (SstatusWin);
   wnoutrefresh (StatusWin);

   ///var/lib/zoneinfo/localtime  or /usr/lib/zoneinfo/localtime <- link 
   chsprintf (chs, "%$% Created %s (%s).\n", INFO, t_compiled, u_compiled);
   FancyOutput2 (CURWIN(), chs, CURWIN()->_cury, 77, 0);

   Flash (0, 1);

    if (!NO_ENTRIES(utmp_notify))
     {
      utmpCheck ();
      ActivateTimer (6, 0);
     }
       
#ifdef _HAVE_PTHREAD                   
    {
     pthread_t thr;

      pthread_create(&thr, NULL, (void *)DisplayBanner, (void *)0);
    }
#endif

   {
    char s[MEDIUMBUF];

     sprintf (s, "F1 inw_complete();");
     uBINDKEY (s);
     sprintf (s, "F2 lastmsg_action();");
     uBINDKEY (s);
     sprintf (s, "F3 loadmenubar(0);");
     uBINDKEY (s);
     sprintf (s, "F4 prog(\"SERVER\", \"-SCRL\");");
     uBINDKEY (s);
   }

   {
    uLOAD ("ircit.fpl");
   }
    
    {
     register void *auxptr;

      if ((auxptr=EventRegistered(EVNT_STARTUP)))
       {
        ExecHook (auxptr, "");
       }
    }


     if (mt_ptr->c_output&BACKGROUND)
      {
       SurrenderControllingTerminal ();
      }

    if ((st_ptr->c_server[0])||(st_ptr->c_nick[0])||(st_ptr->c_serverport))
     {
      int port;
      register Server *vptr;

       vptr=_GetServerfromList (servers, st_ptr->c_server[0]?NAME:NUMBER,
                                st_ptr->c_server[0]?st_ptr->c_server:(char *)1,
                                st_ptr->c_serverport?
                                (port=st_ptr->c_serverport):
                                (port=atoi(valueof("DEFAULT_SERVER_PORT"))));
        if (vptr==(Server *)NULL&&!st_ptr->c_server[0])//NUMBER arg no srv lst
         {
          say ("%$% I Cannot do this.\n", MILD);
     
          return;
         }
        else
        if (vptr==(Server *)NULL)
         {
          vptr=AddtoServersList (servers, st_ptr->c_server, port); 
         }
        else
        if ((vptr)&&(vptr->port!=port))
         {
          vptr=AddtoServersList (servers, vptr->server, port);
         }

        if (_ConnectToServer(vptr->server, vptr->port, SOCK_IRCSERVER)>0)
         {
          say ("%$% Waiting for response from server...\n", INFO);
         }
     } 

 }  /**/


 void UpdateMisc (void)

 {
  time_t most,
         hour,
         minute;
  char t[15];

   most=time (NULL);

   strftime (t, 10, "%H:%M", localtime(&most));

   wmove (SstatusWin, 0, 54);
   Fwprintf (SstatusWin, "%$205[%s]%$205%$196%$205[%s]%$205%$196",
             t, secstostr(most-tt_ptr->online, 0));

    if (away)
     {
      int x;

       most=time (NULL);
       most=most-away_when;

       hour=(time_t)((int)most/3600);
       most=most-(hour*3600);
       minute=(time_t)((int)most/60);

       x=26+6+strlen(curnick());
       wmove (StatusWin, 1, x);
       swprintf ("%02d%$%%02d", hour, 58|A_BLINK, minute);
     }

   doupdate ();

 }  /**/


 void ShowMiscTable (int what)

 {

 }  /**/
                 

 void UpdateHistoryTable (int what, int newvalue, const char c)

 {
   switch (what)
    {
     case H_LINES:
          c=='-'?
           ht_ptr->nLines--:
           (c=='+'?(ht_ptr->nLines++):(ht_ptr->nLines=newvalue));
          break;
     case H_LINEID:
          ht_ptr->c_lineid=newvalue;
          break;
     case H_TABS:
          c=='-'?
           ht_ptr->nTabs--:
           (c=='+'?(ht_ptr->nTabs++):(ht_ptr->nTabs=newvalue));
          //c=='-'?ht_ptr->nTabs--:ht_ptr->nTabs++;
          break;
     case H_TABID:
          ht_ptr->c_tabid=newvalue;
          break;
     case H_AWAYMSGS:
          c=='-'?ht_ptr->nAwaymsgs--:ht_ptr->nAwaymsgs++;
          break;
     case H_NOTIFY:
          c?(c=='-'?ht_ptr->nNotify--:ht_ptr->nNotify++):
            (ht_ptr->nNotify=newvalue);
          break;
     case H_UTMP: 
          c?(c=='-'?ht_ptr->nUTMP--:ht_ptr->nUTMP++):
            (ht_ptr->nUTMP=newvalue);
          break;
    }

   //if (!status_busy)
    {
     DisplayHistoryTable (what, what?(H_LINEID?(c=='-'?0:1):0):0);
    }

 }  /**/

/* *********************************************************************** */


 void UpdateServerTable (int what, int newvalue, char *new, const char c)

 {
   switch (what)
          {
           case SERV_STATUS:
                c=='-'?(st_ptr->status&=~newvalue):(st_ptr->status|=newvalue);
                break;
           case SERV_SERVERS:
                st_ptr->nServers=newvalue;
                break;
           case SERV_ID:
                st_ptr->c_serverid=newvalue;
                break;
           case SERV_LAG:
                st_ptr->c_serverlag=newvalue;
                break;
           case SERV_PORT:
                st_ptr->c_serverport=newvalue;
                break;
           case SERV_SOCKET:
                st_ptr->c_serversocket=newvalue;
                break;
           case SERV_NICK:
                strcpy (st_ptr->c_nick, new);
                break;
           case SERV_SERVER:
                strcpy (st_ptr->c_server, new);
                break;
           case SERV_UMODE:
                c=='*'?(st_ptr->c_usermode=newvalue):
                ({c=='-'?(st_ptr->c_usermode&=~newvalue):
                         (st_ptr->c_usermode|=newvalue);1;});
                break;
          }

     DisplayServerTable (what);

 }  /**/


 void DisplayServerTable (int what)

 {
   switch (what)
          {
           case EVERYTHING:
                UpdateUpper ();
                DisplayLowerStatus ();
                break;

           case SERV_SERVERS:
                _wattron (MmainWin, COLOR_PAIR(CYAN));
                mmwmove (18, 4);
                Fwprintf (MmainWin, "%02d:", st_ptr->nServers);
                _wattroff (MmainWin, COLOR_PAIR(CYAN));
                break;

           case SERV_ID:
                _wattron (MmainWin, COLOR_PAIR(CYAN));
                mmwmove (18, 6);
                Fwprintf (MmainWin, ":%02d", st_ptr->c_serverid);
                _wattroff (MmainWin, COLOR_PAIR(CYAN));
                break;

           case SERV_SERVER:
                mmwmove (18, 13);
                Fwprintf (MmainWin, "%.20s", st_ptr->c_server);
                break;

           case SERV_PORT:
                {
                 int len=13+1+strlen(st_ptr->c_server);

                  _wattron (MmainWin, COLOR_PAIR(CYAN));
                  mmwmove (18, len);
                  Fwprintf (MmainWin, "%04d", st_ptr->c_serverport);
                  _wattroff (MmainWin, COLOR_PAIR(CYAN));
                  break;
                }

           case SERV_LAG:
                UpdateServerLag ();
                break;

           case SERV_NICK:
                swmove (1, 9);
                swprintf ("[%.10s (%$%%$249%$%) (%$%%$249%$%%$249%$%)] ", 
                          st_ptr->c_nick,
                          isop()?111:111|COLOR_PAIR(55),
                          isvoice()?118:118|COLOR_PAIR(55),
                          isinvisible()?105:105|COLOR_PAIR(55),
                          iswallop()?119:119|COLOR_PAIR(55),
                          isrestricted()?114:114|COLOR_PAIR(55));

                away?Away(1):Away(0);
                break;

           case SERV_UMODE:
                {
                 int len=11+strlen(st_ptr->c_nick);
                  
                  swmove (1, len);

                  swprintf ("(%$%%$249%$%) (%$%%$249%$%%$249%$%)",
                            isop()?111:111|COLOR_PAIR(55),
                            isvoice()?118:118|COLOR_PAIR(55),
                            isinvisible()?105:105|COLOR_PAIR(55),
                            iswallop()?119:119|COLOR_PAIR(55),
                            isrestricted()?114:114|COLOR_PAIR(55)); 
                  break;
                }
          }

   wUpdate (MmainWin);


 }  /**/


 void UpdateUpperBorder (int what, int newvalue, char *new, const char c) 

 {
   switch (what)
          {
           case EVERYTHING:
                UpdateUpper ();
                return;
           case CH_CHANNELS:
                c=='-'?cht_ptr->nChannels--:cht_ptr->nChannels++;
                break;
           case CH_CHANNELID:
                cht_ptr->c_channelid=newvalue;
                break;
           case CH_USERS:
                c=='-'?cht_ptr->nUsers--:cht_ptr->nUsers++;
                break;
           case CH_NUSERS:
                cht_ptr->nUsers=newvalue;
                break;
           case CH_CHANNEL:
                strcpy (cht_ptr->c_channel, new);
                break;
           case CH_CHANMODE:
                strcpy (cht_ptr->c_channelmode, new);
                break;
          }

   DisplayUpperBorder (what);   

 }  /**/


 void DisplayUpperBorder (int what)

 {
   switch (what)
          {
           case EVERYTHING:
                UpdateUpper ();
                return;

           case CH_CHANNELS:
                mmwmove (0, 74);
                _wattron (MmainWin, COLOR_PAIR(CYAN));
                Fwprintf (MmainWin, 0, 74, "%02d", cht_ptr->nChannels);
                _wattroff (MmainWin, COLOR_PAIR(CYAN));

                 if (IS_STATUS_CHANNELS())
                  {
                   DisplayUpperStatusIII ();
                  }

                break;

           case CH_CHANNELID:
                _wattron (MmainWin, COLOR_PAIR(CYAN));
                _wmove (MmainWin, 0, 76);
                Fwprintf (MmainWin, "%02d", cht_ptr->c_channelid);
                _wattroff (MmainWin, COLOR_PAIR(CYAN));
                break;

           case CH_USERS:
           case CH_NUSERS:
                _wattron (MmainWin, COLOR_PAIR(CYAN));
                _wmove (MmainWin, 0, 64);
                Fwprintf (MmainWin, "%03d", cht_ptr->nUsers);
                _wattroff (MmainWin, COLOR_PAIR(CYAN)); 
                break;

           case CH_CHANNEL:
                break;

           case CH_CHANMODE:
                {
                 int len;
            
                  len=80-
                      (19+4+strlen(cht_ptr->c_channel)+
                       strlen(cht_ptr->c_channelmode));

                  mmwmove (0, 1);
                  whline (MmainWin, ACS_HLINE, len-1);

                  mmwmove (0, len); 
                  waddch_(MmainWin, ACS_BULLET);
                  _wattron (MmainWin, COLOR_PAIR(CYAN));
                  Fwprintf (MmainWin, "%.40s", cht_ptr->c_channelmode);
                  _wattroff (MmainWin, COLOR_PAIR(CYAN));
                }
                break;
          }

  wUpdate (MmainWin);

 }  /**/


 void UpdateUpper (void)

 {
  int len,
      i;

   _wmove (MmainWin, 0, 0);
   len=76-(strlen(cht_ptr->c_channel))-(strlen(cht_ptr->c_channelmode))-11-9;

   Fwprintf (MmainWin, "%$218%$196,*"
                       "%$254%s%$254%$196"
                       "%$254%s%$254%$196"
                       "%$254%03d%$254"
                       "%$196,3"
                       "%$254%02d:%02d%$254%$196%$191",
                       len, 
                       cht_ptr->c_channelmode,
                       cht_ptr->c_channel,
                       cht_ptr->nUsers,
                       cht_ptr->nChannels, cht_ptr->c_channelid);

   wUpdate (MmainWin);

    if (IS_STATUS_CHANNELS())
     {
      DisplayUpperStatusIII ();
     }
   
 }  /**/


 void UpdateLower (int mode)

 {
  register int len;

   _wmove (MmainWin, 18, 0);

    if (!mode)
     {
      Fwprintf (MmainWin, "%$192%$196%$196%$254%02d:%02d%$254"
                          "%$196,*%$217",
                          st_ptr->nServers, st_ptr->c_serverid, 69);
     }
    else
     {
      len=strlen(st_ptr->c_server)+5;
      len=79-(len+13);

      Fwprintf (MmainWin, "%$192%$196%$196%$254%02d:%02d%$254%$196"
                          "%$254%.30s:%04d%$254%$196,*%$217",
                          st_ptr->nServers, st_ptr->c_serverid, 
                          st_ptr->c_server, st_ptr->c_serverport, len);
     }

   wUpdate (MmainWin);

 }  /**/


 void UpdateServerLag (void)

 {
  register int n,
               len;

   n=st_ptr->c_serverlag-tt_ptr->ping_replay;

   len=13+strlen(st_ptr->c_server)+5;

    if (/*!st_ptr->c_serverlag*/!n)
     {
      mmwmove (18, len+1);
      whline (MmainWin, ACS_HLINE, (80-(len+2)));
     }
    else
     {
      mmwmove (18, len+1);
      Fwprintf (MmainWin, "%$196%$254lag:%02d%$254",
                (abs(n))>99?99:(abs(n)));
                /*st_ptr->c_serverlag>99?99:st_ptr->c_serverlag);*/
     }

   wUpdate (MmainWin);

 }  /**/


 void PingServer (void)
  
 {
  int n;

    if (isup())
     {
      n=st_ptr->c_serverlag-tt_ptr->ping_replay;

        if (n<0) 
         {
           if (abs(n)>90)
            {
             if ((welcomed())&&(abs(n)>240))
              {
               say ("%$% IRC server %s appears to be stuck.\n",
                    MILD, curserver());
              }
            }

          UpdateServerLag ();
         }

      ToServer ("PING :%lu %s\n", (tt_ptr->ping_replay=time(NULL)),curserver());
     }
    else  /* put down the damn timer */
     {
     }

#if 0
    if (isup())
     {
      time_t now=time (NULL);

        if (st_ptr->c_serverlag) 
         {
           if (st_ptr->c_serverlag>90)
            {
             if (welcomed()||(st_ptr->c_serverlag>240))
              {
               say ("%$% IRC server %s appears to be stuck.\n",
                    MILD, curserver());
              }
            }

          st_ptr->c_serverlag+=now-tt_ptr->ping_replay;

          UpdateServerLag ();
         }

       tt_ptr->ping_replay=now;

       ToServer ("PING :%lu %s\n", now, curserver());
     }
    else  /* put down the damn timer */
       {
       }
#endif
 }  /**/


 void ResetMiscTable (int id)

 {
   if (id&EVERYTHING)
    {
     memset (mt_ptr, 0, sizeof(MiscTable));

     mt_ptr->idx=-1;

     return;
    }  
 
 }  /**/


 void ResetUserdataTable (int id)
 
 {
   memset (&user_t, 0, sizeof(UserdataTable));

 }  /**/


 void ResetHistoryTable (int id)

 {
   if (id&EVERYTHING)
      {
       memset (ht_ptr, 0, sizeof(HistoryTable));
       return;
      }

   if (id&0x15)
      ht_ptr->nLines=0;
   else
   if (id&0x16)
      ht_ptr->c_lineid=0;
   else
   if (id&0x17)
      ht_ptr->nTabs=0;
   else
   if (id&0x18)
      ht_ptr->c_tabid=0;
   else
   if (id&0x19)
      ht_ptr->nAwaymsgs=0;

 }  /**/


 void ResetChannelTable (int id)

 {
   if (id&EVERYTHING)
      {
       memset (cht_ptr, 0, sizeof(ChannelTable));
       return;
      }

   if (id&0x29)
      cht_ptr->nChannels=0;
   else
   if (id&0x2A)
      cht_ptr->c_channelid=0;
   else
   if (id&0x2B)
      cht_ptr->nUsers=0;
   else
   if (id&0x2C)
      cht_ptr->c_channel[0]=0;
   else
   if (id&0x2D)
      cht_ptr->c_channelmode[0]=0;

 }  /**/


 void ResetServerTable (int id)

 {
   if (id&EVERYTHING)
      {
       memset (st_ptr, 0, sizeof(ServerTable));

       return;
      }
   
   if (id&SERV_STATUS)
      st_ptr->status|=NOTCONNECTED;
   else
   if (id&SERV_SERVERS)
      st_ptr->nServers=0;
   else
   if (id&SERV_ID)
      st_ptr->c_serverid=0;
   else
   if (id&SERV_LAG)
      st_ptr->c_serverlag=0;
   else
   if (id&SERV_PORT)
      st_ptr->c_serverport=0;
   else
   if (id&SERV_PORT)
      st_ptr->c_serverport=0;
   else
   if (id&SERV_SOCKET)
      st_ptr->c_serversocket=0;
   else
   if (id&SERV_NICK)
      st_ptr->c_nick[0]=0;
   else
   if (id&SERV_SERVER)
      st_ptr->c_server[0]=0;
   else
   if (id&SERV_UMODE)
      st_ptr->c_usermode=0;

 }  /**/


 void ServerUp (char *server, int port, int socket)

 {
  register Server *vptr;
  extern List *servers;

   Flash (1, 1);

   ResetChannelsTable ();
   ResetChannelTable (0xffff);
   ResetIRCNotify ();

   st_ptr->status&=~(TRYING|NOTCONNECTED);
   st_ptr->status|=CONNECTED;
   st_ptr->c_serverlag=tt_ptr->ping_replay=time (NULL);
   st_ptr->c_usermode=0;
   st_ptr->c_serversocket=socket;
   st_ptr->c_serverport=port;
   st_ptr->c_serverid=serverid (server, port);
   strcpy (st_ptr->c_server, server);

   UpdateUpper ();
   UpdateLower (1);
   PostConnection ();
   DisplayLowerStatus();
   ircNotify ();

    if (vptr=_GetServerfromList(servers, NAME, server, port))
     {
      vptr->when=time (NULL);
     }

    {
     register void *auxptr;

      if ((auxptr=EventRegistered(EVNT_SERVERUP)))
       {
        ExecHook (auxptr, "%s %u", st_ptr->c_server, st_ptr->c_serverport);
       }
    }

 }  /**/


 void ServerDown (char *server, int port, int socket)

 {
   Flash (1, 0);

   st_ptr->status&=~(CONNECTED|TRYING);
   st_ptr->status|=NOTCONNECTED;
   st_ptr->c_serversocket=0;
   st_ptr->c_serverlag=1;
   UpdateHistoryTable (H_NOTIFY, 0, 0);

   mt_ptr->welcomed=0;
   mt_ptr->myuserhost[0]=0;

   ResetChannelsTable ();
   ResetChannelTable (0xffff);
   UpdateUpper ();
   UpdateLower (0);
   ResetIRCNotify ();

   DeActivateTimer (1, NULL);
   DeActivateTimer (2, NULL);
   DeActivateTimer (7, NULL);

    {
     register void *auxptr;

      if ((auxptr=EventRegistered(EVNT_SERVERDOWN)))
       {
        ExecHook (auxptr, "%s %u", st_ptr->c_server, st_ptr->c_serverport);
       }
    }

 }  /**/

 
 void setstatus (int status, int on)

 {
   on?(st_ptr->status|=status):(st_ptr->status&=~status);

 }  /**/


 /*
 ** true if passed status is the status of the current server connection
 */
 boolean isstatus (int status)

 {
   return ((boolean)st_ptr->status&status?1:0);

 }  /**/


 void *GetSessionTable (int table)

 {
   switch (table)
          {
           case MISC_T:
                return (&t_master.misc);
                break;
           case USER_T:
                return (&t_master.userdata);
                break;
           case SERVER_T:
                return (&t_master.server);
                break;
           case CHANNEL_T:
                return (&t_master.channel);
                break;
           case HISTORY_T:
                return (&t_master.history);
                break;
           case STATS_T:
                return (&t_master.stats);
                break;
          }

 }  /**/


 void *GetTable (int table)

 {
  static MiscTable       mtab;
  static ServerTable     vtab;
  static TimersTable     ttab;
  static ChannelTable    ctab;
  static HistoryTable    htab;
  static UserdataTable   utab;
  static StatisticsTable stab;

   switch (table)
          {
           case MISC_T:
                {
                 memset (&mtab, 0, sizeof(MiscTable));
                 memcpy ((MiscTable *)&mtab,
                         &t_master.misc,
                         sizeof(MiscTable));
                 return &mtab;
                }
           case USER_T:
                {
                 memset (&utab, 0, sizeof(UserdataTable));
                 memcpy ((UserdataTable *)&utab,
                         &t_master.userdata,
                         sizeof(UserdataTable));
                 return &utab;
                }
           case SERVER_T:
                {
                 memset (&vtab, 0, sizeof(ServerTable));
                 memcpy ((ServerTable *)&vtab,
                         &t_master.server,
                         sizeof(ServerTable));
                 return &vtab;
                }
           case CHANNEL_T:
                {
                 memset (&ctab, 0, sizeof(ChannelTable));
                 memcpy ((ChannelTable *)&ctab,
                         &t_master.channel,
                         sizeof(ChannelTable));
                 return &ctab;
                }
           case HISTORY_T:
                {
                 memset (&htab, 0, sizeof(HistoryTable));
                 memcpy ((HistoryTable *)&htab,
                         &t_master.history,
                         sizeof(HistoryTable));
                 return &htab;
                }
           case STATS_T:
                {
                 memset (&stab, 0, sizeof(StatisticsTable));
                 memcpy ((StatisticsTable *)&stab,
                          &t_master.stats,
                          sizeof(StatisticsTable));
                 return &stab;
                }
          }

 }  /**/

