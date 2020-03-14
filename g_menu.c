#include "g_local.h"
#include "g_menu.h"
#include "g_ctffunc.h" //surt for some nice wrapper functions
#include "g_tourney.h" // Needed for our referee menus
#include "g_skins.h"
#include "bat.h"
#include "g_vote.h"    //Vampire -- voting menu

void Cmd_Team_f (edict_t *ent);
void Team_Change(edict_t *ent, int newnum);
void ChaseCam (edict_t *ent);
void Menu_Blank ();
void Observe (edict_t *ent);
void f(edict_t *ent);
void Show_String(int x, int y, char *string, char *Text);

void Help_Menu (edict_t *ent);
void HowToPlay_Menu (edict_t *ent);
//void Command_Menu (edict_t *ent);
void Radio_Menu (edict_t *ent);
void PickSound (edict_t *ent);

void Voice_Menu (edict_t *ent);
void Voice_Exec (edict_t *ent);

// Old skin menus
void SetOldSkin (edict_t *ent);
void Skin_Old_Menu (edict_t *ent);

#ifdef OLDOBSERVERCODE
//observer stuff
void Obs_Main_Menu (edict_t *ent);
void Obs_CamLock_Exec (edict_t *ent);
void Obs_Reverse_Exec (edict_t *ent);
#else
void Obs_Main_Menu (edict_t *ent);
#endif

//referee & rcon stuff
void Ref_Main_Menu (edict_t *ent);
void Ref_PingFloor_Menu (edict_t *ent);
void PingFloor_Exec (edict_t *ent);
void Ref_PingCeiling_Menu (edict_t *ent);
void PingCeiling_Exec (edict_t *ent);
void Ref_Settings_Menu (edict_t *ent);
void Ref_DMFlags_Menu (edict_t *ent);
void Ref_CTFFlags_Menu (edict_t *ent);
void Ref_Timelimit_Menu (edict_t *ent);
void Ref_Fraglimit_Menu (edict_t *ent);
void Ref_Match_Menu (edict_t *ent);
void Ref_End_Match (edict_t *ent);
void SaveServer_Exec(edict_t *ent);
void Ref_Help_Menu(edict_t *ent);
void Ref_Practice_Menu(edict_t *ent);
void Ref_PracticeFlagRed_Exec(edict_t *ent);
void Ref_PracticeFlagBlue_Exec(edict_t *ent);
void ClearPassword_Exec (edict_t *ent);
void Ref_Kick_Menu (edict_t *ent);
void RefTogglePause(edict_t *ent);
void Ref_Map_Menu (edict_t *ent);
void Ref_Match_A_Menu (edict_t *ent);
void Ref_Match_B_Menu (edict_t *ent);
void Ref_Match_C_Menu (edict_t *ent);
void Ref_Map_A_Menu (edict_t *ent);
void Ref_Map_B_Menu (edict_t *ent);
void Ref_Map_C_Menu (edict_t *ent);
void Ref_Match_Maplist_Menu (edict_t *ent);
void Ref_Map_Maplist_Menu (edict_t *ent);

extern void Observer_Stop (edict_t *ent);
extern void Observer_Start (edict_t *ent);

//player menu support functions
void Change_Team_Exec(edict_t *ent);
void Observe_Exec(edict_t *ent);
void Cmd_Observe_f(edict_t *ent, int Observer_Type);


menuitem mainmenu[] =
{
	{"LM CTF Option Menu", 0},
	{"------------------", 0},
	{"", 0 },
//-bat
//put back in
//#ifdef OLDOBSERVERCODE
	{ "Become Observer", Observe },
//#endif
	{ "Change Team", Cmd_Team_f },
	{ "Change Skin", Skin_Menu },
	{ "Vote menu", Vote_Menu },          //Vampire - voting menu
	{ "", 0 },
	{ "Help", Help_Menu },
};

menuitem skinmenu[] =
{
	{ "Male", 0 },
	{ "     Skin 1", SetOldSkin },
	{ "     Skin 2", SetOldSkin },
	{ "     Skin 3", SetOldSkin },
	{ "", 0 },
	{ "Female", 0 },
	{ "     Skin 1", SetOldSkin },
	{ "     Skin 2", SetOldSkin },
};

menuitem helpmenu[] =
{
	{"LM CTF Help Menu", 0},
	{"------------------", 0},
	{"", 0 },
	{ "How to Play", HowToPlay_Menu },
	{ "Commands", NULL }, //Command_Menu },
	{ "Radio Sounds", Radio_Menu },
	{ "", 0 },
	{ "Main Menu", Main_Menu },
};

menuitem howtoplaymenu[] =
{
	{"How to Play", 0},
	{"------------", 0},
	{"Two teams, red and blue, ", 0 },
	{"each attempt to steal the", 0 },
	{"opposing team's flag and ", 0 },
	{"return it to their own   ", 0 },
	{"base, where they must    ", 0 },
	{"touch it to their own    ", 0 },
	{"flag.  If their own flag ", 0 },
	{"is taken, they must kill ", 0 },
	{"the enemy flag carrier,  ", 0 },
	{"and touch their flag to  ", 0 },
	{"return it home.", 0 },
	{ "", 0 },
	{ "Help Menu", Help_Menu },
};

menuitem commandmenu[] =
{
	{"LM CTF Command List", 0},
	{"-------------------", 0},
	{"ctfmenu", f },
	{"play_team <sound>", f },
	{"radio <off/text/on/both>", f },
	{"team <red/blue>", f },
	{"flagstatus", f },
	{"+hook", f },
	{"-hook", f },
//-bat
//put back in
//#ifdef OLDOBSERVERCODE
	{"observer", f },
//#endif
	{"chasecam", f },
	{"radiomenu", f },
	{ "", 0 },
	{ "Help Menu", Help_Menu },
};

menuitem radiomenu[] =
{
	{"attack", PickSound },
	{"attack10", PickSound },
	{"capit", PickSound },
	{"clear", PickSound },
	{"defense", PickSound },
	{"escort", PickSound },
	{"incoming", PickSound },
	{"overrun", PickSound },
	{"q60", PickSound },	
	{"qattack", PickSound },
	{"quad", PickSound },
	{"qwaiting", PickSound },
	{"ready", PickSound },
	{"recover", PickSound },
	{"regroup", PickSound },
	{"roger", PickSound },
	{"status", PickSound },
	{"work", PickSound },
};



menudata menulist[] =
{
	{ NULL, 0 }, // LOCAL MENU -- Should not be used
	{ mainmenu, 9 },     //Vampire -- voting menu - increase from 8 to 9 to include the new menuitem
	{ skinmenu, 8 },
	{ helpmenu, 8 },
	{ howtoplaymenu, 15},
	{ commandmenu, 14 },
	{ radiomenu, 18 },
};

/*
=================
CTF Menus
=================
*/

char *skin[] = 
{
	"",
	"male/rb-rm1",
	"male/rb-rm2",
	"male/rb-rm3",
	"",
	"",
	"female/rb-rf1",
	"female/rb-rf2",
};

char *radiosound[] =
{
	"attack",
	"attack10",
	"capit",
	"clear",
	"defense",
	"escort",
	"incoming",
	"overrun",
	"q60",
	"qattack",
	"quad",
	"qwaiting",
	"ready",
	"recover",
	"regroup",
	"roger",
	"status",
	"work",
};



void Ctf_Menu (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	cl->showscores = false;
	cl->showhelp = false;
	cl->showinventory = false;
	cl->showmod = false;
	cl->showctfhud = false;
	cl->showsquadboard = false; // ADC

	if (cl->showmenu)
	{
		cl->showmenu = false;
		Menu_Blank();
		gi.unicast (ent, true);
		return;
	}

	cl->showmenu = true;
	Main_Menu(ent);
}


