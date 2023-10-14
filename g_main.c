
#include "g_local.h"
#include "g_ctffunc.h" //surt for some nice wrapper functions
#include "g_tourney.h"

#include "stdlog.h"	//	StdLog - Mark Davies
#include "gslog.h"	//	StdLog - Mark Davies
#include "bat.h"

#ifdef _WIN32
_CrtMemState startup1;	// memory diagnostics
#endif

game_locals_t	game;
level_locals_t	level;
game_import_t	gi;
game_export_t	globals;
spawn_temp_t	st;

int	sm_meat_index;
int	snd_fry;
int meansOfDeath;

int quad_respawn_time = LM_QUAD_DEFAULT_TIME;

edict_t		*g_edicts;

cvar_t	*deathmatch;
cvar_t	*coop;
cvar_t	*dmflags;
cvar_t	*skill;
cvar_t	*fraglimit;
cvar_t	*timelimit;
cvar_t	*railtime;
cvar_t	*password;
cvar_t	*spectator_password;
cvar_t	*needpass;
cvar_t	*maxclients;
cvar_t	*maxspectators;
cvar_t	*maxentities;
cvar_t	*g_select_empty;
cvar_t	*dedicated;

cvar_t	*filterban;

cvar_t	*sv_maxvelocity;
cvar_t	*sv_gravity;

cvar_t	*sv_rollspeed;
cvar_t	*sv_rollangle;
cvar_t	*gun_x;
cvar_t	*gun_y;
cvar_t	*gun_z;

cvar_t	*run_pitch;
cvar_t	*run_roll;
cvar_t	*bob_up;
cvar_t	*bob_pitch;
cvar_t	*bob_roll;

cvar_t	*sv_cheats;

cvar_t	*runes; // CTF CODE -- LM_JORM
cvar_t	*ctfflags; // CTF CODE -- LM_JORM
cvar_t	*refset; //CTF CODE -- LM_SURT
cvar_t	*logrename; //CTF CODE -- LM_SURT
cvar_t	*hostname; // CTF CODE -- LM_JORM
cvar_t	*gamedir; // CTF CODE -- LM_JORM
cvar_t	*skinset; // CTF CODE -- LM_JORM
#ifdef OLDOBSERVERCODE
cvar_t	*autoobserve; // CTF CODE -- LM_JORM
#endif
cvar_t	*refpassword; // CTF CODE -- LM_JORM
cvar_t	*rconpassword; // CTF CODE -- LM_SURT
cvar_t	*motd_file;	// CTF CODE -- LM_SURT
cvar_t	*server_file;	// CTF CODE -- LM_SURT
cvar_t	*maplist_file;	// CTF CODE -- LM_SURT
cvar_t	*skin_file;	// CTF CODE -- LM_SURT
cvar_t	*skin_debug; // For debugging skin files
cvar_t	*disabled_weps;	// CTF CODE -- LM_SURT
cvar_t  *flag_init;
cvar_t  *fastswitch;
cvar_t  *mod_website;
cvar_t  *autolock;
cvar_t  *countdown_time;

#ifdef ZBOT
cvar_t  *use_zbotdetect; // ZBOT Detect -- LM_Hati
#endif

edict_t	*redflag; // CTF CODE -- LM_JORM
edict_t	*blueflag; // CTF CODE -- LM_JORM

cvar_t	*sv_maplist;

char	motd[1000]; // CTF CODE -- LM_JORM

MapInfo	maplist[300]; // CTF CODE -- LM_JORM
int		maplistindex  = 0; // CTF CODE -- LM_JORM

int		bluescore = 0, redscore = 0; // CTF CODE -- LM_JORM
char	helptext[1000][25]; // CTF CODE -- LM_JORM

void SpawnEntities (char *mapname, char *entities, char *spawnpoint);
void ClientThink (edict_t *ent, usercmd_t *cmd);
qboolean ClientConnect (edict_t *ent, char *userinfo);
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void ClientDisconnect (edict_t *ent);
void ClientBegin (edict_t *ent);
void ClientCommand (edict_t *ent);
void WriteGame (char *filename, qboolean autosave);
void ReadGame (char *filename);
void WriteLevel (char *filename);
void ReadLevel (char *filename);
void InitGame (void);
void G_RunFrame (void);


extern int Time_Left;



//===================================================================


