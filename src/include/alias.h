
 struct Alias {
         char *name,
              *what,
              *help;
         unsigned flags;
        };
 typedef struct Alias Alias;

 int MarkWords (char *, char **);
 int AddAlias (List *, const char *, char *, const char *);
 int RemoveAlias (List *, const char *);
 int ExpandAlias (char *, char *);
 int TryAliasesInstead (char *, char *);
 void ExecAlias (void *, char *);
 void uALIAS (char *);
 void ShowAliases (List *, const char *);

