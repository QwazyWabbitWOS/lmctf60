#include "g_local.h"
#include "m_player.h"
#include "time.h" // TEAM CODE -- LM_JORM
#include "g_ctffunc.h" //surt for some nice wrapper functions
#include "g_tourney.h"
#include "g_skins.h"
#include "p_stats.h"
#include "g_vote.h" //Vampire -- voting 

#include "stdlog.h"	// StdLog - Mark Davies
#include "gslog.h"	// StdLog - Mark Davies
#include "bat.h"

// Lithium II Zbot detect plugin
qboolean ZbotCheck(edict_t *ent, usercmd_t *ucmd);

void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void Check_Drop_Flag(edict_t *ent);
void SP_misc_teleporter_dest (edict_t *ent);
int Team_Observer_OK(int Team_To_View, edict_t *ent);
void Cmd_Observe_f(edict_t *ent, int Observer_Type);
int Team_To_Join(edict_t *ent);

//
// Gross, ugly, disgustuing hack section
//

// this function is an ugly as hell hack to fix some map flaws
//
// the coop spawn spots on some maps are SNAFU.  There are coop spots
// with the wrong targetname as well as spots with no name at all
//
// we use carnal knowledge of the maps to fix the coop spot targetnames to match
// that of the nearest named single player spot

static void SP_FixCoopSpots (edict_t *self)
{
	edict_t	*spot;
	vec3_t	d;

	spot = NULL;

	while(1)
	{
		spot = G_Find(spot, FOFS(classname), "info_player_start");
		if (!spot)
			return;
		if (!spot->targetname)
			continue;
		VectorSubtract(self->s.origin, spot->s.origin, d);
		if (VectorLength(d) < 384)
		{
			if ((!self->targetname) || Q_stricmp(self->targetname, spot->targetname) != 0)
			{
//				gi.dprintf("FixCoopSpots changed %s at %s targetname from %s to %s\n", self->classname, vtos(self->s.origin), self->targetname, spot->targetname);
				self->targetname = spot->targetname;
			}
			return;
		}
	}
}

// now if that one wasn't ugly enough for you then try this one on for size
// some maps don't have any coop spots at all, so we need to create them
// where they should have been

static void SP_CreateCoopSpots (edict_t *self)
{
	edict_t	*spot;

	if(Q_stricmp(level.mapname, "security") == 0)
	{
		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 - 64;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 + 64;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 + 128;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		return;
	}
}

// TEAM CODE -- LM_JORM


edict_t *ClientHasFlag(edict_t *ent)
{
	edict_t	*flag;
	int		teamnum = ent->client->ctf.teamnum;
	
	// ctf_validateflags(); //make sure nothing wrong with flags before calling this
	
	// Were we carrying a flag?
	flag = ctf_getteamflag(teamnum, CTF_TEAM_OPPOSING);
	
	if (flag && flag->item)
	{
		if (ent->client->pers.inventory[ITEM_INDEX(flag->item)])
			return flag;
		else
			return NULL;
	}
	return NULL;
	
}

// END TEAM CODE

void ClientUserinfoChanged (edict_t *ent, char *userinfo);

void SP_misc_teleporter_dest (edict_t *ent);

/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
The normal starting point for a level.
*/
void SP_info_player_start(edict_t *self)
{
	if (!coop->value)
		return;
	if(Q_stricmp(level.mapname, "security") == 0)
	{
		// invoke one of our gross, ugly, disgusting hacks
		self->think = SP_CreateCoopSpots;
		self->nextthink = level.time + FRAMETIME;
	}
}

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for deathmatch games
*/
void SP_info_player_deathmatch(edict_t *self)
{
	if (!deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
	SP_misc_teleporter_dest (self);
}

/*QUAKED info_player_coop (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for coop games
*/

void SP_info_player_coop(edict_t *self)
{
	if (!coop->value)
	{
		G_FreeEdict (self);
		return;
	}

	if((Q_stricmp(level.mapname, "jail2") == 0)   ||
	   (Q_stricmp(level.mapname, "jail4") == 0)   ||
	   (Q_stricmp(level.mapname, "mine1") == 0)   ||
	   (Q_stricmp(level.mapname, "mine2") == 0)   ||
	   (Q_stricmp(level.mapname, "mine3") == 0)   ||
	   (Q_stricmp(level.mapname, "mine4") == 0)   ||
	   (Q_stricmp(level.mapname, "lab") == 0)     ||
	   (Q_stricmp(level.mapname, "boss1") == 0)   ||
	   (Q_stricmp(level.mapname, "fact3") == 0)   ||
	   (Q_stricmp(level.mapname, "biggun") == 0)  ||
	   (Q_stricmp(level.mapname, "space") == 0)   ||
	   (Q_stricmp(level.mapname, "command") == 0) ||
	   (Q_stricmp(level.mapname, "power2") == 0) ||
	   (Q_stricmp(level.mapname, "strike") == 0))
	{
		// invoke one of our gross, ugly, disgusting hacks
		self->think = SP_FixCoopSpots;
		self->nextthink = level.time + FRAMETIME;
	}
}
// CTF CODE -- LM_JORM
void SP_info_player_red(edict_t *self)
{
	vec3_t	end, angles;
	
	// If we have no facing for this spawn point...
	//  Turn it to face the flag
	if (!self->s.angles[1] && redflag)
	{
		VectorSubtract(redflag->s.origin, self->s.origin, end);
		vectoangles(end, angles);
		
		self->s.angles[1] = angles[1];
	}
	self->classname = "info_player_red";
}

void SP_info_player_blue(edict_t *self)
{
	vec3_t	end, angles;
	
	// If we have no facing for this spawn point...
	//  Turn it to face the flag
	if (!self->s.angles[1] && blueflag)
	{
		VectorSubtract(blueflag->s.origin, self->s.origin, end);
		vectoangles(end, angles);
		
		self->s.angles[1] = angles[1];
	}
	self->classname = "info_player_blue";
}
// END CTF CODE -- LM_JORM


/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The deathmatch intermission point will be at one of these
Use 'angles' instead of 'angle', so you can set pitch or roll as well as yaw.  'pitch yaw roll'
*/
void SP_info_player_intermission(edict_t* ent)
{
}


//=======================================================================


void player_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	// player pain is handled at the end of the frame in P_DamageFeedback
}


qboolean IsFemale (edict_t *ent)
{
	char		*info;

	if (!ent->client)
		return false;

	info = Info_ValueForKey (ent->client->pers.userinfo, "gender");
	if (info[0] == 'f' || info[0] == 'F')
		return true;
	return false;
}

qboolean IsNeutral (edict_t *ent)
{
	char		*info;

	if (!ent->client)
		return false;

	info = Info_ValueForKey (ent->client->pers.userinfo, "gender");
	if (info[0] != 'f' && info[0] != 'F' && info[0] != 'm' && info[0] != 'M')
		return true;
	return false;
}



char Death_Msg_String[64];

#define DIF_MSG_DEATHS 44

char *Random_MSG_Kills1[DIF_MSG_DEATHS] =
{
"bit the dust of",
"visits the Grim Reaper by",
"becomes one with death by",
"drops 6 feet under by",
"takes a high ride in the sky by",
"camps under a gravestone from",
"needs a tissue because of",
"likes the taste of blood from",
"has a crap connection from",
"can't see so good from",
"smokes a peace pipe with the gods and",
"practices with",
"cry's home to his momma from",
"soiled his shorts from",
"steps into a coffin from",
"whines of his connection from",
"would rather play alone than with",
"gets caught checkin' Briana's booty and takes it from",
"sees his brain splatter on the wall from",
"is picking up pieces of his skull from",
"is busy playing with his joystick and",
"becomes bored with life from",
"buys a ticket on the train of death from",
"paints a pretty picture with his blood from",
"is stinking up the field from",
"leaves a poop stain on his seat from",
"needs help finding his teeth from",
"bites the big one from",
"searches for his severed head from",
"sucks on",
"get's a lesson from",
"enjoy's getting killed by",
"was eeged by",
"starts to cry from",
"is hunted by",
"gets flipped the bird from",
"curses",
"gets mud in the face from",
"plays with",
"takes it in the cooter from",
"gets jarred in the jaw from",
"bleeds in the gut from",
"chokes on",
"takes it up the arse from",
};


#define DIF_MALE_DEATHS	4

char *Random_Male_Deaths[DIF_MALE_DEATHS] = 
{
"can't find his head",
"gets a date with Death",
"picks up his eyeball",
"gets caught yanking his chain"
};

#define DIF_FEMALE_DEATHS	4


char *Random_Female_Deaths[DIF_FEMALE_DEATHS] = 
{
"can't find her head",
"gets a date with Hades",
"picks up her eyeball",
"is busy combing her hair"
};


// return 1 for enemy kill
void Gimme_Any_Death_Message(void)
{
int index;

	index = rand() % DIF_MSG_DEATHS;
	strcpy(Death_Msg_String, Random_MSG_Kills1[index]);
}

