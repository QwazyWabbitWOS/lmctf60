
#include "g_local.h"

#include "stdlog.h"	//	StdLog - Mark Davies
#include "gslog.h"	//	StdLog - Mark Davies
#include "g_skins.h"
#include "g_ctffunc.h" //surt for log renaming
#include "bat.h"
#if defined(_WIN32) && !(defined(__MINGW32__) || defined(__MINGW64__))
  #include "GitRevisionInfo.h" // Derived from template via GitWCRev
#endif // _WIN32

#define Function(f) {#f, f}

void Randomize_Map_List(int Num_Of_Maps);

mmove_t mmove_reloc;

field_t fields[] = {
	{"classname", FOFS(classname), F_LSTRING},
	{"model", FOFS(model), F_LSTRING},
	{"spawnflags", FOFS(spawnflags), F_INT},
	{"speed", FOFS(speed), F_FLOAT},
	{"accel", FOFS(accel), F_FLOAT},
	{"decel", FOFS(decel), F_FLOAT},
	{"target", FOFS(target), F_LSTRING},
	{"targetname", FOFS(targetname), F_LSTRING},
	{"pathtarget", FOFS(pathtarget), F_LSTRING},
	{"deathtarget", FOFS(deathtarget), F_LSTRING},
	{"killtarget", FOFS(killtarget), F_LSTRING},
	{"combattarget", FOFS(combattarget), F_LSTRING},
	{"message", FOFS(message), F_LSTRING},
	{"team", FOFS(team), F_LSTRING},
	{"wait", FOFS(wait), F_FLOAT},
	{"delay", FOFS(delay), F_FLOAT},
	{"random", FOFS(random), F_FLOAT},
	{"move_origin", FOFS(move_origin), F_VECTOR},
	{"move_angles", FOFS(move_angles), F_VECTOR},
	{"style", FOFS(style), F_INT},
	{"count", FOFS(count), F_INT},
	{"health", FOFS(health), F_INT},
	{"sounds", FOFS(sounds), F_INT},
	{"light", 0, F_IGNORE},
	{"dmg", FOFS(dmg), F_INT},
	{"mass", FOFS(mass), F_INT},
	{"volume", FOFS(volume), F_FLOAT},
	{"attenuation", FOFS(attenuation), F_FLOAT},
	{"map", FOFS(map), F_LSTRING},
	{"origin", FOFS(s.origin), F_VECTOR},
	{"angles", FOFS(s.angles), F_VECTOR},
	{"angle", FOFS(s.angles), F_ANGLEHACK},

	{"goalentity", FOFS(goalentity), F_EDICT, FFL_NOSPAWN},
	{"movetarget", FOFS(movetarget), F_EDICT, FFL_NOSPAWN},
	{"enemy", FOFS(enemy), F_EDICT, FFL_NOSPAWN},
	{"oldenemy", FOFS(oldenemy), F_EDICT, FFL_NOSPAWN},
	{"activator", FOFS(activator), F_EDICT, FFL_NOSPAWN},
	{"groundentity", FOFS(groundentity), F_EDICT, FFL_NOSPAWN},
	{"teamchain", FOFS(teamchain), F_EDICT, FFL_NOSPAWN},
	{"teammaster", FOFS(teammaster), F_EDICT, FFL_NOSPAWN},
	{"owner", FOFS(owner), F_EDICT, FFL_NOSPAWN},
	{"mynoise", FOFS(mynoise), F_EDICT, FFL_NOSPAWN},
	{"mynoise2", FOFS(mynoise2), F_EDICT, FFL_NOSPAWN},
	{"target_ent", FOFS(target_ent), F_EDICT, FFL_NOSPAWN},
	{"chain", FOFS(chain), F_EDICT, FFL_NOSPAWN},

	{"prethink", FOFS(prethink), F_FUNCTION, FFL_NOSPAWN},
	{"think", FOFS(think), F_FUNCTION, FFL_NOSPAWN},
	{"blocked", FOFS(blocked), F_FUNCTION, FFL_NOSPAWN},
	{"touch", FOFS(touch), F_FUNCTION, FFL_NOSPAWN},
	{"use", FOFS(use), F_FUNCTION, FFL_NOSPAWN},
	{"pain", FOFS(pain), F_FUNCTION, FFL_NOSPAWN},
	{"die", FOFS(die), F_FUNCTION, FFL_NOSPAWN},

	{"stand", FOFS(monsterinfo.stand), F_FUNCTION, FFL_NOSPAWN},
	{"idle", FOFS(monsterinfo.idle), F_FUNCTION, FFL_NOSPAWN},
	{"search", FOFS(monsterinfo.search), F_FUNCTION, FFL_NOSPAWN},
	{"walk", FOFS(monsterinfo.walk), F_FUNCTION, FFL_NOSPAWN},
	{"run", FOFS(monsterinfo.run), F_FUNCTION, FFL_NOSPAWN},
	{"dodge", FOFS(monsterinfo.dodge), F_FUNCTION, FFL_NOSPAWN},
	{"attack", FOFS(monsterinfo.attack), F_FUNCTION, FFL_NOSPAWN},
	{"melee", FOFS(monsterinfo.melee), F_FUNCTION, FFL_NOSPAWN},
	{"sight", FOFS(monsterinfo.sight), F_FUNCTION, FFL_NOSPAWN},
	{"checkattack", FOFS(monsterinfo.checkattack), F_FUNCTION, FFL_NOSPAWN},
	{"currentmove", FOFS(monsterinfo.currentmove), F_MMOVE, FFL_NOSPAWN},

	{"endfunc", FOFS(moveinfo.endfunc), F_FUNCTION, FFL_NOSPAWN},

	// temp spawn vars -- only valid when the spawn function is called
	{"lip", STOFS(lip), F_INT, FFL_SPAWNTEMP},
	{"distance", STOFS(distance), F_INT, FFL_SPAWNTEMP},
	{"height", STOFS(height), F_INT, FFL_SPAWNTEMP},
	{"noise", STOFS(noise), F_LSTRING, FFL_SPAWNTEMP},
	{"pausetime", STOFS(pausetime), F_FLOAT, FFL_SPAWNTEMP},
	{"item", STOFS(item), F_LSTRING, FFL_SPAWNTEMP},

	//need for item field in edict struct, FFL_SPAWNTEMP item will be skipped on saves
	{"item", FOFS(item), F_ITEM},

	{"gravity", STOFS(gravity), F_LSTRING, FFL_SPAWNTEMP},
	{"sky", STOFS(sky), F_LSTRING, FFL_SPAWNTEMP},
	{"skyrotate", STOFS(skyrotate), F_FLOAT, FFL_SPAWNTEMP},
	{"skyaxis", STOFS(skyaxis), F_VECTOR, FFL_SPAWNTEMP},
	{"minyaw", STOFS(minyaw), F_FLOAT, FFL_SPAWNTEMP},
	{"maxyaw", STOFS(maxyaw), F_FLOAT, FFL_SPAWNTEMP},
	{"minpitch", STOFS(minpitch), F_FLOAT, FFL_SPAWNTEMP},
	{"maxpitch", STOFS(maxpitch), F_FLOAT, FFL_SPAWNTEMP},
	{"nextmap", STOFS(nextmap), F_LSTRING, FFL_SPAWNTEMP},

	{0, 0, 0, 0}

};

