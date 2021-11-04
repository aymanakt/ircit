/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>
#include <terminal.h>
#include <updates.h>

#include <signal.h>

 MODULEID("$Id: main.c,v 1.8 1998/04/06 06:13:44 ayman Beta $");


 int main (int argc, char *argv[])

 {
   CheckCommandLine (argc, &*argv);
   InitTerminal ();
   /* InitSignals (); */
   InitInterpreter ();
   BuildPreferencesTable ();
   InitPeople ();
   ProcessConfigfile ();
   BuildSymbolTable ();
   InitNet ();
   InitTimers ();
   InitSignals (); /*!*/
   SessionUp ();

    while (1)
     {
      ProbeSockets();
     }

 }  /*EOM*/


 void Help (void)

 {
   printf (" ircit [-h]\n"
           "       [-v]\n"
           "       [-r [ttyn [idx]]]\n"
           "       [[-b] [-p port] [-c channel] [nick] [server]]\n");
 
   _exit (1);

 }  /**/


 void InitStuffandThings (void)

 {
   InitSessionTable ();
   InitMisc ();
   InitHistories ();
   InitSocketsTable ();
   InitChannelsTable ();
   InitHoldQueue ();
   InitStack ();

 }  /**/


 void CheckCommandLine (int argc, char *argv[])
 
 {
  int i, 
      j=1,
      cnt=0;
  extern char *c_version,
              *u_compiled,
              *t_compiled;

    if (argc==1)
     {
      return;
     }
    
    for (i=1; i<argc; i++)
     { 
      switch (argv[i][0])
       { 
        case '-':
               while ((j)&&(argv[i][j]))
                { 
                 switch (argv[i][j])
                  { 
                   case 'h':
                   case 'H':
                   case '?':
                   Help ();

                   case 'v':
                   case 'V':
                         fprintf (stderr, 
                                  "\n %s (c) Ayman Akt.\n"
                                  " Compiled by %s on %s\n", 
                                  c_version, u_compiled, t_compiled);

                         _exit (1);

                   case 'p':
                         i++;
                         j--;
                          if (argv[i])
                           {
                            st_ptr->c_serverport=atoi(argv[i]);
                           }
                         break;

                   case 'c':
                         i++;
                         j--;
                          if (argv[i])
                           {
                            mstrncpy (cht_ptr->c_channel, argv[i], MAXCHANLEN);
                           } 
                         break;
                                      
                   case 'b':
                         i++;
                         j--;
                         mt_ptr->c_output|=BACKGROUND;
                         break;

                   case 'r':
                         i++;
                         j--;
                         fprintf (stderr, " %s (c) Ayman Akt\n\n", c_version);

                         RestoreSession (argv[i]?argv[i]:NULL,
                                         argv[i+1]?atoi(argv[i+1]):0); 

                         return;

                   default:
                         j--; 
                         break;
                  } 
                 j++;  
                 break; 
                }  
              break;  

        default:
              cnt++;
              j=1;
              switch (cnt)
               {
                case 1:
                     strcpy (st_ptr->c_nick, argv[i]/*, MAXNICKLEN*/);
                     break;

                case 2:
                     strcpy (st_ptr->c_server, argv[i]/*, MAXHOSTLEN*/);
                     break;
                         
                default:
                     break;
               }
       }  
     }  

 }  /**/                     


 int RestoreSession (char *, int);

 #include <sys/stat.h>
 #include <sys/un.h>
 #include <sys/socket.h>

 int RestoreSession (char *tty, int idx)

 {
  int fd,
      len;
  char path[_POSIX_PATH_MAX];
  fd_set rfd;
  struct sockaddr_un addr;
  struct param {
          pid_t pgrp,
                pid;
          char tty[20];
        };
  struct param x;

  fprintf (stderr, "\n Waking up the living from the dead...");

   if ((fd=socket(AF_UNIX, SOCK_STREAM, 0))<0)
    {
     printf ("\n Unable to obtain socket for IPC client - %s.\n",
            strerror(errno));

     _exit (1);
    }

   memset (&addr, 0, sizeof(addr));

   addr.sun_family=AF_UNIX;
   sprintf (addr.sun_path, "%s/.ircit/knock-knock", ut_ptr->homedir);
   strcpy (path, addr.sun_path);
   len=strlen(addr.sun_path)+sizeof(addr.sun_family);

   unlink (addr.sun_path);

    if (bind(fd, (struct sockaddr *)&addr, len)<0)
     {
      printf ("\n Unable to bind address: %s\n", strerror(errno));

      _exit (1);
     }

    if (chmod(addr.sun_path, S_IRWXU)<0)
     {
      printf ("\n Unable to procede: %s\n",strerror(errno));

      unlink (addr.sun_path);

      _exit (1);
     }

   memset (&addr, 0, sizeof(addr));

   addr.sun_family=AF_UNIX;
   sprintf (addr.sun_path, "%s/.ircit/ipc-%d", ut_ptr->homedir, idx);
   len=strlen(addr.sun_path)+sizeof(addr.sun_family);

    if (connect(fd, (struct sockaddr *)&addr, len)<0)
     {
      printf ("\n Unable to contact your client (%s), is it running at all?\n"                , strerror(errno));

      unlink (path);

      _exit (1);
     }

   x.pgrp=getpgrp ();
   x.pid=getpid ();
   sprintf (x.tty, "/dev/%s", tty);

   printf ("\n Sending connection details: pgrp: %d  pid: %d %s\n", 
           x.pgrp, x.pid, x.tty);

   write (fd, &x, sizeof(struct param));

    if (!read(fd, &x, sizeof(struct param)))
     {
      printf (" Unable to re-launch your client.\n");
      unlink (path);

      _exit (1);
     }

   fprintf (stderr, "\a Done! Your client should be attached to %s by now!\n", 
                    x.tty);

   unlink (path);

   _exit (1);

 }  /**/

