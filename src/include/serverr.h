/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: serverr.h,v 1.4 1998/03/21 09:42:29 ayman Beta $")
**
*/

 struct InResponseToServer {
                            const char *identifier;
                            void (*answer) (char *, char *);
                           };
 typedef struct InResponseToServer ServerCmnd;

 void sERROR (char *, char *);
 void sINVITE (char *, char *);
 void sJOIN (char *, char *);
 void sKICK (char *, char *);
 void sMISC (char *, char *);
 void sMISC_H (char *, char *);
 void sMODE (char *, char *);
 void sNICK (char *, char *);
 void sNOTICE (char *, char *);
 void sPART (char *,  char *);
 void sPING (char *, char *);
 void sPONG (char *,  char *);
 void sPRIVMSG (char *, char *);
 void sQUIT (char *, char *);
 void sTOPIC (char *, char *);
 void s001 (char *, char *);
 void s221 (char *, char *); //own umode
 void s301 (char *, char *);
 void s302 (char *, char *);//AYmAN.net=302=AYmANn :AYmANn=+ayman@AYmAN.net
 void s303 (char *, char *); //ISON replay
 void s305 (char *, char *); //away confirmation
 void s306 (char *, char *); //unaway confirmation
 void s311 (char *, char *);
 void s312 (char *, char *);
 void s313 (char *, char *);
 void s314 (char *, char *);  //whowas header
 void s317 (char *, char *); //134 :seconds idle
 void s318 (char *, char *); //end of /whois
 void s319 (char *, char *);
 void s322 (char *, char *); // /list output
 void s324 (char *, char *); //chan mode info
 void s329 (char *, char *); //channel creation time
 void s332 (char *, char *); //topic 
 void s333 (char *, char *);
 void s341 (char *, char *); //AYmAN.net=341=AYmANn YY #ayman confirming invite
 void s352 (char *, char *); //who replay
 void s353 (char *, char *); //names list
 void s366 (char *, char *); //eo names list
 void s367 (char *, char *); //AYmAN.net=367=AYmANn #ayman yy!*@* 
 void s368 (char *, char *); //AYmAN.net=368=AYmANn #ayman :End of Channel Ban List
 void s315 (char *, char *); //eo /who list
 void s381 (char *, char *); //you are now irc op
 void s401 (char *, char *); //no such nick/channel
 void s402 (char *, char *); //so such server
 void s404 (char *, char *); //cant send to channel
 void s433 (char *, char *); //nick in use
 void s442 (char *, char *); //:You're not on that channel 
 void s443 (char *, char *); //:is already on channel 
 void s451 (char *, char *); //haven't registered yet
 void s473 (char *, char *);//Cannot join channel (+i)
 void s474 (char *, char *);//Cannot join channel (+b)
 void s475 (char *, char *);//Cannot join channel (+k)
 void s477 (char *, char *);//AYmAN.net=477=AYmANn +kko :Channel doesn't support modes 
 void sMOTD (char *, char *);// 375(start of motd) 372(line in motd) 376(eo motd
 //377 as well 
 int cmpservcmnd (const ServerCmnd *, const ServerCmnd *);

