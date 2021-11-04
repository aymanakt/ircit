/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <main.h>
#include <terminal.h>
#include <output.h>
#include <updates.h>
#include <prefs.h>
#include <list.h>
#include <scroll.h>
#include <history.h>
#include <servers.h>
#include <notify.h>
#include <misc.h>
#include <configfile.h>

#include <sys/stat.h>

 MODULEID("$Id: configfile.c,v 1.8 1998/03/21 09:41:50 ayman Beta $");


 /*
 ** should return the size of the file
 */
 static __inline__ unsigned long FileExist (char *file)

 {
  char path[_POSIX_PATH_MAX];
  struct stat st;

    if (ut_ptr->homedir)
     {
      sprintf (path, "%s/.ircit/ircit.conf", ut_ptr->homedir);
     }
    else
     {
      strcpy (path, "ircit.conf");
     }

   strcpy (file, path);

    if ((access(path, F_OK|R_OK))<0)
     {
      say ("%$% Configuration file's missing! Reverting to defaults - %s.\n",
           HOT, strerror(errno));

      return 0;
     }

    if ((stat(path, &st))<0)
     {
      say ("%$% Configuration file's missing! Reverting to defaults - %s.\n", 
           HOT, strerror(errno));

      return 0;
     }

    /*if (st.st_size==0);
       {
        say ("%$% Configuration file has zero length! Reverting to defaults.\n",
             HOT);

        return 0;
       }*/

    //strcpy (file, path);

    return st.st_size;

 }  /**/


 static __inline__ ProcessSERVERS (char *line)

 {
  char *s;
  unsigned long port;
  extern List *servers;
 
   s=splitcolon (&line);

   line?port=atoul(line):atoul(valueof("DEFAULT_SERVER_PORT"));

    AddtoServersList (servers, s, port);

 }  /**/


 static __inline__ ProcessCHANNELS (char *line)

 {


 }  /**/


 static __inline__ ProcessSETS (char *line)

 {
  char *s;

   s=splitw (&line);

    if (!line)
     {
      /* `DEFAULT_NAME' empty set definition */
      return;
     }

   wipespaces (line);

   SetPref (s, line);  /* check for errors here */

   return;

 }  /**/


 static __inline__ ProcessIRCNOTIFY (char *line)

 {
  extern List *irc_notify;

   AddtoNotify (irc_notify, line, 0);
   UpdateHistoryTable (H_UTMP, ht_ptr->nNotify, 0);

 }  /**/


 static __inline__ ProcessUTMPNOTIFY (char *line)

 {
  extern List *utmp_notify;

   AddtoNotify (utmp_notify, line, 0);
   UpdateHistoryTable (H_UTMP, ht_ptr->nUTMP, 0);

 }  /**/


 static __inline__ ProcessINETNOTIFY (char *line)

 {


 }  /**/


 static __inline__ void ActuallyProcess (char *line, int mode)

 {
    if (mode&SERVERS)
     {
      ProcessSERVERS (line);
     }
    else
    if (mode&CHANNELS)
     {
      ProcessCHANNELS (line);
     }
    else
    if (mode&IRCNOTIFY)
     {
      ProcessIRCNOTIFY (line);
     }
    else
    if (mode&UTMPNOTIFY)
     {
      ProcessUTMPNOTIFY (line);
     }
    else
    if (mode&SETS)
     {
      ProcessSETS (line);
     }

 }  /**/


 void ProcessSection (char *section, int mode)

 {
  int fetch=0;
  char line[80],
       path[_POSIX_PATH_MAX];
  unsigned long _section=0;
  FILE *fp;

    if (!FileExist(path))
     {
      return;
     }

    if (!(fp=fopen(path, "r")))
     {
      say ("%$% Configuration file's missing! Reverting to defaults.\n",HOT);

      return;
     }
  
   say ("%$% Rehashing section %s in your configuration file...\n",
        INFO, section); 

    while (!feof(fp))
          {
           fgets (line, 79, fp);

            if (!feof(fp))
               {
                wipespaces (line);

                 if ((line[0]!='#')&&(line[0]))
                    {
                      if (line[0]=='[')
                         {
                           if (arethesame(line, section))
                              {
                                if (fetch)
                                   {
                                    say ("%$% Inconsistent Configuration file"
                                         " structure.\n", MILD);
                                    fclose (fp);

                                    return;
                                   }
                                else 
                                   {
                                    fetch=1;
                                   }
                              }
                           else
                              {
                                if (fetch)
                                   {
                                    return;
                                   }
                                else
                                   {
                                    continue;
                                   }
                              }
                         }
                      else 
                         {
                           if (fetch)
                              {
                               ActuallyProcess (line, mode);
                              }
                         }
                    }
               }
          }

   fclose (fp);

 }  /**/


 #define progress \
        if (j<=20.0) \
            z=PRO10; \
         else \
         if ((j>20.0)&&(j<=40.0)) \
            z=PRO20; \
         else \
         if ((j>40.0)&&(j<=60.0)) \
            z=PRO30; \
         else \
         if ((j>60.0)&&(j<=80.0)) \
            z=PRO40; \
         else \
         if ((j>80.0)) \
            z=PRO50

 
 int ProcessConfigfile (void)

 {
  char line[80],
       path[_POSIX_PATH_MAX];
  int n=0;
  unsigned long size,
                section=0;
  float j;
  FILE *fp;
  chtype *z=0;
  chtype PRO00[]={[0 ... 4]=TIN, [5]=0},
         PRO10[]={[0]=TIMB, [1 ... 4]=TIM, [5]=0},
         PRO20[]={[0 ... 1]=TIMB, [2 ... 4]=TIM, [5]=0},
         PRO30[]={[0 ... 2]=TIMB, [3 ... 4]=TIM, [5]=0},
         PRO40[]={[0 ... 3]=TIMB, [4]=TIM, [5]=0},
         PRO50[]={[0 ... 4]=TIMB, [5]=0};
  struct stat st;

    size=FileExist (path);
      
    /*if ((stat(path, &st))<0)
       {
        say ("%$% Configuration file's missing! Reverting to defaults.\n",HOT);

        return;
       }*/

    if (!size)
       {
        mt_ptr->s_attrs|=S_NOCONFIG;

        return;
       }

    if (!(fp=fopen(path, "r")))
       {
        say ("%$% Configuration file's missing! Reverting to defaults.\n",HOT);

        mt_ptr->s_attrs|=S_NOCONFIG;

        return;
       }

   say ("%$% Processing Configuration File", INFO);

    while (!feof(fp))
          {
           fgets (line, 79, fp);
           n++;

            if (!feof(fp))
               {
                j+=((float)strlen(line)/(float)/*st.st_*/size)*100.0;
                progress;
                mwmove (CURWIN()->_cury, 32);
                Fwprintf (CURWIN(), "[%z]  %d lines...", z, n);
                doupdate ();
                //napms(2);

                wipespaces (line);

                if ((line[0]!='#')&&(line[0]))
                   {
                    if (line[0]=='[')
                       {
                         if ((strncasecmp(&line[0], "[SERVERS]", 9)==0) &&
                             !(section&SERVERS))
                            {
                             section|=SERVERS;
                             section&=~(CHANNELS|SETS|IRCNOTIFY|UTMPNOTIFY);
                             continue;
                            }

                         if ((strncasecmp(&line[0], "[CHANNELS]", 10)==0) &&
                             !(section&CHANNELS))
                            {
                             section|=CHANNELS;
                             section&=~(SERVERS|SETS|IRCNOTIFY|UTMPNOTIFY);
                             continue;
                            }

                         if ((strncasecmp(&line[0], "[SETS]", 6)==0) &&
                             !(section&SETS))
                            {
                             section|=SETS;
                             section&=~(SERVERS|CHANNELS|IRCNOTIFY|UTMPNOTIFY);
                             continue;
                            }

                         if ((strncasecmp(&line[0], "[IRC NOTIFY]", 12)==0) &&
                             !(section&IRCNOTIFY))
                            {
                             section|=IRCNOTIFY;
                             section&=~(SERVERS|CHANNELS|SETS|UTMPNOTIFY);
                             continue;
                            }

                          if ((strncasecmp(&line[0], "[UTMP NOTIFY]", 13)==0) &&
                             !(section&UTMPNOTIFY))
                            {
                             section|=UTMPNOTIFY;
                             section&=~(SERVERS|CHANNELS|SETS|IRCNOTIFY);
                             continue;
                            }

                        section&=~THELOT;
                       }

                    else
                    if (line[0])
                       {
                        ActuallyProcess (line, section);
                       }
                   }
               }
          }

   fclose (fp);

   mwmove (CURWIN()->_cury+1, 0);

 }  /**/


 #define HELP(x) ((*(x)=='h')||(*(x)=='H')||(*(x)=='?'))&&(!*((x)+1))

 #define RESET_SERVERS() \
         if (servers->nEntries) \
            { \
             ClearServersList (servers, ALL); \
            }

 #define RESET_UTMPNOTIFY() \
         if (utmp_notify->nEntries) \
            { \
             ClearNotifyList (utmp_notify, EVERYTHING, NULL); \
            }

 #define RESET_IRCNOTIFY() \
         if (irc_notify->nEntries) \
            { \
             ClearNotifyList (irc_notify, EVERYTHING, NULL); \
            }

 /*
 ** if the user requested a rehash and the config file wasn't there everything 
 ** will be wiped out, also true for indivisual sections of the file
 */
 STD_IRCIT_COMMAND(uREHASH)

 {
  int mode=0;
  extern List *servers;
  extern List *irc_notify;
  extern List *utmp_notify;

    if ((args)&&(*args=='-'))
       {
         if (arethesame(args, "-*"))
            {
             ;
            }
         else
         if (arethesame(args, "-SRV"))
            {
             mode|=1;
            }
         else
         if (arethesame(args, "-SET"))
            {
             mode|=2;
            }
         else
         if (arethesame(args, "-CHAN"))
            {
             mode|=4;
            }
         else
         if (arethesame(args, "-IRC"))
            {
             mode|=8;
            }
         else
         if (arethesame(args, "-UTMP"))
            {
             mode|=16;
            }
         else
         if (arethesame(args, "-INET"))
            {
             mode|=32;
            }
         else
         if (HELP(args+1))
            {
             say ("%$% Help on REHASH:\n%s\n%s\n", INFO,
                  "  REHASH [-section]",
                  "  Valid sections: *.*, SRV, IRC, UTMP, INET.");

            return;
           }
        else
           {
            say ("%$% You really need -h.\n", MILD);

            return;
           }
       }
    else
    if (args)
       {
        say ("%$% You really need -h.\n", MILD);

        return;
       }

   switch (mode&(1|2|4|8|16|32))
          {
           case 1:
               RESET_SERVERS();

               ProcessSection ("[SERVERS]", 1);
               break;

           case 2:
               ProcessSection ("[SETS]", 2);
               break;

           case 4:
               ProcessSection ("[CHANNELS]", 4);
               break;

           case 8:
               RESET_IRCNOTIFY();

               ProcessSection ("[IRC NOTIFY]", 8);
               break;

           case 16:
                RESET_UTMPNOTIFY();

                ProcessSection ("[UTMP NOTIFY]", 16);
                break;

           case 32:
               say ("%$% INET Notification isn't implemented yet.\n", MILD);
               break;

           default:
               RESET_SERVERS();
               RESET_IRCNOTIFY();
               RESET_UTMPNOTIFY();

               ProcessConfigfile ();
          }

 }  /**/


 static void WriteSERVERS (FILE *fpi)

 {
  register ListEntry *lptr;
  extern List *servers;
  #define vptr ((Server *)lptr->whatever)

   fprintf (fpi, "[SERVERS]\n");

    if (!servers->nEntries)
     {
      say ("%$% Servers List empty. Not proceeding with write.\n", MILD);

      return;
     }

    for (lptr=servers->head; lptr!=NULL; lptr=lptr->next)
     {
      fprintf (fpi, " %s:%u\n", vptr->server, vptr->port);
     }

   fprintf (fpi, "\n");
   fflush (fpi);

 }  /**/


 static void PrintSETEntries (FILE *fpi, PrefsTable *ptr)

 {
    if (ptr)
     {
      PrintSETEntries (fpi, ptr->left);
       if (ptr->entry.value)
        {
         fprintf (fpi, " %s %s\n", ptr->entry.preference, ptr->entry.value);            }
      PrintSETEntries (fpi, ptr->right);
     }

 }  /**/