void ShutdownGame (void)
{
	gi.dprintf ("==== ShutdownGame ====\n");

	sl_GameEnd( &gi, level );	// StdLog - Mark Davies

	gi.FreeTags (TAG_LEVEL);
	gi.FreeTags (TAG_GAME);

#ifdef _WIN32
	OutputDebugString("ShutdownGame() was called.\n");
	OutputDebugString("Dump objects since startup.\n");
	_CrtMemDumpAllObjectsSince(&startup1);
	OutputDebugString("Memory stats since startup.\n");
	_CrtMemDumpStatistics(&startup1);
	_CrtDumpMemoryLeaks();
#endif

}


/*
=================
GetGameAPI

Returns a pointer to the structure with all entry points
and global variables
=================
*/
q_exported game_export_t *GetGameAPI (game_import_t *import)
{
	gi = *import;

	globals.apiversion = GAME_API_VERSION;
	globals.Init = InitGame;
	globals.Shutdown = ShutdownGame;
	globals.SpawnEntities = SpawnEntities;

	globals.WriteGame = WriteGame;
	globals.ReadGame = ReadGame;
	globals.WriteLevel = WriteLevel;
	globals.ReadLevel = ReadLevel;

	globals.ClientThink = ClientThink;
	globals.ClientConnect = ClientConnect;
	globals.ClientUserinfoChanged = ClientUserinfoChanged;
	globals.ClientDisconnect = ClientDisconnect;
	globals.ClientBegin = ClientBegin;
	globals.ClientCommand = ClientCommand;

	globals.RunFrame = G_RunFrame;

	globals.ServerCommand = ServerCommand;

	globals.edict_size = sizeof(edict_t);

	return &globals;
}

#ifndef GAME_HARD_LINKED
// this is only here so the functions in q_shared.c and q_shwin.c can link
void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	gi.error (ERR_FATAL, "%s", text);
}

void Com_Printf (char *msg, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	gi.dprintf ("%s", text);
}

#endif

//======================================================================


/*
=================
ClientEndServerFrames
=================
*/
void ClientEndServerFrames (void)
{
	int		i;
	edict_t	*ent;

	// calc the player views now that all pushing
	// and damage has been added
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse || !ent->client)
			continue;
		ClientEndServerFrame (ent);
	}

}

/*
=================
CreateTargetChangeLevel

Returns the created target changelevel
=================
*/
edict_t *CreateTargetChangeLevel(char *map)
{
	edict_t *ent;

	ent = G_Spawn ();
	ent->classname = "target_changelevel";
	Com_sprintf(level.nextmap, sizeof(level.nextmap), "%s", map);
	ent->map = level.nextmap;
	return ent;
}



#define MAX_MAPS	300

short Maps_Picked[MAX_MAPS];

short Last_Map = 0;

void Randomize_Map_List(int Num_Of_Maps)
{
	int i;
	int Rand_Num;

	if (Num_Of_Maps > MAX_MAPS)
		Num_Of_Maps = MAX_MAPS;

	for (i = 0; i < MAX_MAPS; i++)
		Maps_Picked[i] = 0;

	for (i = 1; i < Num_Of_Maps; i++)
	{
		Rand_Num = rand() % (Num_Of_Maps - 2);
		while (Maps_Picked[Rand_Num])
		{
			Rand_Num++;
			if (Rand_Num == Num_Of_Maps)
				Rand_Num = 0;
		}

		Maps_Picked[Rand_Num] = i;
	}
}

MapInfo *getRandomMapByPlayerCount(int count) {
	MapInfo *criteriaList = NULL;
	MapInfo *clPtr = NULL;
	int mapCounter = 0;
	int randNum;

	for(int i = 0; maplist[i].mapname; i++) {
		if (maplist[i].minplayers > count || maplist[i].maxplayers < count) {
			gi.dprintf("Excluding %s due to min/max playercount\n", maplist[i].mapname);
			continue;
		}
		if (!criteriaList) {
			criteriaList = clPtr = &maplist[i];			
		} else {
			clPtr->next = &maplist[i];
			clPtr = clPtr->next;
			clPtr->next = NULL;
		}
		mapCounter++;
	}

	//QW// Added. This should never happen.
	if (!clPtr || !criteriaList) {
		gi.error("%s line %i: NULL pointer error.", __func__, __LINE__);
		abort(); // Shut up compiler.
	}

	clPtr->next = criteriaList;
	srand(time(0));
	randNum = rand() % (mapCounter - 2);
	for(int i = 0; i < randNum; i++) 
		clPtr = clPtr->next;	
	return clPtr;
}

