/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>
#include <terminal.h>
#include <list.h>

#include <netinet/in.h>

#include <net.h>
#include <dnscache.h>


 MODULEID("$Id: dnscache.c,v 1.1 1998/03/23 10:05:46 ayman Beta $");

 static List DNSCache;

 static ListEntry *FindLeastAccessed (void);
 static void RemoveDNSCacheEntry (ListEntry *);

 #define DNSCacheEmpty() (DNSCache.nEntries>0?0:1)
 #define nDNSCacheEntries() (DNSCache.nEntries)
 #define dnsptr ((DNSCacheEntry *)eptr->whatever)

 static ListEntry *FindLeastAccessed (void)

 {
  time_t now=time (NULL);
  register ListEntry *eptr=DNSCache.head,
                     *aux=DNSCache.head;

    for ( ; eptr!=NULL; eptr=eptr->next)
     {
       if (dnsptr->when<((DNSCacheEntry *)aux->whatever)->when)  aux=eptr;
     }

   return (ListEntry *)aux;
 
 }  /**/


 static __inline__ void RemoveDNSCacheEntry (ListEntry *eptr)

 {
    if (eptr)
     {
      memset (eptr->whatever, 0, sizeof(DNSCacheEntry));
      free (eptr->whatever);

      RemovefromList (&DNSCache, eptr);
     }

 }  /**/


 DNSCacheEntry *
  AddDNSCacheEntry (struct in_addr saddr, const char *dns, const char *dotted)

 {
  const size_t sizeofDNSCacheEntry=sizeof(DNSCacheEntry);
  ListEntry *eptr;
  DNSCacheEntry *dnscptr;

    if (DNSCache.nEntries>=10)
     {
      RemoveDNSCacheEntry(FindLeastAccessed());
     }

   eptr=AddtoList (&DNSCache);
   
   xmalloc(dnscptr, sizeofDNSCacheEntry);
   memset (dnscptr, 0, sizeofDNSCacheEntry);

   dnscptr->when=time (NULL);
   memcpy (&dnscptr->saddr, &saddr, sizeof(struct in_addr));
   strcpy (dnscptr->dns, dns);
   strcpy (dnscptr->dotted, dotted);

   (DNSCacheEntry *)eptr->whatever=dnscptr;
   (ListEntry *)dnscptr->link=eptr;

   return (DNSCacheEntry *)dnscptr;

 }  /**/


 DNSCacheEntry *FindDNSEntry (const char *host, void *ptr)

 {
  register ListEntry *eptr=DNSCache.head;

    if (DNSCacheEmpty())  return (DNSCacheEntry *)NULL;

    for ( ; eptr!=NULL; eptr=eptr->next)
     {
       if ((arethesame(host, dnsptr->dns))||(arethesame(host, dnsptr->dotted)))
        {
         dnsptr->when=time (NULL);
              
         return (DNSCacheEntry *)eptr->whatever;
        }
     }
     
   return (DNSCacheEntry *)NULL;
  
 }  /**/

