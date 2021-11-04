/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: misc.h,v 1.6 1998/04/08 08:54:34 ayman Beta $")
**
*/

 struct LocalNetData {
         char *local_name;
         unsigned long local_ip;
         int unsigned telnet_port;
        };

 struct UserData {
         char *login;    
         char *realname;
         char *homedir;
         char *shell;
         char *password;
         char *mailbox;
         int userid;
         int usergid;
         struct LocalNetData local;
         char *pid;
       };

 struct Logfiles {
         int i;
         char file1[20];
         char file2[20];
         char file3[20];
       };

 struct SoundFile {
         size_t idx;
         char   *file;
        };
 typedef struct SoundFile SoundFile;


 typedef void signal_f(int); 

 signal_f *nsignal (int, signal_f *);
 void InitSignals (void);
 void BlockSignals (void);
 void UnblockSignals (void);
 int LockFile (int, int);
 int UnlockFile (int);
 void InitMisc (void);
 int AllRightsReserved (void);
 void log (int, ...);
 int mmkdir (char *) __attribute__ ((unused));
 int fgbg (void);
 int isfile (char *);
 void maskhost (char *, char *);
 char *secstostr (time_t, int);
 char *CPUUsage (void);
 int CountMail (void);
 void CheckMail (void);
 void SurrenderControllingTerminal (void);
 int copyfile (char *, char *, int);
 void About (void *, int);
 int PatternMatch (const char *, char *);

