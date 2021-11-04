/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>
#include <terminal.h>
#include <updates.h>
#include <list.h>
#include <prefs.h>

#include <alias.h>
#include <events.h>
#include <events-data.h>
#include <FPL.h>
#include <FPL_protos.h>
#include <intrins.h>
#include <fpl.h>

 MODULEID("$Id: fpl.c,v 1.3 1998/09/19 02:46:18 ayman Beta $");

 void *key;
 static char fpl_err[FPL_ERRORMSG_LENGTH];
 Event *const event_ptr=IRCITEvents;

 static long fplinterface (struct fplArgument *);
 static void AddIntrinsicFunctions (void);
 static void AddIntrinsicVariables (void);
 static void AddConstantVariable (char *);


 static long fplinterface (struct fplArgument *arg)

 {
  switch (arg->ID)
    {
    case FPL_GENERAL_ERROR:
      {
	long col;
	char *name;

	fplSendTags (arg->key,
		     FPLSEND_GETVIRLINE, &col,
		     FPLSEND_GETPROGNAME, &name,
		     FPLSEND_DONE);

	say ("%$% %s\n%$% line %d in file %s...\n",
	     FPLM, fpl_err, FPLM, col, name);

	break;
      }

    case FPL_WARNING:
      say ("%$% FPL_WARNING - %s.\n", FPLM, fpl_err);
      break;

    case FPL_FILE_REQUEST:
      say ("%$% FPL_FILE_REQUEST - someone wants file %s...\n",
	   FPL, arg->argv[0]);

      fplSendTags (key, FPLSEND_PROGRAM, arg->argv[0]);
      break;

    case FPL_FLUSH_FILE:
      say ("%$% FPL_FLUSH_FILE - whether to flush file (%s) or not...\n",
	   FPL, arg->argv[0]);
      break;
    }

  return FPL_OK;

}   /**/


extern  int STD_SCRIPT_COMMAND(intrin_getresource);

  typedef struct Intrinsics {
                  char *name;
                  char retcode;
                  char *param;
                  void *f;
                 } Intrinsics;

 void AddIntrinsicFunctions (void)

 {
  register int i;
  Intrinsics intrins[]=
{
 {"alias",                   'I', "SSS",     intrin_alias                 },
 {"assoc",                   'I', "ISS",     intrin_assoc                 },
 {"bindkey",                 'I', "ISS",     intrin_bindkey               },
 {"chanid",                  'I', "S",       intrin_chanid                },
 {"chusermodes",             'I', "SS",      intrin_chusermodes           },
 {"eventidx",                'I', "S",       intrin_eventidx              },
 {"getserv",                 'S', "I",       intrin_getserv               },
 {"inw_clear",               'I', "",        intrin_inw_clear             },
 {"inw_complete",            'I', "",        intrin_inw_complete          },
 {"inw_out",                 'I', "S",       intrin_inw_out               },
 {"lastmsg_action",          'I', "",        intrin_lastmsg_action        },
 {"loadmenubar",             'I', "I",       intrin_loadmenubar           },
 {"match",                   'I', "SS",      intrin_match                 },
 {"parse",                   'I', "So>",     intrin_parse                 },
 {"prog",                    'I', "SS",      intrin_prog                  },
 {"say",                     'I', "So>",     intrin_say                   },
 {"servidx",                 'I', "S",       intrin_servidx               },
 {"set",                     'I', "SS",      intrin_set                   },
 {"strftime",                'S', "S",       intrin_strftime              },
 {"toserver",                'I', "S",       intrin_toserver              },
 {"tosock",                  'I', "IS",      intrin_tosock                },
 {"unbindkey",               'I', "I",       intrin_unbindkey             },
 {"valueof",                 'S', "S",       intrin_valueof               }
};

   for (i=0; i<sizeof(intrins)/sizeof(intrins[0]); i++)
    {
     if (fplAddFunctionTags(key,
                            intrins[i].name,
                            /*intrins[i].func,*/0,
                            intrins[i].retcode,
                            intrins[i].param,
                            FPLTAG_FUNCTION, intrins[i].f))
      {
       say ("%$% Unable to add intrinsic - %s\n", HOT, fpl_err);
      }
    }

   //fplAddVariableTags (key, "HaHa", 1, FPL_INTARG, (int *)2, FPLTAG_DONE);
   AddConstantVariable ("");

 }  /**/


#include <stack.h>
#include <sockets.h>

static void 
AddIntrinsicVariables (void)