void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	int			mod;
	char		*message;
	char		*message2;
	qboolean ff;
	char		msg[MAX_INFO_STRING];
	int Sex;

	int New_Death_Messages;

	if((int)ctfflags->value & CTF_RANDOM_DEATH_MSG)
		New_Death_Messages = true;
	else
		New_Death_Messages = false;


	if(IsFemale(self))
		Sex = FEMALE;
	else
		Sex = MALE;

	if (coop->value && attacker->client)
		meansOfDeath |= MOD_FRIENDLY_FIRE;

	if (deathmatch->value || coop->value)
	{
		ff = meansOfDeath & MOD_FRIENDLY_FIRE;
		mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
		message = NULL;
		message2 = "";

		switch (mod)
		{
		case MOD_SUICIDE:
			message = "suicides";
			break;
		case MOD_FALLING:
			message = "cratered";
			break;
		case MOD_CRUSH:
			message = "was squished";
			break;
		case MOD_WATER:
			message = "sank like a rock";
			break;
		case MOD_SLIME:
			message = "melted";
			break;
		case MOD_LAVA:
			message = "does a back flip into the lava";
			break;
		case MOD_EXPLOSIVE:
		case MOD_BARREL:
			message = "blew up";
			break;
		case MOD_EXIT:
			message = "found a way out";
			break;
		case MOD_TARGET_LASER:
			message = "saw the light";
			break;
		case MOD_TARGET_BLASTER:
			message = "got blasted";
			break;
		case MOD_BOMB:
		case MOD_SPLASH:
		case MOD_TRIGGER_HURT:
			message = "was in the wrong place";
			break;
		}
		if (attacker == self)
		{
			switch (mod)
			{
			case MOD_HELD_GRENADE:
				message = "tried to put the pin back in";
				break;
			case MOD_HG_SPLASH:
			case MOD_G_SPLASH:
				if (IsNeutral(self))
					message = "tripped on its own grenade";
				else if (Sex)
					message = "tripped on her own grenade";
				else
					message = "tripped on his own grenade";
				break;
			case MOD_R_SPLASH:
				if (IsNeutral(self))
					message = "blew itself up";
				else if (Sex)
					message = "blew herself up";
				else
					message = "blew himself up";
				break;
			case MOD_BFG_BLAST:
				message = "should have used a smaller gun";
				break;
// SKWiD MOD
#include "plasma.h"
 			case MOD_PLASMA:
 				message = "get's shocked";
 				break;
// END

			default:
				if (IsNeutral(self))
					message = "killed itself";
				else if (Sex)
					message = "killed herself";
				else
					message = "killed himself";
				break;
			}
		}
		if (message)
		{
			if(Match_CanScore())
			{
				sprintf(msg, "%s %s.\n", self->client->pers.netname, message);
				ctf_BSafePrint(PRINT_HIGH, msg);
				if (deathmatch->value)
				{
					self->client->resp.score--;
					stats_add(self, STATS_SCORE, -1);
					stats_add(self, STATS_DEATHS, 1);
				}
			}
			self->enemy = NULL;
			return;
		}

		self->enemy = attacker;
		if (attacker && attacker->client)
		{
			switch (mod)
			{
			case MOD_BLASTER:
				message = "was blasted by";
				if(New_Death_Messages)
					message2 = "'s blaster"; //bat
				break;

			case MOD_SHOTGUN:
				message = "was gunned down by";
				if(New_Death_Messages)
					message2 = "'s shotgun"; //bat
				break;

			case MOD_SSHOTGUN:
				message = "was blown away by";
				message2 = "'s super shotgun";
				break;

			case MOD_MACHINEGUN:
				message = "was machinegunned by";
				if(New_Death_Messages)
					message2 = "'s machinegun";	//bat
				break;

			case MOD_CHAINGUN:
				message = "was cut in half by";
				message2 = "'s chaingun";
				break;

			case MOD_GRENADE:
				message = "was popped by";
				message2 = "'s grenade";
				break;
			case MOD_G_SPLASH:
				message = "was shredded by";
				message2 = "'s shrapnel";
				break;

			case MOD_ROCKET:
				message = "ate";
				message2 = "'s rocket";
				break;
			case MOD_R_SPLASH:
				message = "almost dodged";
				message2 = "'s rocket";
				break;

			case MOD_HYPERBLASTER:
				message = "was melted by";
				message2 = "'s hyperblaster";
				break;
			case MOD_RAILGUN:
				message = "was railed by";
				if(New_Death_Messages)
					message2 = "'s railgun"; //bat
				break;

			case MOD_BFG_LASER:
				message = "saw the pretty lights from";
				message2 = "'s BFG";
				break;

			case MOD_BFG_BLAST:
				message = "was disintegrated by";
				message2 = "'s BFG blast";
				break;

			case MOD_BFG_EFFECT:
				message = "couldn't hide from";
				message2 = "'s BFG";
				break;

// SKWiD MOD
			case MOD_PLASMA:
				message = "got an infusion of plasma from";
				if(New_Death_Messages)
					message2 = "'s plasma rifle";
				break;
// END
			case MOD_HANDGRENADE:
				message = "caught";
				message2 = "'s handgrenade";
				break;

			case MOD_HG_SPLASH:
				message = "didn't see";
				message2 = "'s handgrenade";
				break;

			case MOD_HELD_GRENADE:
				message = "feels";
				if(New_Death_Messages)
					message2 = "'s paingrenade";
				else
					message2 = "'s pain";
				break;

			case MOD_TELEFRAG:
				message = "tried to invade";
				message2 = "'s personal space";
				break;

			case MOD_CTF_GRAPPLE:
				message = "was gored by";
				message2 = "'s grappling hook";
				break;
			}

			if (message)
			{
				if((int)ctfflags->value & CTF_RANDOM_DEATH_MSG)
				{
					Gimme_Any_Death_Message();
					message = &Death_Msg_String[0];
				}
				
				
				sprintf(msg, "%s %s %s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2);

				ctf_BSafePrint(PRINT_HIGH, msg);
				if (deathmatch->value)
				{
					if (ff && matchstate == MATCH_RAILGUN_INPLAY)
					{
						stats_add(attacker, STATS_SCORE, -1);
						stats_add(attacker, STATS_DEATHS, 1);
						attacker->client->resp.score--;
					}
					else
					{
						stats_add(attacker, STATS_SCORE, 1);
						stats_add(attacker, STATS_FRAGS, 1);
						attacker->client->resp.score++;
					}
				}
				return;
			}
		}
	 }
	 

	 sprintf(msg, "%s died.\n", self->client->pers.netname);
	 ctf_BSafePrint(PRINT_HIGH, msg);
	 if (deathmatch->value)
	 {
		 self->client->resp.score--;
		 stats_add(self, STATS_SCORE, -1);
		 stats_add(self, STATS_DEATHS, 1);
	 }
}


void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

void TossClientWeapon (edict_t *self)
{
	gitem_t		*item;
	edict_t		*drop;
	qboolean	quad;
	float		spread;

	if (!deathmatch->value)
		return;

	item = self->client->pers.weapon;
	if (!item)
		return;
	if (! self->client->pers.inventory[self->client->ammo_index] )
		item = NULL;
	if (item && (strcmp (item->pickup_name, "Blaster") == 0))
		item = NULL;
	
	// CTF CODE -- LM_JORM
	if (item && !strcmp (item->pickup_name, "Grappling Hook"))
		item = NULL;
	// END CTF CODE -- LM_JORM
	if (!((int)(dmflags->value) & DF_QUAD_DROP))
		quad = false;
	else
		quad = (self->client->quad_framenum > (level.framenum + 10));

	if (item && quad)
		spread = 22.5;
	else
		spread = 0.0;

	if (item)
	{
		self->client->v_angle[YAW] -= spread;
		drop = Drop_Item (self, item);
		self->client->v_angle[YAW] += spread;
		drop->spawnflags = DROPPED_PLAYER_ITEM;
	}

	if (quad)
	{
		self->client->v_angle[YAW] += spread;
		drop = Drop_Item (self, FindItemByClassname ("item_quad"));
		self->client->v_angle[YAW] -= spread;
		drop->spawnflags |= DROPPED_PLAYER_ITEM;
		drop->touch = Touch_Item;
		drop->nextthink = level.time + (self->client->quad_framenum - level.framenum) * FRAMETIME;
		drop->think = G_FreeEdict;
	}
}


/*
==================
LookAtKiller
==================
*/
void LookAtKiller (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	vec3_t		dir;

	if (attacker && attacker != world && attacker != self)
	{
		VectorSubtract (attacker->s.origin, self->s.origin, dir);
	}
	else if (inflictor && inflictor != world && inflictor != self)
	{
		VectorSubtract (inflictor->s.origin, self->s.origin, dir);
	}
	else
	{
		self->client->killer_yaw = self->s.angles[YAW];
		return;
	}

	if (dir[0])
		self->client->killer_yaw = 180/M_PI*atan2(dir[1], dir[0]);
	else {
		self->client->killer_yaw = 0;
		if (dir[1] > 0)
			self->client->killer_yaw = 90;
		else if (dir[1] < 0)
			self->client->killer_yaw = -90;
	}
	if (self->client->killer_yaw < 0)
		self->client->killer_yaw += 360;
	

}