void Main_Menu (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	Menu_Free(ent);
	ent->client->menu = MENU_LOCAL;
	ent->client->menuselect = 1;

	Menu_Set(ent, 0, "LM CTF Option Menu", Help_Menu);
	Menu_Set(ent, 1, "------------------", NULL);
//-bat
//put back in
//#ifdef OLDOBSERVERCODE
	//if (ctf_validateplayer(ent, CTF_TEAM_OBSERVER))
	if(ent->client->ctf.teamnum ==  CTF_TEAM_OBSERVER ||
		ent->client->ctf.teamnum ==  CTF_TEAM_OBSERVER_RED ||
		ent->client->ctf.teamnum ==  CTF_TEAM_OBSERVER_BLUE)
			Menu_Set(ent, 3, "Observer Options", Obs_Main_Menu);
	else
	{
		Menu_Set(ent, 3, "Become Observer", Observe_Exec);
		Menu_Set(ent, 4, "Change Team", Change_Team_Exec);
	}
//#else
//	Menu_Set(ent, 4, "Change Team", Change_Team_Exec);
//#endif
	Menu_Set(ent, 5, "Change Skin", Skin_Menu);
	Menu_Set(ent, 6, "Radio Sounds", Radio_Menu);
	Menu_Set(ent, 7, "Voice Sounds", Voice_Menu);
	if (!((int)ctfflags->value & CTF_VOTEMENU_OFF))
		Menu_Set(ent, 8, "Voting Menu", Vote_Menu);              //Vampire -- voting menu
	if (cl->ctf.extra_flags & CTF_EXTRAFLAGS_REFEREE)
		Menu_Set(ent, 9, "Referee Menu", Ref_Main_Menu);
	Menu_Set(ent, 10, "Help", Help_Menu);

	cl->menuselect = 0;
		
	Menu_Draw (ent);
	gi.unicast (ent, true);
}

void Skin_Old_Menu (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	cl->menu = MENU_SKIN;
	cl->menuselect = 1;


	Menu_Draw (ent);
	gi.unicast (ent, true);
}


void f(edict_t *ent)
{
}


void SetOldSkin (edict_t *ent)
{
	Info_SetValueForKey (ent->client->pers.userinfo, "skin", skin[ent->client->menuselect]);
	ClientSetSkin(ent, skin[ent->client->menuselect]);
}


void SetSkin (edict_t *ent)
{
	int i;
	char **skinlist;

	i = (ent->client->menuselect - 2)+ent->client->menulastpage*15;

	skinlist = SkinGetList(ent);

	ClientSetSkin(ent, skinlist[i]);
	ent->client->currmenu = Skin_Menu;
	ent->client->menupage = 0;
	Skin_Menu(ent);
}

void Skin_Menu (edict_t *ent)
{
	int i, j, start;
	char **skinlist;

	if (!SkinListInUse())
	{
		Skin_Old_Menu(ent);
		return;
	}

	// Calculate our page
	start = 15*ent->client->menupage;
	
	
	skinlist = SkinGetList(ent);

	// Find if last page was the last
	if (start > 14)
	{
		for (i=start-15;i < start; i++)
		{
			if (!skinlist[i]) // Last entry
			{
				start = 0;			// Go to first page
				ent->client->menupage = 0;
			}
		}
	}

	Menu_Free(ent);
	ent->client->menu = MENU_LOCAL;
	ent->client->menuselect = 0;

	if (ent->client->ctf.teamnum == CTF_TEAM_RED)
	{
		Menu_Set(ent, 0, "LMCTF Red Skins", Main_Menu);
		Menu_Set(ent, 1, "---------------", NULL);
	}
	else
	{
		Menu_Set(ent, 0, "LMCTF Blue Skins", Main_Menu);
		Menu_Set(ent, 1, "----------------", NULL);
	}

	for (i=start, j=2; skinlist[i] && j < 17; i++,j++)
	{
		Menu_Set(ent, j, skinlist[i], SetSkin);
	}

	Menu_Set(ent, 17, "<next page>", Skin_Menu);

	Menu_Draw (ent);
	gi.unicast (ent, true);
}



#ifdef OLDOBSEVERCODE
void Observe (edict_t *ent)
{
	if (ent->client->ctf.teamnum > CTF_TEAM_UNDEFINED)
	{
		Drop_All(ent);
		ctf_SetEntTeam(ent, CTF_TEAM_OBSERVER);
		ent->client->showmenu = false;
		Menu_Blank();
		gi.unicast (ent, true);
		Observer_Start(ent);
	}
	else
	{
		Drop_All(ent);
		TeamJoin (ent);
		ent->client->showmenu = false;
		Menu_Blank();
		gi.unicast (ent, true);
		Observer_Stop(ent);
	}
}

void Observe_Exec(edict_t *ent)
{
	Observe(ent);
	Ctf_Menu(ent);
}

void ChaseCam (edict_t *ent)
{
	if (ent->client->ctf.teamnum > CTF_TEAM_UNDEFINED)
		return;

	if (ent->client->camera_target)
	{
		Camera_Stop(ent);
	}
	else
	{
		Camera_Start(ent);
	}
	Obs_Main_Menu(ent);
}

void Obs_CamLock_Exec (edict_t *ent)
{
	if (ent->client->ctf.extra_flags & CTF_EXTRAFLAGS_CAMERA_LOCKED)
	{
		gi.centerprintf(ent,"Camera trailing target.\n");
		ent->client->ctf.extra_flags &= ~CTF_EXTRAFLAGS_CAMERA_LOCKED;
	}
	else
	{
		gi.centerprintf(ent, "Camera locked on target.\n");
		ent->client->ctf.extra_flags |= CTF_EXTRAFLAGS_CAMERA_LOCKED;
	}
	Obs_Main_Menu(ent);
}

void Obs_Reverse_Exec (edict_t *ent)
{
	if (ent->client->ctf.extra_flags & CTF_EXTRAFLAGS_CAMERA_REVERSE)
	{
		gi.centerprintf(ent,"Camera following target view.\n");
		ent->client->ctf.extra_flags &= ~CTF_EXTRAFLAGS_CAMERA_REVERSE;
	}
	else
	{
		gi.centerprintf(ent, "Camera in reverse view mode.\n");
		ent->client->ctf.extra_flags |= CTF_EXTRAFLAGS_CAMERA_REVERSE;
	}
	Obs_Main_Menu(ent);
}
#else //bat
void Observe(edict_t *ent)
{
	Cmd_Observe_f(ent, CTF_TEAM_OBSERVER);
}

void Observe_Red(edict_t *ent)
{
	Cmd_Observe_f(ent, CTF_TEAM_OBSERVER_RED);
}

void Observe_Blue(edict_t *ent)
{
	Cmd_Observe_f(ent, CTF_TEAM_OBSERVER_BLUE);
}


void Observe_Exec(edict_t *ent)
{
	if(ent->client->ctf.teamnum <= CTF_TEAM_OBSERVER)
		TeamJoin(ent);
	else
		Observe(ent);

	Ctf_Menu(ent);
}

#endif

/*
void Help_Menu (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	cl->menu = MENU_HELP;
	cl->menuselect = 1;
	
	Menu_Draw (ent);
	gi.unicast (ent, true);
}
*/

void HowToPlay_Menu (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	cl->menu = MENU_HOWTOPLAY;
	cl->menuselect = 1;
	
	Menu_Draw (ent);
	gi.unicast (ent, true);
}

