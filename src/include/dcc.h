
#define DCC_CHAT 1
#define DCC_SEND 2
#define DCC_GET  3

#define IS_DCC(x) ((x)->protocol==PROTOCOL_DCC)
#define IS_CHATTOUS(x) ((x)->type&DCCCHATTOUS)
#define IS_CHATTOHIM(x) ((x)->type&DCCCHATTOHIM)
#define IS_CHAT(x) (IS_CHATTOUS(x)||IS_CHATTOHIM(x))
#define IS_SEND(x) ((x)->type&DCCGETTING)
#define IS_GET(x) ((x)->type&DCCSENDING)
#define IS_DCCWAITING(x) ((x)->flags&AWAITINGCONNECTION)
#define IS_DCCCONNECTED(x) ((x)->flags&CONNECTED)

 void *InitDCCCHAT (Someone *);
 int InitDCCSEND (Someone *const, const char *);
 int AnswerDCCRequest_ (Socket *);

 int AttemptDCCConnection_ (Someone *, struct DCCParams *);

 int CheckFileBeforeSending (DCCParams *);
 int CheckFileBeforeGetting (DCCParams *);

 int ParseDCC_ (Socket *);
 int GetFilePacketfromSocket (Socket *);
 int SendFilePackettoSocket (Socket *);
 
 void EOFDCCConnection (Socket *, DCCParams *);
 int GetAllPendingFiles (Someone *);
 Index *FindDCCIndex (int, Someone *, ...);
 void CheckIdle_TimeoutDCC (void);
 void CloseDCCConnection (int, Someone *, ...);
 int CallFiringSquad (Someone *);
 void CloseDCCSocket (Socket *, DCCParams *);
 char *PrintDCCState (Index *iptr);
 void DisplayDCCSummary (Socket *, DCCParams *);
 int ShowDCCConnections (void);
 DCCParams *FillDCCEntry (Index *);
 void RemoveDCCEntry (DCCParams *);
 List *GetDCCListptr (void);

 void ScrollingDCCConnections (void *, int);

