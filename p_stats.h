#ifndef _P_STATS_H_
#define _P_STATS_H_

#define STATS_PLAYER_SAMPLE_RATE 20 //how many frames between ping samples, 1 frame = 100 ms

#define STATS_PING_TOTAL		0	// long ping_total; //add to this every time ping is sampled
#define STATS_PING_SAMPLES		1	// long ping_samples; //# of samples(+1 each time you sample ping)
#define STATS_TIME				2	// long time; //time on server in seconds? minutes? 
												 // seconds would be cool for more precise calculation
#define STATS_SCORE				3	// int score; //the overall score
#define STATS_CAPTURES			4	// int captures; //how many direct flag captures
#define STATS_FRAGS				5	// int frags; //how many direct kills
#define STATS_DEATHS			6	// int deaths; //again, how many times you died
#define STATS_DEFENSE_FLAG		7	// int defense_flag; //defended the flag
#define STATS_DEFENSE_BASE		8	// int defense_base; //defended the base (whether flag present or not)
#define STATS_DEFENSE_CARRIER	9	// int defense_carrier; //defended flag carrier
#define STATS_ASSISTS			10	// int assists; //number of assists
#define STATS_RETURNS			11	// int returns; //returned the flag
#define STATS_OFFENSE_FLAG		12	// int offense_flag; //took the enemy flag
#define STATS_OFFENSE_CARRIER	13	// int offense_carrier; //killed enemy flag carrier
#define STATS_OFFENSE_FLAGLOST	14	// int offense_flaglost; //lost the enemy flag
#define MAX_PLAYER_STATS		15	


typedef struct {
	char name[MAX_INFO_STRING];
	int teamnum;
} stats_client_s;

typedef struct _stats_player {
	qboolean dropped; // whether this player was dropped

	stats_client_s info;

	long stats[MAX_PLAYER_STATS];

	struct _stats_player *pNext;
} stats_player_s;

typedef enum {
	STATS_SUICIDE,
	STATS_FRAG,
	STATS_FC_FRAG,
	STATS_FC_DEFENSE,
	STATS_FLAG_DEFENSE,
	STATS_FLAG_TOUCH,
	STATS_FLAG_RETURN,
	STATS_FLAG_CAPTURE,
	STATS_BASE_DEFENSE
} stats_event_t;
	
typedef struct {
	stats_client_s killer;
	stats_client_s killee;
	int mod;
	qboolean quad;
} stats_frag_data_s;

typedef struct {
	stats_client_s perp;
} stats_single_data_s;


void stats_add(edict_t *ent, int stat, long amount);
void stats_set(edict_t *ent, int stat, long amount);
long stats_get(edict_t *ent, int stat);
void stats_set_name(edict_t *ent, char *name);
void stats_clear(edict_t *ent);
void Cmd_PlayerStats_f (edict_t *ent);
void stats_log_init();
void stats_log_reset();
// returns pointer to lmctf_player_s struct of a dropped player given playername
stats_player_s* stats_find_dropped_player(char* name);
stats_player_s* stats_new_player(char* name);
void stats_cleanup(); // clean up stats before switching to next level
void Cmd_StatsAll_f (edict_t *ent);

#endif
