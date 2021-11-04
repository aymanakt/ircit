/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: timer.h,v 1.4 1998/04/08 08:54:34 ayman Beta $")
**
*/

#define DOMEONCE  1
#define LEAVEME   2
#define UNUSED    4

#define TEMP   1
#define PERM   2
#define UNUSED 4

extern void CheckIdle_TimeoutDCC (void);
extern void DoUpperStatus (void);
extern void CheckSplitUsers (void);

 struct Timer {
         int id;
         int flag;
         int lapse;
         time_t when;
         char *info;
         void (*func)(void);
        };

#define NRTIMERS 9

 void initTimers (void);
 struct Timer *GetFreeEntry (void);
 int AddmetoTimers (int, int, char *, void(*)(void) );
 void ResetThisTimerEntry (int);
 int istimeractive (int);
 void ActivateTimer (int, char *);
 void DeActivateTimer (int, char *);
 void ExceTimers (int);
 int RegisterWithEvery30Seconds (void (*)(void));
 int UnregisterWithEvery30Seconds (void (*)(void));
 int RegisteredWithEvery30Seconds (void (*)(void));
 void ExceEvery30Seconds (void);
 void ShowTimerEntries (void);

 void ScrollingTimers (void *, int);