field_t		levelfields[] =
{
	{"changemap", LLOFS(changemap), F_LSTRING},

	{"sight_client", LLOFS(sight_client), F_EDICT},
	{"sight_entity", LLOFS(sight_entity), F_EDICT},
	{"sound_entity", LLOFS(sound_entity), F_EDICT},
	{"sound2_entity", LLOFS(sound2_entity), F_EDICT},

	{NULL, 0, F_INT}
};

field_t		clientfields[] =
{
	{"pers.weapon", CLOFS(pers.weapon), F_ITEM},
	{"pers.lastweapon", CLOFS(pers.lastweapon), F_ITEM},
	{"newweapon", CLOFS(newweapon), F_ITEM},

	{NULL, 0, F_INT}
};

/*
============
InitGame

This will be called when the dll is first loaded, which
only happens when a new game is started or a save game
is loaded.
============
*/
void InitGame(void)
{

#ifdef	_WIN32
	_CrtMemCheckpoint(&startup1);
#endif
	// CTF CODE -- LM_JORM
	FILE* file;
	char		line[MAX_INFO_STRING];
	char		fname[MAX_QPATH];
	size_t		i;

	// END CTF CODE -- LM_JORM

	gi.dprintf("==== InitGame %s %d-%s %s ====\n", GAMEVERSION, LMCTF_REVISION, LMCTF_VERSION, __DATE__);

	// seed the random number generator
	srand((unsigned)time(NULL));

	gun_x = gi.cvar("gun_x", "0", 0);
	gun_y = gi.cvar("gun_y", "0", 0);
	gun_z = gi.cvar("gun_z", "0", 0);

	//FIXME: sv_ prefix is wrong for these
	sv_rollspeed = gi.cvar("sv_rollspeed", "200", 0);
	sv_rollangle = gi.cvar("sv_rollangle", "2", 0);
	sv_maxvelocity = gi.cvar("sv_maxvelocity", "2000", 0);
	sv_gravity = gi.cvar("sv_gravity", "800", 0);

	// noset vars
	dedicated = gi.cvar("dedicated", "0", CVAR_NOSET);

	gi.cvar("revision", va("%d", LMCTF_REVISION), CVAR_SERVERINFO);
	gi.cvar_set("revision", va("%d-%s", LMCTF_REVISION, LMCTF_VERSION));

	// latched vars
	sv_cheats = gi.cvar("cheats", "0", CVAR_SERVERINFO | CVAR_LATCH);
	gi.cvar("gamename", GAMEVERSION, CVAR_SERVERINFO | CVAR_LATCH);
	gi.cvar("gamedate", __DATE__, CVAR_SERVERINFO | CVAR_LATCH);

	maxclients = gi.cvar("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
	//maxspectators = gi.cvar ("maxspectators", "4", CVAR_SERVERINFO);
	//-bat
	maxspectators = gi.cvar("maxspectators", "24", CVAR_SERVERINFO);
	deathmatch = gi.cvar("deathmatch", "0", CVAR_LATCH);
	coop = gi.cvar("coop", "0", CVAR_LATCH);
	skill = gi.cvar("skill", "1", CVAR_LATCH);
	maxentities = gi.cvar("maxentities", "1024", CVAR_LATCH);

	// change anytime vars
	dmflags = gi.cvar("dmflags", "0", CVAR_SERVERINFO);
	fraglimit = gi.cvar("fraglimit", "0", CVAR_SERVERINFO);
	timelimit = gi.cvar("timelimit", "0", CVAR_SERVERINFO);
	railtime = gi.cvar("railtime", "0", CVAR_SERVERINFO);
	password = gi.cvar("password", "", CVAR_USERINFO);
	spectator_password = gi.cvar("spectator_password", "", CVAR_USERINFO);
	needpass = gi.cvar("needpass", "0", CVAR_SERVERINFO);
	filterban = gi.cvar("filterban", "1", 0);

	g_select_empty = gi.cvar("g_select_empty", "0", CVAR_ARCHIVE);

	run_pitch = gi.cvar("run_pitch", "0.002", 0);
	run_roll = gi.cvar("run_roll", "0.005", 0);
	bob_up = gi.cvar("bob_up", "0.005", 0);
	bob_pitch = gi.cvar("bob_pitch", "0.002", 0);
	bob_roll = gi.cvar("bob_roll", "0.002", 0);

	runes = gi.cvar("runes", "15", CVAR_SERVERINFO); // CTF CODE -- LM_CTF
	ctfflags = gi.cvar("ctfflags", "0", CVAR_SERVERINFO); // CTF CODE -- LM_CTF
	refset = gi.cvar("refset", "0", CVAR_SERVERINFO); // CTF CODE -- LM_CTF
	logrename = gi.cvar("logrename", "", 0); // CTF CODE -- LM_CTF
	hostname = gi.cvar("hostname", "noname", CVAR_SERVERINFO); // CTF CODE -- LM_CTF
	gamedir = gi.cvar("game", "lmctf", CVAR_SERVERINFO); // CTF CODE -- LM_CTF
	skinset = gi.cvar("skinset", "0", CVAR_SERVERINFO); // CTF CODE -- LM_CTF
#ifdef OLDOBSERVERCODE
	autoobserve = gi.cvar("autoobserve", "0", CVAR_USERINFO); // CTF CODE -- LM_CTF
#endif
	refpassword = gi.cvar("refpassword", "", 0); // CTF CODE -- LM_CTF
	rconpassword = gi.cvar("rcon_password", "", 0); // CTF CODE -- LM_CTF
	disabled_weps = gi.cvar("disabled_weps", "0", 0);
	fastswitch = gi.cvar("fastswitch", "0", 0);
	mod_website = gi.cvar("mod_website", "http://lmctf.com", 0);
	autolock = gi.cvar("autolock", "0", 0);
	countdown_time = gi.cvar("countdown_time", "15", 0);


#ifdef ZBOT
	use_zbotdetect = gi.cvar("use_zbotdetect", "0", CVAR_SERVERINFO);
#endif

	//configuration files
	motd_file = gi.cvar("motd_file", "motd.txt", 0); //CTF CODE -- LM_CTF
	server_file = gi.cvar("server_file", "server.cfg", 0); //CTF CODE -- LM_CTF
	maplist_file = gi.cvar("maplist_file", "maplist.txt", 0);  //CTF CODE -- LM_CTF
	skin_file = gi.cvar("skin_file", "skins.ini", 0);
	skin_debug = gi.cvar("skin_debug", "0", 0);	// for debugging team skins in SkinsReadFile
	flag_init = gi.cvar("flag_init", "0", 0);	// flag spawning frame initialization.



	//logging startup after vars initialized
	ctf_SetLogName();
	sl_Logging(&gi, GAMEVERSION);	// StdLog - Mark Davies (Only required to set patch name)

	// dm map list
	sv_maplist = gi.cvar("sv_maplist", "", 0);

	// items
	InitItems();

	Com_sprintf(game.helpmessage1, sizeof(game.helpmessage1), "");

	Com_sprintf(game.helpmessage2, sizeof(game.helpmessage2), "");

	// initialize all entities for this game
	game.maxentities = maxentities->value;
	g_edicts = gi.TagMalloc(game.maxentities * sizeof(g_edicts[0]), TAG_GAME);
	globals.edicts = g_edicts;
	globals.max_edicts = game.maxentities;

	// initialize all clients for this game
	game.maxclients = maxclients->value;
	game.clients = gi.TagMalloc(game.maxclients * sizeof(game.clients[0]), TAG_GAME);
	globals.num_edicts = game.maxclients + 1;


	// CTF CODE -- LM_JORM
	if (!maplistindex) // Have we done this before?
	{
		//		sprintf (fname, "%s/maplist.txt", gamedir->string);
		sprintf(fname, "%s/%s", gamedir->string, maplist_file->string);
		file = fopen(fname, "r");
		if (!file)
			file = fopen("maplist.txt", "r");

		if (file)
		{
			// If there is a maplist file, read it
			//   And use it for our list of maps.

			maplistindex = 0;
			while (fgets(line, 255, file))
			{
				char* tempname = gi.TagMalloc(1000, TAG_GAME);
				int tempmin = 0;
				int tempmax = 99;
				if (sscanf(line, "%s %d %d", tempname, &tempmin, &tempmax) != 3) {
					if (sscanf(line, "%s %d", tempname, &tempmin) != 2) {
						if (sscanf(line, "%s", tempname) != 1) {
							sprintf(line, "Bad entry in maplist");
							gi.dprintf(line);
							gi.TagFree(tempname);
							continue;
						}
					}
				}
				//, maplist[maplistindex]))
				// convert to lower case for subsequent comparisons
				for (i = 0; i < strlen(tempname); i++)
					tempname[i] = tolower(tempname[i]);
				maplist[maplistindex].mapname = tempname;
				maplist[maplistindex].minplayers = tempmin;
				maplist[maplistindex].maxplayers = tempmax;

				maplistindex++;

			}
			SortMaplist(maplist, 0, maplistindex - 1);
			//maplist[maplistindex][0] = 0; // Blank last entry
			sprintf(line, "%d entries in maplist.\n", maplistindex);
			gi.dprintf(line);
			fclose(file);
			if (maplistindex)  // Did we read anything?
			{
				Randomize_Map_List(maplistindex);
				maplistindex = -1; // This means first time through the list
			}
			else
				maplistindex = -2; // Not going to use it

		}
		else
		{
			sprintf(line, "Can't find %s.  Reverting to standard maps.\n", fname);
			gi.dprintf(line);
			maplistindex = -2; // Not going to use it
		}
	}


	// Reading the help text file

	sprintf(fname, "%s/help.txt", gamedir->string);
	file = fopen(fname, "r");
	if (!file)
		file = fopen("help.txt", "r");

	if (file)
	{
		// If there is a help file, read it

		i = 0;
		while (fgets(line, 255, file))
		{
			// Nul termination is guaranteed from fgets.
			memcpy(helptext[i], line, 24);
			i++;
		}
		helptext[i][0] = 0; // Blank last entry
		fclose(file);
		sprintf(line, "Read %zu lines of %s.\n", i, fname);
		gi.dprintf(line);
	}
	else
	{
		sprintf(line, "Can't find %s.  You get no help!\n", fname);
		gi.dprintf(line);
		strcpy(helptext[0], "No help available."); // Not going to use it
		helptext[1][0] = 0;
	}

	if (motd_file && strcmp(motd_file->string, "") != 0)
		sprintf(fname, "%s/%s", gamedir->string, motd_file->string);
	else
		sprintf(fname, "%s/motd.txt", gamedir->string);
	file = fopen(fname, "r");
	if (!file)
		file = fopen("motd.txt", "r");
	if (file)
	{
		size_t count = fread(motd, sizeof motd, 1, file);
		if (count)
			; // don't worry, be happy
		fclose(file);
	}

	SkinsReadFile(); // READ our skins.ini


	// END CTF CODE -- LM_JORM

}

void SortMaplist(MapInfo arr[], int min, int max) {
	if (min < max) {
		int ndx = MapDivide(arr, min, max);
		SortMaplist(arr, min, ndx - 1);
		SortMaplist(arr, ndx + 1, max);
	}
}


int MapDivide(MapInfo arr[], int min, int max) {
	MapInfo tmp = arr[max];
	int nndx = (min - 1);

	for (int x = min; x <= max - 1; x++) {
		if (strcmp(arr[x].mapname, tmp.mapname) < 0) {
			nndx++;
			flip(&arr[nndx], &arr[x]);
		}
	}
	flip(&arr[nndx + 1], &arr[max]);
	return(nndx + 1);
}

void flip(MapInfo* x, MapInfo* y) {
	MapInfo tmp = *x;
	*x = *y;
	*y = tmp;
}



//=========================================================

void WriteField1(FILE* f, field_t* field, byte* base)
{
	void* p;
	size_t		len;
	int			index;

	if (field->flags & FFL_SPAWNTEMP)
		return;

	p = (void*)(base + field->ofs);
	switch (field->type)
	{
	case F_INT:
	case F_FLOAT:
	case F_ANGLEHACK:
	case F_VECTOR:
	case F_IGNORE:
		break;

	case F_LSTRING:
	case F_GSTRING:
		if (*(char**)p)
			len = strlen(*(char**)p) + 1;
		else
			len = 0;
		*(size_t*)p = len;
		break;
	case F_EDICT:
		if (*(edict_t**)p == NULL)
			index = -1;
		else
			index = *(edict_t**)p - g_edicts;
		*(int*)p = index;
		break;
	case F_CLIENT:
		if (*(gclient_t**)p == NULL)
			index = -1;
		else
			index = *(gclient_t**)p - game.clients;
		*(int*)p = index;
		break;
	case F_ITEM:
		if (*(edict_t**)p == NULL)
			index = -1;
		else
			index = *(gitem_t**)p - itemlist;
		*(int*)p = index;
		break;

		//relative to code segment
	case F_FUNCTION:
		if (*(byte**)p == NULL)
			index = 0;
		else
			index = *(byte**)p - ((byte*)InitGame);
		*(int*)p = index;
		break;

		//relative to data segment
	case F_MMOVE:
		if (*(byte**)p == NULL)
			index = 0;
		else
			index = *(byte**)p - (byte*)&mmove_reloc;
		*(int*)p = index;
		break;

	default:
		gi.error("WriteEdict: unknown field type");
	}
}


void WriteField2(FILE* f, field_t* field, byte* base)
{
	size_t		len;
	void* p;

	if (field->flags & FFL_SPAWNTEMP)
		return;

	p = (void*)(base + field->ofs);
	switch (field->type)
	{
	case F_LSTRING:
		if (*(char**)p)
		{
			len = strlen(*(char**)p) + 1;
			fwrite(*(char**)p, len, 1, f);
		}
		break;
	default:// MJD Added because the compile FREAKS, here
		break;
	}
}

void ReadField(FILE* f, field_t* field, byte* base)
{
	void* p;
	int			len;
	int			index;

	if (field->flags & FFL_SPAWNTEMP)
		return;

	p = (void*)(base + field->ofs);
	switch (field->type)
	{
	case F_INT:
	case F_FLOAT:
	case F_ANGLEHACK:
	case F_VECTOR:
	case F_IGNORE:
		break;

	case F_LSTRING:
		len = *(int*)p;
		if (!len)
			*(char**)p = NULL;
		else
		{
			*(char**)p = gi.TagMalloc(len, TAG_LEVEL);
			size_t count = fread(*(char**)p, len, 1, f);
			if (count)
				; // don't worry, be happy
		}
		break;
	case F_EDICT:
		index = *(int*)p;
		if (index == -1)
			*(edict_t**)p = NULL;
		else
			*(edict_t**)p = &g_edicts[index];
		break;
	case F_CLIENT:
		index = *(int*)p;
		if (index == -1)
			*(gclient_t**)p = NULL;
		else
			*(gclient_t**)p = &game.clients[index];
		break;
	case F_ITEM:
		index = *(int*)p;
		if (index == -1)
			*(gitem_t**)p = NULL;
		else
			*(gitem_t**)p = &itemlist[index];
		break;

		//relative to code segment
	case F_FUNCTION:
		index = *(int*)p;
		if (index == 0)
			*(byte**)p = NULL;
		else
			*(byte**)p = ((byte*)InitGame) + index;
		break;

		//relative to data segment
	case F_MMOVE:
		index = *(int*)p;
		if (index == 0)
			*(byte**)p = NULL;
		else
			*(byte**)p = (byte*)&mmove_reloc + index;
		break;

	default:
		gi.error("ReadEdict: unknown field type");
	}
}

//=========================================================


static gclient_t	temp;	// For WriteClient

/*
==============
WriteClient

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteClient(FILE* f, gclient_t* client)
{
	field_t* field;

	// all of the ints, floats, and vectors stay as they are
	temp = *client;

	// change the pointers to lengths or indexes
	for (field = clientfields; field->name; field++)
	{
		WriteField1(f, field, (byte*)&temp);
	}

	// write the block
	fwrite(&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field = clientfields; field->name; field++)
	{
		WriteField2(f, field, (byte*)client);
	}
}

/*
==============
ReadClient

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadClient(FILE* f, gclient_t* client)
{
	field_t* field;
	size_t	count;

	count = fread(client, sizeof(*client), 1, f);
	if (count)
		; // don't worry, be happy

	for (field = clientfields; field->name; field++)
	{
		ReadField(f, field, (byte*)client);
	}
}

/*
============
WriteGame

This will be called whenever the game goes to a new level,
and when the user explicitly saves the game.

Game information include cross level data, like multi level
triggers, help computer info, and all client states.

A single player death will automatically restore from the
last save position.
============
*/
void WriteGame(char* filename, qboolean autosave)
{
	FILE* f;
	int		i;
	char	str[MAX_INFO_STRING];

	if (!autosave)
		SaveClientData();

	f = fopen(filename, "wb");
	if (!f)
	{
		gi.error("Couldn't open %s", filename);
		return;
	}

	memset(str, 0, sizeof(str));
	strcpy(str, __DATE__);
	fwrite(str, sizeof(str), 1, f);

	game.autosaved = autosave;
	fwrite(&game, sizeof(game), 1, f);
	game.autosaved = false;

	for (i = 0; i < game.maxclients; i++)
		WriteClient(f, &game.clients[i]);

	fclose(f);
}

void ReadGame(char* filename)
{
	FILE* f;
	int		i;
	char	str[MAX_INFO_STRING] = { 0 };
	size_t	count;

	gi.FreeTags(TAG_GAME);

	f = fopen(filename, "rb");
	if (!f)
	{
		gi.error("Couldn't open %s", filename);
		return;
	}

	count = fread(str, sizeof(str), 1, f);
	if (count)
		; // don't worry, be happy
	if (strcmp(str, __DATE__))
	{
		fclose(f);
		gi.error("Savegame from an older version.\n");
	}

	g_edicts = gi.TagMalloc(game.maxentities * sizeof(g_edicts[0]), TAG_GAME);
	globals.edicts = g_edicts;

	count = fread(&game, sizeof(game), 1, f);
	game.clients = gi.TagMalloc(game.maxclients * sizeof(game.clients[0]), TAG_GAME);
	for (i = 0; i < game.maxclients; i++)
		ReadClient(f, &game.clients[i]);

	fclose(f);
}

//==========================================================


/*
==============
WriteEdict

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteEdict(FILE* f, edict_t* ent)
{
	field_t* field;
	edict_t		temp;

	// all of the ints, floats, and vectors stay as they are
	temp = *ent;

	// change the pointers to lengths or indexes
	for (field = fields; field->name; field++)
	{
		WriteField1(f, field, (byte*)&temp);
	}

	// write the block
	fwrite(&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field = fields; field->name; field++)
	{
		WriteField2(f, field, (byte*)ent);
	}

}

/*
==============
WriteLevelLocals

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteLevelLocals(FILE* f)
{
	field_t* field;
	level_locals_t		temp;

	// all of the ints, floats, and vectors stay as they are
	temp = level;

	// change the pointers to lengths or indexes
	for (field = levelfields; field->name; field++)
	{
		WriteField1(f, field, (byte*)&temp);
	}

	// write the block
	fwrite(&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field = levelfields; field->name; field++)
	{
		WriteField2(f, field, (byte*)&level);
	}
}


/*
==============
ReadEdict

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadEdict(FILE* f, edict_t* ent)
{
	field_t* field;
	size_t	count;

	count = fread(ent, sizeof(*ent), 1, f);
	if (count)
		; // don't worry, be happy

	for (field = fields; field->name; field++)
	{
		ReadField(f, field, (byte*)ent);
	}
}

/*
==============
ReadLevelLocals

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadLevelLocals(FILE* f)
{
	field_t* field;
	size_t	count;

	count = fread(&level, sizeof(level), 1, f);
	if (count)
		; // don't worry, be happy

	for (field = levelfields; field->name; field++)
	{
		ReadField(f, field, (byte*)&level);
	}
}

/*
=================
WriteLevel

=================
*/
void WriteLevel(char* filename)
{
	int		i;
	edict_t* ent;
	FILE* f;
	void	(*base)(void);	/* Pointer to function with no arguments */

	f = fopen(filename, "wb");
	if (!f)
	{
		gi.error("Couldn't open %s", filename);
		return;
	}

	// write out edict size for checking
	i = sizeof(edict_t);
	fwrite(&i, sizeof(i), 1, f);

	// write out a function pointer for checking
	base = InitGame;
	fwrite(&base, sizeof(base), 1, f);

	// write out level_locals_t
	WriteLevelLocals(f);

	// write out all the entities
	for (i = 0; i < globals.num_edicts; i++)
	{
		ent = &g_edicts[i];
		if (!ent->inuse)
			continue;
		fwrite(&i, sizeof(i), 1, f);
		WriteEdict(f, ent);
	}
	i = -1;
	fwrite(&i, sizeof(i), 1, f);

	fclose(f);
}


/*
=================
ReadLevel

SpawnEntities will already have been called on the
level the same way it was when the level was saved.

That is necessary to get the baselines
set up identically.

The server will have cleared all of the world links before
calling ReadLevel.

No clients are connected yet.
=================
*/
void ReadLevel(char* filename)
{
	int		entnum;
	FILE* f;
	int		i;
	void* base;
	edict_t* ent;
	size_t	count;

	f = fopen(filename, "rb");
	if (!f)
		gi.error("Couldn't open %s", filename);

	// free any dynamic memory allocated by loading the level
	// base state
	gi.FreeTags(TAG_LEVEL);

	// wipe all the entities
	memset(g_edicts, 0, game.maxentities * sizeof(g_edicts[0]));
	globals.num_edicts = maxclients->value + 1;

	// check edict size
	count = fread(&i, sizeof(i), 1, f);
	if (i != sizeof(edict_t))
	{
		fclose(f);
		gi.error("ReadLevel: mismatched edict size");
		return; //never reached
	}

	// check function pointer base address
	count = fread(&base, sizeof(base), 1, f);
#ifdef _WIN32
	if (base != (void*)InitGame)
	{
		fclose(f);
		gi.error("ReadLevel: function pointers have moved");
		return;
	}
#else
	gi.dprintf("Function offsets %d\n", ((byte*)base) - ((byte*)InitGame));
#endif

	// load the level locals
	ReadLevelLocals(f);

	// load all the entities
	while (1)
	{
		count = fread(&entnum, sizeof(entnum), 1, f);
		if (count != 1)
		{
			fclose(f);
			gi.error("ReadLevel: failed to read entnum");
		}
		if (entnum == -1)
			break;
		if (entnum >= globals.num_edicts)
			globals.num_edicts = entnum + 1;

		ent = &g_edicts[entnum];
		ReadEdict(f, ent);

		// let the server rebuild world links for this ent
		memset(&ent->area, 0, sizeof(ent->area));
		gi.linkentity(ent);
	}

	fclose(f);

	// mark all clients as unconnected
	for (i = 0; i < maxclients->value; i++)
	{
		ent = &g_edicts[i + 1];
		ent->client = game.clients + i;
		ent->client->pers.connected = false;
	}

	// do any load time things at this point
	for (i = 0; i < globals.num_edicts; i++)
	{
		ent = &g_edicts[i];

		if (!ent->inuse)
			continue;

		// fire any cross-level triggers
		if (ent->classname)
			if (strcmp(ent->classname, "target_crosslevel_target") == 0)
				ent->nextthink = level.time + ent->delay;
	}
}
