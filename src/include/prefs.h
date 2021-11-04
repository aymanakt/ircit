/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: prefs.h,v 1.3 1998/03/16 10:09:38 ayman Beta $")
**
*/

#define READONLY  1
#define READWRITE 2

#define BOOLEAN 1
#define DIGITS  2
#define OTHER   3

#define BAREACCESS 1
#define FULLACCESS 2
#define OWNER      4

 struct Preference 
            {
              const char *preference;
              void (*func)(struct Preference *, char *);
              char *value;
              int level;
            };
 typedef struct Preference Preference;

 struct PrefsTable
                 {
                  struct Preference entry;
                  struct PrefsTable *right;
                  struct PrefsTable *left;
                 };
 typedef struct PrefsTable PrefsTable;

 PrefsTable *BuildPreferencesTable (void);
 void PrintEntries (const PrefsTable *);
 void ShowPreferences (const PrefsTable *, int);
 PrefsTable *FindEntry (const char *);
 char *valueof (const char *);
 int SetPref (const char *, char *);
 boolean set (char *);
 boolean sets (char *);

 void _COLOR_UPPERSTATUS (Preference *pref, char *);
 void _COLOR_LOWERSTATUS (Preference *pref, char *);
 void _COLOR_MAINBORDER (Preference *, char *);
 void _COLORS (Preference *pref, char *);

