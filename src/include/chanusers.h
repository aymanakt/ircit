/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: chanusers.h,v 1.3 1998/03/21 09:42:29 ayman Beta $")
**
*/

 ChanUser *userbychannel (const char *, const char *);
 ChanUser *userbychptr (const Channel *, const char *);
 ChanUser *FindChanneluser (const char *);
 ChanUser *AddUserToChannel (const char *, const char *);
 ChanUser *AddChanuser (Channel *, const char *);
 int AddUser (const char *, const char *, const char *);
 void ProcessUser (char *);
 void UpdateChanuserNick (const char *, const char *);
 Channel *RemoveUserFromChannel (const char *, const char *);
 int RemoveChanUserbychptr (Channel *, const char *);
 void CheckSplitUsers (void);
 int ShowChanUsers (const ChanUser *uptr, const int);
 void ShowSplitUsers (const Channel *, int, int);
 void ShowUsers (const Channel *, int, int, int); 

 int ResetChanUsers (Channel *);

 void HashChanuser (int , Channel *, ChanUser *);
 void UnhashChanuser (const char *, Channel *, ChanUser *);
 int PRIVMSGChanusesrs (const Channel *, const char *);

 void ScrollingChanusers (void *, int);

 boolean UsersRecordEmpty (const Channel *);

