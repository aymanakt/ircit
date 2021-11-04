 
/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: serverio.h,v 1.4 1998/03/16 10:09:38 ayman Beta $")
**
*/

 /*
 ** Holds all IRCIT's built-in commands and the dynamically craeted ones
 ** through /ALIAS command.
 ** An array of this structure is created during initialization process.
 ** The initial entries are copied from static array (uFunction[]).
 ** See BuilSymbolTable().
 */
 typedef struct ITCommand {
                char *name,
                     *help;
                void (*how)(char *);
                unsigned attrs;
               } ITCommand;

 void BuildSymbolTable (void);
 void UpdateSymbolTable (char *, void *, char *, int);
 void RemovefromSymbolTablee (const char *);
 int CompleteCommand (const char *, ITCommand **, int);
 int  cmpcmnd (const void *, const void *);

 void  WriteToServer  (int, ...);
 void mCTCPToServer (const char *, const char *, ...) 
      __attribute__ ((format (printf, 2, 3)));
 void nCTCPToServer (const char *, const char *, ...) 
      __attribute__ ((format (printf, 2, 3)));
 int ToServer (const char *, ...) __attribute__ ((format (printf, 1, 2)));
 int  ReadFromServer (void);
 int ParseServerMsg (void);
 void ParseUserInput (void);
 
 void ScrollingHelp (void *, int);
 void ScrollingHelpEntries (ScrollingList *);
 void HelpEnter (void *, int);

 void LastmsgList (void *, int);
 void LastmsgInit (ScrollingList *);