int playerCount() {
        edict_t * player = NULL;
        unsigned int num_players = 0;

        player = ctf_findplayer(NULL, NULL, CTF_TEAM_IGNORETEAM);
        while (player)
        {
                num_players++;
                player = ctf_findplayer(player, NULL, CTF_TEAM_IGNORETEAM);
        }
        return (num_players);
}

HIGHSCORE_STATS_TYPE Default_Highscore_Table[MAX_HIGHSCORE_ENTRIES] = 
{
	{"defiance[7ds]", 60},
	{"SpunkMonkey[IT]", 55},
	{"Virtue[7ds]", 50},
	{"Rage[7ds]", 45},
	{"Greed[7ds]", 40},
	{"Envy[7ds]", 35},
	{"Sloth[7ds]", 30},
};

HIGHSCORE_STATS_TYPE Highscore_Table[MAX_HIGHSCORE_ENTRIES]; 



void Create_New_File(void)
{
int i;
HIGHSCORE_STATS_TYPE *ptr0, *ptr1;

	ptr0 = &Highscore_Table[0];
	ptr1 = &Default_Highscore_Table[0];

	for(i = 0; i < MAX_HIGHSCORE_ENTRIES; i++, ptr0++, ptr1++)
		*ptr0 = *ptr1;
}



void Check_Made_Table(edict_t *cl_ent)
{
long score;
int index = 0;
int i;

   	score = stats_get(cl_ent, STATS_SCORE);

	if(score <= Highscore_Table[MAX_HIGHSCORE_ENTRIES - 1].Score)
		return;


	for(i = 0; i < MAX_HIGHSCORE_ENTRIES; i++)
	{
		if(score > Highscore_Table[i].Score)
		{
			index = i;
			break;
		}
	}

	for(i = MAX_HIGHSCORE_ENTRIES - 1; i > index; i--)
	{
		Highscore_Table[i].Score = Highscore_Table[i - 1].Score;
		strcpy(Highscore_Table[i].Player, Highscore_Table[i-1].Player);
	}


	Highscore_Table[index].Score = score;
	strcpy(Highscore_Table[i].Player, cl_ent->client->pers.netname);

}

//bat
void Write_Highscore_Table(void)
{
edict_t     *cl_ent;
char Buf[512];
FILE *fp;
int i;
size_t Obj_Read;

	if(maplistindex < 0)
		return;

	sprintf(Buf, "%s/%s.scr", gamedir->string, level.mapname);


	fp = fopen(Buf, "rb");

	if(fp == 0)
		Create_New_File();
	else
	{
		Obj_Read = fread(Highscore_Table, sizeof(HIGHSCORE_STATS_TYPE), MAX_HIGHSCORE_ENTRIES, fp);
		if(Obj_Read < MAX_HIGHSCORE_ENTRIES)
		{
			fclose(fp);
			Create_New_File();
		}
	}

	fp = fopen(Buf, "wb");
	if(fp == 0)
		return;


    for(i = 0; i < game.maxclients; i++)
    {
        cl_ent = g_edicts + 1 + i;
		Check_Made_Table(cl_ent);
    }

	
	fwrite(Highscore_Table, sizeof(HIGHSCORE_STATS_TYPE), MAX_HIGHSCORE_ENTRIES, fp);
	fclose(fp);
}


void Set_Mode_Railgun_Shootout(void)
{
	matchstate = MATCH_RAILGUN_COUNTDOWN;
	SpawnTourneyClock ();
}


edict_t *Railgun_Victor;


int One_Man_Left(void)
{
edict_t *cl_ent;
int i;
int Living_People = 0;

	for(i = 0; i < game.maxclients; i++)
    {
        cl_ent = g_edicts + 1 + i;

		if(cl_ent->health > 0)
			Living_People++;

		if(Living_People > 1)
			return(false);
	}

	return(true);

}

