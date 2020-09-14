#include "g_local.h"

void Vote_Menu (edict_t *ent);
void Vote_YES (edict_t *ent);
void Vote_NO (edict_t *ent);
void Check_Vote(void);

// vote->value flags
#define CTF_VOTE_STARTED        1  
//assorted client flags, in the true meaning
#define CTF_EXTRAFLAGS_VOTE_YES			64
#define CTF_EXTRAFLAGS_VOTE_NO			128

//votetypes
#define CTF_VOTETYPE_SKIP		1
#define CTF_VOTETYPE_GOTOMAP	2
#define CTF_VOTETYPE_REFPLAYER	4

#define PERCENT_MAJORITY_REQUIRED  75 //requires a 75% majority for the vote to pass

extern int VoteStarted;
extern float VoteTime;
extern int VoteType;
