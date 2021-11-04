
 int intrin_alias (struct fplArgument *);
 int intrin_say (struct fplArgument *);
 int intrin_assoc (struct fplArgument *);
 int intrin_bindkey (struct fplArgument *);
 int intrin_chanid (struct fplArgument *);
 int intrin_chusermodes (struct fplArgument *);
 int intrin_inw_complete (struct fplArgument *);
 int intrin_eventidx (struct fplArgument *);
 int intrin_toserver (struct fplArgument *);
 int intrin_tosock (struct fplArgument *);
 int intrin_lastmsg_action (struct fplArgument *);
 int intrin_loadmenubar (struct fplArgument *);
 int intrin_match (struct fplArgument *);
 char *intrin_getserv (struct fplArgument *);
 int intrin_servidx (struct fplArgument *);
 int intrin_strftime (struct fplArgument *);
 int intrin_unbindkey (struct fplArgument *);
 int intrin_inw_out (struct fplArgument *);
 int intrin_inw_clear (struct fplArgument *);
 int intrin_valueof (struct fplArgument *);
 int intrin_set (struct fplArgument *);
 int intrin_parse (struct fplArgument *);
 int intrin_prog (struct fplArgument *);

 extern int intrin_menubar_define (struct fplArgument *);
 extern int intrin_menuentry_set (struct fplArgument *);
 extern int intrin_menubar_attachmenubox (struct fplArgument *);
 extern int intrin_menubarbutton_add (struct fplArgument *);
 extern int intrin_menubar_additem (struct fplArgument *);
 extern int intrin_menuboxitem_add (struct fplArgument *);
 extern int intrin_menuboxbutton_add (struct fplArgument *);
 extern int intrin_menubarentry_set (struct fplArgument *);
 extern int intrin_menuboxsep_add (struct fplArgument *);
 extern int intrin_menuentryselection_get (struct fplArgument *);

 extern int intrin_addcommandbutton (struct fplArgument *);
 extern int intrin_addmenubutton (struct fplArgument *);
 extern int intrin_addverticalseperator (struct fplArgument *);