/*
==================
player_die
==================
*/
void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;
	edict_t	*attacker_flag=NULL, *defender_flag=NULL;  // CTF CODE -- LM_JORM
	// int		teamnum;// CTF CODE -- LM_JORM
	vec3_t	dir;	// CTF CODE -- LM_JORM
	vec_t dist1, dist2;	// CTF CODE -- LM_JORM
	char	*flagcolor = NULL;	// CTF CODE -- LM_JORM
	char message[MAX_INFO_STRING];
	// MJD Uninitialized - Caught but Fixed
	
	
	// CTF CODE -- LM_JORM
	
	// CTF bonuses for kills
	
	//self has just died.  was self interesting in some way?
	ctf_validateflags();
	
	//stats_add(self, STATS_DEATHS, 1); // STATS - LM_Hati
	
	//first, make sure self and attacker were valid players
	if (ctf_validateplayer(attacker, CTF_TEAM_ANYTEAM) &&
		ctf_validateplayer(self,CTF_TEAM_ANYTEAM) &&
		self != attacker &&				// Can't get bonus on suicides
		(self->deadflag != DEAD_DEAD) ) //and victim not already dead
	{
		if (! OnSameTeam(attacker, self)) //must be different teams
		{
			// Find attacker's flag
			if (attacker->client->ctf.teamnum == CTF_TEAM_RED)  // Red
			{
				attacker_flag = redflag;
				defender_flag = blueflag;
			}
			else //must be blue
			{
				attacker_flag = blueflag;
				defender_flag = redflag;
			}
			
			if ( attacker_flag && !(ctf_validateplayer(attacker_flag->owner, CTF_TEAM_ANYTEAM)) ) //attacker's flag was not in anyone's posession
			{
				//so it's either at home, or lying around
				// 			if (FlagCompare (attacker_flag->homeposition, attacker_flag->s.origin))
				if (ctf_flagathome(attacker_flag))
				{
					// Flag at home
					
					VectorSubtract (attacker->s.origin, attacker_flag->s.origin, dir);
					dist1 = VectorLength(dir); //if this is small, i'm close to my flag
					
					VectorSubtract (self->s.origin, attacker_flag->s.origin, dir);
					dist2 = VectorLength(dir); //if this is small, deadguy was near my flag
					
					//surt duh, this was using | not ||
					if ((dist1 < CTF_DEFEND_FLAG_RADIUS) || (dist2 < CTF_DEFEND_FLAG_RADIUS)) // was 512  MJD Added parens
					{
						switch (attacker->client->ctf.teamnum)
						{
						case CTF_TEAM_RED: flagcolor = "red"; break;
						case CTF_TEAM_BLUE: flagcolor = "blue"; break;
						}
						sprintf(message, "%s defends the %s flag.\n",
								attacker->client->pers.netname,
								flagcolor );
						ctf_BSafePrint(PRINT_HIGH, message);
						attacker->client->resp.score += 2;
						stats_add(attacker, STATS_SCORE, 2);
						attacker->client->defend_flag_time = level.time;
						
						stats_add(attacker, STATS_DEFENSE_FLAG, 1); // STATS - LM_Hati
						// STDLog Flag Defense - Surt
						sl_LogScore( &gi,
									 attacker->client->pers.netname,
									 NULL,
									 "F Def",
									 NULL,
									 2, //surt fixme symbolic
									 level.time );
					}
				}
				else // Flag is lying around, do we get credit for defending the base?
				{
					// Defend the Base
					VectorSubtract (attacker->s.origin, attacker_flag->homeposition, dir);
					dist1 = VectorLength(dir); //attacker was near his pedastal
					
					VectorSubtract (self->s.origin, attacker_flag->homeposition, dir);
					dist2 = VectorLength(dir); //dead guy was in attacker's base
					
					//surt duh, this was using | not ||
					if ((dist1 < CTF_DEFEND_BASE_RADIUS) || (dist2 < CTF_DEFEND_BASE_RADIUS)) // was 512  MJD Added Parens
					{
						switch (attacker->client->ctf.teamnum)
						{
						case CTF_TEAM_RED: flagcolor = "red"; break;
						case CTF_TEAM_BLUE: flagcolor = "blue"; break;
						}
						sprintf(message, "%s defends the %s base.\n",
								attacker->client->pers.netname,
								flagcolor );
						ctf_BSafePrint(PRINT_HIGH, message);
						attacker->client->resp.score += 1;
						stats_add(attacker, STATS_SCORE, 1);
						
						stats_add(attacker, STATS_DEFENSE_BASE, 1); // STATS - LM_Hati
						// Log Flag Defense - MarkDavies
						sl_LogScore( &gi,
									 attacker->client->pers.netname,
									 NULL,
									 "F Base Def",
									 NULL,
									 1, //surt fixme symbolic
									 level.time );
					}
					
					// Defend the Away Flag
					VectorSubtract (attacker->s.origin, attacker_flag->s.origin, dir);
					dist1 = VectorLength(dir); //i was standing near my flag, which was not at base
					
					VectorSubtract (self->s.origin, attacker_flag->s.origin, dir);
					dist2 = VectorLength(dir); //deadguy was standing near my flag, which was not at base
					
					//surt duh, this was using | not ||
					if ((dist1 < CTF_DEFEND_REMOTE_RADIUS) || (dist2 < CTF_DEFEND_REMOTE_RADIUS)) // was 512	MJD Added Parens
					{
						attacker->client->defend_flag_time = level.time;
					}
					
				}
				
				//is the dead guy's flag in someone's posession?
				//if so, then potentially attacker was defending his flag carrier
				//but not if the dead guy's flag owner is us.
				if ( (ctf_validateplayer(defender_flag->owner, CTF_TEAM_ANYTEAM) ) &&
					 (defender_flag->owner != attacker) ) // We can't defend ourselves
				{
					
					if (level.time < self->client->hit_carrier_time + 2) //dead guy recently hit carrier
					{
						switch (attacker->client->ctf.teamnum)
						{
						case CTF_TEAM_RED: flagcolor = "red"; break;
						case CTF_TEAM_BLUE: flagcolor = "blue"; break;
						}
						sprintf(message, "%s defends the %s flag carrier from an aggressive enemy.\n",
								attacker->client->pers.netname,
								flagcolor );
						ctf_BSafePrint(PRINT_HIGH, message);
						attacker->client->resp.score += 3;
						stats_add(attacker, STATS_SCORE, 3);
						
						// STDLog Flag Danger Carrier Protect Frag - Surt
						sl_LogScore( &gi,
									 attacker->client->pers.netname,
									 NULL,
									 "FC Def",
									 NULL,
									 3, //value of this event FIXME symbolic surt
									 level.time );
						
						
						stats_add(attacker, STATS_DEFENSE_CARRIER, 1); // STATS - LM_Hati
					}
					else
					{
						dist1 = 0;
						dist2 = 0;
						
						VectorSubtract (attacker->s.origin, defender_flag->owner->s.origin, dir); //i'm standing near the carrier of the dead guy's flag
						dist1 = VectorLength(dir);
						
						VectorSubtract (self->s.origin, defender_flag->owner->s.origin, dir); //dead guy near my flag carrier
						dist2 = VectorLength(dir);
						
						if ((dist1 < CTF_DEFEND_CARRIER_RADIUS) || (dist2 < CTF_DEFEND_CARRIER_RADIUS)) //MJD Added Parens
						{
							switch (attacker->client->ctf.teamnum)
							{
							case CTF_TEAM_RED: flagcolor = "red"; break;
							case CTF_TEAM_BLUE: flagcolor = "blue"; break;
							}
							sprintf(message, "%s defends the %s flag carrier.\n",
									attacker->client->pers.netname,
									flagcolor );
							ctf_BSafePrint(PRINT_HIGH, message);
							attacker->client->resp.score += 2;
							stats_add(attacker, STATS_SCORE, 2);
							
							stats_add(attacker, STATS_DEFENSE_CARRIER, 1); // STATS - LM_Hati
							// STDLog Flag Carrier Protect Frag - Surt
							sl_LogScore( &gi,
										 attacker->client->pers.netname,
										 NULL,
										 "FC Def",
										 NULL,
										 2,
										 level.time );
						}
					}
				}
			}
		}
		
		// was the dead player the flag carrier?
		defender_flag = ClientHasFlag(self);
		if (defender_flag) //surt to avoid compiler warns on linux/solaris (assignment in conditional)
		{
			if (attacker->client->ctf.teamnum != self->client->ctf.teamnum) //implicit attacker != self
			{
				sprintf(message, "%s killed the enemy flag carrier.\n", attacker->client->pers.netname);
				ctf_BSafePrint(PRINT_HIGH, message);
				attacker->client->resp.score += 2;
				stats_add(attacker, STATS_SCORE, 2);
				attacker->client->kill_carrier_time = level.time;
				
				// STDLog Flag Carrier Frag - Surt
				sl_LogScore( &gi,
							 attacker->client->pers.netname,
							 NULL,
							 "FC Frag",
							 NULL,
							 2, //value of this event FIXME we need to make these symbolic surt
							 level.time );
				
				stats_add(attacker, STATS_OFFENSE_CARRIER, 1); // STATS - LM_Hati
			}
		}
		
		
	}
	 
	if (self->client->hook)
	{
		G_FreeEdict (self->client->hook);
		self->client->hook = NULL;
	}
	if (self->client->rune && self->client->rune->item)
	{
		Drop_Rune(self, self->client->rune->item);
	}
	 
	self->s.renderfx = 0;
	self->s.effects = 0;
	 
	 
	//surt regardless of whether everybody was valid players, if self had the flag, better drop it!
	defender_flag = ClientHasFlag(self);
	if (defender_flag) //surt to avoid compiler warns on linux/solaris (assignment in conditional)
	{
		ctf_playerdropflag(self, defender_flag->item);
	}
	 
	// END CTF CODE -- LM_JORM
	 
	 

	VectorClear (self->avelocity);

	self->takedamage = DAMAGE_YES;
	self->movetype = MOVETYPE_TOSS;

	self->s.modelindex2 = 0;	// remove linked weapon model

	self->s.angles[0] = 0;
	self->s.angles[2] = 0;

	self->s.sound = 0;
	self->client->weapon_sound = 0;

	self->maxs[2] = -8;

//	self->solid = SOLID_NOT;
	self->svflags |= SVF_DEADMONSTER;

	if (!self->deadflag)
	{
		self->client->respawn_time = level.time + 1.0;
		LookAtKiller (self, inflictor, attacker);
		self->client->ps.pmove.pm_type = PM_DEAD;
		ClientObituary (self, inflictor, attacker);
		
		//-bat added this for stdlogging.
		sl_WriteStdLogDeath( &gi, level, self, inflictor, attacker);
		
		TossClientWeapon (self);
		if (deathmatch->value)
			Cmd_Help_f (self);		// show scores

		// clear inventory
		// this is kind of ugly, but it's how we want to handle keys in coop
		for (n = 0; n < game.num_items; n++)
		{
			if (coop->value && itemlist[n].flags & IT_KEY)
				self->client->resp.coop_respawn.inventory[n] = self->client->pers.inventory[n];
			self->client->pers.inventory[n] = 0;
		}
	}

	// remove powerups
	self->client->quad_framenum = 0;
	self->client->invincible_framenum = 0;
	self->client->breather_framenum = 0;
	self->client->enviro_framenum = 0;
	self->flags &= ~FL_POWER_ARMOR;

	if (self->health < -40)
	{	// gib
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowClientHead (self, damage);

		self->takedamage = DAMAGE_NO;
	}
	else
	{	// normal death
		if (!self->deadflag)
		{
			static int i;

			i = (i+1)%3;
			// start a death animation
			self->client->anim_priority = ANIM_DEATH;
			if (self->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				self->s.frame = FRAME_crdeath1-1;
				self->client->anim_end = FRAME_crdeath5;
			}
			else switch (i)
			{
			case 0:
				self->s.frame = FRAME_death101-1;
				self->client->anim_end = FRAME_death106;
				break;
			case 1:
				self->s.frame = FRAME_death201-1;
				self->client->anim_end = FRAME_death206;
				break;
			case 2:
				self->s.frame = FRAME_death301-1;
				self->client->anim_end = FRAME_death308;
				break;
			}
			gi.sound (self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
		}
	}

	self->deadflag = DEAD_DEAD;

	gi.linkentity (self);
}

//=======================================================================

/*
==============
InitClientPersistant

This is only called when the game first initializes in single player,
but is called after each death and level change in deathmatch
==============
*/
void InitClientPersistant (gclient_t *client)
{
	gitem_t		*item;

	memset (&client->pers, 0, sizeof(client->pers));
	
	// CTF CODE -- LM_JORM
	item = FindItem("Grappling Hook");
	client->pers.inventory[ITEM_INDEX(item)] = 1;
	// END CTF CODE -- LM_JORM
	
	
	item = FindItem("Blaster");
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1;

	client->pers.weapon = item;

	client->pers.health			= 100;
	client->pers.max_health		= 100;

	client->pers.max_bullets	= 200;
	client->pers.max_shells		= 100;
	client->pers.max_rockets	= 50;
	client->pers.max_grenades	= 50;
	client->pers.max_cells		= 200;
	client->pers.max_slugs		= 50;

	client->pers.connected = true;
}

//QW// This was originally a stack variable in
// InitClientResp and PutClientInServer.
// I'm not even sure why this housekeeping
// is needed since resp stuff is not in client->ctf.
static client_ctf_t ctftemp;

void InitClientResp (gclient_t *client)
{
	//surt
	//	  int teamnum, radio, extra_flags;
	//	  qboolean	goodskin;	// Flags that skin has been approved, no need to forcecommand
	//	  int 		pingalertfloor;
	//	  int 		pingalertceiling;
	//	  int 	compass;
	stats_player_s *playertemp;
	
	ctftemp = client->ctf;
	playertemp = client->p_stats_player;
	
	memset (&client->resp, 0, sizeof(client->resp));
	client->resp.enterframe = level.framenum;
	
	client->ctf = ctftemp;
	client->p_stats_player = playertemp;

	if ((int)ctfflags->value & CTF_TEAM_RESET)
	{
		//if (client->ctf.teamnum != CTF_TEAM_OBSERVER)
		//bat
		if (client->ctf.teamnum != CTF_TEAM_OBSERVER && 
			client->ctf.teamnum != CTF_TEAM_OBSERVER_RED &&
			client->ctf.teamnum != CTF_TEAM_OBSERVER_BLUE)
				client->ctf.teamnum = CTF_TEAM_UNDEFINED; // LM_JORM -- This will cause us to be on a new team
	}
	//surt above is allowed as a special case for now, otherwise only set teamnum
	//in ctf_SetEntTeam
	
	client->resp.coop_respawn = client->pers;
}

/*
==================
SaveClientData

Some information that should be persistant, like health, 
is still stored in the edict structure, so it needs to
be mirrored out to the client structure before all the
edicts are wiped.
==================
*/
void SaveClientData (void)
{
	int		i;
	edict_t	*ent;

	for (i=0 ; i<game.maxclients ; i++)
	{
		ent = &g_edicts[1+i];
		if (!ent->inuse)
			continue;
		game.clients[i].pers.health = ent->health;
		game.clients[i].pers.max_health = ent->max_health;
		game.clients[i].pers.savedFlags = (ent->flags & (FL_GODMODE|FL_NOTARGET|FL_POWER_ARMOR));
		if (coop->value)
			game.clients[i].pers.score = ent->client->resp.score;
	}
}

void FetchClientEntData (edict_t *ent)
{
	ent->health = ent->client->pers.health;
	ent->max_health = ent->client->pers.max_health;
	ent->flags |= ent->client->pers.savedFlags;
	if (coop->value)
		ent->client->resp.score = ent->client->pers.score;
}



/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
PlayersRangeFromSpot

Returns the distance to the nearest player from the given spot
================
*/
float	PlayersRangeFromSpot (edict_t *spot)
{
	edict_t	*player;
	float	bestplayerdistance;
	vec3_t	v;
	int		n;
	float	playerdistance;


	bestplayerdistance = 9999999;

	for (n = 1; n <= maxclients->value; n++)
	{
		player = &g_edicts[n];

		if (!player->inuse)
			continue;

		if (player->health <= 0)
			continue;

		VectorSubtract (spot->s.origin, player->s.origin, v);
		playerdistance = VectorLength (v);

		if (playerdistance < bestplayerdistance)
			bestplayerdistance = playerdistance;
	}

	return bestplayerdistance;
}