//Need to do this at time = 0, before ending match.
edict_t *Declare_Railgun_Victor(void)
{
int Highscore = -9999;
edict_t *Winner;
edict_t *cl_ent;
int i;
int score;
    
	Winner = g_edicts + 1;

	for(i = 0; i < game.maxclients; i++)
    {
        cl_ent = g_edicts + 1 + i;

		if(cl_ent->health <= 0)
			continue;

		score = stats_get(cl_ent, STATS_SCORE);

    	if(score > Highscore)
		{
			Winner = cl_ent;
			Highscore = score;
		}
    }

	return(Winner);

}


/*
=================
EndDMLevel

The timelimit or fraglimit has been exceeded
=================
*/
void EndDMLevel (void)
{
edict_t		*ent;
int map_count = 0;


	if((matchstate != MATCH_RAILGUN_OVER) && (railtime->value > 0))
	{
		Set_Mode_Railgun_Shootout();
		return;
	}

	KillMatch();
	Write_Highscore_Table();


	// stay on same level flag
	if ((int)dmflags->value & DF_SAME_LEVEL)
	{
		BeginIntermission (CreateTargetChangeLevel (level.mapname) );
		return;
	}

	// CTF CODE -- LM_JORM
	else if (maplistindex != -2) // -2 means no list
	{	// go to a specific map
		ent = G_Spawn ();
		ent->classname = "target_changelevel";
		
		//bat
		while(Maps_Picked[maplistindex] == Last_Map)
		{
			if(maplistindex > -1 && !maplist[maplistindex].mapname) // Did we reach the end of the list?
				maplistindex = 0;

			maplistindex++;

			//if all the maps are the same, this is a precaution.
			if(map_count++ > 30)
				break;
		}

		if((int)ctfflags->value & CTF_RANDOM_MAPS)
		{
			MapInfo *map = getRandomMapByPlayerCount(playerCount());
			ent->map = map->mapname;
			gi.dprintf("Map :  %s\n",  map->mapname);
		}
		else
		{
			ent->map = maplist[maplistindex].mapname;
			gi.dprintf("Map #%d:  %s\n", maplistindex+1, maplist[maplistindex].mapname);
		}

		maplistindex++; 
	}
	// END CTF CODE -- LM_JORM

	else if (level.nextmap[0])
	{	// go to a specific map
		ent = G_Spawn ();
		ent->classname = "target_changelevel";
		ent->map = level.nextmap;
	}
	else
	{	// search for a changeleve
		ent = G_Find (NULL, FOFS(classname), "target_changelevel");
		if (!ent)
		{	// the map designer didn't include a changelevel,
			// so create a fake ent that goes back to the same level
			ent = G_Spawn ();
			ent->classname = "target_changelevel";
			ent->map = level.mapname;
		}
	}
	

	BeginIntermission (ent);
}


/*
=================
CheckNeedPass
=================
*/
void CheckNeedPass (void)
{
	int need;

	// if password or spectator_password has changed, update needpass
	// as needed
	if (password->modified || spectator_password->modified) 
	{
		password->modified = spectator_password->modified = false;

		need = 0;

		if (*password->string && Q_stricmp(password->string, "none"))
			need |= 1;
		if (*spectator_password->string && Q_stricmp(spectator_password->string, "none"))
			need |= 2;

		gi.cvar_set("needpass", va("%d", need));
	}
}

