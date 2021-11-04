/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: headers.h,v 1.4 1998/04/08 08:54:34 ayman Beta $")
**
*/

 const struct Header headers[][10][2]=
  {
    {
     {"No", 2}, {"Name", 6}, {"When", 5}, {"What", 6}
    },
    {
     {"fd", 2}, {"Type", 7}, {"When", 5}, {"Lport", 5}, {"Remote Address", 20},
     {"Rport", 5}, {"Que", 3}, {"Recv", 4}, {"Xfer", 4}
    },
    {
     {"No", 2}, {"ID", 4}, {"Channel", 12}, {"nUser", 5}, {"Modes", 6},
     {"When", 5}, {"Log Level", 20}
    },
    {
     {"No", 2}, {"Server ", 33}, {"Port", 5}, {"When", 5}
    },
    {
     {"No", 3}, {"Nick", 15}, {"Host", 15}, {"ON/OFF", 6}, {"When", 5}
    },
    {
     {"No", 3}, {"Description", 14}, {"Lapse", 5}, {"Flag", 8},
     {"Time left", 9}
    },
    {
     {"No", 3}, {"  ", 3}, {"Nick", 13}, {"Host", 19}, {"Server", 15},
     {"When", 5}
    },
    { //7/8
     {"No", 3}, {"fd", 4}, {"Nick", 13}, {"Type", 6}, {"When", 5}, {"Stat", 6},
     {"File", 11}, {"Complete", 11}
    },
    { //8/6
     {"Login", 9}, {"Real name", 11}, {"UID", 4}, {"EUID", 4}, {"GUID", 4},
     {"Shell", 11}, {"$HOME", 15}
    },
    { //9/4
     {"No", 3}, {"Name", 13}, {"nIdx", 4}, {"Doing..", 15}, {"Misc...", 15}
    },
    { //10/10
     {"fd", 3}, {"Name", 13}, {"idx", 3}, {"Type", 5}, {"when", 5}, {"Time",8},
     {"Xfer", 4}, {"Recv", 4}, {"File", 11}, {"Size", 7}
    },
    {
     {"No", 2}, {"Preference", 20}, {"Current value  ", 25}
    },
    { /* 12/4 */
     {"No", 2}, {"Name", 15}, {"ON/OFF", 6}, {"When", 5}
    },
    { /* 13/4 */
     {"No", 2}, {"Ignored", 15}, {"Ignore mask", 30}, {"When", 5},
     {"Hits", 4}
    },
    { /* 14/2 */
     {"No", 3}, {"History Entry", 60}
    },
    { /* 15/5 */
     {"No", 2}, {"PID", 5}, {"Command", 15}, {"When", 5}, {"Output", 6}
    },
    { /* 16/4 */
     {"NO", 3}, {"Event", 10}, {"Association", 25}, {"Condition", 10}
    },
    { /* 17/3 */
     {"NO", 3}, {"Alias Name", 15}, {"Alias Definition", 40}
    },
    { /* 18/3 */
     {"Last Log", 8}, {"Max", 5}, {"Entries", 7}
    },
    { /* 19/3 */
     {"Key", 4}, {"Describtion", 15}, {"Function", 23}
    }
  };