/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point, but NOT the two points closest
to other players
================
*/
edict_t *SelectRandomDeathmatchSpawnPoint (void)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		count++;
		range = PlayersRangeFromSpot(spot);
		if (range < range1)
		{
			range1 = range;
			spot1 = spot;
		}
		else if (range < range2)
		{
			range2 = range;
			spot2 = spot;
		}
	}

	if (!count)
		return NULL;

	if (count <= 2)
	{
		spot1 = spot2 = NULL;
	}
	else
		count -= 2;

	selection = rand() % count;

	spot = NULL;
	do
	{
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
		if (spot == spot1 || spot == spot2)
			selection++;
	} while(selection--);

	return spot;
}

/*
================
SelectFarthestDeathmatchSpawnPoint

================
*/
edict_t *SelectFarthestDeathmatchSpawnPoint (void)
{
	edict_t	*bestspot;
	float	bestdistance, bestplayerdistance;
	edict_t	*spot;


	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		bestplayerdistance = PlayersRangeFromSpot (spot);

		if (bestplayerdistance > bestdistance)
		{
			bestspot = spot;
			bestdistance = bestplayerdistance;
		}
	}

	if (bestspot)
	{
		return bestspot;
	}

	// if there is a player just spawned on each and every start spot
	// we have no choice to turn one into a telefrag meltdown
	spot = G_Find (NULL, FOFS(classname), "info_player_deathmatch");

	return spot;
}

// CTF CODE -- LM_JORM

/*
================
SelectTeamSpawnPoint
-  
================
*/
edict_t *SelectTeamSpawnPoint (edict_t *ent)
{
	edict_t	*bestspot;
	float	bestdistance, bestplayerdistance;
	edict_t	*spot;
	char	*spawntype;
	
	if (ent->client->ctf.teamnum == CTF_TEAM_RED) // RED team
		spawntype = "info_player_red";
	else // BLUE team
		spawntype = "info_player_blue";
	
	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	while ((spot = G_Find (spot, FOFS(classname), spawntype)) != NULL)
	{
		bestplayerdistance = PlayersRangeFromSpot (spot);
		
		if (bestplayerdistance > bestdistance)
		{
			bestspot = spot;
			bestdistance = bestplayerdistance;
		}
	}
	
	if (bestspot)
	{
		return bestspot;
	}
	
	// if there is a player just spawned on each and every start spot
	// we have no choice to turn one into a telefrag meltdown
	spot = G_Find (NULL, FOFS(classname), spawntype);
	
	return spot;
}

/*
================
SelectAnySpawnPoint
-  
================
*/
edict_t *SelectAnySpawnPoint (edict_t *ent)
{
	float	distance1, distance2;
	edict_t	*spot1, *spot2;
	
	
	if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)
		spot1 = SelectFarthestDeathmatchSpawnPoint ();
	else
		spot1 = SelectRandomDeathmatchSpawnPoint ();
	
	if (!spot1)
		return NULL;
	
	distance1 = PlayersRangeFromSpot (spot1);
	
	
	spot2 = SelectTeamSpawnPoint(ent);
	if (!spot2) // Do we have any team spawn points?
		return spot1;
	distance2 = PlayersRangeFromSpot (spot2);
	
	if (distance1 > distance2) // Is deathmatch better than team?
		return spot1;
	else
		return spot2;
}

// END CTF CODE -- LM_JORM


edict_t *SelectDeathmatchSpawnPoint (void)
{
	if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)
		return SelectFarthestDeathmatchSpawnPoint ();
	else
		return SelectRandomDeathmatchSpawnPoint ();
}


edict_t *SelectCoopSpawnPoint (edict_t *ent)
{
	int		index;
	edict_t	*spot = NULL;
	char	*target;

	index = ent->client - game.clients;

	// player 0 starts in normal player spawn point
	if (!index)
		return NULL;

	spot = NULL;

	// assume there are four coop spots at each spawnpoint
	while (1)
	{
		spot = G_Find (spot, FOFS(classname), "info_player_coop");
		if (!spot)
			return NULL;	// we didn't have enough...

		target = spot->targetname;
		if (!target)
			target = "";
		if ( Q_stricmp(game.spawnpoint, target) == 0 )
		{	// this is a coop spawn point for one of the clients here
			index--;
			if (!index)
				return spot;		// this is it
		}
	}


	return spot;
}


/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, coop start, etc
============
*/
void	SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles)
{
	edict_t	*spot = NULL;
	
	// CTF CODE -- LM_JORM
	if (ent->client->resp.enterframe == level.framenum) // Just entered
		spot = SelectTeamSpawnPoint (ent);
	
	if (!spot)
		spot = SelectAnySpawnPoint (ent);
	
	if (!spot)
		spot = SelectDeathmatchSpawnPoint ();
	
	if (!spot)
		spot = SelectCoopSpawnPoint (ent);
	
	if (!spot)
		spot = G_Find (spot, FOFS(classname), "info_flag_red");
	if (!spot)
		spot = G_Find (spot, FOFS(classname), "info_flag_blue");
	
	// END CTF CODE -- LM_JORM
	
	// find a single player start spot
	if (!spot)
	{
		while ((spot = G_Find (spot, FOFS(classname), "info_player_start")) != NULL)
		{
			if (!game.spawnpoint[0] && !spot->targetname)
				break;

			if (!game.spawnpoint[0] || !spot->targetname)
				continue;

			if (Q_stricmp(game.spawnpoint, spot->targetname) == 0)
				break;
		}

		if (!spot)
		{
			if (!game.spawnpoint[0])
			{	// there wasn't a spawnpoint without a target, so use any
				spot = G_Find (spot, FOFS(classname), "info_player_start");
			}
			if (!spot) 
			{
				gi.error("Couldn't find spawn point %s\n", game.spawnpoint);
				return;
			}
		}
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);
}

//======================================================================


void InitBodyQue (void)
{
	int		i;
	edict_t	*ent;

	level.body_que = 0;
	for (i=0; i<BODY_QUEUE_SIZE ; i++)
	{
		ent = G_Spawn();
		ent->classname = "bodyque";
	}
}

void body_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int	n;

	if (self->health < -40)
	{
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		self->s.origin[2] -= 48;
		ThrowClientHead (self, damage);
		self->takedamage = DAMAGE_NO;
	}
}

void CopyToBodyQue (edict_t *ent)
{
	edict_t		*body;

	// grab a body que and cycle to the next one
	body = &g_edicts[(int)maxclients->value + level.body_que + 1];
	level.body_que = (level.body_que + 1) % BODY_QUEUE_SIZE;

	// FIXME: send an effect on the removed body

	gi.unlinkentity (ent);

	gi.unlinkentity (body);
	body->s = ent->s;
	body->s.number = body - g_edicts;

	body->svflags = ent->svflags;
	VectorCopy (ent->mins, body->mins);
	VectorCopy (ent->maxs, body->maxs);
	VectorCopy (ent->absmin, body->absmin);
	VectorCopy (ent->absmax, body->absmax);
	VectorCopy (ent->size, body->size);
	body->solid = ent->solid;
	body->clipmask = ent->clipmask;
	body->owner = ent->owner;
	body->movetype = ent->movetype;

	body->die = body_die;
	body->takedamage = DAMAGE_YES;

	gi.linkentity (body);
}


void respawn (edict_t *self)
{

	if(matchstate == MATCH_RAILGUN_INPLAY)
	{
		//Drop_All(self);
		self->movetype = MOVETYPE_NOCLIP;
		self->solid = SOLID_NOT;
		self->svflags |= SVF_NOCLIENT;
		self->health = 0;
		return;
	}

	if (deathmatch->value || coop->value)
	{
		// spectator's don't leave bodies
		if (self->movetype != MOVETYPE_NOCLIP)
			CopyToBodyQue (self);
		self->svflags &= ~SVF_NOCLIENT;
		Menu_Free (self); // free the menu rather than lose it during the memset
		PutClientInServer (self);
		// add a teleportation effect
		self->s.event = EV_PLAYER_TELEPORT;

		// hold in place briefly
		self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		self->client->ps.pmove.pm_time = 14;

		//bat changed to avoid the overflows.
		self->client->respawn_time = level.time;
		//self->client->respawn_time = level.time + (0.2 * Respawn_Count);

		// Set the squadboard status to "respawned"...
		strncpy (self->client->pers.squadStatus, RESPAWNED_STATUS_STR, MAX_STATUS_LEN); // ADC
		self->client->pers.squadStatus[MAX_STATUS_LEN-1] = 0; // ADC

		return;
	}

	// restart the entire server
	gi.AddCommandString ("menu_loadgame\n");
}

/* 
 * only called when pers.spectator changes
 * note that resp.spectator should be the opposite of pers.spectator here
 */
void spectator_respawn (edict_t *ent)
{
int i, numspec;
//int New_Team;

	// if the user wants to become a spectator, make sure he doesn't
	// exceed max_spectators

	//sprintf(DBuffer, "sr0 t %d p %d r %d", ent->client->ctf.teamnum,
	//	ent->client->pers.spectator, ent->client->resp.spectator);
	//Debug_Show(DBuffer);


	if (ent->client->pers.spectator) 
	{
		char *value = Info_ValueForKey (ent->client->pers.userinfo, "spectator");
		if (*spectator_password->string && 
			strcmp(spectator_password->string, "none") && 
			strcmp(spectator_password->string, value)) {
			gi.cprintf(ent, PRINT_HIGH, "Spectator password incorrect.\n");
			ent->client->pers.spectator = false;
			gi.WriteByte (svc_stufftext);
			gi.WriteString ("spectator 0\n");
			gi.unicast(ent, true);
			return;
		}

		//This section here really should not need to be in because I am checking previous
		//to this, but I am leaving it in as a precaution
		//-bat

		// count spectators
		for (i = 1, numspec = 0; i <= maxclients->value; i++)
			if (g_edicts[i].inuse && g_edicts[i].client->pers.spectator)
				numspec++;

		if (numspec >= maxspectators->value) {
			gi.cprintf(ent, PRINT_HIGH, "Server spectator limit is full.");
			ent->client->pers.spectator = false;
			// reset his spectator var
			gi.WriteByte (svc_stufftext);
			gi.WriteString ("spectator 0\n");
			gi.unicast(ent, true);
			return;
		}
	} 
	else 
	{
		// he was a spectator and wants to join the game
		// he must have the right password
		char *value = Info_ValueForKey (ent->client->pers.userinfo, "password");
		if (*password->string && strcmp(password->string, "none") && 
			strcmp(password->string, value)) {
			gi.cprintf(ent, PRINT_HIGH, "Password incorrect.\n");
			ent->client->pers.spectator = true;
			gi.WriteByte (svc_stufftext);
			gi.WriteString ("spectator 1\n");
			gi.unicast(ent, true);
			return;
		}

	}

	// clear client on respawn
	ent->client->resp.score = ent->client->pers.score = 0;

	ent->svflags &= ~SVF_NOCLIENT;
	
	PutClientInServer (ent);

	// add a teleportation effect
	if (!ent->client->pers.spectator)  {
		// send effect
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_LOGIN);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

		// hold in place briefly
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;
	}

	ent->client->respawn_time = level.time;

	if (ent->client->pers.spectator) 
	{
		//I added this -bat
		Drop_All(ent);
		if(ent->client->ctf.teamnum == CTF_TEAM_OBSERVER_RED)
			gi.bprintf(PRINT_HIGH, "%s is observing the red team\n", ent->client->pers.netname);
		else if(ent->client->ctf.teamnum == CTF_TEAM_OBSERVER_BLUE)
			gi.bprintf(PRINT_HIGH, "%s is observing the blue team\n", ent->client->pers.netname);
		else
		{
			gi.bprintf (PRINT_HIGH, "%s has moved to the sidelines\n", ent->client->pers.netname);
			ent->client->ctf.teamnum = CTF_TEAM_OBSERVER;
		}
	}
	else
	{
		//I only want to do this shit if they have typed the spectator command!
		if(ent->client->ctf.teamnum != CTF_TEAM_RED &&
			ent->client->ctf.teamnum != CTF_TEAM_BLUE)
		 		ent->client->ctf.New_Team = Team_To_Join(ent);

		gi.bprintf (PRINT_HIGH, "%s joined the game\n", ent->client->pers.netname);
	}
	
	if(ent->client->ctf.New_Team == CTF_TEAM_RED)
	{
		ent->client->ctf.teamnum = CTF_TEAM_RED;
		ClientSetSkin(ent, NULL);
	}
	else if(ent->client->ctf.New_Team == CTF_TEAM_BLUE)
	{
		ent->client->ctf.teamnum = CTF_TEAM_BLUE;
		ClientSetSkin(ent, NULL);
	}


	ent->client->ctf.New_Team = CTF_TEAM_UNDEFINED;

}