/*
void Command_Menu (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	cl->menu = MENU_COMMAND;
	cl->menuselect = 1;


	// Validate our highlighed selection
	while (!menulist[cl->menu].menu[cl->menuselect].func)
		cl->menuselect = (cl->menuselect + 1) % menulist[cl->menu].size;
	
	Menu_Draw (ent);
	gi.unicast (ent, true);
}
*/

void Toggle_Radio_Menu (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	cl->showscores = false;
	cl->showhelp = false;
	cl->showinventory = false;
	cl->showmod = false;
	cl->showctfhud = false;
	cl->showsquadboard = false; // ADC

	if (cl->showmenu)
	{
		cl->showmenu = false;
		Menu_Blank();
		gi.unicast (ent, true);
		return;
	}

	cl->showmenu = true;
	Radio_Menu(ent);
}

void Radio_Menu (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	cl->menu = MENU_RADIO;
	cl->menuselect = 1;
	
	Menu_Draw (ent);
	gi.unicast (ent, true);
}

void PickSound (edict_t *ent)
{
	PlayTeamSound(ent, radiosound[ent->client->menuselect]);

	ent->client->showmenu = false;
	Menu_Blank();
	gi.unicast (ent, true);
	return;
}

#ifdef OLDOBSERVERCODE
void Obs_Main_Menu (edict_t *ent)
{
	char text[MAX_INFO_STRING];

	Menu_Free(ent);
	ent->client->menu = MENU_LOCAL;
	ent->client->menuselect = 1;


	Menu_Set(ent, 1, "LMCTF Observer Menu", NULL);
	Menu_Set(ent, 2, "-------------------", NULL);
	Menu_Set(ent, 4, "Play CTF", Observe_Exec);
	if (ent->client->camera_target)
		sprintf(text, "ChaseCam:        ON");
	else
		sprintf(text, "ChaseCam:       OFF");
	Menu_Set(ent, 6, text, ChaseCam);
	if (ent->client->ctf.extra_flags & CTF_EXTRAFLAGS_CAMERA_LOCKED)
		sprintf(text, "Camera Lock:     ON");
	else
		sprintf(text, "Camera Lock:    OFF");
	Menu_Set(ent, 7, text, Obs_CamLock_Exec);
	if (ent->client->ctf.extra_flags & CTF_EXTRAFLAGS_CAMERA_REVERSE)
		sprintf(text, "Reverse Cam:     ON");
	else
		sprintf(text, "Reverse Cam:    OFF");
	Menu_Set(ent, 8, text, Obs_Reverse_Exec);

	Menu_Set(ent, 10, "Help", Help_Menu);

	Menu_Draw (ent);
	gi.unicast (ent, true);

}
#else //bat
void Obs_Main_Menu (edict_t *ent)
{
char text[MAX_INFO_STRING];

	Menu_Free(ent);
	ent->client->menu = MENU_LOCAL;
	ent->client->menuselect = 1;


	Menu_Set(ent, 1, "LMCTF Observer Menu", NULL);
	Menu_Set(ent, 2, "-------------------", NULL);
	Menu_Set(ent, 4, "Play CTF", Observe_Exec);

	sprintf(text, "Observe Red");
	Menu_Set(ent, 6, text, Observe_Red);
	
	sprintf(text, "Observe_Blue");
	Menu_Set(ent, 7, text, Observe_Blue);

	sprintf(text, "Observe_All");
	Menu_Set(ent, 8, text, Observe);

	Menu_Set(ent, 10, "Help", Help_Menu);

	Menu_Draw (ent);
	gi.unicast (ent, true);
}
#endif

void Ref_Main_Load (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	cl->showscores = false;
	cl->showhelp = false;
	cl->showinventory = false;
	cl->showmod = false;
	cl->showctfhud = false;
	cl->showsquadboard = false; // ADC

	if (cl->showmenu)
	{
		cl->showmenu = false;
		Menu_Blank();
		gi.unicast (ent, true);
		return;
	}

	cl->showmenu = true;

	cl->menu = MENU_LOCAL;
	cl->menuselect = 1;

	Ref_Main_Menu(ent);
}

void Ref_Main_Menu (edict_t *ent)
{
	char text[MAX_INFO_STRING];

	Menu_Free(ent);
	ent->client->menu = MENU_LOCAL;
	ent->client->menuselect = 1;


	Menu_Set(ent, 1, "LMCTF Referee Menu", Main_Menu);
	Menu_Set(ent, 2, "------------------", NULL);
	Menu_Set(ent, 3, "Change Map", Ref_Map_Menu);
	Menu_Set(ent, 4, "Server Settings", Ref_Settings_Menu);
	sprintf(text, "Ping Floor:          %3d", ent->client->ctf.pingalertfloor);	
	Menu_Set(ent, 5, text, Ref_PingFloor_Menu);
	sprintf(text, "Ping Ceiling:        %3d", ent->client->ctf.pingalertceiling);	
	Menu_Set(ent, 6, text, Ref_PingCeiling_Menu);
	if (Match_Mode())
		Menu_Set(ent, 7, "Stop Match", Ref_End_Match);
	else
		Menu_Set(ent, 7, "Start Match", Ref_Match_Menu);
	Menu_Set(ent, 8, "Kick Player", Ref_Kick_Menu);
	
	if (GamePaused())
		Menu_Set(ent, 9, "Continue Match", RefTogglePause);
	else
		Menu_Set(ent, 9, "Pause Match", RefTogglePause);
	Menu_Set(ent, 10, "Practice Settings", Ref_Practice_Menu);
	Menu_Set(ent, 11, "Referee Help", Ref_Help_Menu);
		
	if (ent->client->ctf.extra_flags & CTF_EXTRAFLAGS_RCON)
		Menu_Set(ent, 13, "Save Config (RCON)", SaveServer_Exec);

	Menu_Draw (ent);
	gi.unicast (ent, true);
}

void RefTogglePause(edict_t *ent)
{
	if (GamePaused())
		SetPause(false);
	else
		SetPause(true);
	
	Ref_Main_Menu(ent);
}

void SaveServer_Exec(edict_t *ent)
{
	FILE	*fp;

	char	name[MAX_INFO_STRING];
	char	text[MAX_INFO_STRING];
	int		size;
	char*	tempbuf;
	long	i = 0;

	strcpy(name, gamedir->string);
//	strcat (name, "/server.cfg");
	strcat(name,"/");
	strcat(name, server_file->string);

	fp = fopen (name, "r");
	if (!fp)
		return;
	
	tempbuf = (char *) malloc(40000); //server.cfg files cannot exceed 40k arbitrary

	if (tempbuf)
	{
		size = fread(tempbuf, 1, 40000, fp);
		fclose(fp);

		if (size > 0 && size < 40000)
		{
			ctf_SafePrint(ent,PRINT_HIGH,"Read server.cfg successfully");
		}
		else
		{
			ctf_SafePrint(ent,PRINT_HIGH,"Failed to read server.cfg");
		}

		fp = fopen(name, "w");
		if (!fp)
			return;

		//dump out the server settings we record
		sprintf(text, "set dmflags %d\n", (int) dmflags->value);
		fwrite(text, 1, strlen(text), fp);
		sprintf(text, "set ctfflags %d\n", (int) ctfflags->value);
		fwrite(text, 1, strlen(text), fp);
		sprintf(text, "set timelimit %d\n", (int) timelimit->value);
		fwrite(text, 1, strlen(text), fp);
		sprintf(text, "set fraglimit %d\n", (int) fraglimit->value);
		fwrite(text, 1, strlen(text), fp);

		//now dump out remainder of file, ignoring the things we dumped out above
		while (i <= size)
		{
			if (
				strncmp(&tempbuf[i], "set dmflags", 11) == 0 || strncmp(&tempbuf[i], "dmflags", 7) == 0 ||
				strncmp(&tempbuf[i], "set ctfflags", 12) == 0 || strncmp(&tempbuf[i], "ctfflags", 8) == 0 ||
				strncmp(&tempbuf[i], "set timelimit", 13) == 0 || strncmp(&tempbuf[i], "timelimit", 9) == 0 ||
				strncmp(&tempbuf[i], "set fraglimit", 13) == 0 || strncmp(&tempbuf[i], "fraglimit", 9) == 0
				)
			{
				while(tempbuf[i] != '\n' && i < size)
					i++;
				i++; //get past that additional \n character
			}
			else
			{
				while(tempbuf[i] != '\n' && i < size)
				{
					fwrite(&tempbuf[i], 1, 1, fp);
					i++;
				}
				fwrite("\n",1,1,fp);
				i++;
			}
		}
		fclose(fp);
		ctf_SafePrint(ent,PRINT_HIGH,"Success: Current server values saved to server.cfg.\n");

	}
	else
	{
		ctf_SafePrint(ent,PRINT_HIGH,"Error: unable to malloc memory for server.cfg read.\n");
	}

	free(tempbuf);
	return;
}

