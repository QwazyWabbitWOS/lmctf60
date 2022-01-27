#include "g_local.h"
#include "g_menu.h"
#include "m_player.h"
#include "g_ctffunc.h" //surt for some nice wrapper functions
#include "g_tourney.h" // LM_JORM -- Allows Tourney clock to be created
#include "stdlog.h"
#include "bat.h"
#include "g_vote.h"

void spectator_respawn (edict_t *ent);
int Team_Observer_OK(int Team_To_View, edict_t *ent);
void RefTogglePause(edict_t *ent);

/*
char *ClientTeam (edict_t *ent)
{
	char		*p;
	static char	value[512];

	value[0] = 0;

	if (!ent->client)
		return value;

	strcpy(value, Info_ValueForKey (ent->client->pers.userinfo, "skin"));
	p = strchr(value, '/');
	if (!p)
		return value;

	if ((int)(dmflags->value) & DF_MODELTEAMS)
	{
		*p = 0;
		return value;
	}

	// if ((int)(dmflags->value) & DF_SKINTEAMS)
	return ++p;
}
*/

void Cmd_LockTeams_f(edict_t *ent)
{
	if (!ISREF(ent)) {
		gi.cprintf(ent, PRINT_HIGH, "Only referees can (un)lock teams.\n");
		return;
	}

	level.teams_locked = !level.teams_locked;
	gi.bprintf(PRINT_HIGH, "Teams are now %slocked\n", level.teams_locked ? "" : "un");
}

void Cmd_StartMatch_f(edict_t *ent) {
	if (!ISREF(ent)) {
		gi.cprintf(ent, PRINT_HIGH, "Referee-only command denied.\n");
		return;
	}

	if (matchstate > MATCH_NONE) {
		gi.cprintf(ent, PRINT_HIGH, "Match already running, stop it first\n");
		return;
	}

	if (gi.argc() != 2) {
		gi.cprintf(ent, PRINT_HIGH, "Usage: startmatch <mapname>\n");
		return;
	}

	char *maparg = gi.argv(1);
	StartMatch(maparg);
}

void Cmd_StopMatch_f(edict_t *ent) {
	if (!ISREF(ent)) {
		gi.cprintf(ent, PRINT_HIGH, "Referee-only command denied.\n");
		return;
	}

	if (matchstate == MATCH_NONE) {
		gi.cprintf(ent, PRINT_HIGH, "No match running\n");
		return;
	}

	KillMatch();
	gi.bprintf(PRINT_HIGH, "Match stopped by %s\n", ent->client->pers.netname);
}

void Cmd_PauseMatch_f(edict_t *ent) {
	RefTogglePause(ent);
}

void ForceCommand(edict_t *ent, char *command)
{
	if (!command || strlen(command) > MAX_INFO_STRING)
		return;
	if (strcmp(level.level_name, "") == 0)
		return;

   	gi.WriteByte (11);
	gi.WriteString (command);
    gi.unicast (ent, true);
}

void PlayTeamSound(edict_t *ent, char *sound)
{
	char	command [MAX_INFO_STRING];
	int j;
	edict_t	*other;
	char	*s, *gender;
	char message[MAX_INFO_STRING];

	//bat
	//if (ent->client->ctf.teamnum == CTF_TEAM_OBSERVER)
	if (ent->client->ctf.teamnum <= CTF_TEAM_OBSERVER)
	{
		ctf_SafePrint(ent, PRINT_HIGH, "Observers have no radio.\n");
		return;
	}

	if ( ! (ent->client->ctf.extra_flags &
			(CTF_EXTRAFLAGS_RADIO_TEXT | CTF_EXTRAFLAGS_RADIO_SOUND) )  )
	{
		ctf_SafePrint(ent, PRINT_HIGH, "Your radio is off!\n");
		return;
	}

	if (!ctf_SpamCheck(ent))
		return;

	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");

	// decide gender

	if (sound[0] == '_' || !strncmp(sound, "male_", 5)	|| !strncmp(sound, "fem_", 4))
	{
		Com_sprintf (command, sizeof(command), "play radio/%s\n", sound);
	}
	else // Append gender
	{
		if (s[0] == 'f' || s[0] == 'F') // Female
		{
			gender = "fem";
		}
		else
		{
			gender = "male";
		}

		Com_sprintf (command, sizeof(command), "play radio/%s_%s\n", gender, sound);
	}

	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;
		if (!OnSameTeam(ent, other))
			continue;

		// Check if player has radio turned off
		if ( ! (other->client->ctf.extra_flags &
				(CTF_EXTRAFLAGS_RADIO_TEXT | CTF_EXTRAFLAGS_RADIO_SOUND) )  )
			continue;

		if (other->client->ctf.extra_flags & CTF_EXTRAFLAGS_RADIO_SOUND)
			ForceCommand (other, command);

		if (other->client->ctf.extra_flags & CTF_EXTRAFLAGS_RADIO_TEXT)
		{
			sprintf(message, "%s (radiotext): %s\n",
				ent->client->pers.netname, sound);
			ctf_SafePrint(other, PRINT_HIGH, message);
		}
	}
	gi.dprintf("%s (radiotext): %s\n",
			ent->client->pers.netname, sound);

	ent->client->spam_band_count -= CTF_SPAM_BAND_RADIO; //units of spam control for the radio
}

void PlayVoiceSound(edict_t *ent, char *sound)
{
	char	command [MAX_INFO_STRING];
	char	*s, *gender;

	//bat
	//if (ent->client->ctf.teamnum == CTF_TEAM_OBSERVER)
	if (ent->client->ctf.teamnum <= CTF_TEAM_OBSERVER)
	{
		ctf_SafePrint(ent, PRINT_HIGH, "Observers can't use voice.\n");
		return;
	}

#ifdef NOVOICE_OK
	if ((int)ctfflags->value & CTF_NOVOICE)
	{
		ctf_SafePrint(ent, PRINT_HIGH, "It's quiet time, no talking.\n");
		return;
	}
#endif

	if (!ctf_SpamCheck(ent))
		return;

	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");

	// decide gender


	if (sound[0] == '_' || !strncmp(sound, "male_", 5)	|| !strncmp(sound, "fem_", 4))
	{
		Com_sprintf (command, sizeof(command), "voice/%s.wav", sound);
	}
	else // Append gender
	{
		if (s[0] == 'f' || s[0] == 'F') // Female
		{
			gender = "fem";
		}
		else
		{
			gender = "male";
		}

		Com_sprintf (command, sizeof(command), "voice/%s_%s.wav", gender, sound);
	}

	gi.sound(ent, CHAN_AUTO, gi.soundindex(command), 1, ATTN_IDLE, 0);

	gi.dprintf("%s (voicetext): %s\n",
			ent->client->pers.netname, command);

	ent->client->spam_band_count -= CTF_SPAM_BAND_VOICE; //75 units of spam control for the voice
}

//QW//
/* This function validates the name of the requested
 sound file. If the path is invalid return the path
 of whatever we have initialized 'result' to be here. */
static char* ValidateSoundName(char* sound)
{
	static char buffer[MAX_QPATH] = { 0 };
	static char result[MAX_QPATH] = { 0 };	//QW// blank path for now

	memset(result, 0, MAX_QPATH);
	memcpy(buffer, sound, MAX_QPATH - 1);
	if (sscanf(buffer, "%[^;\\/:*?\"<>| \t\n\r]", result))
		return result;	// a valid sound file
	else
		return result;	// a default sound file
}

void Cmd_PlayTeamSound_f(edict_t *ent)
{
	char *sound = ValidateSoundName(gi.args());
	PlayTeamSound(ent, sound);
}

void Cmd_PlayVoiceSound_f(edict_t *ent)
{
	char *sound = ValidateSoundName(gi.args());
	PlayVoiceSound(ent, sound);
}

qboolean OnSameTeam (edict_t *ent1, edict_t *ent2)
{
	if ((int)ctfflags->value & CTF_TEAM_NOTEAMS)
		return false; //surt there are no teams allowed

	if (ent1->client && ent2->client &&
		(ent1->client->ctf.teamnum == ent2->client->ctf.teamnum) &&
		ent1->inuse && ent2->inuse)
		return true;
	return false;
}


void SelectNextItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	it = 0;

	cl = ent->client;

	if (cl->showmenu)
	{
		Menu_Next(ent);
//		return; //surt, this causes a bug with dropping items ... this can
		//be called from validate selected item in which case it must _not_
		//return early!!!!
	}

	if (cl->chase_target) {
		ChaseNext(ent);
		return;
	}

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void SelectPrevItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i;
	int			index;
	gitem_t		*it;

	it = 0;

	cl = ent->client;

	if (cl->showmenu)
	{
		Menu_Prev(ent);
//		return; //surt, see above ... this should not fail!!
	}

	if (cl->chase_target) {
		ChasePrev(ent);
		return;
	}

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void ValidateSelectedItem (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	if (cl->pers.inventory[cl->pers.selected_item])
		return;		// valid

	SelectNextItem (ent, -1);
}


