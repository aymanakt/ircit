/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the ircIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>
#include <flood.h>

 MODULEID("$Id: flood.c,v 1.3 1998/02/15 11:27:29 ayman Beta $");

 struct Flood flooder[NROFFLOODS];

 /*
 ** threshold is the number of allowed action  in a given  time (lapse)
 ** e.g. the threshold of nickflood is '3' nick changes in '60' (lapse)
 ** mins. channles floods are per-chanel basis,  i.e. each  channel has
 ** its own settings
 */ 
 void CheckFlood (int type, char *nick, char *host, char *channel)
 
 {
  int lapse=0,
      threshold=0;
  time_t now;

    if (channel)
       {
        switch (type)
               {
                case DEOPFLOOD:
                     //load the threshold and lapse for this flood
                     break;
                case JOINFLOOD:
                     break;
                case KICKFLOOD:
                     break;
                case MSGFLOOD:
                     break;
                case NICKFLOOD:
                     break;
                case NOTICEFLOOD:
                     break;
                case PUBLICFLOOD:
                     break; 
               }
       }
    else
       {                  
        switch (type)
               {
                case CTCPFLOOD:
                     break;
                case CTCPACTFLOOD:
                     break;
                case DCCFLOOD:
                     break;
                case INVITEFLOOD:
                     break;
                case MSGFLOOD:
                     break;
                case NOTICEFLOOD:
                     break;
               }
       }

   now=time (NULL);

    if (strcasecmp(flooder[type].nick, nick)) //new
       {
        strcpy (flooder[type].nick, nick);
        flooder[type].when=now;
        flooder[type].counter=0;

        return;
       }

    if (flooder[type].when<now-lapse)  //expired
       {
        flooder[type].when=now;
        flooder[type].counter=0;

        return;
       }

   flooder[type].counter++;  //increment the instance of that action

    if (flooder[type].counter>=threshold) //flood
       {
        flooder[type].when=0L;
        flooder[type].counter=0;
        flooder[type].nick[0]=0;
        //do whatever you want
       }

 }  /**/     
  
