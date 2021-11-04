/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: mvsprintf.h,v 1.3 1998/03/21 09:42:29 ayman Beta $")
**
*/

#define PROCESS_0X2() \
        field_width++; \
         if ((attr&A_BOLD)==0)  attr|=A_BOLD; \
         else  attr&=~A_BOLD

#define PROCESS_0X1F() \
        field_width++; \
         if ((attr&COLOR_PAIR(CYAN))==0)  attr|=COLOR_PAIR(CYAN); \
         else  attr&=~COLOR_PAIR(CYAN)

#define PROCESS_0X16() \
        field_width++; \
         if ((attr&A_REVERSE)==0)  attr|=A_REVERSE; \
         else  attr&=~A_REVERSE

#define PROCESS_0X7() \
        field_width++; \
        beep ()

#define PROCESS_1(x) \
        { \
         int cnt=0; \
         int c=0; \
                  \
          field_width++; \
           while ((*++(x))&&(isdigit(*(x)))&&(cnt<3)) \
            { \
             cnt++; \
             c=(c*10)+((*(x))-'0'); \
            } \
              \
           if (cnt>2)  (x)--; \
           if ((!cnt)||(*(x)!=','))  goto ouch; \
                                                    \
           /*if ((cnt)||(*(x)==','))*/ \
            { \
             while ((*++(x))&&(isdigit(*(x)))&&(cnt<5))  cnt++; \
             if (cnt>4)  (x)--; \
            } \
              \
          ouch: \
           if (cnt==0) \
            { \
             attr&=~COLOR_PAIR(*(colors+colr)); \
             (x)--; \
            } \
           else \
            { \
             attr&=~COLOR_PAIR(*(colors+colr)); \
             colr=c>15?0:c; \
             attr|=COLOR_PAIR(*(colors+colr)); \
             (x)--; \
            } \
              \
        }
 
#define PROCESS_2(x) \
        { \
         int cnt=0; \
         int c=0; \
                  \
           while ((*++(x))&&(isdigit(*(x)))&&(cnt<3)) \
            { \
             cnt++; \
             c=(c*10)+((*(x))-'0'); \
            } \
              \
           if (cnt>2)  (x)--; \
           if ((!cnt)||(*(x)!=',')) goto ouchh; \
                                                    \
           /*if ((cnt)||(*(x)==','))*/ \
            { \
             while ((*++(x))&&(isdigit(*(x)))&&(cnt<5))  cnt++; \
             if (cnt>4)  (x)--; \
            } \
              \
          ouchh: \
           if (cnt==0) \
            { \
             attr&=~COLOR_PAIR(*(colors+colr)); \
             (x)--; \
            } \
           else \
            { \
             attr&=~COLOR_PAIR(*(colors+colr)); \
             colr=c>15?0:c; \
             attr|=COLOR_PAIR(*(colors+colr)); \
             (x)--; \
            } \
              \
        }
 
  #define FIX_1_2(x) \
          { \
           int cnt=0; \
                      \
             while ((*++(x))&&(isdigit(*(x)))&&(cnt<3))  cnt++; \
                                                                \
             if (cnt>2)  (x)--; \
             if ((!cnt)||(*(x)!=','))  goto _ouch; \
                                                   \
             while ((*++(x))&&(isdigit(*(x)))&&(cnt<5))  cnt++; \
             if (cnt>4)  (x)--; \
                               \
            _ouch: \
            (x)--; \
          }

  #define ADD1(x) \
          if (((x)<32)&&((x)!='\n')) \
           { \
            if ((x)==0x2) \
             { \
              PROCESS_0X2(); \
              continue; \
             } \
            else \
            if ((x)==0x3) \
             { \
              PROCESS_2(fmt); \
             } \
            else \
            if ((x)==0x1f) \
             { \
              PROCESS_0X1F(); \
              continue; \
             } \
            else \
            if ((x)==0x16) \
             { \
              PROCESS_0X16(); \
              continue; \
             } \
            else \
            if ((x)==0x7) \
             { \
              PROCESS_0X7(); \
              continue; \
             } \
            else \
             { \
              *str++=(unsigned long)((x)+64)|attr|WA_REVERSE; \
             } \
           } \
          else \
           { \
            d=(x); \
            *str++=d|attr; \
           }

  #define ADD2(x) \
          if (((x)<32)&&((x)!='\n')) \
           { \
            if ((x)==0x2) \
             { \
              PROCESS_0X2(); \
             } \
            else \
            if ((x)==0x3) \
             { \
              if (IS_DISCARD_MIRC())  FIX_1_2(s) \
              else  PROCESS_1(s); \
             } \
            else \
            if ((x)==0x1f) \
             { \
              PROCESS_0X1F(); \
             } \
            else \
            if ((x)==0x16) \
             { \
              PROCESS_0X16(); \
             } \
            else \
            if ((x)==0x7) \
             { \
              PROCESS_0X7(); \
             } \
            else \
             { \
              *str++=(chtype)intch[(unsigned char)(x)]|attr; \
             } \
           } \
          else \
             { \
              *str++=(chtype)intch[(unsigned char)(x)]|attr; \
             }