char *refhelptext[] = 
{
	//23456789012345678901234
	"",
	"gotomap",
	"",
	"- Allows a referee to",
	"change the current map.",
	"",
	"",
	"users",
	"",
	"- Lists the player",
	"numbers for all active",
	"users.",
	"",
	"",
	"",
	// Page 2
	//23456789012345678901234

	"",
	"kick",
	"",
	"- Allows a referee to",
	"disconnect a player.",
	"Requires a player",
	"number.",
	"",
	"",
	"match",
	"",
	"- Starts a match with",
	"given map name. Map",
	"must be in maplist.",
	"",

	// Page 3
	//23456789012345678901234

	"",
	"pingalert",
	"",
	"- Warns a referee if any",
	"players' ping is above a",
	"max or below a min.",
	"",
	"",
	"refmenu",
	"",
	"- Opens the referee",
	"menu.",
	"",
	"",
	0


};

void Ref_Help_Menu(edict_t *ent)
{
	char text[MAX_INFO_STRING];
	int i,j, start;

	// Calculate our page
	start = 15*ent->client->menupage;
	
	// Find if last page was the last
	if (start > 14)
	{
		for (i=start-15;i < start; i++)
		{
			if (!refhelptext[i]) // Last entry
			{
				start = 0;			// Go to first page
				ent->client->menupage = 0;
			}
		}
	}

	Menu_Free(ent);
	ent->client->menu = MENU_LOCAL;
	ent->client->menuselect = 0;

	Menu_Set(ent, 0, "LMCTF Ref Commands", Ref_Main_Menu);
	Menu_Set(ent, 1, "------------------", NULL);
	for (i=2, j=start; i < 17 && refhelptext[j]; i++, j++)
	{
		sprintf(text, "%.24s", refhelptext[j]);
		Menu_Set(ent, i, text, NULL);
	}
	Menu_Set(ent, 17, "<next page>", Ref_Help_Menu);

	Menu_Draw (ent);
	gi.unicast (ent, true);
}


void Ref_Practice_Menu(edict_t *ent)
{
	char text[MAX_INFO_STRING];
	Menu_Free(ent);
	ent->client->menu = MENU_LOCAL;
	ent->client->menuselect = 1;
	ctf_validateflags();
	Menu_Set(ent, 1, "LMCTF Practice Menu", Ref_Main_Menu);
	Menu_Set(ent, 2, "-------------------", NULL);
	if (redflag && ((int)refset->value & CTF_RED_FLAG_FROZEN) )
		sprintf(text, "Red Flag:     %s", "FROZEN");
	else
		sprintf(text, "Red Flag:     %s", "NORMAL");
	Menu_Set(ent, 3, text, Ref_PracticeFlagRed_Exec);
	if (blueflag && ((int)refset->value & CTF_BLUE_FLAG_FROZEN) )
		sprintf(text, "Blue Flag:    %s", "FROZEN");
	else
		sprintf(text, "Blue Flag:    %s", "NORMAL");
	Menu_Set(ent, 4, text, Ref_PracticeFlagBlue_Exec);
	Menu_Draw (ent);
	gi.unicast (ent, true);
}

void Ref_PracticeFlagRed_Exec(edict_t *ent)
{
	if (redflag && ((int)refset->value & CTF_RED_FLAG_FROZEN) )
		gi.cvar_set("refset", va("%d", ((int) refset->value & ~CTF_RED_FLAG_FROZEN)));
	else
		gi.cvar_set("refset", va("%d", ((int) refset->value | CTF_RED_FLAG_FROZEN)));
	Ref_Practice_Menu(ent);
}
	
void Ref_PracticeFlagBlue_Exec(edict_t *ent)
{
	if (blueflag && ((int)refset->value & CTF_BLUE_FLAG_FROZEN) )
		gi.cvar_set("refset", va("%d", ((int) refset->value & ~CTF_BLUE_FLAG_FROZEN)));
	else
		gi.cvar_set("refset", va("%d", ((int) refset->value | CTF_BLUE_FLAG_FROZEN)));
	Ref_Practice_Menu(ent);
}

int pingfloor[] = 
{
	0,
	0,
	0,
	50,
	100,
	150,
	160,
	170,
	180,
	190,
	200,
	210,
	220,
	230,
	240,
	250,
	350,
	500
};

void Ref_PingFloor_Menu (edict_t *ent)
{
	char text[MAX_INFO_STRING];
	int i;

	Menu_Free(ent);
	ent->client->menu = MENU_LOCAL;
	ent->client->menuselect = 0;

	sprintf(text, "Current Ping Floor:  %3d", ent->client->ctf.pingalertfloor);	
	Menu_Set(ent, 0, text, Ref_Main_Menu);
	Menu_Set(ent, 1, "------------------", NULL);
	for (i=2; i < 18; i++)
	{
		sprintf(text, "%d", pingfloor[i]);
		Menu_Set(ent, i, text, PingFloor_Exec);
	}

	Menu_Draw (ent);
	gi.unicast (ent, true);
}

void PingFloor_Exec (edict_t *ent)
{
	ent->client->ctf.pingalertfloor = pingfloor[ent->client->menuselect];
	Ref_Main_Menu(ent);
}

void Ref_PingCeiling_Menu (edict_t *ent)
{
	char text[MAX_INFO_STRING];
	int i;

	Menu_Free(ent);
	ent->client->menu = MENU_LOCAL;
	ent->client->menuselect = 0;

	sprintf(text, "Current Ping Ceiling: %3d", ent->client->ctf.pingalertceiling);	
	Menu_Set(ent, 0, text, Ref_Main_Menu);
	Menu_Set(ent, 1, "------------------", NULL);
	for (i=2; i < 18; i++)
	{
		sprintf(text, "%d", pingfloor[i]);
		Menu_Set(ent, i, text, PingCeiling_Exec);
	}

	Menu_Draw (ent);
	gi.unicast (ent, true);
}

void PingCeiling_Exec (edict_t *ent)
{
	ent->client->ctf.pingalertceiling = pingfloor[ent->client->menuselect];
	Ref_Main_Menu(ent);
}

