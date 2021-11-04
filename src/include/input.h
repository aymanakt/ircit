/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: input.h,v 1.3 1998/03/16 10:09:38 ayman Beta $")
**
*/

 int Gets (int);
 void F1 (void);
 void F2 (void);
 void F4 (void);
 void F3 (void);

 int BindKey (int, const char *, const char *);
 int unBindKey (int);
 void uBINDKEY (char *);
 void InputWinOut (char *);
 void InputWinClr (void);
// void ShowThisKeyBinding (const FunctionKeys *);
// void ShowKeyBindings (const FunctionKeys *);

