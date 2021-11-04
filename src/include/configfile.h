/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: configfile.h,v 1.2 1998/03/16 10:09:38 ayman Beta $")
**
*/

 #define SERVERS     1
 #define SETS        2
 #define CHANNELS    4
 #define IRCNOTIFY   8
 #define UTMPNOTIFY  16
 #define INETNOTIFY  32

 #define THELOT \
 (SERVERS|SETS|CHANNELS|IRCNOTIFY|UTMPNOTIFY|INETNOTIFY)

 static unsigned long FileExist (char *);

 int ProcessConfigfile (void);
 void ProcessSection (char *, int);
 static ProcessSERVERS (char *);
 static ProcessCHANNELS (char *);
 static ProcessSETS (char *);
 static ProcessIRCNOTIFY (char *);
 static ProcessUTMPNOTIFY (char *);
 static ProcessINETNOTIFY (char *);
 static void ActuallyProcess (char *, int);

 static void SaveConfigFile (char *, void (*)(FILE *));
 static int WriteSection (FILE *, FILE *, char *, void (*func)(FILE *));
 static void WriteSERVERS (FILE *); 
 static void WriteSETS (FILE *);
 static void PrintSETEntries (FILE *, PrefsTable *);
 static void WriteCHANNELS (FILE *);
 static void WriteIRCNOTIFY (FILE *);
 static void WriteUTMPNOTIFY (FILE *);

 void uREHASH (char *);
 void uSAVE (char *);

 void MenuRehash (void *, int);
 void MenuSave (void *, int);

