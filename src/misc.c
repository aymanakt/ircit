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
#include <signal.h>
#include <setjmp.h>
#include <pwd.h>
#include <sys/stat.h>
#include <utmp.h>
#include <fcntl.h>
#ifdef _HAVE_PTHREAD
# include <pthread.h>
#endif
#include <list.h>
#include <msgbox.h>
#include <scroll.h>
#include <history.h>
#include <servers.h>
#include <updates.h>
#include <notify.h>
#include <misc.h>

 MODULEID("$Id: misc.c,v 1.9 1998/04/06 06:13:44 ayman Beta $");

 int first=1;
 int flock_fd=0;
 char *empty_string="";
 struct UserData data;
 struct Logfiles Logs=
                      {
                       2, 
                       "/tmp/ircit.debug", 
                       "pointers", 
                       "ircit.conf"
                      };
 time_t MailboxFile;
 extern time_t online;
 
 static void SIGCONTHandler (int);
 static void CTRL_CHandler (int);
 static void CTRL_ZHandler (int);
 static void ChildHandler (int);
 static void SIGHUPHandler (int);
 static void ABRTHandler (int);
 static void SigIO (int);

 signal_f *nsignal (int signo, signal_f *func)

 {
  struct sigaction  sa,
                    osa;

   sa.sa_handler=func;
   sigemptyset (&sa.sa_mask);
   sa.sa_flags|=SA_RESTART;

    if (sigaction(signo, &sa, &osa)<0)
     {
      return (SIG_ERR);
     }

   return (osa.sa_handler);

 }  /**/


 void InitSignals (void)

 {
  extern void ExceTimers (int);

    if (nsignal(SIGWINCH, ResizeTerminal)==SIG_ERR)
     {
      psignal (SIGWINCH, "Couldn't install signal handler");
      exit (0);
     }

    /*if (nsignal(SIGTSTP, CTRL_ZHandler)==SIG_ERR)
       {
        psignal (SIGTSTP, "Couldn't install signal handler");
        exit (0);
       }*/

    if (nsignal(SIGCONT, SIGCONTHandler)==SIG_ERR)
     {
      psignal (SIGCONT, "Couldn't install signal handler");
      exit (0);
     }

    if (nsignal(SIGPIPE, SIG_IGN)==SIG_ERR)
     {
      psignal (SIGPIPE, "Couldn't install signal handler");
      exit (0);
     }

    /*if (nsignal(SIGCHLD, ChildHandler)==SIG_ERR)
       {
        psignal (SIGTSTP, "Couldn't install signal handler");
        exit (0);
       }*/

    if (nsignal(SIGHUP, SIGHUPHandler)==SIG_ERR)
     {
      psignal (SIGHUP, "Couldn't install signal handler");
      exit (0);
     }

    /*if (nsignal(SIGSEGV, CTRL_CHandler)==SIG_ERR)
     {
      psignal (SIGSEGV, "Couldn't install signal handler");
      exit (1);
     }*/ 

    if (nsignal(SIGINT, CTRL_CHandler)==SIG_ERR)
     {
      psignal (SIGINT, "Couldn't install signal handler");
      exit (1);
     }

    /*if (nsignal(SIGIO, SigIO)==SIG_ERR)
       {
        Yell (HOT, "Couldn't install signal handler - SIGIO error.\n");
        exit (1);
       }*/

    if (nsignal(SIGQUIT, SIG_IGN)==SIG_ERR)
     {
      say ("%$% Couldn't install signal handler - SIGQUIT error.\n", HOT);
      exit (1);
     }

    if (nsignal(SIGABRT, ABRTHandler)==SIG_ERR)
     {
      say ("%$% Couldn't install signal handler - SIGABRT error.\n", HOT);
      exit (1);
     }
 
    if (nsignal(SIGALRM, ExceTimers)==SIG_ERR)
     {
      psignal (SIGALRM, "Couldn't install signal handler");
      exit (1);
     }

 }  /**/
    

 void CTRL_CHandler (int signo)

 {
  static int count;
  static time_t when;
  time_t now=time(NULL);

    if ((++count==3)&&((now-when)<=2))
     {
      exit (1);
     }
    else
    if ((count>3)||(now-when>2))
     {
      count=0;
      when=now;
     }

  beep ();

 } /**/


 void CTRL_ZHandler (int signo)

 {
   say ("%$% I don't know how to go background.\n", MILD); 

 }  /**/


 static void SIGCONTHandler (int signo)

 {
  curs_set (0);

 }  /**/


 void ChildHandler (int signo)

 {
  //int lc;
  //pid_t pid;

   //pid=wait (&lc);
   say ("%$% Child decayed...\n", MILD);
   

 }  /**/


 void SIGHUPHandler (int signo)

 {
    if ((!isatty(1))&&(mt_ptr->c_output&BACKGROUND))
     {
      /* log (1, "Recieved SIGHUP, but I'm already detached; ignoring..."); */

      return;
     }

   log (1, "Recieved SIGHUP; giving up the controlling terminal...");

   SurrenderControllingTerminal ();

 }  /**/


 void ABRTHandler (int signo)

 {
   AdjustScrollingList ();
 
 }  /**/


 void SigIO (int signo)

 {

 }  /**/


 sigset_t nmask,
          omask;

 void BlockSignals (void)

 {
   sigemptyset (&nmask);
   sigaddset (&nmask, SIGALRM);

    if (sigprocmask(SIG_BLOCK, &nmask, &omask)<0)
     {
      say ("%$% Error blocking signals - Bye...\n", HOT);

      exit (1);
     }

 }  /**/