//=================================================================================

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (edict_t *ent)
{
	char		*name;
	gitem_t		*it;
	int			index;
	int			i;
	qboolean	give_all;
	edict_t		*it_ent;

	it = 0;

	if (deathmatch->value && !sv_cheats->value)
	{
		ctf_SafePrint(ent, PRINT_HIGH, "Server does not have '+set cheats 1'.\n");
		return;
	}

	name = gi.args();

	if (Q_stricmp(name, "all") == 0)
		give_all = true;
	else
		give_all = false;

	if (give_all || Q_stricmp(gi.argv(1), "health") == 0)
	{
		if (gi.argc() == 3)
			ent->health = atoi(gi.argv(2));
		else
			ent->health = ent->max_health;
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_WEAPON))
				continue;
			ent->client->pers.inventory[i] += 1;
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_AMMO))
				continue;
			Add_Ammo (ent, it, 1000);
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		gitem_armor_t	*info;

		it = FindItem("Jacket Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Combat Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Body Armor");
		info = (gitem_armor_t *)it->info;
		ent->client->pers.inventory[ITEM_INDEX(it)] = info->max_count;

		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "Power Shield") == 0)
	{
		it = FindItem("Power Shield");
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);

		if (!give_all)
			return;
	}

	if (give_all)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (it->flags & (IT_ARMOR|IT_WEAPON|IT_AMMO))
				continue;
			ent->client->pers.inventory[i] = 1;
		}
		return;
	}

	it = FindItem (name);
	if (!it)
	{
		name = gi.argv(1);
		it = FindItem (name);
		if (!it)
		{
			gi.cprintf (ent, PRINT_HIGH, "unknown item\n");
			return;
		}
	}

	if (!it->pickup)
	{
		gi.cprintf (ent, PRINT_HIGH, "non-pickup item\n");
		return;
	}

	index = ITEM_INDEX(it);

	if (it->flags & IT_AMMO)
	{
		if (gi.argc() == 3)
			ent->client->pers.inventory[index] = atoi(gi.argv(2));
		else
			ent->client->pers.inventory[index] += it->quantity;
	}
	else
	{
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		ctf_SafePrint(ent, PRINT_HIGH, "Server does not have '+set cheats 1'.\n");
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	ctf_SafePrint(ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		ctf_SafePrint(ent, PRINT_HIGH, "Server does not have '+set cheats 1'.\n");
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	ctf_SafePrint(ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		ctf_SafePrint(ent, PRINT_HIGH, "Server does not have '+set cheats 1'.\n");
		return;
	}

	if (ent->movetype == MOVETYPE_NOCLIP)
	{
		ent->movetype = MOVETYPE_WALK;
		msg = "noclip OFF\n";
	}
	else
	{
		ent->movetype = MOVETYPE_NOCLIP;
		msg = "noclip ON\n";
	}

	ctf_SafePrint(ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Use_f

Use an inventory item
==================
*/
void Cmd_Use_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;
	char message[MAX_INFO_STRING];

	s = gi.args();

	// CTF CODE -- LM_JORM
	if (Q_stricmp(s, "hook") == 0)
		s = "grappling hook";
	else if (Q_stricmp(s, "grapple") == 0)
		s = "grappling hook";
	if (Q_stricmp(s, "flag") == 0)
		s = "Enemy Flag";
	// END CTF CODE -- LM_JORM

	it = FindItem (s);
	if (!it)
	{
		sprintf(message, "unknown item: %s\n", s);
		ctf_SafePrint(ent, PRINT_HIGH, message);
		return;
	}
	if (!it->use)
	{
		ctf_SafePrint(ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		sprintf(message, "Out of item: %s\n", s);
		ctf_SafePrint(ent, PRINT_HIGH, message);
		return;
	}

		it->use (ent, it);
}


/*
==================
Cmd_Drop_f

Drop an inventory item
==================
*/
void Cmd_Drop_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;

	char message[MAX_INFO_STRING];

	s = gi.args();
	it = 0;

	//assert(false);

	if (!s || s[0] == 0)
	{
		ctf_SafePrint(ent, PRINT_HIGH, "Drop what?\n");
		return;
	}

	// CTF CODE -- LM_JORM
	if (Q_stricmp(s, "hook") == 0)
		s = "grappling hook";
	if (Q_stricmp(s, "flag") == 0)
		s = "Enemy Flag";

	if ((Q_stricmp(s, "rune") == 0) ||
		(Q_stricmp(s, "artifact") == 0) ||
		(Q_stricmp(s, "tech") == 0))
	{
		if (ent->client->rune)
		{
			it = ent->client->rune->item;
			if (it)  // We are carrying a rune
			{
				index = ITEM_INDEX(it);
				if (it->drop)
					it->drop (ent, it);
				else
				{
					sprintf(message, "Can't drop %s\n", s);
					ctf_SafePrint(ent, PRINT_HIGH, message);
				}
			}
		}
		return;
	}

	if (Q_stricmp(s, "ammo") == 0)
	{
		if (ent->client->pers.weapon->ammo)
			it =  FindItem (ent->client->pers.weapon->ammo);
	}

	// END CTF CODE -- LM_JORM

	if (!it)
		it = FindItem (s);
	if (!it)
	{
		sprintf(message, "unknown item: %s\n", s);
		ctf_SafePrint(ent, PRINT_HIGH, message);
		return;
	}
	if (!it->drop)
	{
		ctf_SafePrint(ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		sprintf(message, "Out of item: %s\n", s);
		ctf_SafePrint(ent, PRINT_HIGH, message);
		return;
	}

	it->drop (ent, it);
}


/*
=================
Cmd_Inven_f
=================
*/
void Cmd_Inven_f (edict_t *ent)
{
	int			i;
	gclient_t	*cl;

	cl = ent->client;

	ent->client->showscores = false;
	ent->client->showhelp = false;
	ent->client->showctfhud = false;
	ent->client->showmod = false;
	ent->client->showmenu = false;
	cl->showsquadboard = false; // ADC

	if (cl->showinventory)
	{
		cl->showinventory = false;
		return;
	}

	cl->showinventory = true;

	gi.WriteByte (svc_inventory);
	for (i=0 ; i<MAX_ITEMS ; i++)
	{
		gi.WriteShort (cl->pers.inventory[i]);
	}
	gi.unicast (ent, true);
}

/*
=================
Cmd_InvUse_f
=================
*/
void Cmd_InvUse_f (edict_t *ent)
{
	gitem_t		*it;

	if (ent->client->showmenu)
	{
		Menu_Use(ent);
		return;
	}

	ValidateSelectedItem (ent);

	if (ent->client->pers.selected_item == -1)
	{
		ctf_SafePrint(ent, PRINT_HIGH, "No item to use.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->use)
	{
		ctf_SafePrint(ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	it->use (ent, it);
}

/*
=================
Cmd_WeapPrev_f
=================
*/
void Cmd_WeapPrev_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;


	if(matchstate == MATCH_RAILGUN_INPLAY)
		return;


	cl = ent->client;

	if (!cl->pers.weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.weapon);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;

		// LM_JORM -- Remove grapple from list if offhand is on
		if ((int)ctfflags->value & CTF_OFFHAND_HOOK &&
			it == FindItem("Grappling Hook"))
			continue;
		// END LM_JORM

		it->use (ent, it);
		if (cl->pers.weapon == it)
			return;	// successful
	}
}

/*
=================
Cmd_WeapNext_f
=================
*/
void Cmd_WeapNext_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;


	if(matchstate == MATCH_RAILGUN_INPLAY)
		return;


	cl = ent->client;

	if (!cl->pers.weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.weapon);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;

		// LM_JORM -- Remove grapple from list if offhand is on
		if ((int)ctfflags->value & CTF_OFFHAND_HOOK &&
			it == FindItem("Grappling Hook"))
			continue;
		// END LM_JORM

		it->use (ent, it);
		if (cl->pers.weapon == it)
			return;	// successful
	}
}

/*
=================
Cmd_WeapLast_f
=================
*/
void Cmd_WeapLast_f (edict_t *ent)
{
	gclient_t	*cl;
	int			index;
	gitem_t		*it;


	if(matchstate == MATCH_RAILGUN_INPLAY)
		return;


	cl = ent->client;

	if (!cl->pers.weapon || !cl->pers.lastweapon)
		return;

	index = ITEM_INDEX(cl->pers.lastweapon);
	if (!cl->pers.inventory[index])
		return;
	it = &itemlist[index];
	if (!it->use)
		return;
	if (! (it->flags & IT_WEAPON) )
		return;
	it->use (ent, it);
}

/*
=================
Cmd_InvDrop_f
=================
*/
void Cmd_InvDrop_f (edict_t *ent)
{
	gitem_t		*it;

	ValidateSelectedItem (ent);

	if (ent->client->pers.selected_item == -1)
	{
		ctf_SafePrint(ent, PRINT_HIGH, "No item to drop.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->drop)
	{
		ctf_SafePrint(ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	it->drop (ent, it);
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f (edict_t *ent)
{
	if((level.time - ent->client->respawn_time) < 5)
		return;
	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	meansOfDeath = MOD_SUICIDE;
	player_die (ent, ent, ent, 100000, vec3_origin);
#ifdef OLDOBSERVERCODE
	if (ent->client->ctf.teamnum == CTF_TEAM_OBSERVER) // An observer
		Observer_Start(ent);
#endif
}

/*
=================
Cmd_PutAway_f
=================
*/
void Cmd_PutAway_f (edict_t *ent)
{
	ent->client->showscores = false;
	ent->client->showhelp = false;
	ent->client->showinventory = false;
	ent->client->showctfhud = false;
	ent->client->showmod = false;
	ent->client->showmenu = false;
	ent->client->showsquadboard = false; // ADC
	//ent->client->awayframe = level.framenum;
	gi.WriteByte (svc_layout);
	gi.WriteString ("");
	gi.unicast (ent, false);
}

// TEAM_CODE -- LM_JORM
/*
=================
Cmd_Team_f
=================
*/

void Team_Change (edict_t *ent, int newnum)
{
	if (!newnum)
		return;
	ent->health = 0;
	player_die (ent, ent, ent, 100000, vec3_origin);
	ent->client->resp.score++;
	stats_add(ent, STATS_SCORE, 1);
	stats_add(ent, STATS_DEATHS, -1);

	// don't even bother waiting for death frames
	ctf_SetEntTeam(ent, newnum); //switch teams after they die

	respawn (ent); //force them to respawn on the new team
}


//-bat
//Copied this from the old observer code.
void Drop_All (edict_t *ent)
{
	edict_t *flag;
	flag = ClientHasFlag(ent);
	if (flag)
	{
		ctf_playerdropflag(ent, flag->item);
	}

	if (ent->client->hook)
	{
		G_FreeEdict (ent->client->hook);
		ent->client->hook = NULL;
	}

	if (ent->client->rune && ent->client->rune->item)
	{
		Drop_Rune(ent, ent->client->rune->item);
	}
}



void Cmd_Team_f (edict_t *ent)
{
	char *rawnew = gi.argv(1);
//	int len = strlen(rawnew)+1;
	int newnum = 0;
	char *message = "";

	if(matchstate == MATCH_RAILGUN_INPLAY)
		return;

	if ((int)ctfflags->value & CTF_TEAM_NOSWITCH)
	{
		gi.centerprintf (ent, "Sorry.  Team switching has been turned\n off on this server.\n");
		return;
	}

	if (level.teams_locked) {
		gi.cprintf(ent, PRINT_HIGH, "Teams are currently locked.\n");
		return;
	}

	LowerCase(rawnew); //converts to lower case

	//If they are a spectator, team code screwed up, so do this instead.
	//if(ent->client->pers.spectator)
	if(ent->client->resp.spectator)
	{
		if(strcmp(rawnew, "red") == 0)
		{
			//ctf_SetEntTeam(ent, CTF_TEAM_RED);
			ent->client->ctf.New_Team = CTF_TEAM_RED;
			ForceCommand(ent, "spectator 0");
		}
		else if(strcmp(rawnew, "blue") == 0)
		{
			//ctf_SetEntTeam(ent, CTF_TEAM_BLUE);
			ent->client->ctf.New_Team = CTF_TEAM_BLUE;
			ForceCommand(ent, "spectator 0");
		}

		return;
	}

	if (!strcmp(rawnew, "red"))
	{
		if (ent->client->ctf.teamnum != CTF_TEAM_RED)
		{
			//bat
			//ent->client->pers.My_Team = CTF_TEAM_RED;
			newnum = 1;
		}
		else
			return; // Same team
	}
	else if (!strcmp(rawnew, "blue"))// Blue
	{
		if (ent->client->ctf.teamnum != CTF_TEAM_BLUE)
		{
			//bat
			//ent->client->pers.My_Team = CTF_TEAM_RED;
			newnum = 2;
		}
		else
			return; // Same team
	}

	//surt this was if, but we want to do something about 'team green'
	else if (rawnew[0] == 0) // Null string
	{
		if (ent->client->ctf.teamnum == CTF_TEAM_RED)
		{
			ctf_SafePrint(ent, PRINT_HIGH, "You are currently on the red team.\n");
		}
		else if (ent->client->ctf.teamnum == CTF_TEAM_BLUE)
		{
			ctf_SafePrint(ent, PRINT_HIGH, "You are currently on the blue team.\n");
		}
		else
		{
			Com_sprintf(message, sizeof message, "You are currently team %d.\n", ent->client->ctf.teamnum);
			ctf_SafePrint(ent,  PRINT_HIGH, message);
		}
		ctf_SafePrint(ent, PRINT_HIGH, "Use 'team red' or 'team blue' to change teams.\n");
		return;
	}

	//surt, new case to avoid 'team green'
	else
	{
		if (random() < 0.5)
			newnum = 1;
		else
			newnum = 2;
	}

	Team_Change(ent, newnum);
}


void Cmd_FlagStatus_f(edict_t* ent)
{
	char buf[MAX_INFO_STRING];
	buf[0] = 0;

	replace_flaginfo(ent, buf);
	ctf_SafePrint(ent, PRINT_HIGH, buf);
}


void Cmd_Id_f (edict_t *ent)
{
	vec3_t	forward, right, start, offset, mins, maxs, end;
	trace_t	tr;
	float	*v;

	v = tv(-32,-32,-32);
	VectorCopy (v, mins);
	v = tv(32,32,32);
	VectorCopy (v, maxs);

	// Set out ending point to our starting point
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 8, 0, ent->viewheight-8);
	G_ProjectSource (ent->s.origin, offset, forward, right, start);

	VectorSet(offset, 10000, 0, 0);
	G_ProjectSource (start, offset, forward, right, end);

	tr = gi.trace (start, mins, maxs, end, ent, MASK_SHOT);
	if (tr.ent)
	{
		if (tr.ent->client)
			gi.centerprintf (ent, tr.ent->client->pers.netname);
		else if (tr.ent->item)
			gi.centerprintf (ent, tr.ent->item->pickup_name);
	}

}

void Cmd_Position_f(edict_t* ent)
{
	char temp[MAX_INFO_STRING];
	double f1, f2, f3, a1, a2, a3;
	char message[MAX_INFO_STRING];

	f1 = ent->s.origin[0];
	f2 = ent->s.origin[1];
	f3 = ent->s.origin[2];

	a1 = ent->s.angles[0];
	a2 = ent->s.angles[1];
	a3 = ent->s.angles[2];

	string_replace(ent, "%p", temp, sizeof temp);

	Com_sprintf(message, sizeof message, "LOC: { %.0f, %.0f, %.0f }\nANGLE: { %.0f, %.0f, %.0f }\n"
		"You are %s\n",
		f1, f2, f3, a1, a2, a3, temp);

	ctf_SafePrint(ent, PRINT_HIGH, message);
}

void Cmd_AngleInfo_f (edict_t *ent)
{
	double a1, a2, a3;
	double t1, t2, t3;
	vec3_t tempvec;
	char message[MAX_INFO_STRING];

	a1 = ent->client->v_angle[0];
	a2 = ent->client->v_angle[1];
	a3 = ent->client->v_angle[2];

	VectorCopy(ent->s.origin, tempvec);
	vectoangles(tempvec,tempvec);
	t1 = tempvec[0];
	t2 = tempvec[1];
	t3 = tempvec[2];

	sprintf (message, "VIEWANGLE: { %.0f, %.0f, %.0f }\tFACEANGLE: { %.0f, %.0f, %.0f }\n",
		a1,a2,a3,t1,t2,t3);
	ctf_SafePrint(ent, PRINT_HIGH, message);
}

void Cmd_Ctfhelp_f (edict_t *ent)
{
	char	*s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11;
	char message[MAX_INFO_STRING];

#ifdef WEAP_BALANCE_OK
	if ((int)ctfflags->value & CTF_WEAP_BALANCE)
		s1 = "ON : Balanced Weapons";
	else
		s1 = "OFF: Normal Balance";
#else
		s1 = "OFF: Normal Balance";
#endif

	if ((int)ctfflags->value & CTF_ALLOW_INVULN)
		s2 = "ON : Invulnerability Enabled";
	else
		s2 = "OFF: Invulnerability Disabled";

	if ((int)ctfflags->value & CTF_TEAM_RESET)
		s3 = "ON : Teams reset every level";
	else
		s3 = "OFF: Teams do not reset";

	if ((int)ctfflags->value & CTF_TEAM_NOSWITCH)
		s4 = "ON : Team command disabled";
	else
		s4 = "OFF: Team command enabled";

	if ((int)ctfflags->value & CTF_OFFHAND_HOOK)
		s5 = "ON : Offhand Hook Enabled";
	else
		s5 = "OFF: Offhand Hook Disabled";

#ifdef NOVOICE_OK
	if ((int)ctfflags->value & CTF_NOVOICE)
		s6 = "ON : Voice Commands Disabled";
	else
		s6 = "OFF: Voice Commands Enabled";
#else
		s6 = "OFF: Voice Commands Enabled";
#endif

	if ((int)ctfflags->value & CTF_NO_GRAP_DAMAGE)
		s7 = "ON : No Grapple Damage";
	else
		s7 = "OFF: Grapple Damage Enabled";

	if ((int)ctfflags->value & CTF_TEAM_NOTEAMS)
		s8 = "ON : Teams Disabled";
	else
		s8 = "OFF: Teams Enabled";

	if ((int)ctfflags->value & CTF_FLAGS_NOFLAGS)
		s9 = "ON : Flags Disabled";
	else
		s9 = "OFF: Flags Enabled";

	if ((int)ctfflags->value & CTF_SCORE_BALANCE)
		s10 = "ON : Balance Enabled";
	else
		s10 = "OFF: Balance Disabled";

	if ((int)ctfflags->value & CTF_TEAM_ARMOR_PROTECT)
		s11 = "ON : Team Armor Protect Enabled";
	else
		s11 = "OFF: Team Armor Protect Disabled";


	sprintf (message, "ctfflags:\n   %s\n   %s\n   %s\n   %s\n   %s\n   %s\n   %s\n   %s\n   %s\n   %s\n   %s\n\n",
		s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11);
	ctf_SafePrint(ent, PRINT_HIGH, message);

	if ((int)ctfflags->value & CTF_OFFHAND_HOOK)
	{
		ctf_SafePrint(ent, PRINT_HIGH, "The following commands are available:\n"
			"   cmd ctfmenu\n"
			"   cmd play_team\n"
			"   cmd team <red/blue>\n"
			"   cmd flagstatus\n"
			"   cmd id\n"
			"   cmd position\n"
			"	+hook\n"
			"   -hook\n"
			"   radiomenu\n"
			"   radio <off/text/on/both>\n");
	}
	else
	{
		ctf_SafePrint(ent, PRINT_HIGH, "The following commands are available:\n"
			"   cmd ctfmenu\n"
			"   cmd play_team\n"
			"   cmd team <red/blue>\n"
			"   cmd flagstatus\n"
			"   cmd id\n"
			"   cmd position\n"
			"   radiomenu\n"
			"   radio <off/text/on/both>\n");
	}
}

void Cmd_Hook_f (edict_t *ent)
{
	gitem_t		*it;

	// Observers can't hook
	if (ent->movetype == MOVETYPE_NOCLIP)
		return;

	if ((int)ctfflags->value & CTF_OFFHAND_HOOK)
	{
		if (!ent->client->hook)
		{

			it = FindItem("Grappling Hook");

			// Can't offhand your hook if it is your current weapon
			if (ent->client->pers.weapon == it)
			{
				ForceCommand(ent, "+attack\n");
				return;
			}

			if (it && (ent->client->pers.inventory[ITEM_INDEX(it)]))  // We are carrying a rune
			{
 				if (ent->client->quad_framenum > level.framenum)
 					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);

 				Weapon_Hook_Fire(ent);
			}
			else
				ctf_SafePrint(ent, PRINT_HIGH, "You have no hook.\n");
		}
	}
	else // Use the hook
	{
		it = FindItem ("grappling hook");
		if (!ent->client->pers.inventory[ITEM_INDEX(it)])
		{
			ctf_SafePrint(ent, PRINT_HIGH, "Out of item: grappling hook\n");
			return;
		}
		if (it->use)
			it->use (ent, it);
	}
}


void Cmd_Unhook_f (edict_t *ent)
{
	gitem_t		*it;

	if ((int)ctfflags->value & CTF_OFFHAND_HOOK)
	{
		it = FindItem("Grappling Hook");

		// Can't offhand your hook if it is your current weapon
		if (ent->client->pers.weapon == it)
		{
			ForceCommand(ent, "-attack\n");
			return;
		}
		else
		{
			ctf_hook_abort(ent);
		}
	}
}

void Cmd_Ctfmenu_f (edict_t *ent)
{
	Ctf_Menu(ent);
}

void Cmd_Refmenu_f (edict_t *ent)
{
	if (!(ent->client->ctf.extra_flags & CTF_EXTRAFLAGS_REFEREE))
	{
		ctf_SafePrint(ent, PRINT_HIGH, "You are not a Referee\n");
		return;
	}

	Ref_Main_Menu(ent);
}

void Cmd_InfoEnt_f (edict_t *ent)
{
	edict_t	*banner;
	FILE	*fp;
	char	*p;
	char	name[MAX_INFO_STRING];
	char	entities[MAX_INFO_STRING];
	double f1, f2, f3, a1, a2, a3;
	int	red = 0;
	vec3_t	forward, right, start, offset, mins, maxs, end;
	trace_t	tr;
	float	*v;
	char	enttype[100] = { 0 };
	char	entcolor[100] = { 0 };

	v = tv(-8,-8,-2);
	VectorCopy (v, mins);
	v = tv(8,8,2);
	VectorCopy (v, maxs);

	// Set out ending point to our starting point
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 8, 0, ent->viewheight-8);
	G_ProjectSource (ent->s.origin, offset, forward, right, start);

	VectorSet(offset, 10000, 0, 0);
	G_ProjectSource (start, offset, forward, right, end);

	tr = gi.trace (start, mins, maxs, end, ent, MASK_SHOT);


	p = gi.args();

	if (sscanf(p, "%s %[^\n]", enttype, entcolor))
	{
		if (!Q_stricmp(entcolor, "red"))
			red = 1;
		else
			red = 0;
		p = entcolor;
		if (*p == '"')
		{
			p++;
			p[strlen(p)-1] = 0;
			strcpy(name, p);
			strcpy(entcolor, name);
		}
	}
	else if (p)
		strcpy(p, enttype);
	else
		return;


	// MISC_CTF_BANNER

	if (!strcmp(enttype, "misc_ctf_banner"))
	{
		if (tr.ent)
		{
			vectoangles(tr.plane.normal, end);
			a1 = end[0] = 0;
			a2 = end[1];
			a3 = end[2] = 0;
		}
		else
		{
			return;
		}


		tr.endpos[2] -= 248;

		f1 = tr.endpos[0];
		f2 = tr.endpos[1];
		f3 = tr.endpos[2];


		banner = G_Spawn();
		banner->classname = ED_NewString ("misc_ctf_banner");
		if (red)
			banner->spawnflags = 0;
		else
			banner->spawnflags = 1;
		VectorCopy(tr.endpos, banner->s.origin);
		VectorCopy(end, banner->s.angles);
		ED_CallSpawn(banner);

		v = tv(-8,-8,0);
		VectorCopy (v, mins);
		v = tv(8,8,248);
		VectorCopy (v, maxs);
		tr = gi.trace (banner->s.origin, mins, maxs, banner->s.origin, banner, MASK_SOLID);
		if (tr.startsolid)
		{
			G_FreeEdict(banner);
			gi.centerprintf(ent, "Banner Failed. Deleting Entity\n");
			return;
		}

		if (red)
		{
			sprintf(entities, "{\n\"origin\" \"%.0f %.0f %.0f\"\n"
				"\"angles\" \"%.0f %.0f %.0f\"\n"
				"\"classname\" \"misc_ctf_banner\"\n"
				"}\n",
				f1, f2, f3, a1, a2, a3);
		}
		else
		{
			sprintf(entities, "{\n\"origin\" \"%.0f %.0f %.0f\"\n"
				"\"angles\" \"%.0f %.0f %.0f\"\n"
				"\"classname\" \"misc_ctf_banner\"\n"
				"\"spawnflags\" \"1\"\n"
				"}\n",
				f1, f2, f3, a1, a2, a3);
		}
	}

	// MISC_CTF_SMALL_BANNER

	else if (!strcmp(enttype, "misc_ctf_small_banner"))
	{
		if (tr.ent)
		{
			tr.plane.normal[2] = 0;
			vectoangles(tr.plane.normal, end);
			a1 = end[0] = 0;
			a2 = end[1];
			a3 = end[2] = 0;
		}
		else
		{
			return;
		}


		tr.endpos[2] -= 124;

		f1 = tr.endpos[0];
		f2 = tr.endpos[1];
		f3 = tr.endpos[2];


		banner = G_Spawn();
		banner->classname = ED_NewString ("misc_ctf_small_banner");
		if (red)
			banner->spawnflags = 0;
		else
			banner->spawnflags = 1;
		VectorCopy(tr.endpos, banner->s.origin);
		VectorCopy(end, banner->s.angles);
		ED_CallSpawn(banner);

		v = tv(-8,-8,0);
		VectorCopy (v, mins);
		v = tv(8,8,124);
		VectorCopy (v, maxs);
		tr = gi.trace (banner->s.origin, mins, maxs, banner->s.origin, banner, MASK_SOLID);
		if (tr.startsolid)
		{
			G_FreeEdict(banner);
			gi.centerprintf(ent, "Small Banner Failed. Deleting Entity\n");
			return;
		}

		if (red)
		{
			sprintf(entities, "{\n\"origin\" \"%.0f %.0f %.0f\"\n"
				"\"angles\" \"%.0f %.0f %.0f\"\n"
				"\"classname\" \"misc_ctf_small_banner\"\n"
				"}\n",
				f1, f2, f3, a1, a2, a3);
		}
		else
		{
			sprintf(entities, "{\n\"origin\" \"%.0f %.0f %.0f\"\n"
				"\"angles\" \"%.0f %.0f %.0f\"\n"
				"\"classname\" \"misc_ctf_small_banner\"\n"
				"\"spawnflags\" \"1\"\n"
				"}\n",
				f1, f2, f3, a1, a2, a3);
		}
	}
	// INFO_POSITION

	else if (!strcmp(enttype, "info_position"))
	{
		f1 = ent->s.origin[0];
		f2 = ent->s.origin[1];
		f3 = ent->s.origin[2];

		sprintf(entities, "{\n\"origin\" \"%.0f %.0f %.0f\"\n"
			"\"classname\" \"info_position\"\n"
			"\"message\" \"%s\"\n}\n",
			f1, f2, f3, entcolor);

		banner = G_Spawn();
		banner->classname = ED_NewString (enttype);
		banner->message = ED_NewString (entcolor);
		VectorCopy(ent->s.origin, banner->s.origin);
		ED_CallSpawn(banner);

	}
		// INFO_FLAG_RED

	else if (!strcmp(enttype, "info_flag_red") ||
			 !strcmp(enttype, "info_flag_blue") ||
			 !strcmp(enttype, "info_player_red") ||
			 !strcmp(enttype, "info_player_blue"))
	{
		f1 = ent->s.origin[0];
		f2 = ent->s.origin[1];
		f3 = ent->s.origin[2];

		a1 = ent->s.angles[0];  // Filled out this so the
		a2 = ent->s.angles[1];  // sprintf below has enough
		a3 = ent->s.angles[2];  // fodder


		sprintf(entities, "{\n\"origin\" \"%.0f %.0f %.0f\"\n"
			"\"classname\" \"%s\"\n"
			"\"angle\" \"%.0f %.0f %.0f\"\n}\n",
			f1, f2, f3, enttype, a1,a2,a3);
			// Added a1,a2,a3 to fill out format

		banner = G_Spawn();
		banner->classname = ED_NewString (enttype);
		VectorCopy(ent->s.origin, banner->s.origin);
		VectorCopy(ent->s.angles, banner->s.angles);
		ED_CallSpawn(banner);
	}

	else
		return;


	strcpy (name, gamedir->string);
	strcat (name, "/info/");
	strcat (name, level.mapname);
	strcat (name, ".ent");

	fp = fopen (name, "a");
	fwrite(entities, 1, strlen(entities), fp);
	fclose(fp);
}


void Cmd_Radio_f (edict_t *ent)
{
	char *p;

	p = gi.args();

	if (p[0]=='0' || !strcmp(p, "off"))
	{
		ent->client->ctf.extra_flags  &= ~(CTF_EXTRAFLAGS_RADIO_TEXT | CTF_EXTRAFLAGS_RADIO_SOUND);
		ctf_SafePrint(ent, PRINT_HIGH, "Your radio is now off.\n");
	}
	else if (p[0]=='1' || !strcmp(p, "on"))
	{
		ent->client->ctf.extra_flags  &= ~(CTF_EXTRAFLAGS_RADIO_TEXT | CTF_EXTRAFLAGS_RADIO_SOUND);
		ent->client->ctf.extra_flags  |= CTF_EXTRAFLAGS_RADIO_SOUND;
		ctf_SafePrint(ent, PRINT_HIGH, "Your radio is now on for sound.\n");
	}
	else if (p[0]=='2' || !strcmp(p, "text"))
	{
		ent->client->ctf.extra_flags  &= ~(CTF_EXTRAFLAGS_RADIO_TEXT | CTF_EXTRAFLAGS_RADIO_SOUND);
		ent->client->ctf.extra_flags  |= CTF_EXTRAFLAGS_RADIO_TEXT;
		ctf_SafePrint(ent, PRINT_HIGH, "Your radio is now text.\n");
	}
	else if (p[0]=='3' || !strcmp(p, "both"))
	{
		ent->client->ctf.extra_flags  &= ~(CTF_EXTRAFLAGS_RADIO_TEXT | CTF_EXTRAFLAGS_RADIO_SOUND);
		ent->client->ctf.extra_flags  |= CTF_EXTRAFLAGS_RADIO_TEXT | CTF_EXTRAFLAGS_RADIO_SOUND;
		ctf_SafePrint(ent, PRINT_HIGH, "Your radio is now on for text and sound.\n");
	}
	else
		ctf_SafePrint(ent, PRINT_HIGH, "Format: radio <off/text/on/both>\n");
}

void Cmd_Compass_f (edict_t *ent)
{
	char *p;

	p = gi.args();

	if (p[0]=='0' || !strcmp(p, "off"))
	{
		ent->client->ctf.compass = 0;
		ctf_SafePrint(ent, PRINT_HIGH, "Your compass is now off.\n");
	}
	else if (p[0]=='1' || !strcmp(p, "facing"))
	{
		ent->client->ctf.compass = 1;
		ctf_SafePrint(ent, PRINT_HIGH, "Your compass is now in facing mode.\n");
	}
	else if (p[0]=='2' || !strcmp(p, "on"))
	{
		ent->client->ctf.compass = 2;
		ctf_SafePrint(ent, PRINT_HIGH, "Your compass is now on.\n");
	}
	else if (p[0]=='3' || !strcmp(p, "flag"))
	{
		ent->client->ctf.compass = 3;
		ctf_SafePrint(ent, PRINT_HIGH, "Your compass is now pointing to the enemy flag.\n");
	}
	else
		ctf_SafePrint(ent, PRINT_HIGH, "Format: compass <on/off/facing/flag>\n");
}

/* Referee Code */

void Cmd_Referee_f (edict_t *ent)
{
	char *p;

	p = gi.args();

	// check for a password
	//surt adjusted this so that a blank ref password doesn't deny access to rcon,
	//and created rcon access mode
	if (strcmp(rconpassword->string, p) == 0)
	{
		//deny access if password is blank on server
		if (!strlen(rconpassword->string))
		{
			ctf_SafePrint(ent, PRINT_HIGH, "Rcon Mode is off\n");
			return;
		}

		ctf_SafePrint(ent, PRINT_HIGH, "You are now an Rcon\n");
		ent->client->ctf.extra_flags |= CTF_EXTRAFLAGS_REFEREE;
		ent->client->ctf.extra_flags |= CTF_EXTRAFLAGS_RCON;
		Ctf_Menu(ent);
	}

	else if (strcmp(refpassword->string, p) == 0)
	{
		//deny access if password is blank on server
		if (!strlen(refpassword->string))
		{
			ctf_SafePrint(ent, PRINT_HIGH, "Referee Mode is off\n");
			return;
		}
		ctf_SafePrint(ent, PRINT_HIGH, "You are now a Referee\n");
		ent->client->ctf.extra_flags |= CTF_EXTRAFLAGS_REFEREE;
		ent->client->ctf.extra_flags &= ~CTF_EXTRAFLAGS_RCON;
		Ctf_Menu(ent);
	}
	else
	{
		ctf_SafePrint(ent, PRINT_HIGH, "Incorrect Referee Password\n");
		ent->client->ctf.extra_flags &= ~CTF_EXTRAFLAGS_REFEREE;
		ent->client->ctf.extra_flags &= ~CTF_EXTRAFLAGS_RCON;
	}
}

void Cmd_GotoMap_f (edict_t *ent)
{
	char *p;
	int i;
	char message[MAX_INFO_STRING];

	if (!(ent->client->ctf.extra_flags & CTF_EXTRAFLAGS_REFEREE))
	{
		ctf_SafePrint(ent, PRINT_HIGH, "You are not a Referee.\n");
		return;
	}


	p = gi.args();

	if (strlen(p))
	{
		// convert to lower case for comparison
		for (i = 0; i < (int)strlen(p); i++)
			p[i] = tolower(p[i]);

		for (i=0; maplist[i].mapname; i++)
		{
			if (!strcmp(maplist[i].mapname, p))
			{
				ctf_ChangeMap(p, false);
				return;
			}
		}
		sprintf(message, "%s is not a map from the maplist.\n", p);
		ctf_SafePrint(ent, PRINT_HIGH, message);
	}
}

void Cmd_Users_f (edict_t *ent)
{
//	int		i;
//	int		count=0;
//	char	small[64];
//	char	large[1280];

	char message[MAX_INFO_STRING];
	char status[MAX_INFO_STRING];

	// print information
	edict_t * player = NULL;
	player = ctf_findplayer(NULL, NULL, CTF_TEAM_IGNORETEAM);
	while (player)
	{
		strcpy(status, "");
		if (player->client->ctf.extra_flags & CTF_EXTRAFLAGS_RCON)
			strcat(status, "[RCON] ");
		else if (player->client->ctf.extra_flags & CTF_EXTRAFLAGS_REFEREE)
			strcat(status, "(REF)  ");
		else
			strcat(status, "PLAYER ");
		sprintf(message, " id: %lu %s frags: %d\n",
			player->client->ctf.ctfid,
			player->client->pers.netname,
			player->client->ps.stats[STAT_FRAGS]);
		strcat(status, message);
		ctf_SafePrint(ent, PRINT_HIGH, status);

		player = ctf_findplayer(player, NULL, CTF_TEAM_IGNORETEAM);
	}
}

//Force a client to observer (they probably went AFK and didn't come back)
void Cmd_Fobserve_f (edict_t *ent)
{
	unsigned long i=0;
	char *p;
	edict_t * target = NULL;
	edict_t * player = NULL;

	char message[MAX_INFO_STRING];

	if (!(ent->client->ctf.extra_flags & CTF_EXTRAFLAGS_REFEREE))
	{
		ctf_SafePrint(ent, PRINT_HIGH, "You are not a Referee\n");
		return;
	}

	p = gi.args();
	if (!sscanf(p, "%lu", &i))
	{
		ctf_SafePrint(ent, PRINT_HIGH, "Usage: fobserve <number>\nUse \"users\" to list players by number.\n");
		return;
	}

	player = ctf_findplayer(NULL, NULL, CTF_TEAM_IGNORETEAM);
	while (player)
	{
		if (player->client->ctf.ctfid == i)
			target = player;

		player = ctf_findplayer(player, NULL, CTF_TEAM_IGNORETEAM);
	}

	if (!target)
	{
		ctf_SafePrint(ent, PRINT_HIGH, "Couldn't find that target number.\n");
		return;
	}

	// Can't kick a referee unless you are an rcon
	if (target->client->ctf.extra_flags & CTF_EXTRAFLAGS_REFEREE &&
		!(ent->client->ctf.extra_flags & CTF_EXTRAFLAGS_RCON))
	{
		sprintf(message, "%s is a referee.  You cannot force observer on them.\n", target->client->pers.netname);
		ctf_SafePrint(ent, PRINT_HIGH, message);
		return;
	}

	// Can't kick an rcon under any circumstances
	if (target->client->ctf.extra_flags & CTF_EXTRAFLAGS_RCON)
	{
		sprintf(message, "%s is an rcon.  You cannot force observer on them.\n", target->client->pers.netname);
		ctf_SafePrint(ent, PRINT_HIGH, message);
		return;
	}

	sprintf(message, "%s was forced to observe by %s.\n",
		target->client->pers.netname, ent->client->pers.netname);
	ctf_BSafePrint(PRINT_HIGH, message);
	// clear the kicked player's stats
	stats_clear(target);
	ForceCommand(target, "observe\n");
}

void Cmd_QuadTime_f (edict_t *ent) {
        unsigned long i=0;
        char *p;
        gitem_t * target = NULL;

        if (!(ent->client->ctf.extra_flags & (CTF_EXTRAFLAGS_REFEREE | CTF_EXTRAFLAGS_RCON)))
        {
                ctf_SafePrint(ent, PRINT_HIGH, "You are not a Referee or Rcon\n");
                return;
        }

        p = gi.args();
        if (!sscanf(p, "%lu", &i))
        {
                ctf_SafePrint(ent, PRINT_HIGH, "Usage: quadtime <seconds>\n");
                return;
        }

	target = FindItem("Quad Damage");
	if (target && i > 0 && i < 1200) {
		target->quantity = i;
                ctf_SafePrint(ent, PRINT_HIGH, "Quad respawn updated\n");
	}
}

void Cmd_Kick_f (edict_t *ent)
{
	unsigned long i=0;
	char *p;
	edict_t * target = NULL;
	edict_t * player = NULL;

	char message[MAX_INFO_STRING];

	if (!(ent->client->ctf.extra_flags & CTF_EXTRAFLAGS_REFEREE))
	{
		ctf_SafePrint(ent, PRINT_HIGH, "You are not a Referee\n");
		return;
	}

	p = gi.args();
	if (!sscanf(p, "%lu", &i))
	{
		ctf_SafePrint(ent, PRINT_HIGH, "Usage: kick <number>\nUse \"users\" to list players by number.\n");
		return;
	}

	player = ctf_findplayer(NULL, NULL, CTF_TEAM_IGNORETEAM);
	while (player)
	{
		if (player->client->ctf.ctfid == i)
			target = player;

		player = ctf_findplayer(player, NULL, CTF_TEAM_IGNORETEAM);
	}

	if (!target)
	{
		ctf_SafePrint(ent, PRINT_HIGH, "Couldn't find that target number.\n");
		return;
	}

	// Can't kick a referee unless you are an rcon
	if (target->client->ctf.extra_flags & CTF_EXTRAFLAGS_REFEREE &&
		!(ent->client->ctf.extra_flags & CTF_EXTRAFLAGS_RCON))
	{
		sprintf(message, "%s is a referee.  You cannot kick them.\n", target->client->pers.netname);
		ctf_SafePrint(ent, PRINT_HIGH, message);
	}

	// Can't kick an rcon under any circumstances
	if (target->client->ctf.extra_flags & CTF_EXTRAFLAGS_RCON)
	{
		sprintf(message, "%s is an rcon.  You cannot kick them.\n", target->client->pers.netname);
		ctf_SafePrint(ent, PRINT_HIGH, message);
	}

	sprintf(message, "%s was kicked by %s.\n",
		target->client->pers.netname, ent->client->pers.netname);
	ctf_BSafePrint(PRINT_HIGH, message);
	// clear the kicked player's stats
	stats_clear(target);
	ForceCommand(target, "disconnect\n");
}


void Cmd_Match_f (edict_t *ent)
{
	char *p;
	int i;
	char message[MAX_INFO_STRING];

	if (!(ent->client->ctf.extra_flags & CTF_EXTRAFLAGS_REFEREE))
	{
		ctf_SafePrint(ent, PRINT_HIGH, "You are not a Referee\n");
		return;
	}

	p = gi.args();

	if (!p || !strlen(p))
	{
		ctf_SafePrint(ent, PRINT_HIGH, "USAGE: match <mapname>\n");
		return;
	}
	else
	{
		for (i=0; maplist[i].mapname; i++)
		{
			if (!strcmp(maplist[i].mapname, p))
			{
				ctf_SafePrint(ent, PRINT_HIGH, "Match countdown beginning.\n");
				StartMatch (p);
				return;
			}
		}
		sprintf(message, "%s is not a map from the maplist.\n", p);
		ctf_SafePrint(ent, PRINT_HIGH, message);
	}
}

void Cmd_PingAlert_f (edict_t *ent)
{
	char *p;
	int i, j;

	i = j = 0;

	p = gi.args();
	if (sscanf(p, "%d %d", &i, &j) < 1)
	{
		ctf_SafePrint(ent, PRINT_HIGH, "Usage: pingalert <floor> <ceiling>\nUse zero to set no alert for that value.\n");
		return;
	}
	ent->client->ctf.pingalertfloor = i;
	ent->client->ctf.pingalertceiling = j;
}

// END TEAM CODE

int PlayerSort (void const *a, void const *b)
{
	int		anum, bnum;

	anum = *(int *)a;
	bnum = *(int *)b;

	anum = game.clients[anum].ps.stats[STAT_FRAGS];
	bnum = game.clients[bnum].ps.stats[STAT_FRAGS];

	if (anum < bnum)
		return -1;
	if (anum > bnum)
		return 1;
	return 0;
}

/*
=================
Cmd_Players_f
=================
*/
void Cmd_Players_f (edict_t *ent)
{
	Cmd_Users_f(ent); //surt just call users, it does same thing
}


/*
=================
Cmd_Wave_f
=================
*/
void Cmd_Wave_f (edict_t *ent)
{
	int		i;

	i = atoi (gi.argv(1));

	// can't wave when ducked
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		return;

	if (ent->client->anim_priority > ANIM_WAVE)
		return;

	ent->client->anim_priority = ANIM_WAVE;

	switch (i)
	{
	case 0:
		ctf_SafePrint(ent, PRINT_HIGH, "flipoff\n");
		ent->s.frame = FRAME_flip01-1;
		ent->client->anim_end = FRAME_flip12;
		break;
	case 1:
		ctf_SafePrint(ent, PRINT_HIGH, "salute\n");
		ent->s.frame = FRAME_salute01-1;
		ent->client->anim_end = FRAME_salute11;
		break;
	case 2:
		ctf_SafePrint(ent, PRINT_HIGH, "taunt\n");
		ent->s.frame = FRAME_taunt01-1;
		ent->client->anim_end = FRAME_taunt17;
		break;
	case 3:
		ctf_SafePrint(ent, PRINT_HIGH, "wave\n");
		ent->s.frame = FRAME_wave01-1;
		ent->client->anim_end = FRAME_wave11;
		break;
	case 4:
	default:
		ctf_SafePrint(ent, PRINT_HIGH, "point\n");
		ent->s.frame = FRAME_point01-1;
		ent->client->anim_end = FRAME_point12;
		break;
	}
}

/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0)
{
	int		j;
	edict_t	*other;
	char	*p;
	char	temp[MAX_INFO_STRING];
	char	text[MAX_INFO_STRING];

	if (gi.argc () < 2 && !arg0)
		return;

	if (!ctf_SpamCheck(ent))
		return;

	if (team)
		Com_sprintf (text, sizeof(text), "(%s): ", ent->client->pers.netname);
	else
		Com_sprintf (text, sizeof(text), "%s: ", ent->client->pers.netname);

	strcpy(temp, "");

	if (arg0)
	{
		strcat (temp, gi.argv(0));
		strcat (temp, " ");
		strcat (temp, gi.args());
	}
	else
	{
		p = gi.args();

		if (*p == '"')
		{
			p++;
			p[strlen(p)-1] = 0;
		}
		strcat(temp, p);
	}

	// Temporary
	//sprintf(text, "infoent info_position \"%s\"\n", temp);
	//ForceCommand(ent, text);
	//return;
	//

	string_replace(ent, temp, temp, sizeof temp);
	strcat(text, temp);

	// don't let text be too long for malicious reasons
	if (strlen(text) > 150)
		text[150] = 0;

	ent->client->spam_band_count -= ((long)strlen(text) * 2 + CTF_SPAM_BAND_SAY); //surt spam control

	strcat(text, "\n");

	if (dedicated->value)
		gi.dprintf(text);

	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;
		if (team)
		{
			if (!OnSameTeam(ent, other))
				continue;
		}
		ctf_SafePrint(other, PRINT_CHAT, text);
	}
}

void Cmd_PlayerList_f(edict_t *ent)
{
	int i;
	char st[80];
	char text[MAX_MSGLEN];
	edict_t *e2;

	// connect time, ping, score, name
	*text = 0;
	for (i = 0, e2 = g_edicts + 1; i < maxclients->value; i++, e2++) {
		if (!e2->inuse)
			continue;

		Com_sprintf(st, sizeof(st), "%02d:%02d %4d %3d %s%s id: %i\n",
			(level.framenum - e2->client->resp.enterframe) / 600,
			((level.framenum - e2->client->resp.enterframe) % 600)/10,
			e2->client->ping,
			e2->client->resp.score,
			e2->client->pers.netname,
			e2->client->resp.spectator ? " (spectator)" : "",
			e2->client->ctf.ctfid);
		if (strlen(text) + strlen(st) > sizeof(text) - 50) {
			sprintf(text+strlen(text), "And more...\n");
			gi.cprintf(ent, PRINT_HIGH, "%s", text);
			return;
		}
		strcat(text, st);
	}
	gi.cprintf(ent, PRINT_HIGH, "%s", text);
}


void Cmd_Observe_f(edict_t *ent, int Observer_Type)
{
int i;
int numspec;

	//first check to see if the maxspectator var is set.
	for(i = 1, numspec = 0; i <= maxclients->value; i++)
		if(g_edicts[i].inuse && g_edicts[i].client->pers.spectator)
			numspec++;

	if (numspec >= maxspectators->value)
	{
		gi.cprintf(ent, PRINT_HIGH, "Server spectator limit is full.");
		return;
	}



	if(Observer_Type == CTF_TEAM_OBSERVER_BLUE)
	{
		if(!Team_Observer_OK(CTF_TEAM_BLUE, ent))
			return;

		//gi.bprintf (PRINT_HIGH, "%s is watching the blue team\n", ent->client->pers.netname);
	}
	else if(Observer_Type == CTF_TEAM_OBSERVER_RED)
	{
		if(!Team_Observer_OK(CTF_TEAM_RED, ent))
			return;

		//gi.bprintf (PRINT_HIGH, "%s is watching the red team\n", ent->client->pers.netname);
	}
	//else
		//gi.bprintf (PRINT_HIGH, "%s has moved to the sidelines\n", ent->client->pers.netname);


	Drop_All(ent);
	ent->client->ctf.teamnum = Observer_Type;
	ent->client->chase_target = NULL;
	//ent->client->pers.spectator = 1;
	//ent->client->resp.spectator = 1;
	ForceCommand(ent, "spectator 1");
}

void Cmd_ToggleFastSwitch_f(edict_t *ent)
{
	if (!ISREF(ent)) {
		gi.cprintf(ent, PRINT_HIGH, "Referee-only command\n");
		return;
	}

	char *newval = (((int)fastswitch->value) == 1) ? "0" : "1";
	gi.cvar_set("fastswitch", newval);
	gi.bprintf(PRINT_HIGH, "Fast weapon switching now %sabled\n", (fastswitch->value) ? "en" : "dis");
}

/*
=================
ClientCommand
=================
*/
void ClientCommand (edict_t *ent)
{
	char	*cmd;

	if (!ent->client)
		return;		// not fully in game yet

	cmd = gi.argv(0);


#ifdef BAT_DEBUG
	if(Q_stricmp (cmd, "danman") == 0)
	{
		Cmd_DanMan(ent);
		return;
	}
#endif


	if (Q_stricmp (cmd, "players") == 0)
	{
		Cmd_Players_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "say") == 0)
	{
		Cmd_Say_f (ent, false, false);
		return;
	}
	if (Q_stricmp (cmd, "say_team") == 0)
	{
		Cmd_Say_f (ent, true, false);
		return;
	}
	if (Q_stricmp (cmd, "score") == 0)
	{
		Cmd_Score_f (ent);
		return;
	}
// ADC
	else if (Q_stricmp (cmd, "squadboard") == 0)
	{
		Cmd_Squadboard_f (ent);
		return;
	}
	else if (Q_stricmp (cmd, "squad") == 0)
	{
		char* newcategory = gi.argv(1);

		if (strlen (newcategory) == 0)
			strncpy (ent->client->pers.squad, UNSET_CATEGORY_STR,
			    MAX_CATEGORY_LEN-1);
		else
			strncpy (ent->client->pers.squad, newcategory,
			    MAX_CATEGORY_LEN-1);

		ent->client->pers.squad[MAX_CATEGORY_LEN-1] = 0;
		return;
	}
	else if (Q_stricmp (cmd, "squadstatus") == 0)
	{
		char* newstatus = gi.argv(1);

		if (strlen (newstatus) == 0)
			strncpy (ent->client->pers.squadStatus, UNSET_STATUS_STR,
			    MAX_STATUS_LEN-1);
		else
			strncpy (ent->client->pers.squadStatus, newstatus,
			    MAX_STATUS_LEN-1);

		ent->client->pers.squadStatus[MAX_STATUS_LEN-1] = 0;
		return;
	}
// ADC
	if (Q_stricmp (cmd, "help") == 0)
	{
		Cmd_Help_f (ent);
		return;
	}
	else if (Q_stricmp (cmd, "referee") == 0)
	{
		Cmd_Referee_f (ent);
		return;
	}
	else if (Q_stricmp (cmd, "gameversion") == 0)
	{
		ctf_SafePrint(ent, PRINT_HIGH, GAMEVERSION);
		ctf_SafePrint(ent, PRINT_HIGH, " ");
		ctf_SafePrint(ent, PRINT_HIGH, __DATE__);
		ctf_SafePrint(ent, PRINT_HIGH, "\n");
		return;
	}
	else if (Q_stricmp (cmd, "ctfhelp") == 0)
	{
		Cmd_Ctfhelp_f (ent);
		return;
	}
	else if (Q_stricmp (cmd, "ctfmenu") == 0)
	{
		Cmd_Ctfmenu_f (ent);
		return;
	}
	else if (Q_stricmp (cmd, "refmenu") == 0)
	{
		Cmd_Refmenu_f (ent);
		return;
	}
	else if (Q_stricmp(cmd, "lock") == 0 || Q_stricmp(cmd, "unlock") == 0)
	{
		Cmd_LockTeams_f(ent);
		return;
	}
	else if (Q_stricmp(cmd, "startmatch") == 0)
	{
		Cmd_StartMatch_f(ent);
		return;
	}
	else if (Q_stricmp(cmd, "stopmatch") == 0)
	{
		Cmd_StopMatch_f(ent);
		return;
	}
	else if (Q_stricmp(cmd, "pausematch") == 0 || Q_stricmp(cmd, "unpausematch") == 0)
	{
		Cmd_PauseMatch_f(ent);
		return;
	}
	else if (Q_stricmp(cmd, "togglefastswitch") == 0)
	{
		Cmd_ToggleFastSwitch_f(ent);
		return;
	}
	else if (Q_stricmp (cmd, "users") == 0)
	{
		Cmd_Users_f (ent);
		return;
	}
	else if (Q_stricmp (cmd, "ctfkick") == 0)
	{
		Cmd_Kick_f (ent);
		return;
	}
	else if (Q_stricmp (cmd, "fobserve") == 0)
	{
		Cmd_Fobserve_f (ent);
		return;
	}
	else if (Q_stricmp (cmd, "quadtime") == 0)
	{
		Cmd_QuadTime_f (ent);
		return;
	}
	else if (Q_stricmp (cmd, "angleinfo") == 0)
	{
		Cmd_AngleInfo_f (ent);
		return;
	}
	else if (Q_stricmp (cmd, "gotomap") == 0)
	{
		Cmd_GotoMap_f (ent);
		return;
	}
	else if (Q_stricmp (cmd, "match") == 0)
	{
		Cmd_Match_f (ent);
		return;
	}
	else if (Q_stricmp (cmd, "pingalert") == 0)
	{
		Cmd_PingAlert_f (ent);
		return;
	}
	else if (Q_stricmp (cmd, "voteyes") == 0)   //Vampire -- voting menu
	{
		if (VoteStarted)
			Vote_YES (ent);
		else
			gi.cprintf(ent, PRINT_LOW,"A vote has not been initiated.\n");
		return;
	}
	else if (Q_stricmp (cmd, "voteno") == 0)    //Vampire -- voting menu
	{
		if (VoteStarted)
			Vote_NO (ent);
		else
			gi.cprintf(ent, PRINT_LOW,"A vote has not been initiated.\n");
		return;
	}

	if (level.intermissiontime)
		return;

	if (GamePaused() && !(ent->client->ctf.extra_flags & CTF_EXTRAFLAGS_REFEREE)) // LM_JORM -- Don't let players do certain things if paused
		return;

	if (Q_stricmp (cmd, "use") == 0)
		Cmd_Use_f (ent);
	else if (Q_stricmp (cmd, "hook") == 0)
		Cmd_Hook_f (ent);
	else if (Q_stricmp (cmd, "unhook") == 0)
		Cmd_Unhook_f (ent);
	else if (Q_stricmp (cmd, "weapprev") == 0)
		Cmd_WeapPrev_f (ent);
	else if (Q_stricmp (cmd, "weapnext") == 0)
		Cmd_WeapNext_f (ent);
	else if (Q_stricmp (cmd, "weaplast") == 0)
		Cmd_WeapLast_f (ent);
	else if (Q_stricmp (cmd, "drop") == 0)
		Cmd_Drop_f (ent);
	else if (Q_stricmp (cmd, "help") == 0)
		Cmd_Help_f (ent);
	else if (Q_stricmp (cmd, "inven") == 0)
		Cmd_Inven_f (ent);
	else if (Q_stricmp (cmd, "invnext") == 0)
		SelectNextItem (ent, -1);
	else if (Q_stricmp (cmd, "invprev") == 0)
		SelectPrevItem (ent, -1);
	else if (Q_stricmp (cmd, "invnextw") == 0)
		SelectNextItem (ent, IT_WEAPON);
	else if (Q_stricmp (cmd, "invprevw") == 0)
		SelectPrevItem (ent, IT_WEAPON);
	else if (Q_stricmp (cmd, "invnextp") == 0)
		SelectNextItem (ent, IT_POWERUP);
	else if (Q_stricmp (cmd, "invprevp") == 0)
		SelectPrevItem (ent, IT_POWERUP);
	else if (Q_stricmp (cmd, "invuse") == 0)
		Cmd_InvUse_f (ent);
	else if (Q_stricmp (cmd, "invdrop") == 0)
		Cmd_InvDrop_f (ent);
	else if (Q_stricmp (cmd, "kill") == 0)
		Cmd_Kill_f (ent);
	else if (Q_stricmp (cmd, "putaway") == 0)
		Cmd_PutAway_f (ent);
	else if (Q_stricmp (cmd, "wave") == 0)
		Cmd_Wave_f (ent);
	else if (Q_stricmp (cmd, "fov") == 0)
	{
		ent->client->ps.fov = atoi(gi.argv(1));
		if (ent->client->ps.fov < 1)
			ent->client->ps.fov = 90;
		else if (ent->client->ps.fov > 160)
			ent->client->ps.fov = 160;
	}
	// TEAM_CODE -- LM_JORM
	else if (Q_stricmp (cmd, "team") == 0)
		Cmd_Team_f (ent);
	else if (Q_stricmp (cmd, "flagstatus") == 0)
		Cmd_FlagStatus_f (ent);
	else if (Q_stricmp (cmd, "id") == 0)
		Cmd_Id_f (ent);
	else if (Q_stricmp (cmd, "position") == 0)
		Cmd_Position_f (ent);
	else if (Q_stricmp (cmd, "radiomenu") == 0)
		Toggle_Radio_Menu (ent);
	else if (Q_stricmp (cmd, "play_team") == 0)
		Cmd_PlayTeamSound_f (ent);
	else if (Q_stricmp (cmd, "play_voice") == 0)
		Cmd_PlayVoiceSound_f (ent);
	else if (Q_stricmp (cmd, "radio") == 0)
		Cmd_Radio_f (ent);
#ifdef OLDOBSERVERCODE
	else if (Q_stricmp (cmd, "observe") == 0)
		Observe (ent);
	else if (Q_stricmp (cmd, "chasecam") == 0)
		ChaseCam (ent);
#else
	//bat
	// I'm still gonna let them use the observe command
	//'cause people are used to it.
	else if (Q_stricmp (cmd, "observe") == 0)
		Cmd_Observe_f(ent, CTF_TEAM_OBSERVER);
	else if (Q_stricmp (cmd, "observe_red") == 0)
		Cmd_Observe_f(ent, CTF_TEAM_OBSERVER_RED);
	else if (Q_stricmp (cmd, "observe_blue") == 0)
		Cmd_Observe_f(ent, CTF_TEAM_OBSERVER_BLUE);

#endif
	else if (Q_stricmp (cmd, "stats") == 0) // STATS - LM_Hati
		Cmd_PlayerStats_f (ent);            // STATS - LM_Hati
	else if (Q_stricmp (cmd, "statsall") == 0) // STATS - LM_Surt
		Cmd_StatsAll_f (ent);            // STATS - LM_Surt
#ifdef OLDOBSERVERCODE
	else if (Q_stricmp (cmd, "togglecamera") == 0) //surt for locking the camera view
		Obs_CamLock_Exec (ent); //surt for locking the camera view
#endif
	else if (Q_stricmp (cmd, "compass") == 0)
		Cmd_Compass_f (ent);
	else if (Q_stricmp (cmd, "give") == 0)
		Cmd_Give_f (ent);
	else if (Q_stricmp (cmd, "god") == 0)
		Cmd_God_f (ent);
	else if (Q_stricmp (cmd, "notarget") == 0)
		Cmd_Notarget_f (ent);
	else if (Q_stricmp (cmd, "noclip") == 0)
		Cmd_Noclip_f (ent);
	//else if (Q_stricmp (cmd, "infoent") == 0)
	//	Cmd_InfoEnt_f (ent);


	//bat
	else if(Q_stricmp (cmd, "pause_match") == 0)
	{
		if (ent->client->ctf.extra_flags & CTF_EXTRAFLAGS_REFEREE)
			RefTogglePause(ent);
	}


	// END TEAM CODE
	else if (Q_stricmp(cmd, "playerlist") == 0)
		Cmd_PlayerList_f(ent);
	else	// anything that doesn't match a command will be a chat
		Cmd_Say_f (ent, false, true);
}

