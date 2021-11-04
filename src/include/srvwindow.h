
 struct AwayStatus {
        time_t when;
        int nMsgs;
       };
 typedef struct AwayStatus AwayStatus;

 struct ServerWindow {
        unsigned flags; /* hold-mode, visible... */

        WINDOW *w;
        int nLines;
        int cx,
            cy;

        void *link;  /* link to dListEntry */

        ServerTable srv_t;
        ChannelTable ch_t;
        AwayStatus away;
        //Server server;
        List notify;
        ChannelsTable channels;
        Queue HoldQueue;
       };
 typedef struct ServerWindow ServerWindow;


 void InitServerWindows (void);
 ServerWindow *InitNewServerWindow (dList *);