#define PENDING 128
 
 void UnblockSignals (void)

 {
  sigset_t pmask;

   if (sigpending(&pmask)<0)
    {
     say ("%$% Fatal: error pending signals - Bye...\n", HOT);

     exit (1);
    }

   if (sigismember(&pmask, SIGALRM))
    {
     Flash (8, 1);
    }

   if (sigprocmask(SIG_SETMASK, &omask, NULL)<0)
    {
     say ("%$% Fatal: Error unblocking signals - Bye...\n", HOT);

     exit (1);
    }

  Flash (8, 0);
  
 }  /**/
  

 void InitMisc (void)

 {
  char *str;
  struct passwd *pwd; 

   str=(char *)getlogin ();

    if (!str) 
     {
      str=(char *)getenv ("USER");

       if (!str)
        {
         str=(char *)getenv ("LOGNAME");
        }
     }

    if (!str)
     {
      fprintf (stderr, "I don't know what to do with you...\n");
      _exit (1);
     }

   ut_ptr->pid=getpid ();
   ut_ptr->euid=geteuid ();
   ut_ptr->egid=getegid ();

    if ((pwd=getpwnam(str)))
     {
      ut_ptr->login=strdup (pwd->pw_name);
      ut_ptr->realname=strdup (pwd->pw_name);
      ut_ptr->shell=strdup (pwd->pw_shell);
      ut_ptr->homedir=strdup (pwd->pw_dir);
      ut_ptr->userid=pwd->pw_uid;
      ut_ptr->usergid=pwd->pw_gid;
     }

   str=(char *)getenv ("MAIL");

    if (!str)
     {
      str=(char *)getenv ("mail");
     }

    if (str!=NULL) 
     {
      data.mailbox=strdup (str);
      ut_ptr->mailbox=strdup (str);
      time (&MailboxFile);
     }
    else
     {
      data.mailbox=NULL;
      ut_ptr->mailbox=NULL;
     }

 }   /**/
 

 int AllRightsReserved (void)

 {
  time_t now;
  int i,
      fd,
      res;
  char s[50],
       path[_POSIX_PATH_MAX];

   sprintf (path, "%s/.ircit/IRCIT.0", ut_ptr->homedir);

    if ((flock_fd=open(path, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR))<0)
     {
      say ("%$% Unable to create lock-file: %s\n", HOT, strerror(errno));
      
      exit (1);
     }

    if (!(res=LockFile(flock_fd, 0)))
     {
      now=time (NULL);

      ftruncate (flock_fd, 0);
      sprintf (s, "%d\n%s", getpid(), ctime(&now));
      write (flock_fd, s, strlen(s));

      return 0;
     }
    else
    if (res>0)
     {
      close (flock_fd);  /* not ours */

      say ("%$% You seem to be running multible instances of IRCIT:"
           " pid %d...\n" "  You'll not be able to save changes to your"
           " configuration file.\n", 
           MILD, res);

      mt_ptr->s_attrs|=S_MULTIBLE;

       for (i=1; ;i++)
        {
         sprintf (path, "%s/.ircit/IRCIT.%d", ut_ptr->homedir, i);

          if ((fd=open(path, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR))<0)
           {
            continue;
           }
          else
           {
             if (!(res=LockFile(fd, 0)))
              {
               now=time (NULL);

               ftruncate (fd, 0);
               sprintf (s, "%d\n%s", getpid(), ctime(&now));
               write (fd, s, strlen(s));

               flock_fd=-fd;

               return i;
              }
             else
             if (res>0)
              {
               say ("%$% oopsz yet another running instance of IRCIT:"
                    " pid %d...\n", MILD, res);
               /* close it? */
               continue;
              }
             else  /* fcntl error! */
              {
               continue;
              }
           }
       }
     }

    return -1;

 }  /**/


 int LockFile (int fd, int flag)

 {
  struct flock fl;

   fl.l_type=F_WRLCK;
   fl.l_start=1;
   fl.l_whence=SEEK_SET;
   fl.l_len=0;

    if (fcntl(fd, F_GETLK, &fl)<0)
     {
      return (-1);
     }
    
   fl.l_start=1;
   fl.l_whence=SEEK_SET;
   fl.l_len=0;
         
    if (flag)
     {
      fl.l_type=F_WRLCK;

      return (fcntl(fd, F_SETLKW, &fl));  /* block if necessary! */
     }
    else
     {
       if (fl.l_type==F_UNLCK)
        {
         fl.l_type=F_WRLCK;

          if (fcntl(fd, F_SETLK, &fl)<0)
           {
            return (-1);
           }
          else
           {
            return 0;
           }
        }
       else
        {
         return fl.l_pid;
        }
     }

 }  /**/


 int UnlockFile (int fd)

 {
  struct flock fl;

   fl.l_type=F_UNLCK;
   fl.l_start=1;
   fl.l_whence=SEEK_SET;
   fl.l_len=0;

    return (fcntl(fd, F_SETLK, &fl));

 }  /**/

      
 void log (int i, ...)

 {
  va_list va;
  char *format;
  char s[600];
  char t[10],
       s2[600];
  time_t when;
  FILE *fp;
  
    if (first)
     {
      char s[80];
      FILE *fp;

       first=0;
       when=time (NULL);
        if ((fp=fopen(Logs.file1, "r"))!=NULL)
         {
          sprintf (s, "rm %s\n", Logs.file1);
          system (s);
          fclose (fp);
         }

        if ((fp=fopen(Logs.file2, "r"))!=NULL)
         {
          sprintf (s, "rm %s\n", Logs.file2);
          system (s);
          fclose (fp);
         }

        if ((fp=fopen(Logs.file1, "a+"))!=NULL)
         {
          strftime (s, 60, "%c", localtime(&when));
          fprintf (fp, "       --==[Logging started %s]==--\n\n", s);
          fclose (fp);
         }
     }   
         
   va_start (va, i);
   format=va_arg (va, char *);
   vsprintf (s, format, va);
   va_end (va);

   when=time (NULL);
   strftime (t, 11, "[%X]", localtime(&when));
   sprintf (s2, "%s %s\n", t, s);

   switch (i)
          {
           case 0:
                 say ("%s", s2);
                 break;

           case 1:
                   if ((fp=fopen(Logs.file1, "a+"))!=NULL)
                    {
                     fprintf (fp, "%s", s2);
                     fclose (fp);
                    }
                   else
                   if (errno!=EINTR)
                    {
                     say ( 
                          "%$% Error while opening log file %s: %s.\n", HOT,
                          Logs.file1, strerror(errno));
                    }
                  break;

           case 2:
                   if ((fp=fopen(Logs.file2, "a+"))!=NULL)
                    {
                     fprintf (fp, "%s", s2);
                     fclose (fp);
                    }
                   else
                    {
                     say ("%$% Error while opening log file %s\n", HOT, 
                           Logs.file2);
                    }
                  break;
          }

 }  /**/


 char *secstostr (time_t secs, int showsecs)

 {
  time_t hour,
         minute;
  static char s[]={[0 ... 100]=0};

   memset (s, 0, 100);

    if (secs>31104000)
     {
      sprintf (s, "%d year", (int)(secs/31104000));
      (secs/31104000>=2)?
       strcpy(&s[strlen(s)], "s, "):strcpy(&s[strlen(s)], ", ");
       secs=secs-((secs/31104000)*31104000);
     }

    if (secs>86400)
     {
      char p[20];

       sprintf (p, "%d day", (int)(secs/86400));
        (secs/86400>=2)?strcpy(&p[strlen(p)],"s, "):strcpy(&p[strlen(p)],", ");
       strcpy (&s[strlen(s)], p);
       secs=secs-((secs/86400)*86400);
     }

   hour=(secs/3600);
   secs=secs-(hour*3600);
   minute=(secs/60);

    if (showsecs)
     {
      secs=secs-(minute*60);
      sprintf (&s[strlen(s)], "%02d:%02d:%02d", 
               (int)hour, (int)minute, (int)secs);
     }
    else
     {
      sprintf (&s[strlen(s)], "%02d:%02d", (int)hour, (int)minute);
     }

   return (char *)s;

 }  /**/


 #include <sys/resource.h>

 char *CPUUsage (void)

 {
  time_t minute,
         second;
  struct rusage ru;
  static char s[10];

   getrusage (RUSAGE_SELF, &ru);

   minute=(int)((ru.ru_utime.tv_sec+ru.ru_stime.tv_sec)/60);
   second=(int)((ru.ru_utime.tv_sec+ru.ru_stime.tv_sec)-(minute*60));

   sprintf (s, "%02d:%02d", (int)minute, (int)second);

   return (char *)s;

 }  /**/


 int CountMail (void)

 {
  int n=0;
  char s[300];
  FILE *fp;
 
    if ((fp=fopen(data.mailbox, "r"))!=NULL)
     {
       while (fgets(s, 299, fp))
        {
          if (!feof(fp))
           {
             if (!strncmp("From ", s, 5))
              {
               n++;
              }
           }
        }

      fclose (fp);
     }

   return n;

 }  /**/


 void CheckMail (void) 
 {
  struct stat stbuf;

    if (data.mailbox==(char *)NULL)  return;

    if (stat(data.mailbox, &stbuf)<0)
     {
      data.mailbox=(char *)NULL;

      return;
     }

    if (/*(stbuf.st_size!=0)&&*/(stbuf.st_mtime>mt_ptr->mailbox_mt))
     {
      int cnmail;

        if ((cnmail=CountMail())>mt_ptr->nMail)
         {
          chtype chs[80];

           beep ();
           Flash (7, 1);
           chsprintf (chs, "%$% It appears that you've got new mail.", INFO);
           FancyOutput2 (CURWIN(), chs, CURWIN()->_cury, 77, 0);
           say ("\n");
         }

       mt_ptr->mailbox_mt=stbuf.st_mtime;
       mt_ptr->nMail=cnmail;

       DoUpperStatus ();
     }

   return;

 }  /**/


 void SurrenderControllingTerminal (void)

 {
  extern int flock_fd;

   endwin ();

    if (fork())
     {
      fflush (stdin);
      fflush (stdout);
      fflush (stderr);

      _exit (1);
     }

   setsid ();

   UnblockSignals ();
   alarm (0);
   alarm (15);

   mt_ptr->c_output|=BACKGROUND;
   freopen ("/dev/null", "r", stdin);  /* Yes! */
   freopen ("/dev/null", "w", stdout);
   freopen ("/dev/null", "w", stderr);

   /* log (1, "lock-fd: %d", flock_fd); */

   BlockSignals ();
   log (1,"Lock on file acquired successfully...%d",LockFile(abs(flock_fd), 1));
   UnblockSignals ();

   ut_ptr->pid=getpid ();

 }  /**/


 int copyfile (char *dst, char *src, int flag)

 {
  int fs, 
      fd,
      x; 
  char buf[512]; 
  struct stat st;

    if ((fs=open(src, O_RDONLY, 0))<0)
     {
      say ("%$% Unable to open %s for reading: %s\n", MILD,src,strerror(errno));

      return 0;
     }

   fstat (fs, &st);

    if (!(st.st_mode&S_IFREG)) 
     {
      say ("%$% %s is not regular file.\n", MILD, src);

      return 0;
     }

    if ((fd=creat(dst, (int)(st.st_mode&0777)))<0)
     {
      say ("%$% Unable to open %s for writing: %s\n", MILD,dst,strerror(errno));
      close (fs);

      return 0;
     }

    for (x=1; x>0; ) 
     {
      x=read (fs, buf, 512);

       if (x>0) 
        {
          if (write(fd, buf, x)<x) 
           {
            close (fs); 
            close (fd); 
            unlink (dst); 
   
            return 0;
           }
        }
     }

   close (fs); 
   close (fd); 

   return 1;

 }  /**/


 void About (void *ptr, int idx)

 {

  static Button buttons[]={
   {" Enough already... ", 20,  0, NULL}
  };
  static Buttons button={
   1, buttons
  };

  static MsgBox msgbox={
         {8, 65},
          0, 0,
          0, 51,
          " IRCIT 0.3.1 Copyright (c) 1998 Ayman Akt (ayman@POBoxes.com) \n       IRC client for the Information Terrorists! ",
          &button
         };

  LoadMsgBox (&msgbox);  

 }  /**/


#include <regex.h>


 int PatternMatch (const char *pattern, char *in)

 {
  int ret;
  regex_t regx;

    if (regcomp(&regx, pattern, REG_NOSUB|REG_EXTENDED|REG_ICASE))
     {
      return 0;
     }

   ret=regexec (&regx, in, 0, NULL, 0);

   regfree (&regx);

   return ret==0?1:0;

 }  /**/