#define NRCOMMANDS 52

 const ServerCmnd sFunction []=
        {
         {"001",     (void (*)()) s001    },
         {"002",     (void (*)()) sMISC   },
         {"003",     (void (*)()) sMISC   },
         {"004",     (void (*)()) sMISC   },
         {"005",     (void (*)()) sMISC   },
         {"118",     (void (*)()) sMISC   },
         {"221",     (void (*)()) s221    }, 
         {"250",     (void (*)()) sMISC   },
         {"251",     (void (*)()) sMISC   },
         {"252",     (void (*)()) sMISC   },
         {"253",     (void (*)()) sMISC   },
         {"254",     (void (*)()) sMISC   },
         {"255",     (void (*)()) sMISC   },
         {"256",     (void (*)()) sMISC   },
         {"257",     (void (*)()) sMISC   },
         {"258",     (void (*)()) sMISC   },
         {"259",     (void (*)()) sMISC   },
         {"265",     (void (*)()) sMISC   },
         {"266",     (void (*)()) sMISC   },
         {"301",     (void (*)()) s301    },
         {"302",     (void (*)()) s302    },
         {"303",     (void (*)()) s303    },
         {"305",     (void (*)()) s305    },
         {"306",     (void (*)()) s306    },
         {"311",     (void (*)()) s311    },
         {"312",     (void (*)()) s312    },
         {"313",     (void (*)()) s313    },
         {"314",     (void (*)()) s314    },
         {"315",     (void (*)()) s315    },
         {"317",     (void (*)()) s317    },
         {"318",     (void (*)()) s318    },
         {"319",     (void (*)()) s319    },
         {"321",     (void (*)()) sMISC   },
         {"322",     (void (*)()) s322    },
         {"323",     (void (*)()) sMISC   },
         {"324",     (void (*)()) s324    },
         {"329",     (void (*)()) s329    },
         {"331",     (void (*)()) s401    },
         {"332",     (void (*)()) s332    },
         {"333",     (void (*)()) s333    },
         {"341",     (void (*)()) s341    },
         {"351",     (void (*)()) sMISC   },
         {"352",     (void (*)()) s352    },
         {"353",     (void (*)()) s353    },
         {"364",     (void (*)()) sMISC   },
         {"365",     (void (*)()) sMISC   },
         {"366",     (void (*)()) s366    },
         {"367",     (void (*)()) s367    },
         {"368",     (void (*)()) s368    },
         {"369",     (void (*)()) s318    },
         {"371",     (void (*)()) sMISC   },
         {"372",     (void (*)()) sMOTD   },
         {"374",     (void (*)()) sMISC   },
         {"375",     (void (*)()) sMOTD   },
         {"376",     (void (*)()) sMOTD   },
         {"377",     (void (*)()) sMOTD   },
         {"381",     (void (*)()) s381    },
         {"382",     (void (*)()) sMISC   },
         {"391",     (void (*)()) sMISC   },
         {"401",     (void (*)()) s401    },
         {"402",     (void (*)()) s401    },
         {"403",     (void (*)()) s401    },
         {"404",     (void (*)()) s404    },
         {"406",     (void (*)()) s401    },
         {"421",     (void (*)()) sMISC   },
         {"422",     (void (*)()) sMISC   },
         {"431",     (void (*)()) sMISC   },
         {"432",     (void (*)()) s402    },
         {"433",     (void (*)()) s433    },
         {"437",     (void (*)()) s433    },
         {"441",     (void (*)()) sMISC   },
         {"442",     (void (*)()) s442    },
         {"443",     (void (*)()) s443    },
         {"446",     (void (*)()) sMISC   },
         {"451",     (void (*)()) s451    },
         {"461",     (void (*)()) s401    },
         {"462",     (void (*)()) sMISC_H },
         {"465",     (void (*)()) sMISC_H },
         {"472",     (void (*)()) sMISC_H },
         {"473",     (void (*)()) s473    },
         {"474",     (void (*)()) s474    },
         {"475",     (void (*)()) s475    },
         {"477",     (void (*)()) s477    },
         {"481",     (void (*)()) sMISC_H },
         {"482",     (void (*)()) sMISC   },
         {"484",     (void (*)()) sMISC   },
         {"513",     (void (*)()) sMISC_H },
         {"ERROR",   (void (*)()) sERROR  },
         {"INVITE",  (void (*)()) sINVITE },
         {"JOIN",    (void (*)()) sJOIN   },
         {"KICK",    (void (*)()) sKICK   },
         {"MODE",    (void (*)()) sMODE   },
         {"NICK",    (void (*)()) sNICK   },
         {"NOTICE",  (void (*)()) sNOTICE },
         {"PART",    (void (*)()) sPART   },
         {"PING",    (void (*)()) sPING   },
         {"PONG",    (void (*)()) sPONG   },
         {"PRIVMSG", (void (*)()) sPRIVMSG},
         {"QUIT",    (void (*)()) sQUIT   },
         {"TOPIC",   (void (*)()) sTOPIC  }
        };