//==============================================================

/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/
void PutClientInServer (edict_t *ent)
{
	trace_t		tr; // CTF CODE -- LM_JORM
	
	vec3_t mins = {-16, -16, -24};
	vec3_t maxs = {16, 16, 32};
	int 	index;
	vec3_t spawn_origin;
	vec3_t spawn_angles;
	gclient_t *client;
	int 	i;
 	client_persistant_t	saved;
	client_respawn_t	resp;
	stats_player_s *p_saved_stats = NULL; // STATS - LM_Hati

	static unsigned long unique_id = 6; //guaranteed 0-5 are special
	
	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	SelectSpawnPoint (ent, spawn_origin, spawn_angles);

	index = ent-g_edicts-1;
	client = ent->client;

	// deathmatch wipes most client data every spawn
	if (deathmatch->value)
	{
		char		userinfo[MAX_INFO_STRING];
		char savedsquad [MAX_CATEGORY_LEN]; // ADC
	
		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));

		// We don't want to lose the squad so save it
		// off here first, then copy it back. The unitstatus
		// will be set to "respawned" elsewhere.

		strncpy (savedsquad, ent->client->pers.squad, sizeof savedsquad); // ADC
		savedsquad[MAX_CATEGORY_LEN-1] = 0; // ADC

		InitClientPersistant (client);
		ClientUserinfoChanged (ent, userinfo);

		strncpy (ent->client->pers.squad, savedsquad, MAX_CATEGORY_LEN); // ADC
		ent->client->pers.squad[MAX_CATEGORY_LEN-1] = 0; // ADC
	}
	else if (coop->value)
	{
//		int			n;
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		// this is kind of ugly, but it's how we want to handle keys in coop
//		for (n = 0; n < game.num_items; n++)
//		{
//			if (itemlist[n].flags & IT_KEY)
//				resp.coop_respawn.inventory[n] = client->pers.inventory[n];
//		}
		resp.coop_respawn.game_helpchanged = client->pers.game_helpchanged;
		resp.coop_respawn.helpchanged = client->pers.helpchanged;
		client->pers = resp.coop_respawn;
		ClientUserinfoChanged (ent, userinfo);
		if (resp.score > client->pers.score)
			client->pers.score = resp.score;
	}
	else
	{
		memset (&resp, 0, sizeof(resp));
	}
	

	p_saved_stats = ent->client->p_stats_player;
	ctftemp = client->ctf; //surt never overwrite

	saved = client->pers;
	memset (client, 0, sizeof(*client));
	client->pers = saved;

	
	if(client->pers.health <= 0)
		InitClientPersistant(client);
	
	client->resp = resp;

	// copy some data from the client to the entity
	FetchClientEntData (ent);

	// clear entity values
	ent->groundentity = NULL;
	ent->client = &game.clients[index];
	ent->takedamage = DAMAGE_AIM;
	ent->movetype = MOVETYPE_WALK;
	ent->viewheight = 22;
	ent->inuse = true;
	ent->classname = "player";
	ent->mass = 200;
	ent->solid = SOLID_BBOX;
	ent->deadflag = DEAD_NO;
	ent->air_finished = level.time + 12;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->model = "players/male/tris.md2";
	ent->pain = player_pain;
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags &= ~FL_NO_KNOCKBACK;
	ent->svflags &= ~SVF_DEADMONSTER;

	VectorCopy (mins, ent->mins);
	VectorCopy (maxs, ent->maxs);
	VectorClear (ent->velocity);

	// clear playerstate values
	memset (&ent->client->ps, 0, sizeof(client->ps));

	client->ps.pmove.origin[0] = spawn_origin[0]*8;
	client->ps.pmove.origin[1] = spawn_origin[1]*8;
	client->ps.pmove.origin[2] = spawn_origin[2]*8;

	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	{
		client->ps.fov = 90;
	}
	else
	{
		client->ps.fov = atoi(Info_ValueForKey(client->pers.userinfo, "fov"));
		if (client->ps.fov < 1)
			client->ps.fov = 90;
		else if (client->ps.fov > 160)
			client->ps.fov = 160;
	}

	client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);

	// clear entity state values
	ent->s.effects = 0;
	ent->s.modelindex = 255;		// will use the skin specified model
	ent->s.modelindex2 = 255;		// custom gun model
	// sknum is player num and weapon number
	// weapon number will be added in changeweapon
	ent->s.skinnum = ent - g_edicts - 1;

	ent->s.frame = 0;
	VectorCopy (spawn_origin, ent->s.origin);
	ent->s.origin[2] += 1;	// make sure off ground
	VectorCopy (ent->s.origin, ent->s.old_origin);

	// set the delta angle
	for (i=0 ; i<3 ; i++)
	{
		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);
	}

	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = spawn_angles[YAW];
	ent->s.angles[ROLL] = 0;
	VectorCopy (ent->s.angles, client->ps.viewangles);
	VectorCopy (ent->s.angles, client->v_angle);

	
	// spawn a spectator
	if(client->pers.spectator) {
		client->chase_target = NULL;

		client->resp.spectator = true;

		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->ps.gunindex = 0;
		gi.linkentity (ent);
		//Don't return here!  I still want to save everything!
		//-bat
		//return;
	} else
		client->resp.spectator = false;
	

	if (!KillBox (ent))
	{	// couldn't spawn in?
		
		// CTF CODE -- LM_JORM
		// If we failed Killbox, maybe it was because we are on a teammate.  Kill them anyway
		while (1)
		{
			tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, ent->s.origin, NULL, MASK_PLAYERSOLID);
			if (!tr.ent)
				break;
			
			// FIXME -- This will make it look like a suicide
			T_Damage (tr.ent, tr.ent, tr.ent, vec3_origin, ent->s.origin, vec3_origin, 100000, 0, DAMAGE_NO_PROTECTION, MOD_TELEFRAG);
			
			// If we still didn't kill it, stop
			if (tr.ent->solid)
				break;
			
		}
		// END CTF CODE -- LM_JORM
	}


	gi.linkentity (ent);

	client->p_stats_player = p_saved_stats; // STATS - LM_Hati
	client->ctf = ctftemp; //surt restore

	client->ctf.ctfid = unique_id++;
	client->showctfhud = true; // LM_JORM -- Turn on CTF HUD
	client->ctf.extra_flags |= CTF_EXTRAFLAGS_RADIO_SOUND; // LM_JORM -- Turn on our radio
	
	// force the current weapon up
	client->newweapon = client->pers.weapon;
	ChangeWeapon (ent);

	//sprintf(DBuffer, "pcis-z t %d p %d r %d", ent->client->ctf.teamnum,
	//	ent->client->pers.spectator, ent->client->resp.spectator);
	//Debug_Show(DBuffer);


}

/*
=====================
ClientBeginDeathmatch

A client has just connected to the server in 
deathmatch mode, so clear everything out before starting them.
=====================
*/
void ClientBeginDeathmatch (edict_t *ent)
{
	if (!ent->client)
		return;

#ifdef OLDOBSERVERCODE
	int 		observe = 0; // MJD Uninit'd - No bug, though. Fixed
#endif
	int 		oldteam;
	char		message[MAX_INFO_STRING];
	char		userinfo[MAX_INFO_STRING];

	long i; //loop

	
	//sprintf(DBuffer, "bg t %d p %d r %d", ent->client->ctf.teamnum,
	//	ent->client->pers.spectator, ent->client->resp.spectator);
	//Debug_Show(DBuffer);


	//try to fix unknown pics, associated with skins?
	//this is messing up the spectator stuff
	memcpy(userinfo, ent->client->pers.userinfo, sizeof(userinfo));
	ClientUserinfoChanged(ent, userinfo);

	for (i = PRINT_LOW; i <= PRINT_CHAT; i++)
	{
		ent->client->ctf.printdata[i][0] = 0; //clear this out on connect
	}

	//if(ent->client->ctf.teamnum <= CTF_TEAM_OBSERVER)
	//{
	//	ent->client->pers.spectator = 1;
	//	ent->client->resp.spectator = 1;
	//}
	//else
	//{
	//	ent->client->pers.spectator = 0;
	//	ent->client->resp.spectator = 0;
	//}


	G_InitEdict (ent);

	if (ent->client->p_stats_player->dropped)
	{
		ent->client->resp.score = stats_get(ent, STATS_SCORE);
		ent->client->ctf.teamnum = ent->client->p_stats_player->info.teamnum;
		ent->client->p_stats_player->dropped = false;
	}
	else
	{
		stats_clear(ent);
	}

	InitClientResp (ent->client);

	// TEAM CODE -- LM_JORM //
	sl_WriteStdLogPlayerEntered( &gi, level, ent );	// StdLog - Mark Davies
	
	// OFFHAND HOOK aliases
//	ForceCommand(ent, "alias \"+hook\" \"cmd hook\"\n");
//	ForceCommand(ent, "alias \"-hook\" \"cmd unhook\"\n");
	ForceCommand(ent, "alias +hook cmd hook\n");
	ForceCommand(ent, "alias -hook cmd unhook\n");
	
	oldteam = ent->client->ctf.teamnum;

#ifdef OLDOBSERVERCODE	
	observe = 0;
	if (Match_InPlay()) 	// If Match In Play, enter all new players as observers
	{
		if (oldteam <= CTF_TEAM_UNDEFINED || 
			oldteam >= CTF_TEAM_LIMIT)  // We aren't remembered, so not part of the match
			observe = 1;
	}
#endif
	

	//If they are observers, then let them stay that way!
	//-bat
	if(oldteam == CTF_TEAM_UNDEFINED)
		TeamJoin(ent);						// Join random team
	else if (oldteam < CTF_TEAM_UNDEFINED)
		Cmd_Observe_f(ent, oldteam);
	else // Team already defined
		ctf_SetEntTeamEx(ent, oldteam, 1);			// Join old team, no penalty

	//if (oldteam == CTF_TEAM_UNDEFINED)
	//	TeamJoin(ent);						// Join random team
	//else // Team already defined
	//	ctf_SetEntTeamEx(ent, oldteam, 1);			// Join old team, no penalty

	//sprintf(DBuffer, "ed1 t %d p %d r %d", ent->client->ctf.teamnum,
	//	ent->client->pers.spectator, ent->client->resp.spectator);
	//Debug_Show(DBuffer);



	// END LM_JORM
	
	// locate ent at a spawn point
	PutClientInServer (ent);

	//sprintf(DBuffer, "ed2 t %d p %d r %d", ent->client->ctf.teamnum,
	//	ent->client->pers.spectator, ent->client->resp.spectator);
	//Debug_Show(DBuffer);


	// CTF CODE -- LM_JORM
#ifdef OLDOBSERVERCODE
	if (observe && deathmatch->value) //surt attempt to get single player working
	{
		Observer_Start (ent);
	}
#endif


	// If we are starting a match, don't let us touch anything
	if (Match_InCountdown())
	{
		//ent->solid = SOLID_NOT;
		//ent->movetype = MOVETYPE_NOCLIP;
		ctf_BSafePrint(PRINT_HIGH, "Match about to begin.\n");
	}
	
	// END LM_JORM
	
	if (level.intermissiontime)
	{
		MoveClientToIntermission (ent);
	}
	else
	{
		// send effect
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_LOGIN);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
		MvpDisp = 0;
	}
	
	Com_sprintf(message, sizeof message, "%s entered the game\n", ent->client->pers.netname);
	ctf_BSafePrint(PRINT_HIGH, message);


	//if(ent->client->ctf.teamnum <= CTF_TEAM_OBSERVER)
	//{
	//	ent->client->pers.spectator = 1;
	//	ent->client->resp.spectator = 1;
	//}
	//else
	//{
	//	ent->client->pers.spectator = 0;
	//	ent->client->resp.spectator = 0;
	//}


	// make sure all view stuff is valid
	
	//ent->client->showmod = true; // LM_JORM -- Turn on CTF HUD
	if (oldteam <= CTF_TEAM_UNDEFINED || oldteam >= CTF_TEAM_LIMIT)
		ent->client->showmod = true; // LM_JORM -- Turn on CTF HUD, MOTD
	ClientEndServerFrame (ent);

	//sprintf(DBuffer, "zz t %d p %d r %d", ent->client->ctf.teamnum,
	//	ent->client->pers.spectator, ent->client->resp.spectator);
	//Debug_Show(DBuffer);


}


