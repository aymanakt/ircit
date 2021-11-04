/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <main.h>
#include <terminal.h>
#include <updates.h>
#include <list.h>
#include <sockets.h>
#include <lastlog.h>

#include <FPL.h>
#include <FPL_protos.h>
#include <events.h>
#include <intrins.h>

 #define fpl_return_int(x...) fplSendTags(key, FPLSEND_INT, x, FPLSEND_DONE)
 #define fpl_return_str(x...) \
         fplSendTags(key, FPLSEND_STRING, x, FPLSEND_DONE)

 extern void *key;

 #include <alias.h>

 int STD_SCRIPT_COMMAND(intrin_alias)

 {
  char *args,
       *name,
       *hlp;
  extern List *als_ptr;

    if (!*(name=arg->argv[0]))
     {
      return (fpl_return_int(0));
     }

    if (!*(args=arg->argv[1]))
     {
      RemoveAlias (als_ptr, name);

      return (fpl_return_int(1));
     }  
   
    if (AddAlias(als_ptr, name, args, (char *)arg->argv[2]))
     {
      return (fpl_return_int(1));
     }

   return (fpl_return_int(0));

 }  /**/


 int STD_SCRIPT_COMMAND(intrin_assoc)

 {
  char *func,
       *regx;
  register Event *evntptr;
  extern Event *const event_ptr;

   regx=arg->argv[1];
   func=arg->argv[2];

    if (((int)arg->argv[0]>MAX_EVENTS)||((int)arg->argv[0]<=0)||(!*func))
     {
      say ("%$% Invalid association attempted.\n", MILD, (int)arg->argv[0]);

      return (fpl_return_int(0));
     }

   evntptr=(event_ptr+((int)arg->argv[0]-1));

    if (evntptr->hook)  free (evntptr->hook);
    if (evntptr->regx)  free (evntptr->regx);

    if (!(evntptr->hook=strdup(func))||!(evntptr->regx=strdup(regx)))
     {
      say ("%$% Memory exhausion..\n", HOT);

      return (fpl_return_int(0));
     }

   evntptr->flags|=EVENT_DEFINED;

   /*say ("%$% Event %d associated with %s when %s\n", INFO, 
        (int)arg->argv[0], evntptr->hook, evntptr->regx);*/

   return (fpl_return_int(1));

 }  /**/


#include <input.h>


 int STD_SCRIPT_COMMAND(intrin_bindkey)

 {
    if (((int)arg->argv[0]<KEY_F0)||(((int)arg->argv[0]>KEY_F(MAX_F_KEYS)))||
        (!*((char *)arg->argv[1])))
     {
      return (fpl_return_int(0));
     }

   return(fpl_return_int(BindKey(((int)arg->argv[0]-264),
                                 (const char *)arg->argv[1],
                                 *((const char *)arg->argv[2])?
                                  ((const char *)arg->argv[2]):NULL)));

 }  /**/


