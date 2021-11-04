
 struct Timer timers[]=
  {
   {1, 4, 30, 0L, "Notify",            (void (*)())ircNotify            },
   {2, 4, 30, 0L, "Lag",               (void (*)())PingServer           },
   {3, 2, 60, 0L, "Misc",              (void (*)())UpdateMisc           },
   {4, 2, 45, 0L, "Lights",            (void (*)())CheckLights          },
   {5, 4, 60, 0L, "DCC timeout",       (void (*)())CheckIdle_TimeoutDCC },
   {6, 4, 18, 0L, "Localusers Watcher", (void (*)())utmpNotify           },
   {7, 4, 60, 0L, "Split Users",       (void (*)())CheckSplitUsers      },
   {8, 2, 30, 0L, "Every 30 sec",      (void (*)())ExceEvery30Seconds   },
   {9, 4,  0, 0L, "",                  (void (*)())NULL                 },
  };


 const char *tmro_options[]={
             " Activate Timer   ",
             " Deactivate Timer ",
             " Execute          ",
             " Edit...          ",
           };
 static List tmro_ent={
         0, 4, NULL, NULL
        };
 static header tmro_hdr={4, NULL, tmro_options};

 ScrollingList tmro_list={
          SCR_OPTIONMENU,
          4,
          NULL,
          37, 0, 18, 4,
          0, 0, 0, 0, 0,
          "",
          (int *)NULL,
          (char *)NULL,
          &tmro_hdr,
          (List *)&tmro_ent,
          NULL, NULL, NULL,
          DisplayOptionMenuEntries,
          NULL, NULL
         };


  const char *tmr_hdrs[]={"ID", 
                         "Description",
                         "Period", "Flag", "Time left"};

  static int len[]={2, 15, 6, 7, 9};
  static header tmr_hdr={5,
                         len,
                         tmr_hdrs
                        };
  static int attrs[]={1, 1, 1, 0};
  ScrollingList tmr_list={
          SCR_SCROLLINGLIST,
          0,
          attrs,
          4, 3, 47, 9,
          1, 0, 0, 0, 0,
          " Timers ",
          (int *)NULL,
          (char *)NULL,
          &tmr_hdr,
          NULL, NULL,
          &tmro_list,
          NULL,
          ScrollingTimersEntries,
          ScrollingTimersInit,
          ScrollingTimersEnter,
         };

