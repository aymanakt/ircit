/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: terminal.h,v 1.7 1998/04/08 08:54:34 ayman Beta $")
**
*/

#include <curses.h>

#define RED     1
#define GREEN   2
#define YELLOW  3
#define BLUE    4
#define MAGENTA 5
#define CYAN    6
#define WHITE   7

 enum Colors {red=COLOR_PAIR(1),     green=COLOR_PAIR(2), 
              yellow=COLOR_PAIR(3),  blue=COLOR_PAIR(4), 
              magenta=COLOR_PAIR(5), cyan=COLOR_PAIR(6), 
              white=COLOR_PAIR(7)
             };
 typedef enum Colors Colors;

 enum Objects {_MENUBAR=0, _MENUBOX, _MENUBORDER, 
               _LIST, _LISTBORDER, _LISTMENU, _LISTMENUBORDER,
               _DIALOGBOX, _DIALOGBORDER, 
               _BOX, _BOXBORDER,
               _LOWERSTAT, _STATBORDER, _LOWERSTATTXT,
               _UPPERSTAT, 
               _MAINBORDER, _MAINBORDERTXT,
               _TABLES
              };
 typedef enum Objects Objects;
 #define OBJECTS_CNT 18

 struct ObjectsAndColors {
        Objects obj;
        int color,
            inv;
       };
 typedef struct ObjectsAndColors ObjectsAndColors;

#define FQL 0xAE|A_ALTCHARSET  //french quote left  <<
#define FQR 0xAF|A_ALTCHARSET  //french quote right >>
#define THD 0xF0|A_ALTCHARSET  //3 parallel dashes


#define EAT_IT             1
#define TAKE_IT_EASY       2
#define EXPLODE_IN         4
#define EXPLODE_OUT        8

#define MAINWIN           0x001
#define MAINWINBORDER     0x002
#define UPPERSTATUS       0x004
#define LOWERSTATUS       0x008
#define LOWERSTATUSBORDER 0x010
#define INPUTWIN          0x020

/*
** mt_ptr->c_output flags
*/
#define OUTPUTSCREEN   (1<<0)
#define MENUBAR        (1<<1)
#define MENUBOX        (1<<2)
#define STICKY_MENU    (1<<3)
#define HOLDOUTPUT     (1<<4)
#define ANIMATE_PROMPT (1<<5)
#define STICKY_LISTS   (1<<6)
#define DISCARD_MIRC   (1<<7)
#define OUTPUT_SOUND   (1<<8)
#define BACKGROUND     (1<<9)
#define TELNET_MSG     (1<<10)
#define TELNET_CMND    (1<<11)
#define SHOWOPTMENU    (1<<12)
#define BEEPONMSG      (1<<13)

#define CURWIN() ((WINDOW *)mt_ptr->c_window)

#define ISHOLD()       (mt_ptr->c_output&HOLDOUTPUT)
#define HOLDSCREEN()   (mt_ptr->c_output|=HOLDOUTPUT)
#define UNHOLDSCREEN() (mt_ptr->c_output&=~HOLDOUTPUT)

#define IS_DISCARD_MIRC() (mt_ptr->c_output&DISCARD_MIRC)
#define DISCARDMIRC()     (mt_ptr->c_output|=DISCARD_MIRC)
#define UNDISCARDMIRC()   (mt_ptr->c_output&=~DISCARD_MIRC)

#define IS_OUTPUTSOUND()   (mt_ptr->c_output&OUTPUT_SOUND)
#define SETOUTPUTSOUND()   (mt_ptr->c_output|=OUTPUT_SOUND)
#define UNSETOUTPUTSOUND() (mt_ptr->c_output&=~OUTPUT_SOUND)

#define IS_SHOWTELNETMSG()  (mt_ptr->c_output&TELNET_MSG)
#define IS_SHOWTELNETCMND() (mt_ptr->c_output&TELNET_CMND)

 void wUpdate (WINDOW *);