void Ref_Settings_Menu (edict_t *ent)
{
	char text[MAX_INFO_STRING];

	Menu_Free(ent);
	ent->client->menu = MENU_LOCAL;
	ent->client->menuselect = 1;


	Menu_Set(ent, 1, "Server Settings", Ref_Main_Menu);
	Menu_Set(ent, 2, "---------------", NULL);
	sprintf(text, "Timelimit:           %3d", (int)timelimit->value);	
	Menu_Set(ent, 3, text, Ref_Timelimit_Menu);
	sprintf(text, "Fraglimit:           %3d", (int)fraglimit->value);	
	Menu_Set(ent, 4, text, Ref_Fraglimit_Menu);
	sprintf(text, "DMFlags:           %5d", ((unsigned short)dmflags->value));	
	Menu_Set(ent, 5, text, Ref_DMFlags_Menu);
	sprintf(text, "CTFFlags:          %5d", ((unsigned short)ctfflags->value));	
	Menu_Set(ent, 6, text, Ref_CTFFlags_Menu);
	sprintf(text, "Password: %s", password->string);
	Menu_Set(ent, 7, text, NULL);
	if (ent->client->ctf.extra_flags & CTF_EXTRAFLAGS_RCON)
		Menu_Set(ent, 8, "Clear password (RCON)", ClearPassword_Exec);
	

	Menu_Draw (ent);
	gi.unicast (ent, true);
}

void ClearPassword_Exec (edict_t *ent)
{
	gi.cvar_set("password", "");
	Ref_Settings_Menu(ent);
}

void DMFlags_Exec (edict_t *ent)
{
	int i;

	i = ent->client->menuselect - 2;
	if ((int)dmflags->value & (1 << i)) // is it set?
		gi.cvar_set("dmflags", va("%d", ((int)dmflags->value & ~(1 << i))));
	else
		gi.cvar_set("dmflags", va("%d", ((int)dmflags->value |(1 << i))));
	Ref_DMFlags_Menu(ent);
}

#define DMFLAG(a) ( ((int)dmflags->value & a) ? "ON" : "OFF" )

void Ref_DMFlags_Menu (edict_t *ent)
{
	char text[MAX_INFO_STRING];

	Menu_Free(ent);
	ent->client->menu = MENU_LOCAL;
	ent->client->menuselect = 1;

	sprintf(text, "DMFlags:             %d", ((unsigned short)dmflags->value));	
	Menu_Set(ent, 0, text, Ref_Settings_Menu);
	Menu_Set(ent, 1, "----------------", NULL);
	sprintf(text, "No Health            %s", DMFLAG(DF_NO_HEALTH));	
	Menu_Set(ent, 2, text, DMFlags_Exec);
	sprintf(text, "No Items             %s", DMFLAG(DF_NO_ITEMS));	
	Menu_Set(ent, 3, text, DMFlags_Exec);
	sprintf(text, "Weapons Stay         %s", DMFLAG(DF_WEAPONS_STAY));	
	Menu_Set(ent, 4, text, DMFlags_Exec);
	sprintf(text, "No Falling           %s", DMFLAG(DF_NO_FALLING));	
	Menu_Set(ent, 5, text, DMFlags_Exec);
	sprintf(text, "Instant Items        %s", DMFLAG(DF_INSTANT_ITEMS));	
	Menu_Set(ent, 6, text, DMFlags_Exec);
	sprintf(text, "Same Level           %s", DMFLAG(DF_SAME_LEVEL));	
	Menu_Set(ent, 7, text, DMFlags_Exec);
	sprintf(text, "Skin Teams           %s", DMFLAG(DF_SKINTEAMS));	
	Menu_Set(ent, 8, text, DMFlags_Exec);
	sprintf(text, "Model Teams          %s", DMFLAG(DF_MODELTEAMS));	
	Menu_Set(ent, 9, text, DMFlags_Exec);
	sprintf(text, "Friendly Fire        %s", DMFLAG(DF_NO_FRIENDLY_FIRE));	
	Menu_Set(ent, 10, text, DMFlags_Exec);
	sprintf(text, "Spawn Farthest       %s", DMFLAG(DF_SPAWN_FARTHEST));	
	Menu_Set(ent, 11, text, DMFlags_Exec);
	sprintf(text, "Force Respawn        %s", DMFLAG(DF_FORCE_RESPAWN));	
	Menu_Set(ent, 12, text, DMFlags_Exec);
	sprintf(text, "No Armor             %s", DMFLAG(DF_NO_ARMOR));	
	Menu_Set(ent, 13, text, DMFlags_Exec);
	sprintf(text, "Allow Exit           %s", DMFLAG(DF_ALLOW_EXIT));	
	Menu_Set(ent, 14, text, DMFlags_Exec);
	sprintf(text, "Infinite Ammo        %s", DMFLAG(DF_INFINITE_AMMO));	
	Menu_Set(ent, 15, text, DMFlags_Exec);
	sprintf(text, "Quad Drop            %s", DMFLAG(DF_QUAD_DROP));	
	Menu_Set(ent, 16, text, DMFlags_Exec);
	sprintf(text, "Fixed FOV            %s", DMFLAG(DF_FIXED_FOV));	
	Menu_Set(ent, 17, text, DMFlags_Exec);

	Menu_Draw (ent);
	gi.unicast (ent, true);
}

void CTFFlags_Exec (edict_t *ent)
{
	int i;

	i = ent->client->menuselect - 2;
	if ((int)ctfflags->value & (1 << i)) // is it set?
		gi.cvar_set("ctfflags", va("%d", ((int)ctfflags->value & ~(1 << i))));
	else
		gi.cvar_set("ctfflags", va("%d", ((int)ctfflags->value |(1 << i))));
	Ref_CTFFlags_Menu(ent);
}

#define CTFFLAG(a) ( ((int)ctfflags->value & a) ? "ON" : "OFF" )

void Ref_CTFFlags_Menu (edict_t *ent)
{
	char text[MAX_INFO_STRING];

	Menu_Free(ent);
	ent->client->menu = MENU_LOCAL;
	ent->client->menuselect = 1;

	sprintf(text, "CTFFlags:            %d", ((unsigned short)ctfflags->value));	
	Menu_Set(ent, 0, text, Ref_Settings_Menu);
	Menu_Set(ent, 1, "----------------", NULL);
#ifdef WEAP_BALANCE_OK	
	sprintf(text, "Weapon Balance       %s", CTFFLAG(CTF_WEAP_BALANCE));	
	Menu_Set(ent, 2, text, CTFFlags_Exec);
#endif
	sprintf(text, "Allow Invuln         %s", CTFFLAG(CTF_ALLOW_INVULN));	
	Menu_Set(ent, 3, text, CTFFlags_Exec);
	sprintf(text, "Team Reset           %s", CTFFLAG(CTF_TEAM_RESET));	
	Menu_Set(ent, 4, text, CTFFlags_Exec);
	sprintf(text, "Team No Switch       %s", CTFFLAG(CTF_TEAM_NOSWITCH));	
	Menu_Set(ent, 5, text, CTFFlags_Exec);
	sprintf(text, "Offhand Hook         %s", CTFFLAG(CTF_OFFHAND_HOOK));	
#ifdef NOVOICE_OK	
	Menu_Set(ent, 6, text, CTFFlags_Exec);
	sprintf(text, "No Voice             %s", CTFFLAG(CTF_NOVOICE));	
#endif
	Menu_Set(ent, 7, text, CTFFlags_Exec);
	sprintf(text, "No Grapple Damage    %s", CTFFLAG(CTF_NO_GRAP_DAMAGE));	
	Menu_Set(ent, 8, text, CTFFlags_Exec);
	sprintf(text, "No Teams             %s", CTFFLAG(CTF_TEAM_NOTEAMS));	
	Menu_Set(ent, 9, text, CTFFlags_Exec);
	sprintf(text, "No Flags             %s", CTFFLAG(CTF_FLAGS_NOFLAGS));	
	Menu_Set(ent, 10, text, CTFFlags_Exec);
	sprintf(text, "Score Balance        %s", CTFFLAG(CTF_SCORE_BALANCE));	
	Menu_Set(ent, 11, text, CTFFlags_Exec);
	sprintf(text, "Team Armor Protect   %s", CTFFLAG(CTF_TEAM_ARMOR_PROTECT));	
	Menu_Set(ent, 12, text, CTFFlags_Exec);

	//-bat
	sprintf(text, "Random Map List      %s", CTFFLAG(CTF_RANDOM_MAPS));	
	Menu_Set(ent, 14, text, CTFFlags_Exec);
	sprintf(text, "Random Quad Respawn  %s", CTFFLAG(CTF_RANDOM_QUAD));	
	Menu_Set(ent, 15, text, CTFFlags_Exec);
	sprintf(text, "Random Death Msgs    %s", CTFFLAG(CTF_RANDOM_DEATH_MSG));	
	Menu_Set(ent, 16, text, CTFFlags_Exec);
	

	Menu_Draw (ent);
	gi.unicast (ent, true);
}

