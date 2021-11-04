
 struct DNSCacheEntry {
         time_t when;
         char dns[MAXHOSTLEN],
              dotted[16];
         unsigned flags;
         struct in_addr saddr;

         void *link;
        };
 typedef struct DNSCacheEntry DNSCacheEntry; 
 
 DNSCacheEntry *AddDNSCacheEntry (struct in_addr, const char *, const char *);
 DNSCacheEntry *FindDNSEntry (const char *, void *);