/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the game.  This will happen every level load.
============
*/
void ClientBegin (edict_t *ent)
{
	int 	i;
	char	message[MAX_INFO_STRING];
	
	ent->client = game.clients + (ent - g_edicts - 1);

	//sprintf(DBuffer, "cb0 t %d p %d r %d", ent->client->ctf.teamnum,
	//	ent->client->pers.spectator, ent->client->resp.spectator);
	//Debug_Show(DBuffer);
	//ctf_BSafePrint(PRINT_HIGH, DBuffer);

	//if(ent->client->ctf.teamnum <= CTF_TEAM_OBSERVER)
	//{
	//	ent->client->pers.spectator = 1;
	//	ent->client->resp.spectator = 1;
	//}
	//else
	//{
	//	ent->client->pers.spectator = 0;
	//	ent->client->resp.spectator = 0;
	//}


	if (deathmatch->value)
	{
		ClientBeginDeathmatch (ent);
		return;
	}

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == true)
	{
		// the client has cleared the client side viewangles upon
		// connecting to the server, which is different than the
		// state when the game is saved, so we need to compensate
		// with deltaangles
		for (i=0 ; i<3 ; i++)
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->ps.viewangles[i]);
	}
	else
	{
		// a spawn point will completely reinitialize the entity
		// except for the persistant data that was initialized at
		// ClientConnect() time
		G_InitEdict (ent);
		ent->classname = "player";
		InitClientResp (ent->client);
		PutClientInServer (ent);
	}


	//sprintf(DBuffer, "cb1 t %d p %d r %d", ent->client->ctf.teamnum,
	//	ent->client->pers.spectator, ent->client->resp.spectator);
	//Debug_Show(DBuffer);

	if (level.intermissiontime)
	{
		MoveClientToIntermission (ent);
	}
	else
	{
		// send effect if in a multiplayer game
		if (game.maxclients > 1)
		{
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_LOGIN);
			gi.multicast (ent->s.origin, MULTICAST_PVS);

			sprintf(message, "%s entered the game\n", ent->client->pers.netname);
			ctf_BSafePrint(PRINT_HIGH, message);
		}
	}

	// make sure all view stuff is valid
	ClientEndServerFrame (ent);
}

/*
===========
ClientUserInfoChanged

called whenever the player updates a userinfo variable.

The game can override any of the settings in place
(forcing skins or names, etc) before copying it off.
============
*/
void ClientUserinfoChanged (edict_t *ent, char *userinfo)
{
	char	*s, *skin;
	int 	playernum;
	
	// check for malformed or illegal info strings
	if (!Info_Validate(userinfo))
	{
		strcpy (userinfo, "\\name\\badinfo\\skin\\male/grunt");
	}

	// set name
	s = Info_ValueForKey (userinfo, "name");
	strncpy (ent->client->pers.netname, s, sizeof(ent->client->pers.netname)-1);
	stats_set_name(ent, ent->client->pers.netname);
	
	strcpy (ent->client->pers.squad, "Uncommitted"); // ADC
	strcpy (ent->client->pers.squadStatus, "Unknown"); // ADC

	// set spectator
	s = Info_ValueForKey (userinfo, "spectator");
	// spectators are only supported in deathmatch
	if (deathmatch->value && *s && strcmp(s, "0"))
		ent->client->pers.spectator = true;
	else
		ent->client->pers.spectator = false;


	// set skin
	skin = Info_ValueForKey (userinfo, "skin");

	playernum = ent-g_edicts-1;

	// combine name and skin into a configstring
	gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, skin) );

	// fov
	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	{
		ent->client->ps.fov = 90;
	}
	else
	{
		ent->client->ps.fov = atoi(Info_ValueForKey(userinfo, "fov"));
		if (ent->client->ps.fov < 1)
			ent->client->ps.fov = 90;
		else if (ent->client->ps.fov > 160)
			ent->client->ps.fov = 160;
	}

	// handedness
	s = Info_ValueForKey (userinfo, "hand");
	if (strlen(s))
	{
		ent->client->pers.hand = atoi(s);
	}

	// save off the userinfo in case we want to check something later
	strncpy (ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);
	
	ClientSetSkin(ent, skin);
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
The game can refuse entrance to a client by returning false.
If the client is allowed, the connection process will continue
and eventually get to ClientBegin()
Changing levels will NOT cause this to be called again, but
loadgames will.
============
*/
qboolean ClientConnect (edict_t *ent, char *userinfo)
{
	char	*value;

	// check to see if they are on the banned IP list
	value = Info_ValueForKey (userinfo, "ip");
	if (SV_FilterPacket(value)) {
		Info_SetValueForKey(userinfo, "rejmsg", "Banned.");
		return false;
	}


	// check for a spectator
	value = Info_ValueForKey (userinfo, "spectator");
	if (deathmatch->value && *value && strcmp(value, "0")) {
		int i, numspec;

		if (*spectator_password->string && 
			strcmp(spectator_password->string, "none") && 
			strcmp(spectator_password->string, value)) {
			Info_SetValueForKey(userinfo, "rejmsg", "Spectator password required or incorrect.");
			return false;
		}

		// count spectators
		for (i = numspec = 0; i < maxclients->value; i++)
			if (g_edicts[i+1].inuse && g_edicts[i+1].client->pers.spectator)
				numspec++;

		if (numspec >= maxspectators->value) {
			Info_SetValueForKey(userinfo, "rejmsg", "Server spectator limit is full.");
			return false;
		}
	} else 
	
	{
		// check for a password
		value = Info_ValueForKey (userinfo, "password");
		if (*password->string && strcmp(password->string, "none") && 
			strcmp(password->string, value)) {
			Info_SetValueForKey(userinfo, "rejmsg", "Password required or incorrect.");
			return false;
		}
	}


	// must have name to connect
	value = Info_ValueForKey(userinfo, "name");
	if (strcmp(value, "") == 0)
	{
		Info_SetValueForKey(userinfo, "rejmsg", "You must have a name.");
		return false;
	}
	
	// they can connect
	ent->client = game.clients + (ent - g_edicts - 1);
	
	if (ent->client->p_stats_player)
	{
		ent->client->p_stats_player->dropped = true;
		ent->client->p_stats_player->info.teamnum = ent->client->ctf.teamnum;
		ent->client->p_stats_player = NULL;
	}

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == false)
	{
		// clear the respawning variables
		InitClientResp (ent->client);
		if (!game.autosaved || !ent->client->pers.weapon)
			InitClientPersistant (ent->client);
	}

	ClientUserinfoChanged (ent, userinfo);

#ifdef OLDOBSERVERCODE
	// CTF CODE -- LM_JORM
	value = Info_ValueForKey (userinfo, "autoobserve");
	if (value[0] == '1')
		ent->client->autoobserve = 1;
	else
		ent->client->autoobserve = 0;
#endif

	// Hati - copy enterframe to ctf struct and clear popup entity
	ent->client->ctf.original_enterframe = level.framenum;
	ent->client->ctf.popup_ent = NULL;
	ent->client->last_popup_frame = 0;

	// Hati - initialize spam control variables
	ent->client->spam_band_count = CTF_SPAM_BAND_MAX;
	ent->client->spam_freq_count = CTF_SPAM_FREQ_MIN;
	ent->client->spam_freq_time = level.time;
	
	// STATS-BEGIN LM_Hati
	{
		stats_player_s *p_player = NULL;
		
		if (strcmp(ent->client->pers.netname, "") != 0) //not a blank string
		{
			p_player = stats_find_dropped_player(ent->client->pers.netname);
		}
		if (p_player == NULL)
		{
			p_player = stats_new_player(ent->client->pers.netname);
		}
		ent->client->p_stats_player = p_player;
	}
	// STATS-END LM_Hati

	// END LM_JORM
	

	if (game.maxclients > 1)
		gi.dprintf ("%s connected\n", ent->client->pers.netname);

	ent->svflags = 0; // make sure we start with known default
	ent->client->pers.connected = true;
	return true;
}

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.
============
*/
void ClientDisconnect (edict_t *ent)
{
	int 	playernum;
	edict_t	*flag;  // CTF CODE -- LM_JORM
	char message[MAX_INFO_STRING];
	
	if (!ent->client)
		return;
	
	// STATS-BEGIN LM_Hati
	if (ent->client->p_stats_player)
	{
		ent->client->p_stats_player->dropped = true;
		ent->client->p_stats_player->info.teamnum = ent->client->ctf.teamnum;
	}
	// STATS-END LM_Hati
	
	// CTF CODE -- LM_JORM
	ent->client->ctf.extra_flags &= ~CTF_EXTRAFLAGS_REFEREE; // Turn off referee mode
	ent->client->ctf.extra_flags &= ~CTF_EXTRAFLAGS_RCON; //turn off rcon mode
	
	// Were we carrying a flag?  MJD Suggest Parens Here
	flag = ClientHasFlag(ent);
	if (flag)//surt to avoid compiler warns on linux/solaris (assignment in conditional)
	{
		if (flag->owner == ent) // We owned this flag
		{
			ctf_playerdropflag(ent, flag->item);
		}
	}
	
	if (ent->client->hook)
	{
		G_FreeEdict (ent->client->hook);
		ent->client->hook = NULL;
	}
	
	
	// END CTF CODE -- LM_JORM
	
	sl_LogPlayerDisconnect( &gi, level, ent ); // StdLog - Mark Davies
	
	sprintf(message, "%s disconnected\n", ent->client->pers.netname);
	ctf_BSafePrint(PRINT_HIGH, message);
	
	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGOUT);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.unlinkentity (ent);
	ent->s.modelindex = 0;
	ent->solid = SOLID_NOT;
	ent->inuse = false;
	ent->classname = "disconnected";
	ent->client->p_stats_player = NULL; // LM_Hati
	ent->client->pers.connected = false;

	playernum = ent-g_edicts-1;
	gi.configstring (CS_PLAYERSKINS+playernum, "");
	
	ctf_ClientDisconnect(ent);
	
}


