
#include <u_status.h>
#include <sockets.h>
#include <people.h>
#include <lastlog.h>
#include <chanrec.h>
#include <dcc.h>
#include <servers.h>
#include <serverio.h>

 extern void MenuRehash (void *, int);
 extern void MenuSave (void *, int);
 extern void ScrollingTimers (void *, int);
 extern void About (void *, int);
 extern void TellMsg (void);


 BoxGeometry it1_sb_g={6, 16}; /* 16=strlen(s)+2 */

 MenuboxEntry it1_sb1_ent[]={
  {{" All Sections ", 1, 1}, MenuBoxTerminator, NULL, MenuSave, NULL},
  {{" SETS         ", 1, 1}, MenuBoxTerminator, NULL, MenuSave, NULL},
  {{" SERVERS      ", 1, 1}, MenuBoxTerminator, NULL, MenuSave, NULL},
  {{" CHANNELS     ", 1, 0}, MenuBoxTerminator, NULL, NULL,     NULL},
  {{" IRC NOTIFY   ", 1, 1}, MenuBoxTerminator, NULL, MenuSave, NULL},
  {{" UTMP NOTIFY  ", 1, 1}, MenuBoxTerminator, NULL, MenuSave, NULL}
 };  
 MenuItem i1_sb1={NULL, 1, 0, 0, &it1_sb_g, it1_sb1_ent};

 MenuboxEntry it1_sb2_ent[]={
  {{" All Sections ", 1, 1}, MenuBoxTerminator, NULL, MenuRehash, NULL},
  {{" SETS         ", 1, 1}, MenuBoxTerminator, NULL, MenuRehash, NULL},
  {{" SERVERS      ", 1, 1}, MenuBoxTerminator, NULL, MenuRehash, NULL},
  {{" CHANNELS     ", 1, 0}, MenuBoxTerminator, NULL, NULL,       NULL},
  {{" IRC NOTIFY   ", 1, 1}, MenuBoxTerminator, NULL, MenuRehash, NULL},
  {{" UTMP NOTIFY  ", 1, 1}, MenuBoxTerminator, NULL, MenuRehash, NULL}
 };
 MenuItem i1_sb2={NULL, 1, 0, 0, &it1_sb_g, it1_sb2_ent};

 BoxGeometry it1_g={5, 13};
 MenuboxEntry it1_ent[]={
                 {
                  {" Configure ", 1, 0},
                  NULL, NULL, NULL, NULL
                 },
                 {
                  {" Colors    ", 1, 1},
                  NULL, NULL,
                  SetupColorScheme,
                  NULL
                 },
                 {{"", 1, 1}, NULL, NULL, NULL, NULL},
                 {
                  {" Save      ", 1, 1},
                  NULL, NULL, NULL,
                  (MenuItem *)&i1_sb1
                 },
                 {
                  {" Rehash    ", 1, 1},
                  NULL, NULL, NULL,
                  (MenuItem *)&i1_sb2
                 }
              };

 BoxGeometry it2_g={4, 14};
 MenuboxEntry it2_ent[]={
  {{" Status I   ", 1, 1}, NULL, NULL, LoadUpperStatus, NULL},
  {{" Status II  ", 1, 1}, NULL, NULL, LoadUpperStatus, NULL},
  {{" Status III ", 1, 1}, NULL, NULL, LoadUpperStatus, NULL},
  {{" Status IV  ", 1, 0}, NULL, NULL, NULL, NULL}
 };

 BoxGeometry it3_g={5, 12};
 MenuboxEntry it3_ent[]={
                 {
                  {" Servers  ", 1, 1},
                  NULL, NULL, 
                  ScrollingServers,
                  NULL
                 },
                 {
                  {" Sockets  ", 1, 1},
                  NULL, NULL,
                  ScrollingSockets,
                  NULL
                 },
                 {
                  {" Channels ", 1, 1},
                  NULL, NULL,
                  ScrollingChannels,
                  NULL
                 },
                 {
                  {" DCC List ", 1, 1},
                  NULL, NULL, 
                  ScrollingDCCConnections,
                  NULL
                 },
                 {
                  {" Timers   ", 1, 1},
                  NULL, NULL, 
                  ScrollingTimers, 
                  NULL
                 }
              };

 BoxGeometry it4_g={2, 9};
 MenuboxEntry it4_ent[]={
                 {
                  {" About ", 1, 1},
                  NULL, NULL, 
                  About,
                  NULL 
                 },
                 {
                  {" Help  ", 1, 1},
                  NULL, NULL, 
                  ScrollingHelp,
                  NULL
                 }
                };


 MenuItem i[]={
             {" IRCIT ", 1,
              0,
              3,
              &it1_g,
              it1_ent,
             },
             {" Status ", 1,
              0,
              10,
              &it2_g,
              it2_ent,
             },
             {" List ", 1,
              0,
              19,
              &it3_g,
              it3_ent,
             },
             {" Help ", 1,
              0,
              32,
              &it4_g,
              it4_ent,
             },
            }; 
   
 static int attrs[]={1, 1, 1, 1};
 static Menu menu={
        0,
        0,
        80,
        4,
        attrs,
        0,
        i,
        NULL,
        NULL
       };

