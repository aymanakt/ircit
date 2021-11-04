
 struct LastLog {
         time_t when;
         unsigned type;
         chtype *msg;
        };
 typedef struct LastLog LastLog;

 struct LastLogParams {
         const char *name;
         int max;
         List log;
        };
 typedef struct LastLogParams LastLogParams;

 enum LastLogType {
       LOG_MSG=0,
       LOG_NOTC,
       LOG_CTCP,
       LOG_MISC,
       LOG_TEL,
       LOG_PUB=0,
       LOG_JOIN,
       LOG_PART,
       LOG_KICK,
       LOG_MODE,
       LOG_OTHR
      };
 typedef enum LastLogType LogType;

 #define LLOG_MSG   (0x1<<0)
 #define LLOG_NOTC  (0x1<<1)
 #define LLOG_CTCP  (0x1<<2)

 #define CHLLOG_PUB   (0x1<<0)
 #define CHLLOG_JOIN  (0x1<<1)
 #define CHLLOG_PART  (0x1<<2)
 #define CHLLOG_KICK  (0x1<<3)
 #define CHLLOG_MODE  (0x1<<4)

// #define IS_CHLOGLEVEL(x, y) (((x)->loglevel&(y))?1:0)
 #define IS_CHLOGLEVEL(x, y) ((x)->lastlogs[(y)].max?1:0)
 #define _IS_LOGLEVEL(x) (LastLogs[(x)].max?1:0)

 char *FetchLogLevel (char **);
 LastLogParams *GetLog (const char *, const char *, int *);
 int AddtoLastLog (LogType, void *, chtype *);
 void ClearLastLog (LastLogParams *);
 int isLogLevel (LogType);
 void FinishoffLastLog (ListEntry *);
 int ParseLastLogPref (char *, const char *);
 int ShowLastLog (List *);

