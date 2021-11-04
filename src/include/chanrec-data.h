   const char *ch_options[]={
              " Switch to  ",
              " Users...   ",
              " Leave      ",
              " Mass-op    ",
              " Mass-deop  ",
              " Mass-kick  "
             };
   List cho_ent={
         0, 6, NULL, NULL
        };
  header cho_hdr={6, NULL, ch_options};
  ScrollingList cho_list={
          SCR_OPTIONMENU,
          6,
          NULL,
          37, 0, 12, 2,
          0, 0, 0, 0, 0,
          "",
          (int *)NULL,
          (char *)NULL,
          &cho_hdr,
          (List *)&cho_ent,
          NULL, NULL, NULL,
          DisplayOptionMenuEntries,
          NULL, NULL
         };


  const char *ch_hdrs[]={"Channel", "nUser", "Modes"};
  const int len[]={15, 5, 12};
  header ch_hdr={3, len, ch_hdrs};
  int attrs[]={1, 1, 1, 1, 1, 1};
  ScrollingList ch_list={
          SCR_SCROLLINGLIST,
          0,
          attrs,
          5, 2, 38, 9,
          0, 0, 0, 0, 0,
          " Active Channels ",
          (int *)NULL,
          (char *)NULL,
          &ch_hdr,
          (void *)NULL,
          NULL,
          &cho_list,
          NULL,
          ScrollingChannelsEntries,
          ScrollingChannelsInit,
          ScrollingChannelsEnter
         };