extern const int *const can_refresh;
#define mdoupdate() if (*can_refresh) {wUpdate();doupdate();}

 struct Foreground {
        const char **fg;
        int value;
       };

 struct ColorTable {
        const char **bg;
        const struct Foreground *fg;
        int inv;
       };

 
 struct Header {
         char title[20];
         int len;
       };
 extern  const struct Header headers[][10][2];

  struct Windows {
         WINDOW *w1,
                *w2;
         unsigned flags,
                  state;
         unsigned long *buf;
        };
 typedef struct Windows Windows;

  struct BoxGeometry {
         int nEntries,
             width;
        };
 typedef struct BoxGeometry BoxGeometry;

 #define swmove(y, x) _wmove (StatusWin, y, x)
 #define sswmove(y, x) _wmove (SstatusWin, y, x)
 #define uswmove(y, x) _wmove (UpperStatusWin, y, x)
 #define mwmove(y, x) _wmove (MainWin, y, x)
 #define mmwmove(y, x) _wmove (MmainWin, y, x)

 #define seperator(x, y) { \
                  int i; \
                  int j; \
                  register int l=0; \
                  unsigned long h[80]; \
                  const register struct Header *p; \
                                             \
                   *(h+l++)=' '; \
                   *(h+l++)=ACS_LTEE; \
                                      \
                    for (i=0; i<=(y)-1; i++) \
                        { \
                         p=&headers[(x)][i][0]; \
                          for (j=0; j<=p->len-1; j++) \
                              { \
                               *(h+l++)=ACS_HLINE; \
                              } \
                          if (i!=(y)-1) \
                             { \
                              *(h+l++)=ACS_PLUS; \
                             } \
                        } \
                          \
                   *(h+l++)=ACS_RTEE; \
                   *(h+l++)='\n'; \
                   *(h+l)=0; \
                           \
                   nYell ("%z", h); \
                 }

 #define finish2(x, y) { \
                int i; \
                int j; \
                register int l=0; \
                unsigned long h[80]; \
                register const struct Header *p; \
                                                 \
                 *(h+l++)=' '; \
                 *(h+l++)=0xC0|A_ALTCHARSET; \
                                             \
                  for (i=0; i<=(y)-1; i++) \
                      { \
                       p=&headers[(x)][i][0]; \
                        for (j=0; j<=p->len-1; j++) \
                            { \
                             *(h+l++)=ACS_HLINE; \
                            } \
                        if (i!=y-1) \
                           { \
                            *(h+l++)=0xC1|A_ALTCHARSET; \
                           } \
                      } \
                        \
                 *(h+l++)=0xD9|A_ALTCHARSET; \
                 *(h+l++)='\n'; \
                 *(h+l++)=0; \
                             \
                 nYell ("%z", h); \
                                  \
                 /*MainWin->_attrs=0;*/ \
               }

 #define finish(x) { \
               int i; \
               const ObjectsAndColors *const _tables; \
                      \
                waddch_ (CURWIN(), ' '); \
                waddch_ (CURWIN(), ACS_LLCORNER); \
                 while (x--)  \
                       { \
                        waddch_ (CURWIN(), ACS_HLINE); \
                       } \
                waddch_ (CURWIN(), ACS_LRCORNER); \
                waddch_ (CURWIN(), '\n'); \
                wrefresh (CURWIN()); \
                /*wattrset (MainWin, A_NORMAL);MainWin->_attrs=0;*/ \
              }

 #define start_frame { \
                      /*int i=0; \
                               \
                       waddch_ (CURWIN(), 0xDA|A_ALTCHARSET|WA_BOLD); \
                       horizontal_b; \
                       dash_b; \
                       dash_b; \
                       dash; \
                        while (i++!=40) \
                              { \
                               horizontal; \
                              } \
                       dash; dash; \
                       dot; dot; \
                       newline; \
                       wrefresh (CURWIN());*/ \
                      }

 #define lower_status_figures(x, y, z)  \
  { \
   swmove (1, 1); \
   swprintf ("%$249%$%%$249%$%%$249%$%%$249", \
             (x)?232|A_BLINK:(232|COLOR_PAIR(55)), \
             (y)?5:(5|COLOR_PAIR(55)), \
             (z)?1:(1|COLOR_PAIR(55))); \
  }

 #define HOURGLASS(x) \
  swmove (1, 2); \
  swprintf ("%$%", (x)?232|A_BLINK:(232|COLOR_PAIR(55)))
  

 #define reset_cursor() \
  sswmove (3, 1); \
  whline (SstatusWin, ACS_HLINE, 78); \
  wnoutrefresh (SstatusWin)

 #define clear_infoline()

 #define borderI(x) \
  wborder ((x), 0, 0, 0, 0, 201|A_ALTCHARSET, 187|A_ALTCHARSET, \
           200|A_ALTCHARSET, 188|A_ALTCHARSET)

 #define borderII(x) \
  wborder ((x), 0, 0, 0, 0, 214|A_ALTCHARSET, 183|A_ALTCHARSET, \
           211|A_ALTCHARSET, 189|A_ALTCHARSET)

 #define borderIII(x) \
  wborder ((x), 0, 0, 0, 0, ACS_BULLET, ACS_BULLET, ACS_BULLET, ACS_BULLET)

 #define border_active(x) \
  wborder ((x), 0, 0, 0, 0, ACS_ULCORNER|A_BOLD, ACS_URCORNER|A_BOLD, \
                            ACS_LLCORNER|A_BOLD, ACS_LRCORNER|A_BOLD)


 #define FancyOutput1(w, s, y) FancyOutput ((w), (s), (y), (80), (0))

 #define THN 0xDD|A_ALTCHARSET
 #define TIN  0xDE|A_ALTCHARSET|COLOR_PAIR(BLUE)
 #define TINB 0xDE|A_ALTCHARSET|COLOR_PAIR(BLUE)|WA_BOLD
 #define TIM  0xFE|A_ALTCHARSET|COLOR_PAIR(BLUE)
 #define TIMB 0xFE|A_ALTCHARSET|COLOR_PAIR(BLUE)|WA_BOLD
                              
 #define SRV_INFO 0xF0|A_ALTCHARSET|COLOR_PAIR(CYAN)|WA_BOLD
 #define SRV_MILD 0xF0|A_ALTCHARSET|COLOR_PAIR(YELLOW)|WA_BOLD
 #define SRV_HOT  0xF0|A_ALTCHARSET|COLOR_PAIR(RED)|WA_BOLD
 #define SRV_MOTD 0x10|A_ALTCHARSET|COLOR_PAIR(CYAN)

 #define IT_INFO  0xFE|A_ALTCHARSET|COLOR_PAIR(CYAN)|WA_BOLD
 #define IT_MILD  0xFE|A_ALTCHARSET|COLOR_PAIR(YELLOW)|WA_BOLD
 #define IT_HOT   0xFE|A_ALTCHARSET|COLOR_PAIR(RED)|WA_BOLD
 #define CTCPH    0xFE|A_ALTCHARSET|COLOR_PAIR(MAGENTA)|WA_BOLD
 #define CTCP     0xFE|A_ALTCHARSET|COLOR_PAIR(MAGENTA)
 #define TEL      0xFE|A_ALTCHARSET|COLOR_PAIR(GREEN)
 #define TELH     0xFE|A_ALTCHARSET|COLOR_PAIR(GREEN)|A_BOLD
 #define UTMP     0xFE|A_ALTCHARSET|COLOR_PAIR(BLUE)
 #define UTMPH    0xFE|A_ALTCHARSET|COLOR_PAIR(BLUE)|A_BOLD
 #define FPL      0x4|A_ALTCHARSET|COLOR_PAIR(CYAN)|WA_BOLD
 #define FPLM     0x4|A_ALTCHARSET|COLOR_PAIR(YELLOW)|WA_BOLD


 extern WINDOW *MainWin,     
               *MmainWin,
               *SpareWin,
               *InputWin,   
               *StatusWin,  
               *SstatusWin,
               *UpperStatusWin;

 struct U_Status {
        WINDOW *w;
        int flags;
       };

 void InitTerminal (void);
 void FetchTerminalSize (void);
 void ResizeTerminal (int);
 int BlockttyMsg (int);
 int LocateFreetty (char *);
 void ResetTerminal (void) __attribute__ ((destructor));
 void UpdateWindows (int);
 WINDOW *WinAlloc (int, int, int, int, int, WINDOW *);
 void InitMainWin (void);
 void InitSpareWin (void);
 void InitLowerStatusWin (void);
 void InitUpperStatusWin (void);
 void InitInputWin (void);
 void InitInputField (void);
 void RedrawMainWin (void);
 void RedrawUpperStatus (void);
 void ErrorBox (char *) __attribute__ ((unused));
 int DisplayHeader (int, int);

