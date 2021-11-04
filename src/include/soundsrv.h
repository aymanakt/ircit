/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: soundsrv.h,v 1.2 1998/02/20 14:58:44 ayman Beta $")
**
*/

 void InitSoundServer (void *);
 void SoundServer (int);
 void ProcessSoundRequest (int);
 void ParseRequest (char *);
 void ActuallyPlayIt (const char *); 

