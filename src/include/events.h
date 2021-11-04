
 typedef enum {
         EVNT_STARTUP=1,
         EVNT_SERVERUP, 
         EVNT_SERVERDOWN,
         EVNT_ODDSRVCMND,
         EVNT_CHANJOIN,
         EVNT_CHANUSRJOIN,
         EVNT_CHANPART,
         EVNT_CHANUSRPART,
         EVNT_CHANPRIVMSG,
         EVNT_USRPRIVMSG,
         EVNT_TOPIC,
         EVNT_NICKCHANGE,
         EVNT_INVITE,
         EVNT_CHANCTCP,
         EVNT_USRCTCP,
         EVNT_CHANKICK,
         EVNT_CHANUSRKICK
        } EventCode;
       
 struct Event {
         EventCode event;
         const char *desc;
         char *hook,
              *regx;
         int nparams;           
         unsigned flags;
      };
 typedef struct Event Event;

#define MAX_EVENTS  17

#define EVENT_DEFINED  (0x1<<0)

 void *EventRegistered (EventCode);
 void ExecHook (void *, const char *, ...);