//==============================================================


edict_t	*pm_passent;

// pmove doesn't need to know about passent and contentmask
trace_t	PM_trace (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end)
{
	if (pm_passent->health > 0)
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID);
	else
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_DEADSOLID);
}

unsigned CheckBlock (void *b, int c)
{
	int	v,i;
	v = 0;
	for (i=0 ; i<c ; i++)
		v+= ((byte *)b)[i];
	return v;
}
void PrintPmove (pmove_t *pm)
{
	unsigned	c1, c2;

	c1 = CheckBlock (&pm->s, sizeof(pm->s));
	c2 = CheckBlock (&pm->cmd, sizeof(pm->cmd));
	Com_Printf ("sv %3i:%i %i\n", pm->cmd.impulse, c1, c2);
}



/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame.
==============
*/
void ClientThink (edict_t *ent, usercmd_t *ucmd)
{
	gclient_t	*client;
	edict_t	*other;
	int		i, j;
	pmove_t	pm;
	edict_t		*flag;  // CTF CODE -- LM_JORM
	vec3_t 	offset; // CTF CODE -- LM_JORM

#ifdef ZBOT
	if (use_zbotdetect->value)
	{
		char message[100];

		if (((int) use_zbotdetect->value > 0) && ZbotCheck(ent, ucmd))
		{
			if ((int) use_zbotdetect->value == 2)
			{
				sprintf(message, "%s was kicked by ZBot detection.\n", 
					ent->client->pers.netname);
				ctf_BSafePrint(PRINT_HIGH, message);
				ForceCommand(ent, "disconnect\n");
			}
			else if ((int) use_zbotdetect->value == 1)
			{
				sprintf(message, "WARNING:  ZBOT DETECTOR THINKS %s IS A BOT.\n", 
					ent->client->pers.netname);
				ctf_BSafePrint(PRINT_HIGH, message);
			}
		}
	}
#endif
	
	level.current_entity = ent;
	client = ent->client;
	
	// CTF CODE -- LM_JORM
	//surt spam control
	ent->client->spam_band_count+=CTF_SPAM_BAND_RECOVER; //this locks based on bandwidth
	if (ent->client->spam_band_count > CTF_SPAM_BAND_MAX)
		ent->client->spam_band_count = CTF_SPAM_BAND_MAX;
	if (ent->client->spam_band_count < CTF_SPAM_BAND_MIN)
		ent->client->spam_band_count = CTF_SPAM_BAND_MIN;
	
	ent->client->spam_freq_count--; //this locks based on frequency
	if (ent->client->spam_freq_count > CTF_SPAM_FREQ_MAX)
		ent->client->spam_freq_count = CTF_SPAM_FREQ_MAX;
	if (ent->client->spam_freq_count < CTF_SPAM_FREQ_MIN)
		ent->client->spam_freq_count = CTF_SPAM_FREQ_MIN;
	//surt end spam control

	if (ent->client->ctf.printready)
		ctf_SafePrint(ent, 0, NULL); //trigger a queued print
	
	if (GamePaused() && !(ent->client->ctf.extra_flags & CTF_EXTRAFLAGS_REFEREE))
	{
		client->ps.pmove.pm_type = PM_FREEZE;
		gi.linkentity (ent); //they call this below, but we abort early here!
		return;
	}

#ifdef OLDOBSERVERCODE
	if (!ctf_validateplayer(ent,CTF_TEAM_ANYTEAM)) //surt if not on a team, you're observer 
	{
		if (ent->client->ctf.teamnum != CTF_TEAM_OBSERVER)
		{
			ctf_SetEntTeam(ent, CTF_TEAM_OBSERVER); //catch anyone on wierd teams
			Observer_Start(ent);
		}
		// Let's us check buttons in Camera Think
		client->oldbuttons = client->buttons;
		client->buttons = ucmd->buttons;
		client->latched_buttons |= client->buttons & ~client->oldbuttons;
		gi.linkentity (ent); //they call this below, but we abort early here!
	
		if (Camera_Think(ent))
			return;
	}
#endif
	
	// END CTF CODE -- LM_JORM

	if (level.intermissiontime)
	{
		client->ps.pmove.pm_type = PM_FREEZE;
		// can exit intermission after five seconds
		if (level.time > level.intermissiontime + 5.0 
			&& (ucmd->buttons & BUTTON_ANY) )
			level.exitintermission = true;
		return;
	}

	pm_passent = ent;

	if (ent->client->chase_target) {

		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

	} else {

		// set up for pmove
		memset (&pm, 0, sizeof(pm));

		if (ent->movetype == MOVETYPE_NOCLIP)
			client->ps.pmove.pm_type = PM_SPECTATOR;
		else if (ent->s.modelindex != 255)
			client->ps.pmove.pm_type = PM_GIB;
		else if (ent->deadflag)
			client->ps.pmove.pm_type = PM_DEAD;
		else
			client->ps.pmove.pm_type = PM_NORMAL;

		client->ps.pmove.gravity = sv_gravity->value;

		/* QwazyWabbit */
		/* This causes gravity to go negative if 
		   your player Z-position goes above 0. 
		   Original code by gaia, I added the 
		   macro condition. */

#ifdef WANT_FUNKY_GRAVITY
		if(ent->s.origin[2] > 0) //gaia
			client->ps.pmove.gravity = -800;
		else
			client->ps.pmove.gravity = 800;
#endif

		pm.s = client->ps.pmove;

		for (i=0 ; i<3 ; i++)
		{
			pm.s.origin[i] = ent->s.origin[i]*8;
			pm.s.velocity[i] = ent->velocity[i]*8;
		}

		if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s)))
		{
			pm.snapinitial = true;
	//		gi.dprintf ("pmove changed!\n");
		}

		pm.cmd = *ucmd;

		pm.trace = PM_trace;	// adds default parms
		pm.pointcontents = gi.pointcontents;

		// Stop that damn bobbing of the client!
		if (client->hookstate == 2 && client->hooklength < 50)
		{
			pm.s.gravity = 0;
		}
		// END CTF CODE -- LM_JORM

		// perform a pmove
		gi.Pmove (&pm);

		// save results of pmove
		client->ps.pmove = pm.s;
		client->old_pmove = pm.s;

		for (i=0 ; i<3 ; i++)
		{
			ent->s.origin[i] = pm.s.origin[i]*0.125;
			ent->velocity[i] = pm.s.velocity[i]*0.125;
		}

		VectorCopy (pm.mins, ent->mins);
		VectorCopy (pm.maxs, ent->maxs);

		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

		if (ent->groundentity && !pm.groundentity && (pm.cmd.upmove >= 10) && (pm.waterlevel == 0))
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
			PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
		}

		ent->viewheight = pm.viewheight;
		ent->waterlevel = pm.waterlevel;
		ent->watertype = pm.watertype;
		ent->groundentity = pm.groundentity;
		if (pm.groundentity)
			ent->groundentity_linkcount = pm.groundentity->linkcount;

		if (ent->deadflag)
		{
			client->ps.viewangles[ROLL] = 40;
			client->ps.viewangles[PITCH] = -15;
			client->ps.viewangles[YAW] = client->killer_yaw;
		}
		else
		{
			VectorCopy (pm.viewangles, client->v_angle);
			VectorCopy (pm.viewangles, client->ps.viewangles);
		}

		gi.linkentity (ent);

		if (ent->movetype != MOVETYPE_NOCLIP)
			G_TouchTriggers (ent);

		// touch other objects
		for (i=0 ; i<pm.numtouch ; i++)
		{
			other = pm.touchents[i];
			for (j=0 ; j<i ; j++)
				if (pm.touchents[j] == other)
					break;
			if (j != i)
				continue;	// duplicated
			if (!other->touch)
				continue;
			other->touch (other, ent, NULL, NULL);
		}

	}

	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// save light level the player is standing on for
	// monster sighting AI
	ent->light_level = ucmd->lightlevel;

	// fire weapon from final position if needed
	//bat not needed for team observers
	//if(client->ctf.teamnum != CTF_TEAM_OBSERVER_RED &&
	//	client->ctf.teamnum != CTF_TEAM_OBSERVER_BLUE)
	{
		if (client->latched_buttons & BUTTON_ATTACK)
		{
			if (client->resp.spectator) 
			{

				client->latched_buttons = 0;

				if (client->chase_target) 
				{
					client->chase_target = NULL;
					client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
				} 
				else
					GetChaseTarget(ent);

			} 
			else if (!client->weapon_thunk) 
			{
				client->weapon_thunk = true;
				Think_Weapon (ent);
			}
		}
	}

	if (client->resp.spectator) 
	{
		
		if(client->ctf.teamnum == CTF_TEAM_OBSERVER_RED &&
			!Team_Observer_OK(CTF_TEAM_RED, ent))
		{
			Cmd_Observe_f(ent, CTF_TEAM_OBSERVER);
			return;
		}
		else if(client->ctf.teamnum == CTF_TEAM_OBSERVER_BLUE &&
			!Team_Observer_OK(CTF_TEAM_BLUE, ent))
		{
			Cmd_Observe_f(ent, CTF_TEAM_OBSERVER);
			return;
		}
		
	
		//When we first start, we are observing nobody
		if(client->ctf.teamnum == CTF_TEAM_OBSERVER_RED)
		{
			if(!client->chase_target)
			{
				GetChaseTarget(ent);
				return;
			}
			else if(client->chase_target->client->ctf.teamnum != CTF_TEAM_RED)
			{
				Cmd_Observe_f(ent, CTF_TEAM_OBSERVER);
				return;
			}

		}
		else if(client->ctf.teamnum == CTF_TEAM_OBSERVER_BLUE)
		{
			if(!client->chase_target)
			{	
				GetChaseTarget(ent);
				return;
			}
			else if(client->chase_target->client->ctf.teamnum != CTF_TEAM_BLUE)
			{
				Cmd_Observe_f(ent, CTF_TEAM_OBSERVER);
				return;
			}

		}
		


		if (ucmd->upmove >= 10) 
		{
			if (!(client->ps.pmove.pm_flags & PMF_JUMP_HELD)) 
			{
				client->ps.pmove.pm_flags |= PMF_JUMP_HELD;
				if(client->chase_target)
				{
					//Debug_Show("Chase Next");
					ChaseNext(ent);
				}
				else
				{
					//Debug_Show("Get chase target");
					GetChaseTarget(ent);
				}
			}
		} 
		else
			client->ps.pmove.pm_flags &= ~PMF_JUMP_HELD;
	}
	else
	{
		// CTF CODE -- LM_JORM
		// Were we carrying a flag?  MJD Suggest Parens
		flag = ClientHasFlag(ent);
		if (flag)
		{
			// MOVE THE FLAG!
			VectorCopy(ent->s.origin, flag->s.origin);
			VectorNormalize2(ent->s.angles, offset);
			VectorScale(offset, 6, offset);
			//offset[2] = -10;
			VectorSubtract (flag->s.origin, offset, flag->s.origin);
		
		/*
		// Make sure flag is not inside another object
		tr = gi.trace (ent->s.origin, flag->mins, flag->maxs, flag->s.origin, ent, MASK_SHOT);
		VectorCopy(tr.endpos, flag->s.origin);
		*/
		
			VectorCopy(ent->velocity, flag->velocity);
			VectorCopy (offset, flag->movedir);
			vectoangles (offset, flag->s.angles);
		
		}
	
		// END CTF CODE -- LM_JORM
	}

	// update chase cam if being followed
	for (i = 1; i <= maxclients->value; i++) 
	{
		other = g_edicts + i;
		if (other->inuse && other->client->chase_target == ent)
			UpdateChaseCam(other);
	}
}