extern PrefsTable *preferences;

 static void WriteSETS (FILE *fpi)

 {
   fprintf (fpi, "\n[SETS]\n");

   PrintSETEntries (fpi, preferences);

   fprintf (fpi, "\n");
   fflush (fpi);

 }  /**/


 static void WriteIRCNOTIFY (FILE *fpi)

 {
  register ListEntry *lptr;
  extern List *irc_notify;
  #define nptr ((Notify *)lptr->whatever)

   fprintf (fpi, "\n[IRC NOTIFY]\n");

    if (!irc_notify->nEntries)
     {
      say ("%$% Notify List empty. Not proceeding with write.\n", MILD);

      return;
     }

    for (lptr=irc_notify->head; lptr!=NULL; lptr=lptr->next)
     {
      fprintf (fpi, " %s\n", nptr->name);
     }

   fprintf (fpi, "\n");
   fflush (fpi);

 }  /**/


 static void WriteUTMPNOTIFY (FILE *fpi)

 {
  register ListEntry *lptr;
  extern List *utmp_notify;

   fprintf (fpi, "\n[UTMP NOTIFY]\n");

    if (!utmp_notify->nEntries)
     {
      say ("%$% Notify List empty. Not proceeding with write.\n", MILD);

      return;
     }

    for (lptr=utmp_notify->head; lptr!=NULL; lptr=lptr->next)
     {
      fprintf (fpi, " %s\n", nptr->name);
     }

   fprintf (fpi, "\n");
   fflush (fpi);

 }  /**/


 static __inline__ int
  WriteSection (FILE *fpi, FILE *fpo, char *section, void (*func)(FILE *))

 {
  int was_there=0;
  char line[80],
       *s;
  time_t t=time(NULL);
  extern char *c_version;


    if (arethesame(section, "*.*"))
     {
      was_there++;

      say ("%$% Recreating your Configureation file...\n", INFO);

      fprintf (fpi, "\n ## Configuration file for %s (c) Ayman Akt.\n"
                    " ## Generated %s\n", c_version, ctime(&t));

      WriteSERVERS (fpi);
      WriteSETS (fpi);
      WriteIRCNOTIFY (fpi);
      WriteUTMPNOTIFY (fpi);

      fflush (fpi);
       
      say ("%$% Done!\n", INFO);
     }
    else
     {
      say ("%$% Updating section %s in your Configuration file...\n", 
           INFO, section);

       while (!feof(fpo))
        {
         fgets (line, 79, fpo);
         s=line;
         TRIM(s);

          if (s[strlen(s)-1]=='\n') s[strlen(s)-1]=0;

          if ((*s)&&(*s=='['))
           {
             if (arethesame(s, section))
              {
               was_there++;

               (*func)(fpi);

                while (!feof(fpo))
                 {
                  fgets (line, 79, fpo);

                   if (!feof(fpo))
                    {
                     s=line;
                     TRIM(s);

                      if (*s=='[')  break;
                    }
                 }

               fprintf (fpi, "%s", line);  /* print the last line */

               break;
              }
             else
              {
               fprintf (fpi, "%s\n", line);
              }
           } 
          else
           {
            fprintf (fpi, "%s\n", line);
           }
        }

        //fprintf (fpi, "%s", line);

       while (!feof(fpo))
        {
         fgets (line, 79, fpo);

          if  (!feof(fpo))
           {
            fprintf (fpi, "%s", line);
           }
        }
     }

    if (!was_there)
     {
      say ("%$% Section %s in your Configuration File was missing.\n",
           MILD, section);

      fflush (fpi);
      lseek (fileno(fpi), 0, SEEK_END); /* goto end of file */

      (*func)(fpi);
     }       

 }  /**/


 #include <fcntl.h>

 static void SaveConfigFile (char *section, void (*func)(FILE *))

 {
  char spath[_POSIX_PATH_MAX],
       dpath[_POSIX_PATH_MAX];
  FILE *fpi,
       *fpo;

    if (!FileExist(spath))
     {
      FILE *fp;

       if ((mt_ptr->s_attrs&S_NOCONFIG)==0)
        {
         say ("%$% Your Configuration File gone missing...\n", MILD);
        }

       if (!(fp=fopen(spath, "w")))
        {
         say ("%$% Unable to create new Configuration File: %s\n", MILD,
              strerror(errno));

         return;
        }

      mt_ptr->s_attrs&=~S_NOCONFIG;  /* shouldn't harm anyone..*/
      //fprintf (fp, "\n");
      fclose  (fp);
     }

   strcpy (dpath, spath);

    if (strlen(dpath)==_POSIX_PATH_MAX)
     {
      dpath[strlen(dpath)-1]='~';
     }
    else
     {
      strcat (dpath, "~");
     }

   unlink (dpath);

    if (!copyfile(dpath, spath, 0))
     {
      return;
     }

   unlink (spath);

    if (!(fpi=fopen(dpath, "r")))
     {
      say ("%$% Unable to open %s for reading.", dpath, MILD);

      return;
     }

    if (!(fpo=fopen(spath, "w")))
     {
      say ("%$% Unable to open %s for writing.", spath, MILD);

      return;
     }

   WriteSection (fpo,  fpi, section, func);
   fclose (fpo);
   fclose (fpi);

 }  /**/


 STD_IRCIT_COMMAND(uSAVE)

 {
  int mode=0;

    if ((args)&&(*args=='-'))
       {
         if (arethesame(args, "-*"))
            {
             ;
            }
         else
         if (arethesame(args, "-SRV"))
            {
             mode|=1;
            }
         else
         if (arethesame(args, "-SET"))
            {
             mode|=2;
            }
         else
         if (arethesame(args, "-CHAN"))
            {
             mode|=4;
            }
         else
         if (arethesame(args, "-IRC"))
            {
             mode|=8;
            }
         else
         if (arethesame(args, "-UTMP"))
            {
             mode|=16;
            }
         else
         if (arethesame(args, "-INET"))
            {
             mode|=32;
            }
         else
         if (HELP(args+1))
            {
             say ("%$% Help on SAVE:\n%s\n%s\n", INFO,
                  "  SAVE [section]",
                  "  Valid sections: -*, -SRV, -SET, -IRC, -UTMP, -INET.");

            return;
           }
        else
           {
            say ("%$% You really need -h.\n", MILD);

            return;
           }
       }
    else
    if (args) 
       {
        say ("%$% You really need -h.\n", MILD);

        return;
       }

    if (mt_ptr->s_attrs&S_MULTIBLE)
       {
        say ("%$% You cannot modify your Configuration File.\n", MILD);

        return;
       }

    if (mt_ptr->s_attrs&S_NOCONFIG)
       {
        say ("%$% Session was started with missing Configuration File.\n",INFO);
       }

   switch (mode&(1|2|4|8|16|32))
          {
           case 1:
               SaveConfigFile ("[SERVERS]", (void (*)(FILE *))WriteSERVERS);
               break;

           case 2:
               SaveConfigFile ("[SETS]", WriteSETS);
               break;

           case 4:
               break;

           case 8:
               SaveConfigFile ("[IRC NOTIFY]", WriteIRCNOTIFY);
               break;

           case 16:
                SaveConfigFile ("[UTMP NOTIFY]", WriteUTMPNOTIFY);
                break;

           case 32:
               say ("%$% INET Notification isn't implemented yet.\n", MILD);
               break;

           default:
               SaveConfigFile ("*.*", NULL);
          }

 }  /**/


 void MenuRehash (void *ptr, int idx)

 {
   switch (idx)
          {
           case 0:
                uREHASH ("-*");
                break;

           case 1:
                uREHASH ("-SET");
                break;

           case 2:
                uREHASH ("-SRV");
                break;

           case 3:
                //uREHASH ("-CHAN");
                break;

           case 4:
                uREHASH ("-IRC");
                break;

           case 5:
                uREHASH ("-UTMP");
                break;
          }

 }  /**/


 void MenuSave (void *ptr, int idx)

 {
   switch (idx)
          {
           case 0:
                uSAVE ("-*");
                break;

           case 1:
                uSAVE ("-SET");
                break;

           case 2:
                uSAVE ("-SRV");
                break;

           case 3:
                uSAVE ("-CHAN");
                break;

           case 4:
                uSAVE ("-IRC");
                break;

           case 5:
                uSAVE ("-UTMP");
                break;
          }

 }  /**/

