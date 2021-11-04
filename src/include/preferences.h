/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: preferences.h,v 1.5 1998/04/08 08:54:34 ayman Beta $")
**
*/

 #include "prefsfunc.h"

 extern void _HISTORY_SIZE (Preference *, char *);
 extern void HOLD_MODE (Preference *, char *);
 extern void _STICKY_LISTS (Preference *, char *);
 extern void _LASTLOG_LEVEL (Preference *, char *);

 #define ENTRIES_IN_PREFSTABLE 60

 static const struct Preference Preferences[]=
 {
  {"TMP_DIR",             NULL,          "/tmp",      OTHER  },
  {"JOIN_ON_INVITE",      NULL,          "OFF",       BOOLEAN},
  {"REJOIN_ON_KICK",      NULL,          "ON",        BOOLEAN},
  /*{"AUTOOP_FRIENDS",      NULL,          "ON",        BOOLEAN},
  {"AUTODEOP_BASTARDS",   NULL,          "ON",        BOOLEAN},
  {"AUTOKICK_BASTARDS",   NULL,          "ON",        BOOLEAN},*/
  /*{"AUTO_RECONNECT",      NULL,          "ON",        BOOLEAN},
  {"CYCLED_RECONNECTION", NULL,          "OFF",       BOOLEAN},*/
  {"DEFAULT_NICK",        NULL,          NULL,        OTHER  },
  {"ALT_NICK",            NULL,          NULL,        OTHER  },
  {"KEEP_NICK",           NULL,          "ON",        BOOLEAN},
  {"SCRIPT_DIR",          NULL,          NULL,        OTHER  },
  {"DEFAULT_TELNET_PORT", NULL,          "2500",      DIGITS },
  {"DEFAULT_SERVER_PORT", NULL,          "6667",      DIGITS },
  {"ALLOW_TELNET",        NULL,          "ON",        BOOLEAN},
  {"ALLOW_DCC",           NULL,          "ON",        BOOLEAN},
  {"MSG_BACKLOG",         NULL,          "10",        DIGITS },
  {"DCC_PORT",            NULL,          "9991",      DIGITS },
  {"FINGER_REPLAY",       NULL,          NULL,          OTHER  },
  {"PING_REPLAY",         NULL,          NULL,          OTHER  },
  {"VERSION_REPLAY",      NULL,          NULL,          OTHER  },
  {"AUTO_DCC_CHAT",       NULL,          "ON",        BOOLEAN},
  {"SHOW_MOTD",           NULL,          "ON",        BOOLEAN},
  {"AUTO_DCC_GET",        NULL,          "ON",        BOOLEAN},
  {"DCC_TIMEOUT",         NULL,          "120",       DIGITS },
  {"TELNET_TIMEOUT",      NULL,          "120",       DIGITS },
  {"AWAY_MSG",            NULL,          "Away!",     OTHER  },
  {"BEEP_ON_MSG",         _BEEP_ONMSG,   "ON",        BOOLEAN},
  {"DISCARD_MIRC",        _DISCARD_MIRC, "OFF",       BOOLEAN},
  {"USERNAME",            NULL,          NULL,        OTHER  },
  {"AUTO_HOLD_MODE",      NULL,          "OFF",       BOOLEAN},
  {"ARROWS_HISTORY",      _HISTORY_SIZE, "30",        DIGITS },
  {"TAB_HISTORY",         _HISTORY_SIZE, "30",        DIGITS },
  {"REALNAME",            NULL,          NULL,        OTHER  },
  {"CLEAR_ON_EXIT",       NULL,          "ON",        BOOLEAN},
  {"AUTO_SHOOT",          NULL,          "OFF",       BOOLEAN},
  {"HISTORY_DIR",	  NULL,          NULL,        OTHER  },
  {"STICKY_MENU",         _STICKY_MENU,  "OFF",       BOOLEAN},
  {"REJOIN_WHEN_ALONE",   NULL,          "OFF",       BOOLEAN},
  {"AUTO_SHOW_OPTMENU",   _OPTION_MENU,  "OFF",       BOOLEAN},
  {"SPLIT_DURATION",      NULL,          "120",       DIGITS },
  {"MENUBOX_TERMINATOR",  NULL,          "OFF",       BOOLEAN},
  {"COMMAND_CHAR",        _COMMAND_CHAR, "/",         OTHER  },
  {"HOLD_MODE",           HOLD_MODE,     "OFF",       BOOLEAN},
  {"ANIMATE_PROMPT",      NULL,          "OFF",       BOOLEAN},
  {"WHOIS_ON_NOTIFY",     NULL,          "OFF",       BOOLEAN},
  {"IPC_PATH",            NULL,          NULL,        OTHER  },
  {"STICKY_LISTS",        _STICKY_LISTS, "OFF",       BOOLEAN},
#ifdef LAST__LOG
  {"LASTLOG_LEVEL",       _LASTLOG_LEVEL,NULL,        OTHER  },	
#endif
  {"SHELL",               NULL,          "/bin/sh",   OTHER  },
  {"SHELL_ARGS",          NULL,          "-c",        OTHER  },
  {"DISPATCH_UNKNOWN_TO_SRV", NULL,      "OFF",       BOOLEAN},
  {"BLOCK_SIZE",          _BLOCK_SIZE,   "513",       DIGITS },
  {"COLOR_MENUBAR",       _COLORS,       "cyan,red",  OTHER  },
  {"COLOR_MENUBOX",       _COLORS,       "cyan,red",  OTHER  },
  {"COLOR_MENUBORDER",    _COLORS,       "black,white",OTHER  },
  {"COLOR_LIST",          _COLORS,       "cyan,red",  OTHER  },
  {"COLOR_LISTBORDER",    _COLORS,       "black,white",OTHER  },
  {"COLOR_LISTMENU",      _COLORS,       "cyan,red",  OTHER  },
  {"COLOR_LISTMENUBORDER",_COLORS,       "black,white",OTHER  },
  {"COLOR_DIALOGBOX",     _COLORS,       "cyan,red",  OTHER  },
  {"COLOR_DIALOGBORDER",  _COLORS,       "black,cyan",OTHER  },
  {"COLOR_BOX",           _COLORS,       "cyan,red",   OTHER  },
  {"COLOR_BOXBORDER",     _COLORS,       "black,white", OTHER  },
  /*{"COLOR_LOWERSTAT",     _COLOR_LOWERSTATUS,  "cyan,red",   OTHER  },
  {"COLOR_STATBORDER",    _COLOR_LOWERSTATUS,  "black,white",OTHER  },
  {"COLOR_LOWERSTATTXT",  _COLOR_LOWERSTATUS,  "black,cyan", OTHER  },*/
  {"COLOR_UPPERSTAT",     _COLOR_UPPERSTATUS,  "cyan,red",   OTHER  },
  /*{"COLOR_MAINBORDER",    _COLOR_MAINBORDER,   "black,white",OTHER  },
  {"COLOR_MAINBORDERTXT", _COLOR_MAINBORDER,   "black,cyan", OTHER  },
  {"COLOR_TABLES",        _COLORS,             "black,cyan", OTHER  },*/
  {"TELNET_MSG",          _TELNET_ATTRS,       "OFF",        BOOLEAN  },
  {"TELNET_CMND",         _TELNET_ATTRS,       "OFF",        BOOLEAN  }
 };