int timeslist[] = 
{
	0,
	0,
	0,
	1,
	5,
	10,
	12,
	15,
	20,
	25,
	30,
	35,
	40,
	45,
	60,
	60,
	90,
	120
};

void SetTimelimit (edict_t *ent)
{
	gi.cvar_set("timelimit", va("%f", (float)timeslist[ent->client->menuselect]));
	Ref_Settings_Menu(ent);
}

void Ref_Timelimit_Menu (edict_t *ent)
{
	char text[MAX_INFO_STRING];
	int i;

	Menu_Free(ent);
	ent->client->menu = MENU_LOCAL;
	ent->client->menuselect = 0;

	sprintf(text, "Current Timelimit:  %3d", (int)timelimit->value);	
	Menu_Set(ent, 0, text, Ref_Settings_Menu);
	Menu_Set(ent, 1, "------------------", NULL);
	for (i=2; i < 18; i++)
	{
		sprintf(text, "%d", timeslist[i]);
		Menu_Set(ent, i, text, SetTimelimit);
	}

	Menu_Draw (ent);
	gi.unicast (ent, true);
}

int fragslist[] = 
{
	0,
	0,
	0,
	15,
	30,
	45,
	60,
	75,
	90,
	105,
	120,
	135,
	150,
	165,
	180,
	200,
	300,
	500
};

void SetFraglimit (edict_t *ent)
{
	gi.cvar_set("fraglimit", va("%f", (float)fragslist[ent->client->menuselect]));
	Ref_Settings_Menu(ent);
}

void Ref_Fraglimit_Menu (edict_t *ent)
{
	char text[MAX_INFO_STRING];
	int i;

	Menu_Free(ent);
	ent->client->menu = MENU_LOCAL;
	ent->client->menuselect = 0;

	sprintf(text, "Current Fraglimit:  %3d", (int)fraglimit->value);	
	Menu_Set(ent, 0, text, Ref_Settings_Menu);
	Menu_Set(ent, 1, "------------------", NULL);
	for (i=2; i < 18; i++)
	{
		sprintf(text, "%d", fragslist[i]);
		Menu_Set(ent, i, text, SetFraglimit);
	}

	Menu_Draw (ent);
	gi.unicast (ent, true);
}


// Maps

char *mapalist[] = 
{
	0,
	0,
	"lmctf01",
	"lmctf02",
	"lmctf03",
	"lmctf04",
	"lmctf05",
	"lmctf06",
	"lmctf07",
	"lmctf08",
	"lmctf09",
	"lmctf10",
	0,
	0,
	0,
	0,
	0,
	0
};

char *mapblist[] = 
{
	0,
	0,
	"lmctf11",
	"lmctf12",
	"lmctf13",
	"lmctf14",
	"lmctf15",
	"lmctf16",
	"lmctf17",
	"lmctf18",
	"lmctf19",
	"lmctf20",
	0,
	0,
	0,
	0,
	0,
	0
};

char *mapclist[] = 
{
	0,
	0,
	"lmctf21",
	"lmctf22",
	"lmctf23",
	"lmctf24",
	"lmctf25",
	"lmctf26",
	"lmctf27",
	"lmctf28",
	"lmctf29",
	"lmctf30",
	0,
	0,
	0,
	0,
	0,
	0
};



/*
void SetMatchBMap (edict_t *ent)
{
	StartMatch (mapblist[ent->client->menuselect]);
	Ref_Main_Menu(ent);
}

void SetBMap (edict_t *ent)
{
	char command[100];
	Com_sprintf (command, sizeof(command), "gamemap \"%s\"\n", mapblist[ent->client->menuselect]);
	gi.AddCommandString (command);
	level.changemap = NULL;
	level.exitintermission = 0;
	level.intermissiontime = 0;
	stats_cleanup(); // STATS - LM_Hati

	Ref_Main_Menu(ent);
}
*/

void SetMap (edict_t *ent)
{
	int i;

	if (ent->client->prevmenu == Ref_Match_A_Menu)
	{
		Ctf_Menu(ent); // turn off menu
		StartMatch (mapalist[ent->client->menuselect]);
	}
	else if (ent->client->prevmenu == Ref_Match_B_Menu)
	{
		Ctf_Menu(ent); // turn off menu
		StartMatch (mapblist[ent->client->menuselect]);
	}
	else if (ent->client->prevmenu == Ref_Match_C_Menu)
	{
		Ctf_Menu(ent); // turn off menu
		StartMatch (mapclist[ent->client->menuselect]);
	}
	else if (ent->client->prevmenu == Ref_Match_Maplist_Menu)
	{
		Ctf_Menu(ent); // turn off menu
		i = (ent->client->menuselect - 2)+ent->client->menulastpage*15;
		StartMatch (maplist[i]);
	}
	else if (ent->client->prevmenu == Ref_Map_A_Menu)
	{
		Ctf_Menu(ent); // turn off menu
		ctf_ChangeMap(mapalist[ent->client->menuselect], false);
	}
	else if (ent->client->prevmenu == Ref_Map_B_Menu)
	{
		Ctf_Menu(ent); // turn off menu
		ctf_ChangeMap(mapblist[ent->client->menuselect], false);
	}
	else if (ent->client->prevmenu == Ref_Map_C_Menu)
	{
		Ctf_Menu(ent); // turn off menu
		ctf_ChangeMap(mapclist[ent->client->menuselect], false);
	}
	else if (ent->client->prevmenu == Ref_Map_Maplist_Menu)
	{
		Ctf_Menu(ent); // turn off menu
		i = (ent->client->menuselect - 2)+ent->client->menulastpage*15;
		ctf_ChangeMap(maplist[i], false);
	}
}

void Ref_Match_Maplist_Menu (edict_t *ent)
{
	char text[MAX_INFO_STRING];
	int i,j, start;

	// Calculate our page
	start = 15*ent->client->menupage;
	
	// Find if last page was the last
	if (start > 14)
	{
		for (i=start-15;i < start; i++)
		{
			if (!maplist[i][0]) // Last entry
			{
				start = 0;			// Go to first page
				ent->client->menupage = 0;
			}
		}
	}

	Menu_Free(ent);
	ent->client->menu = MENU_LOCAL;
	ent->client->menuselect = 0;

	Menu_Set(ent, 0, "Match Maplist", Ref_Main_Menu);
	Menu_Set(ent, 1, "-------------", NULL);
	for (i=2, j=start; i < 17 && maplist[j][0]; i++, j++)
	{
		sprintf(text, "%s", maplist[j]);
		Menu_Set(ent, i, text, SetMap);
	}
	Menu_Set(ent, 17, "<next page>", Ref_Match_Maplist_Menu);

	Menu_Draw (ent);
	gi.unicast (ent, true);
}