/*==============
PingAlert
-
Alert client if players fail his pingalert values
==============
*/

void PingAlert (edict_t *ent)
{
	edict_t *other;
	int j;
	char message[MAX_INFO_STRING];
	
	// Check if they are turned off
	if (!ent->client->ctf.pingalertfloor && !ent->client->ctf.pingalertceiling)
		return;
	
	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];
		if (!other->inuse)
			continue;
		//bat
		//if (!other->client || other->client->ctf.teamnum == CTF_TEAM_OBSERVER)
		if (!other->client || other->client->ctf.teamnum <= CTF_TEAM_OBSERVER)
			continue;

		
		if (ent->client->ctf.pingalertfloor &&
			other->client->ping < ent->client->ctf.pingalertfloor)
		{
			sprintf(message, "PING ALERT: %s has a %d ping (below %d).\n",
				other->client->pers.netname, other->client->ping,
				ent->client->ctf.pingalertfloor);
			ctf_SafePrint(ent, PRINT_HIGH, message);
		}
		if (ent->client->ctf.pingalertceiling &&
			other->client->ping > ent->client->ctf.pingalertceiling)
		{
			sprintf(message, "PING ALERT: %s has a %d ping (above %d).\n",
				other->client->pers.netname, other->client->ping,
				ent->client->ctf.pingalertceiling);
			ctf_SafePrint(ent, PRINT_HIGH, message);
		}
		
	}
	
}

/*
==============
ClientBeginServerFrame

This will be called once for each server frame, before running
any other entities in the world.
==============
*/
void ClientBeginServerFrame (edict_t *ent)
{
	gclient_t	*client;
	int			buttonMask;


	// CTF CODE -- LM_JORM
	RuneThinkHook(ent);
	if (VoteStarted)
		Check_Vote();               //Vampire -- Checking if the time is up for the voting menu
	// END CTF CODE -- LM_JORM
	
	if (level.intermissiontime)
		return;

	client = ent->client;

	if (deathmatch->value &&
		client->pers.spectator != client->resp.spectator &&
		(level.time - client->respawn_time) >= 5) {
		spectator_respawn(ent);
		return;
	}


	// STATS-BEGIN LM_Hati
	if (!(level.framenum % STATS_PLAYER_SAMPLE_RATE))
	{
		if (client->p_stats_player)
		{
			stats_add(ent, STATS_PING_TOTAL, client->ping);
			stats_add(ent, STATS_PING_SAMPLES, 1);
		}
		
		// Alert referee if others have failed his pingalerts
		// Only referees can have ping alerts
		if (ent->client->ctf.extra_flags & CTF_EXTRAFLAGS_REFEREE)
			PingAlert(ent);
	}
	// STATS-END LM_Hati
	
	// run weapon animations if it hasn't been done by a ucmd_t
	if (!client->weapon_thunk && !client->resp.spectator)
		Think_Weapon (ent);
	else
		client->weapon_thunk = false;


	//sprintf(DBuffer, "resp cnt %ld", Respawn_Count);
	//Debug_Show(DBuffer);


	if(ent->deadflag)
	{
		// wait for any button just going down
		if ( level.time > client->respawn_time)
		{
			// in deathmatch, only wait for attack button
			if (deathmatch->value)
				buttonMask = BUTTON_ATTACK;
			else
				buttonMask = -1;

			if ( ( client->latched_buttons & buttonMask ) ||
				(deathmatch->value && ((int)dmflags->value & DF_FORCE_RESPAWN) ) )
			{
				respawn(ent);
				client->latched_buttons = 0;
			}
		}
		return;
	}

#ifdef MONSTERS_OK
	// add player trail so monsters can follow
	if (!deathmatch->value)
		if (!visible (ent, PlayerTrail_LastSpot() ) )
			PlayerTrail_Add (ent->s.old_origin);
#endif

	client->latched_buttons = 0;

	if (!client->ctf.goodskin)
	{
		char *s, set[MAX_INFO_STRING];
		s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
		sprintf(set, "skin %s\n", s);
		ForceCommand(ent, set);
		client->ctf.goodskin = true;
	}
}




// TEAM CODE -- LM_JORM
void ClientOldSetSkin(edict_t *ent, char *skin);

void ClientSetSkin(edict_t *ent, char *skin)
{
	char	*newskin, *s;
	int 	playernum;
	
	
	if (!SkinListInUse())
	{
		ClientOldSetSkin(ent, skin);
		return;
	}
	
	// get skin
	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	
	if (!SkinValid(ent, skin))
	{
		if (SkinValid(ent, s))
			newskin = s;
		else
			newskin = SkinRandom(ent);
	}
	else
		newskin = skin;
	
	playernum = ent-g_edicts-1;
	
	// combine name and skin into a configstring
	gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, newskin) );
	
	
	Info_SetValueForKey (ent->client->pers.userinfo, "skin", newskin);
	ent->client->ctf.goodskin = false; // We need to re-force our skin
	return;
}

void ClientOldSetSkin(edict_t *ent, char *input)
{
	char	*s;
	//char *color;
	int 	playernum, err;
	
	char	dir[MAX_INFO_STRING], skin[MAX_INFO_STRING], set[MAX_INFO_STRING] ;
	char	color, gender;
	int 	num, dirvalid, skinvalid;
	int 	skinnum;
	char	*curset;
	
	switch ((int)skinset->value)
	{
	default:
	case 0:
		curset = "rb";
		break;
	case 1:
		curset = "lm";
		break;
	case 2:
		curset = "cr";
		break;
	case 3:
		curset = "w";
		break;
	}
	
	// get skin
	//s = input;
	
	//if (!s)
	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	
	//initialize memory
	num = -1;
	gender = 'u'; //unassigned
	color = 'u'; //unassigned
	
	skinvalid = 0;
	dirvalid = 0;
	skinnum = 0;
	set[0] = 0;
	dir[0] = 0;
	skin[0] = 0;
	
	// See if we have only specified a skin number
	if (sscanf(s, "%d", &skinnum))
	{
		// First, check if skin matches proper format MJD Suggest Parens
		err = sscanf(s, "%[^/]/%[^-]-%c%c%d", dir, set, &color, &gender, &num);
		if (err)
		{
			// Next, check if gender matches properly
			if (!strcmp(dir, "female"))
			{
				dirvalid = 2;
				// Check if skin is valid, ignoring color
				if (gender == 'f' && num <= 2 && num >= 1)
				{
					skinvalid = 1;
				}
			}
			else if (!strcmp(dir, "male"))
			{
				dirvalid = 1;
				// Check if skin is valid, ignoring color
				if (gender == 'm' && num <= 3 && num >= 1)
				{
					skinvalid = 1;
				}
			}
		}
	}
	
	// If our skin is valid, only change if our color doesn't match
	if (skinvalid)
	{
		// Check if color doesn't match, or skin set is wrong
		if ((ent->client->ctf.teamnum == CTF_TEAM_RED && color != 'r') ||
			(ent->client->ctf.teamnum == CTF_TEAM_BLUE && color != 'b') ||
			strcmp(set, curset))
		{
			color = (ent->client->ctf.teamnum == CTF_TEAM_RED) ? 'r' : 'b';
			Com_sprintf(skin, sizeof skin, "%s/%s-%c%c%d", dir, curset, color, gender, num);
			s = skin;
		}
	}
	else
	{
		// Did we have a valid gender?
		if (!dirvalid || dirvalid == 1)
		{
			strcpy(dir, "male");
			gender = 'm';
			color = (ent->client->ctf.teamnum == CTF_TEAM_RED) ? 'r' : 'b';
			if (!(skinnum % 4))
				num = (rand() % 3) + 1;
			else
				num = skinnum % 4;
		}
		else // female
		{
			strcpy(dir, "female");
			gender = 'f';
			color = (ent->client->ctf.teamnum == CTF_TEAM_RED) ? 'r' : 'b';
			if (!(skinnum % 3))
				num = (rand() % 2) + 1;
			else
				num = skinnum % 2;
		}
		Com_sprintf(skin, sizeof skin, "%s/%s-%c%c%d", dir, curset, color, gender, num);
		s = skin;
	}
	
	playernum = ent-g_edicts-1;
	
	// combine name and skin into a configstring
	gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s) );
	
	if (s == skin)
	{
		Info_SetValueForKey (ent->client->pers.userinfo, "skin", s);
		ent->client->ctf.goodskin = false; // We need to re-force our skin
	}
}


int Num_Of_Players(edict_t *ent, int Ctf_Team)
{
int red, blue;
edict_t		*player;

	player = ctf_findplayer(NULL, ent, CTF_TEAM_ANYTEAM);
	
	red = 0;
	blue = 0;

	while (player)
	{
		if(player->client->ctf.teamnum == CTF_TEAM_RED)
			red++;
		else if (player->client->ctf.teamnum == CTF_TEAM_BLUE)
			blue++;
		player = ctf_findplayer(player, ent, CTF_TEAM_ANYTEAM);
	}

	if(Ctf_Team == CTF_TEAM_BLUE)
		return(blue);
	else if(Ctf_Team == CTF_TEAM_RED)
		return(red);
	else
		return(0);

}


//bat
int Team_To_Join(edict_t *ent)
{
int	red, blue;
edict_t	*player;

	red = blue = 0;
	player = ctf_findplayer(NULL, ent, CTF_TEAM_ANYTEAM);
	while (player)
	{
		if (player->client->ctf.teamnum == CTF_TEAM_RED)
			red++;
		else if (player->client->ctf.teamnum == CTF_TEAM_BLUE)
			blue++;

		player = ctf_findplayer(player, ent, CTF_TEAM_ANYTEAM);
	}


	if(red == blue) // Even teams
	{
		if(redscore > bluescore)
			return(CTF_TEAM_BLUE);
		else 
			return(CTF_TEAM_RED);
	}
	else if(red > blue)
		return(CTF_TEAM_BLUE);
	else // if (blue > red)
		return(CTF_TEAM_RED);

}


// Automatically puts us on a team
// I rewrote this, but it has the same functionality as before.
//-bat
void TeamJoin (edict_t *ent)
{
int Old_Team;
int New_Team;

	Old_Team = ent->client->ctf.teamnum;
	New_Team = Team_To_Join(ent);
	
	if(Old_Team > CTF_TEAM_UNDEFINED)
	{
		ctf_SetEntTeam(ent, Old_Team);
		ClientSetSkin(ent, NULL);
	}
	else //they are observing
	{		
		if(Old_Team <= CTF_TEAM_OBSERVER)
		{
			ent->client->ctf.New_Team = New_Team;
			ForceCommand(ent, "spectator 0");
		}
		else
		{
			ctf_SetEntTeam(ent, New_Team);
			ClientSetSkin(ent, NULL);
		}

		//sprintf(DBuffer, "tj t %d p %d r %d", ent->client->ctf.teamnum,
		//	ent->client->pers.spectator, ent->client->resp.spectator);
		//Debug_Show(DBuffer);

		//ForceCommand(ent, "spectator 0");

	}	
}
// END TEAM CODE