#include <chanrec.h>


 static __inline__ const char *Predo (const char *argv0)

 {
  const char *chan="";

    if (!*argv0)
     {
      if (*curchan())
       {
        chan=curchan();
       }
     }
    else
     {
      chan=argv0;
     }

   return chan;

 }  /**/


 int STD_SCRIPT_COMMAND(intrin_chanid)

 {
  const Channel *chptr;

    if (!(chptr=ChannelbyName(Predo((char *)arg->argv[0]))))
     {
      return (fpl_return_int(0));
     }

   return (fpl_return_int(chptr->id));

 }  /**/


 int STD_SCRIPT_COMMAND(intrin_chusermodes)

 {
  register const ChanUser *chuptr;

    if (!(chuptr=(ChanUser *)userbychannel(Predo((char *)arg->argv[0]), 
                               (char *)arg->argv[1])))
     {
      return (fpl_return_int(-1));
     }

   return (fpl_return_int(chuptr->usermodes));

 }  /**/


 int STD_SCRIPT_COMMAND(intrin_inw_complete)

 {
   F1 ();

   return (fpl_return_int(0));

 }  /**/


 int STD_SCRIPT_COMMAND(intrin_eventidx)

 {
  extern Event *const event_ptr;
  register Event *evntptr;
  register int i=0;

    for ( ; i<=MAX_EVENTS-1; i++)
     {
      evntptr=(event_ptr+i);

       if (arethesame(arg->argv[0], (evntptr=&event_ptr[i], evntptr->desc)))
        {
         return (fpl_return_int(i+1));
        }
     }

   return (fpl_return_int(-1));

 }  /**/


 #include <servers.h>

 char *STD_SCRIPT_COMMAND(intrin_getserv)

 {
  char *s=NULL,
       *srv="";
  int idx=(int)arg->argv[0];
  register int i=1;
  register ListEntry *eptr;
  extern List *servers;

    if ((idx<=0)||(idx>servers->nEntries))
     {
      srv=st_ptr->c_server;
      goto down;
     }

    for (eptr=servers->head; eptr!=NULL; eptr=eptr->next)
     {
      if (!(i++^idx))  
       {
        srv=(char *)((Server *)eptr->whatever)->server;
        break;
       }
     }

    if (!*srv)  srv=st_ptr->c_server;

   down:
    if (s=fplAllocString(key, strlen(srv)+1))
     {
      strcpy (s, srv);

      fpl_return_str(s, FPLSEND_DONTCOPY_STRING, TRUE);
     }
    else
     {
      say ("%$% not enough memory..\n", HOT);
      fpl_return_str(NULL);
     }

 }  /**/


 int STD_SCRIPT_COMMAND(intrin_inw_clear)

 {
   InputWinClr ();

   return (fpl_return_int(0));

 }  /**/


 int STD_SCRIPT_COMMAND(intrin_inw_out)

 {
   InputWinOut ((char *)arg->argv[0]);

   return (fpl_return_int(0));

 }  /**/


 int STD_SCRIPT_COMMAND(intrin_lastmsg_action)

 {
   F2 ();

  return(fpl_return_int(1));

 }  /**/


 int STD_SCRIPT_COMMAND(intrin_loadmenubar)

 {
   F3 ();

   return (fpl_return_int(1));

 }  /**/


  /* (pattern, str) */
 int STD_SCRIPT_COMMAND(intrin_match)

 {
   return (fpl_return_int(fnmatch(arg->argv[0], arg->argv[1], 0x1<<4)?0:1));

 }  /**/


 int STD_SCRIPT_COMMAND(intrin_servidx)

 {
  register int i=1;
  register ListEntry *eptr;
  extern List *servers;

    for (eptr=servers->head; eptr!=NULL; eptr=eptr->next, ++i)
     {
      if (arethesame((char *)((Server *)eptr->whatever)->server,
                     (char *)arg->argv[0]))
       {
        return (fpl_return_int(i));
       }
     }

   return (fpl_return_int(0));

 }  /**/


 int STD_SCRIPT_COMMAND(intrin_say)

 {
  int n;
  chtype p[]={[0 ... LARGBUF]=0};

   //n=mvsprintf (p, arg->argv[0], (char *)&arg->argv[1]);
    if ((n=mvsprintf(p, arg->argv[0], (char *)&arg->argv[1])))  say ("%z", p);

   return (fpl_return_int(n));

 }  /**/


 int STD_SCRIPT_COMMAND(intrin_strftime)

 {
  register char *s;
  char t[MEDIUMBUF];
  time_t now=time(NULL);

    strftime (t, MEDIUMBUF, arg->argv[0], localtime(&now));

     if (s=fplAllocString(key, strlen(t)+1))
      {
       strcpy (s, t);

       fpl_return_str(s, FPLSEND_DONTCOPY_STRING, TRUE);
      } 

 }  /**/


 int STD_SCRIPT_COMMAND(intrin_toserver)

 {
  int ret;

    return (fpl_return_int(ret=ToServer(arg->argv[0])));

 }  /**/


 int STD_SCRIPT_COMMAND(intrin_tosock)

 {
  register Socket *sptr;
  int sock=(int)arg->argv[0];
  extern Socket *so_hash[];

    if ((sock<0)||(sock>MAXSOCKSCOUNT)||(!(sptr=so_hash[sock])))
     {
      return (fpl_return_int(0));
     }

   ToSocket (sock, arg->argv[1]);

   return (fpl_return_int(1));

 }  /**/


 int STD_SCRIPT_COMMAND(intrin_unbindkey)

 {
    if (((int)arg->argv[0]<KEY_F0)||((int)arg->argv[0]>KEY_F(MAX_F_KEYS)))
     {
      return (fpl_return_int(0));
     }

   return (fpl_return_int(unBindKey((int)arg->argv[0])));

 }  /**/


 #include <prefs.h>

 int STD_SCRIPT_COMMAND(intrin_valueof)

 {
  char *s,
       *value;

    if (!(value=valueof(arg->argv[0])))  value="";

    if (s=fplAllocString(key, strlen(value)+1))
     {
      strcpy (s, value);

      return (fpl_return_str(s, FPLSEND_DONTCOPY_STRING, TRUE));
     }

 }  /**/


  /* set (pref, nvalue) */
 int STD_SCRIPT_COMMAND(intrin_set)

  {
   register char *s;

     if (!*(s=arg->argv[0])||!*(s=arg->argv[1]))  return (fpl_return_int(0));

    return (fpl_return_int(SetPref(arg->argv[0], arg->argv[1])));

 }  /**/


#include <useri.h>

 int STD_SCRIPT_COMMAND(intrin_parse)

 {
  extern char buf_i[];

    if (!*((char *)arg->argv[0]))  return;
 
   vsnprintf (buf_i, 512, (char *)arg->argv[0], (char *)&arg->argv[1]);
 
   ParseUserInput ();

   return (fpl_return_int(0));
  
 }  /**/


 int STD_SCRIPT_COMMAND(intrin_prog)

 {
  int r;
  register size_t i;
  size_t n,
         l=0;
  register struct InResponseToUser *aux;
  extern const int nUserCmnds;
  extern struct InResponseToUser *u;

   n=nUserCmnds-1;

    while (l<n)
     {
      aux=(u+(i=((l+n)/2)));

       if ((r=strcasecmp(arg->argv[0], aux->identifier))<0)
        {
         n=i;
        }
       else
       if (r>0)
        {
         l=i+1;
        }
       else
        {
         char *cmd;

          (*aux->read)(*(cmd=arg->argv[1])?cmd:NULL);

          return (fpl_return_int(1));
         }
     }

   return (fpl_return_int(0));

 }  /**/

