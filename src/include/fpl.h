
 #define fpl_return_int(x...) fplSendTags(key, FPLSEND_INT, x, FPLSEND_DONE)
 #define fpl_return_str(x...) \
         fplSendTags(key, FPLSEND_STRING, x, FPLSEND_DONE)
 #define arg0 arg->argv[0]
 #define arg1 arg->argv[1]
 #define arg2 arg->argv[2]
 #define arg3 arg->argv[3]
 #define arg4 arg->argv[4]
 #define arg5 arg->argv[5]
 #define arg6 arg->argv[6]

 int InitInterpreter (void);

 void uFPL (char *);
 void uLOAD (char *);
 void uASSOC (char *);

