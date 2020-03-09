#ifndef _P_TOURNEY_H_
#define _P_TOURNEY_H_

void SpawnTourneyClock ();
void StartMatch (char *levelname);
void KillMatch();
qboolean Match_CanScore();
qboolean Match_InPlay();
qboolean Match_Mode();
qboolean Match_InCountdown();
qboolean GamePaused();
void SetPause(qboolean state);
void Victory();
qboolean Match_Over();
edict_t *Query_OMVP();
edict_t *Query_DMVP();
void Reset_MVP();
void Match_End(edict_t *ent);

extern int matchstate;
					  

typedef enum
{
	MATCH_NONE,
	MATCH_ENDLEVEL, //Match_Mode false before here
	MATCH_COUNTDOWN,
	MATCH_INPLAY,
	MATCH_OVER,
	MATCH_RAILGUN_COUNTDOWN, //bat
	MATCH_RAILGUN_INPLAY,
	MATCH_RAILGUN_OVER,
} MATCH_STATES;


#endif
