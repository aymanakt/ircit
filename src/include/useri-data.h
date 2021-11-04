/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: useri.h,v 1.5 1998/04/08 08:54:34 ayman Beta $")
**
*/

 const struct InResponseToUser uFunction []=
        {
         {"ABOUT",    
	  "  ABOUT - See version information.", 
	  uABOUT},

         {"ALIAS",    
	  "  ALIAS  [[-]ALIAS] [DEFINITION]",       
	  uALIAS},

         {"ASSOC",    
	  "",
	  uASSOC},

         {"AWAY",
	 "  AWAY [-SET] [TEXT]\n  AWAY [-UNSET]",
	 uAWAY},

         {"BINDKEY",
	  "  BINDKEY [KEY] [FPL EXPRESSION]\n"                 
          "  BINDKEY [-KEY]",
	  uBINDKEY},
	  
         /*{"BAN",      "",      uBAN     },*/
	  
         {"CD",       
	  "  CD [DIR]",
	  uCD},

         {"CHANNELS", 
	  "  CHANNELS [HOW] [CHAN] [TYPE] [m-n]\n"
          "  HOW: -SCRL, -LST\n"
          "  TYPE: -ALL, -OP, -VOICE, -IRCOP, -SPLIT, -LOG",       
	  uCHANNELS},
	  
         {"CHAT",     
	  "  CHAT - Shortcut to DCC CHAT.",
	  uCHAT},
	  
         {"CLEAR",
	  "  CLEAR  [TYPE]\n"
          "  TYPE: -MW, UW, LW, -ALL",   
	  uCLEAR},
	  
         {"CLOSE",    
	  "  CLOSE - Shortcut to DCC CLOSE.",
	  uCLOSE},

         {"COLORS",   
  	  "  COLORS - Customize IRCIT's color settings through visual interface.",
	  uCOLORS},

         {"CTCP",
	  "  CTCP <NICK[,NICK,...]> <TAG>"
          " [ARGS...]", 
	  uCTCP}, 

         {"DCC",      
	  "  DCC [TYPE] [NICK[,NICK,...]] [ARGS...]\n"
          "  DCC CLOSE [TYPE]  [NICK[,NICK,...]] [ARGS...]\n"
          "  DCC [-LST|-SCRL]\n"
          "  TYPE: CHAT, GET, SEND\n"
          "  ARGS: FILE[,FILE,..]|*",
	  uDCC},

         {"DESCRIBE", 
	  "  DESCRIBE <NICK|CHANNEL[,NICK,CHANNEL,...]>"
          " <ACTION DESCRIBTION>",
	  uDESCRIBE},
	  
         {"DMSG",  
	  "  DMSG <DCC-NICK[,DCC-NICK,...]> [TEXT]",
	  uDMSG},
	  
         {"EXEC",
	  "  EXEC [OUTPUT [TARGET]] [CMND [ARGS...]]\n"
          "  OUTPUT: -WIN, -CHAN, -MSG, -NOTC, -CHAT, -TEL, -SOCK, -CLOSE"
          ", -IN\n" 
          "  TARGET: CHANNEL, NICK, SOCKET, TELNET-USER, CHAT-NICK,...",
	  uEXEC},

	 {"EXIT",  
	  "  EXIT [TEXT]",
	  uEXIT},

	 {"FLUSH",
	  "  FLUSH - Flush screen output.",
	  uFLUSH},

         {"FPL", 
 	  "  FPL <FPL STATEMENTS>",
	  uFPL},
	  
         {"GET",
	  "  GET - Shortcut to DCC GET",    
	  uGET},

         {"HELP",
	  "  HELP [COMMAND]", 
	  uHELP},
	  
         {"HIDE",  
	  "  HIDE [-t <SECONDS>]",  
	  uHIDE},

         {"HISTORY",  
	  "  HISTORY <TYPE> [<-|+>ENTRY]|"
          "[m-n]\n"
          "  HISTORY <TYPE> [WHAT [FILE [m-n]]]\n"
          "  TYPE: -ARR, -TAB\n"
          "  WHAT: -SAVE, -LOAD, PLAY\n"
          "  ENTRY: LITERAL|#n|*", 
	  uHISTORY},
	  
         {"IGNORE",   
	  "  IGNORE [TYPE] [<+|->ENTRY,ENTRY,"
          "...]\n"
          "  TYPE: -ALL, -CTCP, -MSG, -NOTC, -INV, -TEL\n"
          "  ENTRY: LITERAL|#n|*",
	  uIGNORE},

         {"INVITE",
	  "  INVITE <NICK[,NICK,...]> [CHANNEL]",
	  uINVITE},

         {"IRCIT",    
	  "  IRCIT [TYPE] <HOST|n> [PORT]\n"    
          "  TYPE: -IRC, -OTHR", 
	  uIRCIT},
	  
         {"JOIN",    
	  "  JOIN [CHANNEL,CHANNEL,...]  [KEY]",  
	  uJOIN},
	  
         {"KICK",
	  "  KICK [CHANNEL] <NICK[,NICK,WILDCARD,...]>"
          " [REASON]",
	  uKICK},
	  
#ifdef LAST__LOG
         {"LASTLOG",  
	  "  LASTLOG - Last log settings on session and channels.",
	  uLASTLOG },
#endif

         {"LEAVE",    
	  "  LEAVE [CHANNEL[,CHANNEL,...]]|[-*.*]",  
	  uPART},

         {"LOAD",
	  "  LOAD <FILE NAME>",  
	  uLOAD},
	  
         {"LOCALHOST",
	  "  LOCALHOST - Display host machine name",   
	  uLOCALHOST},
	  
         {"LOOKUP",   
	  "  LOOKUP [TYPE] <ADDR|INTERFACE>\n"
          "  TYPE: -IF, -ADDR",
	  uLOOKUP},
	  
         {"LS",  
	  "  LS [DIRECTORY]",  
	  uLS},
	  
         {"LUSERS",  
	  "  LUSERS - ",
	  uLUSERS},
	  
         {"MASS",  
	  "  MASS [CHANNEL] <+|-TYPE> [MASK]\n"
          "  TYPE: O (OP), V (VOICE), K (KICK), B (BAN)",
	  uMASS},

         {"ME",
	  "  ME - Like DESCRIBE, but applies to current channel.",  
	  uME},

         {"MODE", 
	  "  MODE <CHANNEL|NICK> [MODES]",
	  uMODE},

         {"MSG",
	  "  MSG <NICK[,NICK,...]> [TEXT]\n"
          "  MSG <#CHANNEL[,NICK,&CHANNEL,...]> [TEXT]",
	  uMSG},

         {"NICK",  
	  "  NICK [NEW NICK]",
	  uNICK},

         {"NOTICE",  
	  "  NOTICE [NICK,#CHAN,...] [TEXT]",  
	  uNOTICE},

         {"NOTIFY",
	  "  NOTIFY [TYPE] [<+|->ENTRY,ENTRY,...]\n"              "  TYPE: -IRC, -UTMP\n"
          "  ENTRY: LITERAL|#n|*",
	  uNOTIFY},
	  
         {"OPER", 
	  "  OPER [NICK PASSWORD]",  
	  uOPER},

         {"PART",  
	  "  PART [CHANNEL,CHANNEL,...]|[-*.*]",  
	  uPART}, 
	  
         {"PEOPLE",
	  "  PEOPLE - Display internal data on people",
	  uPEOPLE}, 

         {"QUOTE", 
	  "  QUOTE <TEXT TO SERVER>",  
	  uQUOTE},
	  
         {"REHASH",   
	  "  REHASH [SECTION]\n"        
          "  SECTION: -*.*, -SRV, -IRC, -UTMP",  
	  uREHASH},
	  
         {"SAVE",     
	  "  SAVE [SECTION]\n"                                
          "  SECTION: -*.*, -SRV, -IRC, -UTMP",    
	  uSAVE},

         {"SCREENSHOT",
          "  SCREENSHOT - Generate screenshots.",
          uSCREENSHOT},
		 
         {"SEND",
	  "  SEND - Shortcut to DCC SEND",  
	  uSEND},
		 
         {"SERVER",   
	  "  SERVER [[+|-]SERVER] [PORT]",  
	  uSERVER},
		 
         {"SET", 
	  "  SET - Set user preferences.",  
	  uSET},

         {"SETUID", 
	  "  SETUID - Play around with various userid's.",
	  uSETUID},
		 
         {"SHOOT",  
	  "  SHOOT - Close socket.",  
	  uSHOOT},
		 
         /*{"SHOW",
	  "  SHOW - ",   
	  uSHOW},*/
		 
         {"SMSG",
	  "  SMSG <SOCKET> [TEXT]",
	  uSMSG},
		 
         {"SOCKETS",  
	  "  SOCKETS - Display SocketsTable.",
	  uSOCKETS},
		 
         {"STATUS",   
	  "  STATUS - Switch between the various UpperStatus modes.",
	  uSTATUS},

         {"TIMERS" ,  
	  "  TIMERS - Display Timer entyries.",  
	  uTIMERS},

         {"TMSG", 
	  "  TMSG - Send msg over Telnet link.",  
	  uTMSG},
		 
         {"UPDATE",
	  "  UPADTE [#CHANNEL,#CHANNEL,...]\n"
          "  UPDATE [-*.*]",   
	  uUPDATE},
		 
         {"VOLUME",  
	  "  VOLUME - Command disabled.",
	  uVOLUME},
		 
         {"WHO", 
	  "  WHO - ",  
	  uWHO},
		 
         {"WHOAMI", 
	  "  WHOAMI - Display misc user information (not irc-related).",  
	  uWHOAMI}, 
		 
         {"WHOIS",
	  "  WHOIS - ",  
	  uWHOIS},
		 
         {"WHOWAS",   
	  "  WHOWAS - ",  
	  uWHOWAS},
        };

