/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>
#include <terminal.h>
#include <list.h>
#include <scroll.h>
#include <updates.h>
#include <people.h>
#include <sockets.h>
#include <telnet.h>

#include <sys/utsname.h>

 MODULEID("$Id: telnetcmds.c,v 1.4 1998/03/16 10:09:03 ayman Beta $");


 STD_TELNET_COMMAND(TU_HELP)

 {
   ToSocket (so, "\r\n-*-%s:\r\n%s\r\n",
             "Currently supported commands", "   HELP INFO MODE MSG QUIT WHO");

 }  /**/


 STD_TELNET_COMMAND(TU_INFO)

 {
  int ok;
  time_t t=time (NULL);
  struct utsname ut;
  extern char *c_version;

   ok=uname (&ut);

   ToSocket (so, "\r\n-*- This is %s running on %s %s (%s)"
                 "\r\n-*- Localhost %s (%s)"
                 "\r\n-*- On-line for %s seconds"
                 "\r\n-*- Current time is %s", c_version,
                 ok>=0?ut.sysname:"unknown", ok>=0?ut.release:"unknown",
                 ok>=0?ut.machine:"unknown",
                 localhost(), RawIPToDotted(localip()),
                 secstostr(t-tt_ptr->online),
                 ctime(&t));

 }  /**


 /*
 ** mode -|+i will not be listed in 'who' command
 */
  STD_TELNET_COMMAND(TU_MODE)

 {
  int add=0;
  register char *p;
  Socket *sptr;
  Index *iptr;
  register Telnet *tptr;
  extern Socket *so_hash[];

   sptr=so_hash[so];

    if (sptr==(Socket *)NULL)
       {
        say ("%$% Socket hashtable not in sync: Telnet (%d).\n", HOT, so);
        ToSocket (so, "-!- Unable to process your request.\r\n");

        return;
       }

   iptr=SOCKETTOINDEX(sptr);

    if (IS_TELNET(iptr))
       {
        tptr=INDEXTOTELNET(iptr);
       }
    else
       {
        say ("%$% Internal Protocol structures not in sync: Telnet (%d).\n",
              HOT, so);
        ToSocket (so, "-!- Unable to process your request.\r\n");

        return;
       }

    if (!args)
       {
        register int i=1;
        char s[]={[0 ... MINIBUF]=0};

         s[0]='+';

          if (tptr->flags&1)
             s[i++]='i';

         s[i]=0;

          if (!s[1])
             {
              ToSocket (so, "-*- Empty mode-set.\r\n");
             }
          else
             {
              ToSocket (so, "-*- Current mode %s\r\n", s);
             }

         return;
       }

    if (p=args, ((*p!='-')&&(*p!='+')))
       {
        ToSocket (so, "-!- arguments invalid or incomlete.\r\n");

        return;
       }

    do 
      {
       switch (*p)
              {
               case '+':
                    add=1;
                    break;
               case '-':
                    break;
               case 'i':
                    add?(tptr->flags|=1):(tptr->flags&=~1);
                    break;
               default:
                    break;
                    
              }
      }
    while (*p++);

 }  /**/


 /*
 ** .msg nick,nick,.. msg
 */
 STD_TELNET_COMMAND(TU_MSG)

 {
  register int j;
  int i;
  char n[LARGBUF];
  Socket *sptr;
  Someone *pptr;
  register Index *iptr;
  Telnet *tptr;
  extern char tok[][80];

    if (!args)
       {
        ToSocket (so, "-!- arguments invalid or incomplete.\r\n");

        return;
       }

   otokenize (n, args, ' ');
   wipespaces (args);

    if (!*args)
       {
        *args=' ';
        *(args+1)=0;
       }

   i=splitargss (n, ',');

    for (j=0; j<=(i-1); j++)
        {
          if (strlen(tok[j])>11)
             {
              ToSocket (so, "-!- name too long: ``%s''.\n", tok[j]);
              continue;
             }

          if ((pptr=SomeonebyName(tok[j]))==(Someone *)NULL)
             {
              ToSocket (so, "-!- ``%s'' is not on.\n", tok[j]);
              continue;
             }

         if (GOT_INDICES(pptr))
            {
             iptr=pptr->head;

              for ( ; iptr!=NULL; iptr=iptr->next)
                  {
                    if (IS_TELNET(iptr))
                       {
                        tptr=INDEXTOTELNET(iptr);

                         if (IS_CONNECTED_TELNET(tptr))
                            {
                             sptr=INDEXTOSOCKET(iptr);

                             ToSocket (sptr->sock, 
                               "[<%s>] %s\r\n", pptr->name, args);
                            }
                       }
                  }
            }
        }
  
 }  /**/


 STD_TELNET_COMMAND(TU_QUIT)

 {
  char s[MEDIUMBUF];
  Someone *pptr;
  register Socket *sptr;
  register Index *iptr;
  extern Socket *so_hash[];
  extern SocketsTable *const sockets;

   sptr=so_hash[so];

    if (sptr)
       {
        iptr=SOCKETTOINDEX(sptr);
        pptr=INDEXTOSOMEONE(iptr);

        close (so);

        say ("%$% 3Telnet user 3%s [%s] logged off.\n", TELH,
             pptr->name, sptr->haddress);

        RemovefromSocketsTable (sockets, so);

        sprintf (s,"-*- User %s logged off.\r\n", pptr->name);
        BroadcastMsg (s, 0);

        RemoveIndexbyptr (pptr, iptr);
       }

 }  /**/


 STD_TELNET_COMMAND(TU_TIME)

 {
  time_t t=time(NULL);

   ToSocket (so, "-*- Current local time is %s\r\n", ctime(&t));

 }  /**/


 STD_TELNET_COMMAND(TU_WHOM)

 {
  time_t t=time (NULL);
  char s[6];
  register Someone *pptr;
  register Index *iptr;
  register Telnet *tptr;
  register Socket *sptr;
  extern People *const people;
  extern MiscTable *const mt_ptr;


   ToSocket (so, "\r\n-*- %d user%scurrently logged in:\r\n-*- %s\r\n", 
             mt_ptr->nTelnets, mt_ptr->nTelnets>1?"s ":" ",
             "Invisible users will not be listed.");

   ToSocket (so, " %-11s %-15s %-5s %-8s\r\n%s\r\n", 
            "Name", "Address", "When", "Idle",
            "+-----------+---------------+-----+--------+ ");

   pptr=people->head;

    for ( ; pptr!=NULL; pptr=pptr->next)
        {
          if (pptr->nEntries>=1)
             {
              iptr=pptr->head;

                for ( ; iptr!=NULL; iptr=iptr->next)
                    {
                      if (iptr->protocol==PROTOCOL_TELNET)
                         {
                          (Telnet *)tptr=INDEXTOPROTOCOL(iptr);
                          (Socket *)sptr=INDEXTOSOCKET(iptr);
                          
                            if (IS_T_INVISIBLE(tptr))
                               {
                                continue;
                               }
 
                           strftime (s, 10, "%H:%M", localtime(&iptr->when));

                           ToSocket (so, " %-11s %-15.15s %-5s %-8s\r\n",
                                     pptr->name, sptr->haddress, s,
                                     secstostr(t-iptr->idle, 1));
                         }
                    }
             }
        }

 }  /**/

