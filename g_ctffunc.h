//#define CTF_TEAM_LIMIT			3 //you can do loops less than this
//#define CTF_TEAM_BLUE			2
//#define CTF_TEAM_RED			1
//#define CTF_TEAM_UNDEFINED		0
//#define CTF_TEAM_OBSERVER		-1

//-----making teams for observers-----
//-----bat


#define CTF_TEAM_LIMIT			3
#define CTF_TEAM_BLUE			2
#define CTF_TEAM_RED			1
#define CTF_TEAM_UNDEFINED		0
#define CTF_TEAM_OBSERVER		-1 //use this for observing both teams
#define CTF_TEAM_OBSERVER_BLUE	-2
#define CTF_TEAM_OBSERVER_RED	-3

#define CTF_TEAM_MIN_LIMIT		-4

//#define CTF_TEAM_IGNORETEAM	-2
//#define CTF_TEAM_ANYTEAM		-3
//#define CTF_TEAM_OPPOSING		-4
//#define CTF_TEAM_MATCHING		-5

//bat
#define CTF_TEAM_IGNORETEAM		-4
#define CTF_TEAM_ANYTEAM		-5
#define CTF_TEAM_OPPOSING		-6
#define CTF_TEAM_MATCHING		-7



#define CTF_CAPTURE_BONUS_CARRIER		5
#define CTF_CAPTURE_BONUS_TEAM			10

#define CTF_DEFEND_FLAG_RADIUS		800
#define CTF_DEFEND_BASE_RADIUS		600
#define CTF_DEFEND_REMOTE_RADIUS		400
#define CTF_DEFEND_CARRIER_RADIUS	500

#define DIS_BFG                         1
#define DIS_HYPERBLASTER                2
#define DIS_RAILGUN                     4
#define DIS_ROCKETLAUNCHER              8
#define DIS_GRENADELAUNCHER             16
#define DIS_CHAINGUN                    32
#define DIS_MACHINEGUN                  64
#define DIS_SUPERSHOTGUN                128
#define DIS_SHOTGUN                     256
#define DIS_PLASMA                      512

#define CTF_SPAM_BAND_MAX                         450
#define CTF_SPAM_BAND_MIN                         -150
#define CTF_SPAM_BAND_RECOVER                     5
#define CTF_SPAM_BAND_RADIO                       150
#define CTF_SPAM_BAND_VOICE                       75
#define CTF_SPAM_BAND_SAY                         60
#define CTF_SPAM_BAND_WARN_LEVEL                  90
#define CTF_SPAM_LOCKOUT_TIME                     5
#define CTF_SPAM_FREQ_MAX                         180
#define CTF_SPAM_FREQ_MIN                         0
#define CTF_SPAM_FREQ_MAX_ALLOWED                 50
#define CTF_SPAM_FREQ_EXTRA_PENALTY_TIME          0.25f
#define CTF_SPAM_FREQ_EXTRA_PENALTY               20
#define CTF_SPAM_FREQ_BAND_EXTRA_PENALTY_LEVEL    190
#define CTF_SPAM_FREQ_BAND_EXTRA_PENALTY          5

edict_t * ctf_findplayer(edict_t * ent_after, edict_t * ignore, int teamnum_wanted);
qboolean ctf_validateplayer(edict_t * ent, int teamnum_wanted);
qboolean ctf_flagatposition(vec3_t a, vec3_t b);
qboolean ctf_flagathome(edict_t * whichflag);
edict_t * ctf_getteamflag(int teamnum, int teamnum_option);
qboolean ctf_teamstring(char * buf, int teamnum, int teamnum_option);
qboolean ctf_validateflags();
qboolean ctf_resetflagandplayer(edict_t * whichflag, edict_t * whichplayer);
qboolean ctf_spawnflag(int teamnum);
edict_t * ctf_findflagposition(edict_t *whichflag);
void ctf_flagwave (edict_t *ent);
void ctf_playerdropflag(edict_t * whichplayer, gitem_t *item);
qboolean ctf_flagtouch (edict_t *ent, edict_t *other);
void ctf_deletespawnpointsnearflag (edict_t *flag);
edict_t * ctf_flagsearch(int whichteam);
void ctf_hook_abort(edict_t *ent);
void ctf_ResetFlagProps(edict_t * whichflag);

char *ctf_facing(edict_t *ent);
char *ctf_faceNorth(edict_t *ent);
char *ctf_faceEnemyFlag(edict_t *ent);
									  

//stuff for text replace functions
void replace_armor(edict_t *person, char *temp);
void replace_location(edict_t *person, char *temp, qboolean positionvalid, qboolean lineofsight);
qboolean visibility_test(edict_t *one, edict_t *two);
void replace_health(edict_t *person, char *temp);
void replace_artifact(edict_t *person, char *temp);
void replace_weapon(edict_t *person, char *temp);
void replace_team(edict_t *person, char *temp);
void replace_flaginfo(edict_t *person, char *temp);
void replace_viewinfo(edict_t *person, char *temp);
void replace_carrierinfo(edict_t *person, char *temp);

//assorted support functions
void LowerCase(char * src);
qboolean ctf_SpamCheck(edict_t *ent);
//void ctf_ClientDisconnect(edict_t *ent);
void ctf_TossEnt(edict_t * startent, edict_t * tossent);
void ctf_SetEntTeamEx(edict_t* ent, int whatteam, int nopenalty);
void ctf_SetEntTeam(edict_t* ent, int whatteam);
void ctf_SetLogName();
void ctf_SafePrint(edict_t * ent, long print_priority, char * buf);
void ctf_BSafePrint(long print_priority, char * buf);
void ctf_ChangeMap(char *mapname, qboolean startmatch);

//stuff currently in other files but needed 
int ClientShowID(edict_t *ent, char * buf);

void CTFSquadboardMessage (edict_t *ent, edict_t *killer); // ADC
