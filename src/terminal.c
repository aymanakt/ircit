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
#include <scroll.h>
#include <colors.h>
#include <headers.h>
#include <figures.h>
#include <list.h>
#include <updates.h>
#include <history.h>
#include <prefs.h>

#include <stdarg.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/kd.h>
#include <sys/stat.h>
#ifdef _HAVE_PTHREAD
# include <pthread.h>
#endif
#ifdef _HAVE_GPM
# include <gpm.h>
# include <mouse.h>
#endif

#include <misc.h>
#include <u_status.h>

#ifdef _HAVE_LOCALE
# include <locale.h>
#endif

 
 MODULEID("$Id: terminal.c,v 1.11 1998/04/06 06:13:44 ayman Beta $");

 char buf_i[LARGBUF];
 chtype *f=Figs;
 chtype *chf=CHFigs;
 int status_busy;
 int t_cols,
     t_rows;

 WINDOW *MainWin,
        *MmainWin,
        *SpareWin,
        *InputWin,
        *StatusWin,
        *SstatusWin, 
        *UpperStatusWin;

 extern void (*status[])(void);

#include <thread.h>

#define TH_FLAGS  CLONE_FS


#include <termios.h>

 static void FixTerminal (void)

 {
  struct termios term;

    if (!(isatty(STDIN_FILENO)))  return;

    if ((tcgetattr(STDIN_FILENO, &term))<0)
     {
      fprintf (stderr, "Unable to fetch terminal settings - %s.\n",
               strerror(errno));

      return;
     }

   term.c_cc[VSTOP]=_POSIX_VDISABLE;
   term.c_cc[VSTART]=_POSIX_VDISABLE;

   term.c_cc[VINTR]=17;

    if ((tcsetattr(STDIN_FILENO, TCSANOW, &term))<0)
     {
      fprintf (stderr, "Unable to change terminal settings - %s.\n",
               strerror(errno));
     }

 }  /**/


 void InitTerminal (void)

 {
  char p[20];
  char *stack __attribute__((unused));
  extern void InitSoundServer (void *);

   srand (time(NULL));

   FetchTerminalSize ();

#ifdef _HAVE_LOCALE
   setlocale (LC_ALL, "");
#endif

   BlockttyMsg (0);

   /*xmalloc(stack, 1024);

    if ((new_thread(InitSoundServer, (void *)NULL, TH_FLAGS, stack, 1024))<0)
       {
        nYell ("%$% Unable to initiate SoundServer.\n", HOT);
       }*/

 #undef TH_FLAGS

    if (!getenv("TERM"))
     {
      char s[20];

       fprintf (stderr, "\nYour terminal type doesn't seem to be set, "
                        "choose one now!\n" 
                        "Hit enter for default [linux]: ");

       fgets (s, sizeof(s), stdin);

       s[strlen(s)-1]=0;

        if (!strlen(s))
         {
          sprintf (p, "TERM=%s", "linux");
         }
        else
         {
          sprintf (p, "TERM=%s", s);
         }

       putenv (p);

       printf ("\nTerminal type is now set to: %s\n", getenv("TERM"));
       printf ("Loading IRCIT in a sec...\n");

       fflush (stdout);
       fflush (stderr);
       sleep (1);
     }

   sprintf (p, "ESCDELAY=5");
   putenv (p);

    if (!(stdscr=initscr()))
     {
      fprintf (stderr, "stdscr initialization failed - bye...\n");

      _exit (1);
     }

    if (has_colors())
     {
      int i;

       start_color ();

        for (i=1; i<COLOR_PAIRS; i++)
         {
          init_pair (i, i%COLORS, i/COLORS);
         }
     }

   cbreak (); 
   //raw();
   noecho ();
   typeahead (-1);
   curs_set (0);

   FixTerminal ();

   InitMainWin ();
   InitLowerStatusWin ();
   InitUpperStatusWin ();
   InitInputWin ();
   InitSpareWin();
#ifdef _HAVE_GPM
   /*InitMouse ();*/
#endif

 }  /**/


 void FetchTerminalSize (void)

 {
  struct winsize sz;

    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &sz)<0)
       {
        fprintf (stderr, "Unable to fetch terminal size.\n");

        exit (1);
       }

   t_rows=sz.ws_row;
   t_cols=sz.ws_col;

 }  /**/


 void ResizeTerminal (int v)

 {
  int WC,
      WL;

 }  /**/


 int BlockttyMsg (int how)

 {
  struct stat stat;

    if (!isatty(STDIN_FILENO))
       {
        printf ("No stdin defined - Aborting.\n");

        exit (1);
       }
   
   switch (how)
          {
           case 0:
                  if (!fstat(STDERR_FILENO, &stat))
                     {
                      stat.st_mode&=~0100;
                      fchmod (STDERR_FILENO, stat.st_mode);
                     }
                  else
                     {
                      printf ("STDERR_FILENO: %s", strerror(errno));
                     }

                 memset (&stat, 0, sizeof(struct stat));
                    
                  if (!fstat(STDIN_FILENO, &stat))
                     {
                      stat.st_mode&=~022;
                      fchmod (STDIN_FILENO, stat.st_mode);
                     }
                  else
                     {
                      printf ("STDIN_FILENO: %s", strerror(errno));
                     }

          }

 }  /**/


 int LocateFreetty (char *buf)

 {
  register int i;
  int fd;
  register char *idx1;

   strcpy (buf, "/dev/tty  ");
   buf[8]=buf[9]=0;

    for (i=0; i<=3; i++)
        {
         buf[8]=i+48;

          for (idx1="0123456789"; *idx1; idx1++)
              {
                if (i==0)
                   {
                     if (*idx1=='0') continue;

                    buf[8]=*idx1;
                   }
                else
                   {
                    buf[9]=*idx1;
                   }

                if ((fd=open(buf, O_RDWR))<0)
                   {
                     if (errno==ENOENT)
                        {
                         return (-1);
                        }
                     else
                        {
                         continue;
                        }
                   }

               return fd;
              }
        }

   return (-1);

 }  /**/


 void ResetTerminal (void)

 {
    if (mt_ptr->idx>=0)
     {
      char path[_POSIX_PATH_MAX];

       sprintf (path, "%s/.ircit/IRCIT.%d", ut_ptr->homedir, mt_ptr->idx);
       unlink (path);
            
       sprintf (path, "%s-%d", valueof("IPC_PATH"), mt_ptr->idx);
       unlink (path);

        if (stdscr)
         {
           if (set("CLEAR_ON_EXIT"))
            {
             clear ();
             refresh ();
            }
         }
     }

    if (stdscr)  endwin ();

   fflush (stdout);
   fflush (stderr);

 }  /**/


 void UpdateWindows (which)

 {
    if (which&EVERYTHING)
     {
      wnoutrefresh (UpperStatusWin);
      wnoutrefresh (MainWin);
      wnoutrefresh (MmainWin);
      wnoutrefresh (SpareWin);
      wnoutrefresh (StatusWin);
      wnoutrefresh (SstatusWin);
      wnoutrefresh (InputWin);

      goto update;
     }

    if (which&UPPERSTATUS)
       wnoutrefresh (UpperStatusWin);
    if (which&MAINWIN)
       wnoutrefresh (MainWin);
    if (which&MAINWINBORDER)
       wnoutrefresh (MmainWin);
    if (which&LOWERSTATUS)
       wnoutrefresh (StatusWin);
    if (which&LOWERSTATUSBORDER)
       wnoutrefresh (SstatusWin);
    if (which&INPUTWIN)
       wnoutrefresh (InputWin); 
       
   update: 
          doupdate ();

 }  /**/


 void InitMainWin (void)

 {
  int MWX, 
      MWY, 
      MWL, 
      MWC;
  int SMWX, 
      SMWY, 
      SMWL, 
      SMWC;

    if (t_rows<9)
     {
      fprintf (stderr, 
               "Please choose functional values for your terminal size.\n");

      exit (1);
     }

   t_cols>80?(MWC=80):(MWC=t_cols);
   t_rows>25?(MWL=19):(MWL=(t_rows-6));   
   MWY=1;
   MWX=0;

   SMWC=MWC-2;
   SMWL=MWL-2;
   SMWY=SMWX=1;

    if (!(MmainWin=newwin(MWL, MWC, MWY, MWX/*19, 80, 1, 0*/)))
     {
      fprintf (stderr, "-!- Unable to start IRCIT - main WINDOW"
                       " struct creation error!\n");
      exit (1);
     }

   box (MmainWin, ACS_VLINE, ACS_HLINE);
   ExplodeWindow (MmainWin, EXPLODE_OUT);
  
    if (!(MainWin=derwin(MmainWin, SMWL, SMWC, SMWY, SMWX/*17, 78, 1, 1*/)))
     {
      fprintf (stderr, "-!- Unable to start IRCIT - sub-main"
                       " WINDOW struct creation error!\n");
      exit (1);
     }

   scrollok (MainWin,  TRUE);
   scrollok (MmainWin, FALSE);

   (WINDOW *)mt_ptr->c_window=MainWin;

 } /**/


 void RedrawMainWin (void)

 {
  extern int nLines;

    if (CURWIN()!=MainWin)
     {
      say ("%$% Main window currently suspended.\n", MILD);
      
      return;
     }

   ExplodeWindow (MmainWin, EXPLODE_IN|EXPLODE_OUT);

   werase (MainWin);
   box (MmainWin, ACS_VLINE, ACS_HLINE);
   UpdateUpper ();
   isup()?UpdateLower(1):UpdateLower(0);

   wmove (MainWin, 0, 0);
   nLines=0;

   wnoutrefresh (MmainWin);
   wnoutrefresh (MainWin);

 }  /**/


 void InitSpareWin (void)

 {
    if (!(SpareWin=newwin(2, t_cols-2, MmainWin->_regbottom+3, 1)))
     {
      fprintf (stderr, "-!- Unable to create spare WINDOW struct!\n");
      exit (1);
     }

   scrollok (SpareWin, TRUE);
   noecho ();

   werase (SpareWin);

 }  /**/


 void InitUpperStatusWin (void)

 {
  int USWL,
      USWC,
      USWY,
      USWX;
  
   USWL=1;
   USWC=t_cols;
   USWY=USWX=0;

    if (!(UpperStatusWin=newwin(USWL, USWC, USWY, USWX)))
     {
      fprintf (stderr, "Unable to create u-status WINDOW struct!\n");
      exit (1);
     }

   wattrset (UpperStatusWin, COLOR_PAIR(CLR_UPPERSTAT));
   wbkgd (UpperStatusWin, COLOR_PAIR(CLR_UPPERSTAT));

   status[1]=DisplayUpperStatusI;
   status[2]=DisplayUpperStatusII;
   status[3]=DisplayUpperStatusIII;

 }  /**/


 void RedrawUpperStatus (void)

 {
   wattrset (UpperStatusWin, COLOR_PAIR(CLR_UPPERSTAT));
   wbkgd (UpperStatusWin, COLOR_PAIR(CLR_UPPERSTAT));

   DoUpperStatus ();

 }  /**/


 void InitLowerStatusWin (void)

 {
  int LWC,
      LWL,
      LWY,
      LWX;
  int SLWC,
      SLWL,
      SLWY,
      SLWX;

   LWC=t_cols;
   LWL=4;
   LWX=0;
   LWY=t_rows-5;

   SLWC=t_cols-2;
   SLWL=2;
   SLWX=1;
   SLWY=1;

    if (!(SstatusWin=newwin(LWL, LWC, LWY, LWX)))
     {
      fprintf (stderr, "-!- Unable to create l-status WINDOW structure!\n");
      exit (1);
     }

    if (!(StatusWin=derwin(SstatusWin, SLWL, SLWC, SLWY, SLWX)))
     {
      fprintf (stderr, "-!- Unable to create sl-status WINDOW structure!\n");
      exit (1);
     } 

   ExplodeWindow (SstatusWin, EXPLODE_OUT);
   wattrset (StatusWin, COLOR_PAIR(49));
   wbkgd (StatusWin, COLOR_PAIR(49));
   werase (SstatusWin);
   box (SstatusWin, ACS_VLINE, ACS_HLINE); 
   werase (StatusWin);
   lower_status_figures(0, 0, 0);

 } /**/


 void InitInputWin (void)

 {
  int IWX, 
      IWY, 
      IWL, 
      IWC;
 
   IWL=1;
   IWC=t_cols;
   IWY=t_rows-1;
   IWX=0;

    if (!(InputWin=newwin(IWL, IWC, IWY, IWX))) 
     {
      fprintf (stderr, "-!- Unable to create input WINDOW struct %d!\n", IWY);

      exit (1);
     }

   keypad (InputWin, TRUE);
   wtimeout (InputWin, 1000);
   scrollok (InputWin, FALSE);

   FancyOutput1 (InputWin, "<IRCIT> ", 0);

 } /**/


 void InitInputField (void)

 {

 }  /**/


 int DisplayHeader (int x, int y)

 {
  register int i,
               j;
           int k;
  register int l=0;
           int total=0;
  register const struct Header *p;
  static unsigned long h[3][80];
  extern const ObjectsAndColors *const _tables;

   memset (h , 0, 3*80*sizeof(unsigned long));

   for (k=1; k<=3; k++)
       {
         if (k==1)
            {
             h[k-1][l++]=' '; h[k-1][l++]=0xDA|A_ALTCHARSET;
              for (i=0; i<=y-1; i++)
                  {
                   p=(*((*(headers+x))+i)+0);

                   total+=p->len;
                    for (j=0; j<=p->len-1; j++)
                        {
                         h[k-1][l++]=0xC4|A_ALTCHARSET;
                        }
                    if (i!=y-1)
                       {
                        h[k-1][l++]=0xC2|A_ALTCHARSET;
                       }
                  }
             
             h[k-1][l++]=0xBF|A_ALTCHARSET; 
             h[k-1][l++]='\n';
             h[k-1][l]=0;
             l=0; 
             continue;
            }

         if (k==2)
            {
             h[k-1][l++]=' ';
             h[k-1][l++]=ACS_VLINE;

              for (i=0; i<=y-1; i++)
                  {
                   int g=0;

                    p=(*((*(headers+x))+i)+0);
                   
                     while (g<=p->len-1)
                           {
                            (/**(p->title+g)&&*/(isalpha(*(p->title+g))))?
                             (h[k-1][l++]=*(p->title+g)):
                             (h[k-1][l++]=' '); 
                            g++;
                           }
                    h[k-1][l++]=ACS_VLINE;
                   }

             h[k-1][l++]='\n';
             h[k-1][l]=0;
             l=0;
             continue;
            }

         if (k==3)
            {
             h[k-1][l++]=' ';
             h[k-1][l++]=0xC6|A_ALTCHARSET;

              for (i=0; i<=y-1; i++)
                  {
                   p=(*((*(headers+x))+i)+0);

                    for (j=0; j<=p->len-1; j++)
                        {
                         h[k-1][l++]=0xCD|A_ALTCHARSET;
                        }
                    if (i!=y-1)
                       {
                        h[k-1][l++]=0xD8|A_ALTCHARSET;
                       }
                  }
             h[k-1][l++]=0xB5|A_ALTCHARSET;
             h[k-1][l++]='\n';
             h[k-1][l]=0;
             break;
            }
       }

   say ("%z%z%z", h[0], h[1], h[2]);

   return (total+(y-1));

 }  /**/