void Ref_Map_Maplist_Menu (edict_t *ent)
{
	char text[MAX_INFO_STRING];
	int i,j, start;

	// Calculate our page
	start = 15*ent->client->menupage;
	
	// Find if last page was the last
	if (start > 14)
	{
		for (i=start-15;i < start; i++)
		{
			if (!maplist[i][0]) // Last entry
			{
				start = 0;			// Go to first page
				ent->client->menupage = 0;
			}
		}
	}

	Menu_Free(ent);
	ent->client->menu = MENU_LOCAL;
	ent->client->menuselect = 0;

	Menu_Set(ent, 0, "Maplist", Ref_Main_Menu);
	Menu_Set(ent, 1, "-------", NULL);
	for (i=2, j=start; i < 17 && maplist[j][0]; i++, j++)
	{
		sprintf(text, "%s", maplist[j]);
		Menu_Set(ent, i, text, SetMap);
	}
	Menu_Set(ent, 17, "<next page>", Ref_Map_Maplist_Menu);

	Menu_Draw (ent);
	gi.unicast (ent, true);
}


void Ref_Match_A_Menu (edict_t *ent)
{
	char text[MAX_INFO_STRING];
	int i;

	Menu_Free(ent);
	ent->client->menu = MENU_LOCAL;
	ent->client->menuselect = 0;

	Menu_Set(ent, 0, "Set 1 Match", Ref_Main_Menu);
	Menu_Set(ent, 1, "-----------", NULL);
	for (i=2; i < 18; i++)
	{
		if (mapalist[i])
		{
			sprintf(text, "%s", mapalist[i]);
			Menu_Set(ent, i, text, SetMap);
		}
	}

	Menu_Draw (ent);
	gi.unicast (ent, true);
}

void Ref_Map_A_Menu (edict_t *ent)
{
	char text[MAX_INFO_STRING];
	int i;

	Menu_Free(ent);
	ent->client->menu = MENU_LOCAL;
	ent->client->menuselect = 0;

	Menu_Set(ent, 0, "Set 1 Maps", Ref_Main_Menu);
	Menu_Set(ent, 1, "----------", NULL);
	for (i=2; i < 18; i++)
	{
		if (mapalist[i])
		{
			sprintf(text, "%s", mapalist[i]);
			Menu_Set(ent, i, text, SetMap);
		}
	}

	Menu_Draw (ent);
	gi.unicast (ent, true);
}

void Ref_Match_B_Menu (edict_t *ent)
{
	char text[MAX_INFO_STRING];
	int i;

	Menu_Free(ent);
	ent->client->menu = MENU_LOCAL;
	ent->client->menuselect = 0;

	Menu_Set(ent, 0, "Set 2 Match", Ref_Main_Menu);
	Menu_Set(ent, 1, "-----------", NULL);
	for (i=2; i < 18; i++)
	{
		if (mapblist[i])
		{
			sprintf(text, "%s", mapblist[i]);
			Menu_Set(ent, i, text, SetMap);
		}
	}

	Menu_Draw (ent);
	gi.unicast (ent, true);
}


void Ref_Match_C_Menu (edict_t *ent)
{
	char text[MAX_INFO_STRING];
	int i;

	Menu_Free(ent);
	ent->client->menu = MENU_LOCAL;
	ent->client->menuselect = 0;

	Menu_Set(ent, 0, "Set 3 Match", Ref_Main_Menu);
	Menu_Set(ent, 1, "-----------", NULL);
	for (i=2; i < 18; i++)
	{
		if (mapclist[i])
		{
			sprintf(text, "%s", mapclist[i]);
			Menu_Set(ent, i, text, SetMap);
		}
	}

	Menu_Draw (ent);
	gi.unicast (ent, true);
}


void Ref_Map_B_Menu (edict_t *ent)
{
	char text[MAX_INFO_STRING];
	int i;

	Menu_Free(ent);
	ent->client->menu = MENU_LOCAL;
	ent->client->menuselect = 0;

	Menu_Set(ent, 0, "Set 2 Maps", Ref_Main_Menu);
	Menu_Set(ent, 1, "----------", NULL);
	for (i=2; i < 18; i++)
	{
		if (mapblist[i])
		{
			sprintf(text, "%s", mapblist[i]);
			Menu_Set(ent, i, text, SetMap);
		}
	}

	Menu_Draw (ent);
	gi.unicast (ent, true);
}


void Ref_Map_C_Menu (edict_t *ent)
{
	char text[MAX_INFO_STRING];
	int i;

	Menu_Free(ent);
	ent->client->menu = MENU_LOCAL;
	ent->client->menuselect = 0;

	Menu_Set(ent, 0, "Set 3 Maps", Ref_Main_Menu);
	Menu_Set(ent, 1, "----------", NULL);
	for (i=2; i < 18; i++)
	{
		if (mapclist[i])
		{
			sprintf(text, "%s", mapclist[i]);
			Menu_Set(ent, i, text, SetMap);
		}
	}

	Menu_Draw (ent);
	gi.unicast (ent, true);
}



void Ref_End_Match (edict_t *ent)
{
	KillMatch();
	Ref_Main_Menu(ent);
}

void Ref_Match_Menu (edict_t *ent)
{
	Menu_Free(ent);
	ent->client->menu = MENU_LOCAL;
	ent->client->menuselect = 0;

	//gi.dprintf("Ref_Match_Menu\n");
	Menu_Set(ent, 0, "Match Menu", Ref_Main_Menu);
	Menu_Set(ent, 1, "----------", NULL);
	Menu_Set(ent, 2, "LMCTF Set 1", Ref_Match_A_Menu);
	Menu_Set(ent, 3, "LMCTF Set 2", Ref_Match_B_Menu);
	Menu_Set(ent, 4, "LMCTF Set 3", Ref_Match_C_Menu);
	if (maplistindex != -2) // No list
		Menu_Set(ent, 5, "Maplist", Ref_Match_Maplist_Menu);
	Menu_Draw (ent);
	gi.unicast (ent, true);
}

void Ref_Map_Menu (edict_t *ent)
{
	Menu_Free(ent);
	ent->client->menu = MENU_LOCAL;
	ent->client->menuselect = 0;

	//gi.dprintf("Ref_Map_Menu\n");
	Menu_Set(ent, 0, "Map Menu", Ref_Main_Menu);
	Menu_Set(ent, 1, "--------", NULL);
	Menu_Set(ent, 2, "LMCTF Set 1", Ref_Map_A_Menu);
	Menu_Set(ent, 3, "LMCTF Set 2", Ref_Map_B_Menu);
	Menu_Set(ent, 4, "LMCTF Set 3", Ref_Map_C_Menu);
	if (maplistindex != -2) // No list
		Menu_Set(ent, 5, "Maplist", Ref_Map_Maplist_Menu);

	Menu_Draw (ent);
	gi.unicast (ent, true);
}


void SelectKick (edict_t *ent)
{
	int i; //initialize j or you read off into memory
	unsigned long id;
	char kickcommand[MAX_INFO_STRING];

	i = ent->client->menuselect;
	sscanf(ent->client->localmenu[i].text, "%lu", &id );
	if (id)
		sprintf(kickcommand, "\nctfkick %ld\n", id);

	ForceCommand(ent, kickcommand);
}

void Ref_Kick_Menu (edict_t *ent)
{
	int i;
	char message[MAX_INFO_STRING];
	edict_t * player = NULL;

	Menu_Free(ent);
	ent->client->menu = MENU_LOCAL;
	ent->client->menuselect = 0;

	Menu_Set(ent, 0, "LMCTF Kick Menu", Ref_Main_Menu);
	Menu_Set(ent, 1, "---------------", NULL);

	i = 2;
	player = ctf_findplayer(NULL, NULL, CTF_TEAM_IGNORETEAM);
	while (player && i < 17)
	{
		sprintf(message, "%ld %s", player->client->ctf.ctfid, player->client->pers.netname);
		Menu_Set(ent, i, message, SelectKick);
		player = ctf_findplayer(player, NULL, CTF_TEAM_IGNORETEAM);
		i++;
	}

	Menu_Draw (ent);
	gi.unicast (ent, true);
}


