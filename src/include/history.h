/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: history.h,v 1.2 1998/03/16 10:09:38 ayman Beta $")
**
*/

 struct Input {
         char line[512];
         int counter;
         struct Input *next,
                      *prev;
        };
 typedef struct Input Input;

 struct InputHistory
                    {
                     int ID,
                         nLines;
                     Input *head,
                           *tail;
                    };
 typedef struct InputHistory InputHistory;


 void InitHistories (void);
 void InitUserInputHistory (void);
 void InitTabHistory (void);

 int InmyList (InputHistory *, char *);
 Input *AddtoUserInputHistory (InputHistory *, char *, int);

 Input *RemovefromInputHistory (InputHistory *, Input *);
 void RemovefromHistory (char *, int);
 int AdjustHistorySize (InputHistory *, int);

 int _LoadHistoryFile_ (int, InputHistory *, const char *, char *);
 int SavetoHistoryFile (InputHistory *, const char *, char *);
 char *GetHistoryDir (const char *, char *);

 void ShowInputHistory (InputHistory *, int, int);

 void uHISTORY (char *);