/*
=================
CheckDMRules
=================
*/
void CheckDMRules (void)
{
	int			i;
	//gclient_t	*cl;
	edict_t		*ent, *player;

	if (level.intermissiontime)
		return;

	if (!deathmatch->value)
		return;


	if(matchstate == MATCH_RAILGUN_OVER)
	{
		EndDMLevel();
		return;
	}


	//maplist support stuff
	if (maplistindex == -1) // First time through the list
	{
		gi.dprintf ("Using Maplist.\n");
		if (!strcmp(maplist[0].mapname, level.mapname))
		{
			maplistindex = 1;
			return;
		}
		else
		{
			MapInfo *firstmap = &maplist[0];
			ent = G_Spawn ();
			ent->classname = "target_changelevel";
	                if((int)ctfflags->value & CTF_RANDOM_MAPS)
                	        firstmap = getRandomMapByPlayerCount(playerCount());
			ent->map = firstmap->mapname;
			maplistindex = 1;
			level.changemap = ent->map;
			level.exitintermission = 1;
                        gi.dprintf("Startup Map :  %s\n",  firstmap->mapname);
			return;
		}
	}
	
	if (timelimit->value && !Match_Mode()) // Don't end a map if a Match is in play
	{
		for (i=0 ; i<maxclients->value ; i++)
		{
			player = g_edicts + 1 + i;
			if (!player->inuse)
				continue;

			//seconds
			if(level.time >= (timelimit->value - 1)*60)
				Time_Left = (timelimit->value*60) - level.time;
			else
				Time_Left = timelimit->value - (level.time/60) + 1;
		}

		if (level.time >= timelimit->value*60)
		{
			ctf_BSafePrint(PRINT_HIGH, "Timelimit hit.\n");
			EndDMLevel ();
			return;
		}
	}

	if (fraglimit->value && !Match_Mode()) // Don't end a map if a match is in play
	{
		
		for (i=0 ; i<maxclients->value ; i++)
		{
			player = g_edicts + 1 + i;
			if (!player->inuse)
				continue;

			if (stats_get(player, STATS_SCORE) >= fraglimit->value)
			{
				ctf_BSafePrint(PRINT_HIGH, "Fraglimit hit.\n");
				EndDMLevel ();
				return;
			}
		}
	}
}


/*
=============
ExitLevel
=============
*/
void ExitLevel (void)
{
	int		i;
	edict_t	*ent;
	char	command [MAX_INFO_STRING];

	Com_sprintf (command, sizeof(command), "gamemap \"%s\"\n", level.changemap);

	gi.AddCommandString (command);
	level.changemap = NULL;
	level.exitintermission = 0;
	level.intermissiontime = 0;
	ClientEndServerFrames ();

	// clear some things before going to next level
	for (i = 0; i < maxclients->value; i++)
	{
		ent = g_edicts + 1 + i;
		if (ent && ent->inuse)
		{
			if (ent->health > ent->client->pers.max_health)
				ent->health = ent->client->pers.max_health;
		}
	}

	// clean up stats before start of next level
	stats_cleanup(); // STATS - LM_Hati
}

/*
================
G_RunFrame

Advances the world by 0.1 seconds
================
*/
void G_RunFrame (void)
{
	int		i;
	edict_t	*ent;

	// CTF CODE -- LM_JORM
	int			score;
	edict_t		*cl_ent;

	// Paril
	if (GamePaused())
	{
		ClientEndServerFrames();
		return;
	}
	// Paril

	bluescore = 0; 
	redscore = 0;  

	if ((int)(skinset->value) > 3)
		gi.cvar_set("skinset", va("%d", 3));

	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		score = stats_get(cl_ent, STATS_SCORE);

		if (cl_ent->client->ctf.teamnum == CTF_TEAM_RED) // RED TEAM
		{
			redscore += score;
		}
		else if (cl_ent->client->ctf.teamnum == CTF_TEAM_BLUE) // BLUE TEAM
		{
			bluescore += score;
		}
	}
	// END CTF CODE -- LM_JORM


	level.framenum++;
	level.time = level.framenum*FRAMETIME;

	// choose a client for monsters to target this frame
	//no monsters
	//bat
#ifdef MONSTERS_OK
	AI_SetSightClient ();
#endif

	// exit intermissions

	if (level.exitintermission)
	{
		ExitLevel ();
		return;
	}

	//
	// treat each object in turn
	// even the world gets a chance to think
	//
	ent = &g_edicts[0];
	for (i=0 ; i<globals.num_edicts ; i++, ent++)
	{
		if (!ent->inuse)
			continue;

		level.current_entity = ent;

		VectorCopy (ent->s.origin, ent->s.old_origin);

		// if the ground entity moved, make sure we are still on it
		if ((ent->groundentity) && (ent->groundentity->linkcount != ent->groundentity_linkcount))
		{
			ent->groundentity = NULL;
#ifdef MONSTERS_OK
			if ( !(ent->flags & (FL_SWIM|FL_FLY)) && (ent->svflags & SVF_MONSTER) )
			{
				M_CheckGround (ent);
			}
#endif
		}

		if (i > 0 && i <= maxclients->value)
		{
			ClientBeginServerFrame (ent);
			continue;
		}

		G_RunEntity (ent);
	}

	// see if it is time to end a deathmatch
	CheckDMRules ();

	// build the playerstate_t structures for all players
	ClientEndServerFrames ();
}