char *voicelist[] = 
{
	0,
	0,
	"damn",
	"escort",
	"followme",
	"getflag",
	"goodshot",
	"gotcha",
	"laugh",
	"move",
	"silly",
	"stopshoot",
	0,
	0,
	0,
	0,
	0,
	0
};

void Voice_Menu (edict_t *ent)
{
	int i;

	Menu_Free(ent);
	ent->client->menu = MENU_LOCAL;
	ent->client->menuselect = 0;

	Menu_Set(ent, 0, "Play_Voice Menu", Main_Menu);
	Menu_Set(ent, 1, "---------------", NULL);
	for (i=2; i < 18; i++)
	{
		if (voicelist[i])
			Menu_Set(ent, i, voicelist[i], Voice_Exec);
	}

	Menu_Draw (ent);
	gi.unicast (ent, true);
}

void Voice_Exec (edict_t *ent)
{
	PlayVoiceSound(ent, voicelist[ent->client->menuselect]);
	ent->client->showmenu = false;
	Menu_Blank();
	gi.unicast (ent, true);
}

void Change_Team_Exec(edict_t *ent)
{

	if (ent->client)
	{
		if (ent->client->ctf.teamnum == CTF_TEAM_BLUE)
			Team_Change(ent, CTF_TEAM_RED);
		else
			Team_Change(ent, CTF_TEAM_BLUE);
	}

	Ctf_Menu(ent);
}


void Help_Menu (edict_t *ent)
{
	char text[MAX_INFO_STRING];
	int i,j, start;

	// Calculate our page
	start = 15*ent->client->menupage;
	
	// Find if last page was the last
	if (start > 14)
	{
		for (i=start-15;i < start; i++)
		{
			if (!helptext[i][0]) // Last entry
			{
				start = 0;			// Go to first page
				ent->client->menupage = 0;
			}
		}
	}

	Menu_Free(ent);
	ent->client->menu = MENU_LOCAL;
	ent->client->menuselect = 0;

	Menu_Set(ent, 0, "LMCTF Commands", Main_Menu);
	Menu_Set(ent, 1, "--------------", NULL);
	for (i=2, j=start; i < 17 && helptext[j][0]; i++, j++)
	{
		sprintf(text, "%.24s", helptext[j]);
		Menu_Set(ent, i, text, NULL);
	}
	Menu_Set(ent, 17, "<next page>", Help_Menu);

	Menu_Draw (ent);
	gi.unicast (ent, true);
}


/*
=================
Menu Functions
=================
*/

// Free a menu rather than lose the text strings to oblivion
void Menu_Free (edict_t *ent)
{
	int i;
	for (i=0; i < 18; i++)
	{
		if (ent->client->localmenu[i].text)
		{
			gi.TagFree(ent->client->localmenu[i].text);
			ent->client->localmenu[i].text = NULL;
			ent->client->localmenu[i].func = NULL;
		}
	}
}

void Menu_Set (edict_t *ent, int item, char *text, void	(*func)(edict_t *ent))
{
	if (ent->client->localmenu[item].text)
			gi.TagFree(ent->client->localmenu[item].text);

	// Set the menu text to a tagged malloc'ed string, kind of like strdup
	ent->client->localmenu[item].text = G_CopyString(text);
	ent->client->localmenu[item].func = func;
}

void Menu_Draw (edict_t *ent)
{
	char line[MAX_INFO_STRING], string[2000];
	int i;
	menuitem *menu;
	int size, ystart;
	
	int selected;

	// Keep from updating the menu more than once per frame
	if (ent->client->menumovetime == level.framenum)
		return;
	else
		ent->client->menumovetime = level.framenum;
	
	gi.WriteByte (svc_layout);
	strcpy(string, "xv 32 yv 8 picn inventory ");
	
	if (ent->client->menu == MENU_LOCAL) // Special case
	{
		menu = ent->client->localmenu;
		size = 18;
		ystart = 32; // Start one line up from static menu
	}
	else // Static menu
	{
		menu = menulist[ent->client->menu].menu;
		size = menulist[ent->client->menu].size;
		if (size < 18)
			ystart = 40; // Start one line down from local menu
		else
			ystart = 32;
	}

	// Validate our highlighed selection
	while (!menu[ent->client->menuselect].func)
		ent->client->menuselect = (ent->client->menuselect + 1) % size;
	selected = ent->client->menuselect;

	for (i=0 ; i<size ; i++)
	{
		if (!menu[i].text)
			continue;
		if (i == selected)
		{
			Com_sprintf(line, sizeof(line),
				"xv %i yv %i string \"\x0d%s\" ",
				55, ystart + (i * 8), menu[i].text );
		}
		else
		{
			Com_sprintf(line, sizeof(line),
				"xv %i yv %i string2 \" %s\" ",
				55, ystart + (i * 8), menu[i].text );
		}
		strcat(string, line);
	}

	gi.WriteString (string);

}

void Menu_Blank ()
{
	gi.WriteByte (svc_layout);
	gi.WriteString ("");
}


void Menu_Next (edict_t *ent)
{
	gclient_t	*cl;
	menuitem *menu;
	int size;

	cl = ent->client;

	if (ent->client->menu == MENU_LOCAL) // Special case
	{
		menu = ent->client->localmenu;
		size = 18;
	}
	else // Static menu
	{
		menu = menulist[ent->client->menu].menu;
		size = menulist[ent->client->menu].size;
	}

	cl->menuselect = (cl->menuselect + 1) % size;
	while (!menu[cl->menuselect].func)
		cl->menuselect = (cl->menuselect + 1) % size;

	Menu_Draw (ent);
	gi.unicast (ent, true);
}

void Menu_Prev (edict_t *ent)
{
	gclient_t	*cl;
	menuitem *menu;
	int size;

	cl = ent->client;

	if (ent->client->menu == MENU_LOCAL) // Special case
	{
		menu = ent->client->localmenu;
		size = 18;
	}
	else // Static menu
	{
		menu = menulist[ent->client->menu].menu;
		size = menulist[ent->client->menu].size;
	}

	if (!cl->menuselect)
		cl->menuselect = size - 1;
	else
		cl->menuselect--;

	while (!menu[cl->menuselect].func)
	{
		if (!cl->menuselect)
			cl->menuselect = size - 1;
		else
			cl->menuselect--;
	}

	Menu_Draw (ent);
	gi.unicast (ent, true);
}

void Menu_Use (edict_t *ent)
{
	gclient_t	*cl;
	menuitem *menu;
	int size;

	cl = ent->client;

	if (ent->client->menu == MENU_LOCAL) // Special case
	{
		menu = ent->client->localmenu;
		size = 18;
	}
	else // Static menu
	{
		menu = menulist[ent->client->menu].menu;
		size = menulist[ent->client->menu].size;
	}

	if (menu && size)
	{
		if (menu[cl->menuselect].func)
		{
			ent->client->prevmenu = ent->client->currmenu;
			ent->client->currmenu = menu[cl->menuselect].func;

			ent->client->menulastpage = ent->client->menupage;
			if (ent->client->currmenu == ent->client->prevmenu)
				ent->client->menupage++;
			else
				ent->client->menupage = 0;

			menu[cl->menuselect].func(ent);
		}
	}
	//Menu_Draw (ent);
	//gi.unicast (ent, true);
}