{
#if 0
  register int i;
  static const char *ircit_intrin_cServer;
  static const char *ircit_intrin_cChannel;
  static const char *ircit_intrin_cNick;
  extern List *servers;
  extern List *irc_notify;
  extern List *utmp_notify;
  extern SocketsTable *sockets;
  extern Stack *stack;
  Intrinsics intrins[] =
  {
    {"$nServers", 0x2, 'I', NULL}
  };

  for (i = 0; i < sizeof (intrins) / sizeof (intrins[0]); i++)
    {
      if (fplAddVariable (key,
			  intrins[i].name,
			  intrins[i].func,
			  intrins[i].retcode,
			  NULL,
			  NULL))
	{
	  say ("%$% Unable to add intrinsic var - %s\n", HOT, fpl_err);
	}
    }


  ircit_intrin_cServer = st_ptr->c_server;
  ircit_intrin_cChannel = cht_ptr->c_channel;
  ircit_intrin_cNick = st_ptr->c_nick;
#endif
}   /**/


 void AddConstantVariable (char *constvar)

 {
  FILE *fp;
  char s[]="export const int USER_CHANOP=1;"
           "export const int USER_CHANVOICE=2;"
           "export const int USER_HERE=32;"
           "export const int USER_GONE=64;"
           "export const int USER_IRCOP=128;";
  #define SECRETE_PATH "/tmp/.__secRete___"
  extern const char *c_version;

    if (fp=fopen(SECRETE_PATH, "w"))
     {
      fprintf (fp, "export const string _version=\"%s\";", c_version);
     
      fprintf (fp, s);/*"export const int USER_CHANOP=1;"
                   "export const int USER_CHANVOICE=2;"
                   "export const int USER_HERE=32;"
                   "export const int USER_GONE=64;"
                   "export const int USER_IRCOP=128;");*/

      fflush (fp);

      fplExecuteFile (key, SECRETE_PATH, NULL);

      fclose (fp);
      unlink (SECRETE_PATH);
     }

   /*fplExecuteScriptTags (key, &constvar, 1, 
                         FPLTAG_CACHEFILE, FPLCACHE_EXPORTS, 
                         FPLTAG_DONE);*/


 }  /**/


 int InitInterpreter (void)

 {
    if (!(key=fplInitTags(fplinterface,
                          FPLTAG_ERROR_BUFFER, fpl_err,
                          FPLTAG_CACHEALLFILES, 
                          FPLCACHE_EXPORTS,
                          FPLTAG_REREAD_CHANGES, TRUE,
                          FPLTAG_DONE)))
     {
      say ("%$% Unable to initialize interpreter.\n", HOT);

      exit (1);
     }

   AddIntrinsicFunctions ();

 }  /**/


#include <fnmatch.h>


 void *EventRegistered (EventCode event)

 {
  register Event *evntptr;

    if (((event-1)<0)||(event>MAX_EVENTS))
     {
      say ("%$% Event index out of range - %d.\n", MILD, event);

      return NULL;
     }

   evntptr=&IRCITEvents[event-1];

   return evntptr->flags&EVENT_DEFINED?evntptr:NULL;

#if 0
    if (evntptr->flags&EVENT_DEFINED)
     {
       if (fmt)
        {
         va_list va;

          va_start (va, fmt);
          vsnprintf (args, 512, fmt, va);
          va_end (va);
        }

       if (evntptr->regx)  /* this should be true if we reach this far */
        {
         int ok=1;

           if ((*evntptr->regx)&&(*args))
            {
             if ((fnmatch(evntptr->regx, args, FNM_CASEFOLD)))
              {
               ok=0;
              }
            }
                
           if (ok)  
            {
             ExpandAlias (evntptr->hook, args);

             return 1;
            }
        }
     }

   return 0;
#endif
 }  /**/


#ifndef FNM_CASEFOLD
# define FNM_CASEFOLD (1<<4)
#endif

 void ExecHook (void *evntptr, const char *fmt, ...)

 {
  char args[]={[0 ... 513]=0};

    if (evntptr)
     {
       if (fmt)
        {
         va_list va;

          va_start (va, fmt);
          vsnprintf (args, 512, fmt, va);
          va_end (va);
        } 

       if (((Event *)evntptr)->regx)
        {
         int ok=1;

           if ((*((Event *)evntptr)->regx)&&(*args))
            {
             if ((fnmatch(((Event *)evntptr)->regx, args, FNM_CASEFOLD)))
              {
               ok=0;
              }
            }

           if (ok)
            {
             ExpandAlias (((Event *)evntptr)->hook, args);

             return;
            }
        }
     }

 }  /**/


 STD_IRCIT_COMMAND(uFPL)

 {
    if (!args)
     {
      say ("%$% You need -h.\n", MILD);

      return;
     }

   {
    char *stuff=args;

     fplExecuteScriptTags (key, &stuff, 1, FPLTAG_DONE);
   }

 }  /**/


 #include <sys/stat.h>


 STD_IRCIT_COMMAND(uLOAD)

 {
  register char *s;
  char path[_POSIX_PATH_MAX];

    if (!args)
     {
      say ("%$% arguments invalid or incomplete.\n", MILD);

      return;
     }

    if (*args=='/')
     {
      snprintf (path, _POSIX_PATH_MAX, "%s", args);
     }
    else
     {
      s=valueof ("SCRIPT_DIR");

       if (s)
        {
         snprintf (path, _POSIX_PATH_MAX, "%s/%s", s, args);
        }
      else
       {
        snprintf (path, _POSIX_PATH_MAX, "%s/.ircit/%s", homedir(), args);
       }
     }

   {
    struct stat st;

      if ((access(path, R_OK))<0)
       {
        say ("%$% Unable to load file %s - %s.\n", MILD, path, strerror(errno));

        return;
       }

      if ((stat(path, &st))<0)
       {
        say ("%$% Unable to load file %s - %s.\n", MILD, path, strerror(errno));

        return;
       }

      if (!S_ISREG(st.st_mode))
       {
        say ("%$% Unable to load file %s - Not regular file.\n", MILD, path);

        return;
       }
   }

   say ("%$% Loading %s...\n", INFO, args);

   fplExecuteFile (key, path, NULL);

 }  /**/


 STD_IRCIT_COMMAND(uASSOC)

 {
  register Event *evntptr;
  register int i=0, j=1;

   DisplayHeader (16, 4);

    for ( ; i<=MAX_EVENTS-1; i++)
     {
      if (evntptr=(event_ptr+i), evntptr->flags&EVENT_DEFINED)
       {
        say (" %$179%-3d%$179%-10.10s%$179%-25.25s%$179%-10.10s%$179\n",
             j++, evntptr->desc, evntptr->hook, evntptr->regx);
       }
     }

   finish2(16,4);

 }  /**/

