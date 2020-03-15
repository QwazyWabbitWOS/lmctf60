#include "g_local.h"
#include "g_ctffunc.h"
#include "g_tourney.h"
#include "stdlog.h"
#include "p_stats.h"
#include "gslog.h"
#include "bat.h"

edict_t * ctf_findplayer(edict_t * ent_after, edict_t * ignore, int teamnum_wanted)
{
	edict_t * temp;

	if (ent_after == NULL)
		temp = g_edicts + 1;
	else
		temp = ent_after + 1;

	while ( (temp - (g_edicts + 1)) < game.maxclients)
	{
		if (ctf_validateplayer(temp, CTF_TEAM_IGNORETEAM) ) //make sure we're looking at something valid
		{
			if (temp != ignore) //null ignore would get caught by the outer loop
			{
				if (teamnum_wanted == CTF_TEAM_IGNORETEAM) //just want to find players
				{
					return temp;
				}
				else if (teamnum_wanted == CTF_TEAM_ANYTEAM) //want to find players on teams
				{
					if (temp->client->ctf.teamnum > CTF_TEAM_UNDEFINED) //CTF_TEAM_UNDEFINED or less are not real teams
						return temp;
				}
				else if (temp->client->ctf.teamnum == teamnum_wanted) //want player on specific team
				{
					return temp;
				}
			}
		}
		temp++;
	}
		
		
/*
	edict_t * temp;

	temp = G_Find (ent_after, FOFS(classname), "player");
	while ( temp != NULL )
	{
		if (ctf_validateplayer(temp, CTF_TEAM_IGNORETEAM) ) //make sure we're looking at something valid
		{
			if (temp != ignore) //null ignore would get caught by the outer loop
			{
				if (teamnum_wanted == CTF_TEAM_IGNORETEAM) //just want to find players
				{
					return temp;
				}
				else if (teamnum_wanted == CTF_TEAM_ANYTEAM) //want to find players on teams
				{
					if (temp->client->ctf.teamnum > CTF_TEAM_UNDEFINED) //CTF_TEAM_UNDEFINED or less are not real teams
						return temp;
				}
				else if (temp->client->ctf.teamnum == teamnum_wanted) //want player on specific team
				{
					return temp;
				}
			}
		}
		temp = G_Find (temp, FOFS(classname), "player");
	}
	*/

	return NULL; //didn't find anything to return before here.
}

qboolean ctf_validateplayer(edict_t * ent, int teamnum_wanted)
{
	if (ent)
	{
		if (ent->client && ent->inuse && ent->client->pers.connected && (strcmp(ent->classname,"player") == 0) )
		{
			if (!deathmatch->value) //surt, trying to catch stuff that breaks in single player
			{
				return true;
			}
			if (teamnum_wanted == CTF_TEAM_IGNORETEAM) //we don't care about team
			{
				return true; //this object was a player, regardless of team
			}
			else if (teamnum_wanted == CTF_TEAM_ANYTEAM)
			{
				if (ent->client->ctf.teamnum > CTF_TEAM_UNDEFINED && ent->client->ctf.teamnum < CTF_TEAM_LIMIT )
					return true; //this was a player, on some defined team
			}
			else if (teamnum_wanted == ent->client->ctf.teamnum) //we matched the desired team
			{
				return true; //this was a player that matched the right team number
			}
		}
	}
	return false;
}

qboolean ctf_flagatposition(vec3_t a, vec3_t b)
{
	vec3_t dir;
	vec_t dist;
	qboolean result = true;

	VectorSubtract (a, b, dir);
	dist = VectorLength(dir);
	if (dist > 32)
		result = false;
	return result;
}

qboolean ctf_flagathome(edict_t * whichflag)
{
	if (whichflag && (strcmp(whichflag->classname, "flag") == 0))
		return ctf_flagatposition(whichflag->homeposition, whichflag->s.origin);
	else
		return false;
}

//very broken for multiteam
edict_t * ctf_getteamflag(int teamnum, int teamnum_option)
{
	if (teamnum_option == CTF_TEAM_OPPOSING)
	{
		if (teamnum == CTF_TEAM_RED)
			return blueflag;
		else if (teamnum == CTF_TEAM_BLUE)
			return redflag;
		else
			return NULL;
	}
	if (teamnum == CTF_TEAM_RED)
		return redflag;
	else if (teamnum == CTF_TEAM_BLUE)
		return blueflag;
	else
		return NULL;
}

qboolean ctf_teamstring(char * buf, int teamnum, int teamnum_option)
{
	qboolean result;
	if (teamnum_option == CTF_TEAM_OPPOSING)
	{
		if (teamnum == CTF_TEAM_RED)
		{
			strcat(buf,"blue");
			result = true;
		}
		else if (teamnum == CTF_TEAM_BLUE)
		{
			strcat(buf,"red");
			result = true;
		}
		else if (teamnum == CTF_TEAM_OBSERVER_RED)
		{
			strcat(buf,"observer blue");
			result = true;
		}
		else if (teamnum == CTF_TEAM_OBSERVER_BLUE)
		{
			strcat(buf,"observer red");
			result = true;
		}
//bat
//#ifdef OLDOBSERVERCODE
		else if (teamnum == CTF_TEAM_OBSERVER)
		{
			strcat(buf,"observer");
			result = true;
		}
//#endif
		else
		{
			strcat(buf,"unknown");
			result = false;
		}
	}
	else
	{
		if (teamnum == CTF_TEAM_RED)
		{
			strcat(buf,"red");
			result = true;
		}
		else if (teamnum == CTF_TEAM_BLUE)
		{
			strcat(buf,"blue");
			result = true;
		}
//bat
//#ifdef OLDOBSERVERCODE
		else if (teamnum == CTF_TEAM_OBSERVER)
		{
			strcat(buf,"observer");
			result = true;
		}
//#endif
		else if (teamnum == CTF_TEAM_OBSERVER_RED)
		{
			strcat(buf,"observer red");
			result = true;
		}
		else if (teamnum == CTF_TEAM_OBSERVER_BLUE)
		{
			strcat(buf,"observer blue");
			result = true;
		}
		else if (teamnum == CTF_TEAM_UNDEFINED)
		{
			strcat(buf,"undefined");
			result = false;
		}
		else
		{
			strcat(buf,"unknown");
			result = false;
		}
	}

	return result;
}

//this function needs to find out if anything is wrong with the flags
qboolean ctf_validateflags()
{
	edict_t * tmp_player = NULL;
	int teamcount= CTF_TEAM_UNDEFINED+1;
	edict_t * tmp_flag = NULL;

	if (!deathmatch->value || (int)ctfflags->value & CTF_FLAGS_NOFLAGS)
		return true;
	
	if (
		  (!redflag) ||
	     (strcmp(redflag->classname, "flag") != 0) ||
		  (!redflag->item) ||
		  (redflag->flagteam != CTF_TEAM_RED)
		)
	{
		//something is wrong with redflag
		redflag = ctf_flagsearch(CTF_TEAM_RED);
		if (!redflag) //if above returned null, we need to create a new flag
			ctf_spawnflag(CTF_TEAM_RED);
	}
		
	if (
		  (!blueflag) ||
	     (strcmp(blueflag->classname, "flag") != 0) ||
		  (!blueflag->item) ||
		  (blueflag->flagteam != CTF_TEAM_BLUE)
		)
	{
		//something is wrong with redflag
		blueflag = ctf_flagsearch(CTF_TEAM_BLUE);
		if (!blueflag) //if above returned null, we need to create a new flag
			ctf_spawnflag(CTF_TEAM_BLUE);
	}

	tmp_player = G_Find (tmp_player, FOFS(classname), "player");
	while (tmp_player)
	{
		teamcount = CTF_TEAM_UNDEFINED+1;
		while (teamcount < CTF_TEAM_LIMIT)
		{
			tmp_flag = ctf_getteamflag(teamcount,CTF_TEAM_MATCHING);
			if (tmp_player->client && tmp_flag && tmp_player->client->pers.inventory[ITEM_INDEX(tmp_flag->item)] != 0) //player has flag
			{
				if (!ctf_validateplayer(tmp_player,CTF_TEAM_ANYTEAM)) //must be a valid player on valid team to hold a flag
				{
					ctf_resetflagandplayer(tmp_flag, tmp_player);				
				}
			}
			teamcount++;
		}
		tmp_player = G_Find (tmp_player, FOFS(classname), "player");
	}

	if (redflag)
		gi.linkentity(redflag); 
	else
		gi.dprintf("Error, red flag missing, please report this.\n");
	if (blueflag)
		gi.linkentity(blueflag); 
	else
		gi.dprintf("Error, blue flag missing, please report this.\n");
	

	return true;

}

edict_t * ctf_flagsearch(int whichteam)
{
	edict_t* current_flag = NULL;
	edict_t* valid_flag = NULL;
	current_flag = G_Find(current_flag, FOFS(classname), "flag");
	while (current_flag)
	{
		//is this the flag we're looking for?
		//we already know it is classname flag, what else is required?
		if (!current_flag->item) //how did this happen?
			G_FreeEdict(current_flag);
		else if (current_flag->flagteam >= CTF_TEAM_LIMIT ||
					current_flag->flagteam <= CTF_TEAM_UNDEFINED)
		{
			G_FreeEdict(current_flag); //what the heck is this?
		}
		else if (current_flag->flagteam == whichteam)
		{
			if (valid_flag != NULL) //uh oh, multiple flags
			{
				ctf_resetflagandplayer(current_flag, current_flag->owner);
				ctf_resetflagandplayer(valid_flag, valid_flag->owner);
				G_FreeEdict(current_flag); //we'll keep the first one we find
			}
			else
			{
				valid_flag = current_flag;
			}
		}

		current_flag = G_Find(current_flag, FOFS(classname), "flag");
	}

	return valid_flag; //either a flag matching the one we want, or null
}


qboolean ctf_resetflagandplayer(edict_t * whichflag, edict_t * whichplayer)
{

	if (whichflag)
	{
		VectorCopy (whichflag->homeposition, whichflag->s.origin);
		VectorCopy (whichflag->homeangles, whichflag->s.angles);	
		ctf_ResetFlagProps(whichflag);
		// Change the object's position back to it's home position
	}
	if (whichplayer)
	{
		whichplayer->s.effects &= ~EF_COLOR_SHELL;  // Turn off flag carrying affect
		ValidateSelectedItem(whichplayer);
		whichplayer->s.modelindex3 = 0; // Remove model from player
		if (whichplayer->client)
		{
			if (redflag)
				whichplayer->client->pers.inventory[ITEM_INDEX(redflag->item)] = 0;
			if (blueflag)
				whichplayer->client->pers.inventory[ITEM_INDEX(blueflag->item)] = 0;
			if (whichflag)
				whichplayer->client->pers.inventory[ITEM_INDEX(whichflag->item)] = 0;
			ValidateSelectedItem (whichplayer);
		}
	}
	return true;
}

void ctf_ResetFlagProps(edict_t * whichflag)
{
	float		*v;
	trace_t		tr;
	vec3_t		dest;

	if (whichflag->owner)
	{
		whichflag->s.modelindex = whichflag->owner->s.modelindex3; // Turn on model
	}
	if (whichflag->model)
		gi.setmodel (whichflag, whichflag->model);
	else
		gi.setmodel (whichflag, whichflag->item->world_model);
	
	v = tv(-15,-15,-15);
	VectorCopy (v, whichflag->mins);
	v = tv(15,15,33);
	VectorCopy (v, whichflag->maxs);
	
	whichflag->solid = SOLID_TRIGGER;
	gi.linkentity(whichflag); //we changed solidity

	whichflag->movetype = MOVETYPE_TOSS;
	whichflag->touch = Touch_Item; //this then calls the pickup function as a wrapper
	whichflag->owner = NULL;
	whichflag->droptime = 0;
	v = tv(0,0,-128);
	VectorAdd (whichflag->s.origin, v, dest);
	
	tr = gi.trace (whichflag->s.origin, whichflag->mins, whichflag->maxs, dest, whichflag, MASK_SOLID);
	
	VectorCopy (tr.endpos, whichflag->s.origin);
	VectorClear (whichflag->s.angles);
	
	//surt, a flag caught moving can land off the pedastal if you don't clear the velocity
	VectorClear(whichflag->velocity);
	whichflag->flags |= FL_RESPAWN;
	
	whichflag->nextthink = level.time + FRAMETIME;	
	whichflag->think = ctf_flagwave;

}

qboolean ctf_spawnflag(int teamnum)
{
	edict_t		*ent = NULL; // MJD Uninitialized
	edict_t         *spot;

	if (!deathmatch->value)
		return false;

	if ((int)ctfflags->value & CTF_FLAGS_NOFLAGS)
		return false;

	// Check if the flags already exist

	if (teamnum == CTF_TEAM_RED  && !redflag) // RED FLAG
	{	
		spot = G_Find (NULL, FOFS(classname), "info_flag_red");
		if (!spot)
		{
			spot = G_Find (NULL, FOFS(classname), "info_player_deathmatch");
			if (spot)
				spot = ctf_findflagposition(spot);

			if (!spot)
				spot = G_Find (NULL, FOFS(classname), "info_player_start");


			// Make the spawn point glow
			if (spot)
			{
				spot->classname = "info_flag_red";
				spot->s.effects |= EF_COLOR_SHELL;
				spot->s.renderfx |= RF_SHELL_RED;
			}
			else // No spawn location found -- we will fail
				return false;
		}
		
		ent = G_Spawn();
		ent->classname = ED_NewString ("flag");
		ED_CallSpawn(ent);
		
		// Model support for flags
		ent->model = "players/male/flag1.md2";
		// Model support for flags
		
		VectorCopy (spot->s.origin, ent->s.origin);
		VectorCopy (spot->s.origin, ent->homeposition);
		VectorCopy (spot->s.angles, ent->s.angles);
		VectorCopy (spot->s.angles, ent->homeangles);
		//ent->s.effects |= EF_COLOR_SHELL | EF_FLAG1;
		//ent->s.renderfx |= RF_SHELL_RED;
		ent->s.effects = EF_FLAG1;
		redflag = ent;
	}
	else if (teamnum == CTF_TEAM_BLUE && !blueflag) // BLUE FLAG
	{
		spot = G_Find (NULL, FOFS(classname), "info_flag_blue");
		if (!spot)
		{
			spot = G_Find (NULL, FOFS(classname), "info_player_deathmatch");
			if (spot)
			{
				spot = ctf_findflagposition(spot);
				spot = ctf_findflagposition(spot);
			}

			if (!spot)
				spot = G_Find (NULL, FOFS(classname), "target_changelevel");

			// Make the spawn point glow
			if (spot)
			{
				spot->classname = "info_flag_blue";
				spot->s.effects |= EF_COLOR_SHELL;
				spot->s.renderfx |= RF_SHELL_BLUE;
			}
			else // No spawn location found -- we will fail
				return false;
		}

		ent = G_Spawn();
		ent->classname = ED_NewString ("flag");
		ED_CallSpawn(ent);
		// Model support for flags
		ent->model = "players/male/flag2.md2";
		// Model support for flags

		
		VectorCopy (spot->s.origin, ent->s.origin);
		VectorCopy (spot->s.origin, ent->homeposition);
		VectorCopy (spot->s.angles, ent->s.angles);
		VectorCopy (spot->s.angles, ent->homeangles);
		//ent->s.effects |= EF_COLOR_SHELL | EF_FLAG2;
		//ent->s.renderfx |= RF_SHELL_BLUE;
		ent->s.effects = EF_FLAG2;
		blueflag = ent;
	}


	//ent->model = "players/male/test.md2";

	if (ent)
	{
		//common
		ent->flagteam = teamnum; // passed as parameter
		gi.setmodel (ent, ent->model);
		ent->takedamage = DAMAGE_NO; //no damage on flags
		ent->dontfree = 1;
		ctf_resetflagandplayer (ent, NULL);
		ent->solid = SOLID_TRIGGER;
		gi.linkentity(ent); //make it touchable
		ent->movetype = MOVETYPE_TOSS;
		ctf_ResetFlagProps(ent);
		ctf_deletespawnpointsnearflag(ent);
	}
	return true;
}

edict_t * ctf_findflagposition(edict_t *whichflag)
{
	edict_t	*bestspot;
	float	bestdistance, flagdistance;
	edict_t	*spot;
	vec3_t v;


	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		
		VectorSubtract (whichflag->s.origin, spot->s.origin, v);
		flagdistance = VectorLength(v);

		if (flagdistance > bestdistance)
		{
			bestspot = spot;
			bestdistance = flagdistance;
		}
	}

        // MJD This should have parentheses, given that you DO want to do
	// an asignment and not a ==
	spot = G_Find (spot, FOFS(classname), "info_flag_red");
	if (spot)
	{
		
		VectorSubtract (whichflag->s.origin, spot->s.origin, v);
		flagdistance = VectorLength(v);

		if (flagdistance > bestdistance)
		{
			bestspot = spot;
			bestdistance = flagdistance;
		}
	}

        // MJD This should have parentheses, given that you DO want to do
	// an asignment and not a ==
	spot = G_Find (spot, FOFS(classname), "info_flag_blue");
	if (spot)
	{
		
		VectorSubtract (whichflag->s.origin, spot->s.origin, v);
		flagdistance = VectorLength(v);

		if (flagdistance > bestdistance)
		{
			bestspot = spot;
			bestdistance = flagdistance;
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

void ctf_flagwave (edict_t *ent) //flag animation
{
	if (ent->solid != SOLID_NOT)
		ent->s.frame = 173 + (((ent->s.frame - 173) + 1) % 16);
	ent->nextthink = level.time + FRAMETIME;

	if ( (ent->droptime) && (level.time > ent->droptime + 30) )
	{
		if (!ctf_validateplayer(ent->owner,CTF_TEAM_ANYTEAM))
		{
			if (ent == redflag)
				gi.sound(ent, CHAN_CTF, gi.soundindex("ctf/r_returned.wav"), 0.8, ATTN_NONE, 0);	
			else if (ent == blueflag)
				gi.sound(ent, CHAN_CTF, gi.soundindex("ctf/b_returned.wav"), 0.8, ATTN_NONE, 0);	
			ctf_resetflagandplayer(ent, NULL);
		}
	}
}

void ctf_TossEnt(edict_t * startent, edict_t * tossent)
{
	trace_t		tr;
	vec3_t		forward,right,offset;

	AngleVectors (startent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 0, -16);
	G_ProjectSource (startent->s.origin, offset, forward, right, tossent->s.origin);
	tr = gi.trace (startent->s.origin, tossent->mins, tossent->maxs,
		tossent->s.origin, startent, CONTENTS_SOLID);
	VectorCopy (tr.endpos, tossent->s.origin);

	VectorScale(forward, 200, tossent->velocity);
	tossent->velocity[2] = 300;
}

void Drop_Flag_Think (edict_t *ent)
{
	ent->touch = Touch_Item;
	ent->owner = NULL;
	ent->think = ctf_flagwave;
	ent->nextthink = level.time + FRAMETIME;
}

extern void drop_temp_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

void ctf_playerdropflag(edict_t * whichplayer, gitem_t *item)
{
	edict_t * whichflag;
	char flagcolor[MAX_INFO_STRING];
	char message[MAX_INFO_STRING];

	flagcolor[0] = 0;
	//this function doesn't use validate player because it should always succeed
	//even for players that are not inuse

	if (whichplayer && whichplayer->client)
	{
		whichflag = ctf_getteamflag(whichplayer->client->ctf.teamnum, CTF_TEAM_OPPOSING);
		ctf_resetflagandplayer(whichflag,whichplayer);
		if (whichflag)
		{
			//reset & relink the flag
			ctf_ResetFlagProps(whichflag);
			whichflag->owner = whichplayer;
			whichflag->touch = drop_temp_touch; // untouchable?
			whichflag->think = Drop_Flag_Think;
			whichflag->nextthink = level.time + 1;

			//toss the flag out from player
			ctf_TossEnt(whichplayer, whichflag);

			//annouce to the world
			ctf_teamstring(flagcolor, whichplayer->client->ctf.teamnum, CTF_TEAM_OPPOSING);
			sprintf(message,"%s lost the %s flag.\n",
				whichplayer->client->pers.netname,
				flagcolor );
			
			stats_add(whichplayer, STATS_OFFENSE_FLAGLOST, 1); // STATS - LM_Hati

			// STDLog Flag Carrier Frag - Surt
			sl_LogScore( &gi,
				whichplayer->client->pers.netname,
				NULL,
				"FC LostFlag",
				NULL,
				0, //value of this event FIXME we need to make these symbolic surt
				level.time );
			ctf_BSafePrint(PRINT_HIGH, message);

			whichflag->droptime = level.time;
			gi.linkentity (whichflag);

		}
	}
}

qboolean ctf_flagtouch (edict_t *ent, edict_t *other)
{
	char flagcolor[MAX_INFO_STRING];
	edict_t *otherflag;       //stmt, fixed anyway.
	edict_t	*teammate;
	edict_t *assister = NULL; //surt
	char	sound[MAX_INFO_STRING];
//	int		i;
	static float last_flagtktime = 0;
	long redcount, bluecount, scorebonus;

	char message[1000], elsemessage[1000];
	sound[0] = 0;
	message[0] = 0;
	elsemessage[0] = 0;

	
	ctf_validateflags(); //make sure nothing is wierd

	// Make sure it will respawn
	ent->flags |= FL_RESPAWN;

	if(matchstate >= MATCH_RAILGUN_COUNTDOWN)
		return false;


	if (!ctf_validateplayer(other, CTF_TEAM_ANYTEAM))
		return false; //only active players may touch flag

	strcpy(flagcolor,"");
	ctf_teamstring(flagcolor, ent->flagteam, CTF_TEAM_MATCHING);

	// If it is your flag...
	if (other->client->ctf.teamnum == ent->flagteam)
	{
		// Check to see if the flag is home
//		if (FlagCompare (ent->homeposition, ent->s.origin))
		if (ctf_flagathome(ent))
		{
			// Do we have the enemy flag?
			if (other->client->pers.inventory[ITEM_INDEX(ent->item)])
			{
				//MULTIFLAG: while loop here
				// return OTHER flag to origin
				otherflag = ctf_getteamflag(other->client->ctf.teamnum,CTF_TEAM_OPPOSING);
		
				strcpy(flagcolor, "");
				ctf_teamstring(flagcolor, ent->flagteam, CTF_TEAM_OPPOSING);
				
				// Tell everyone about it
				sprintf(message, "%s captured your flag!\n",
					other->client->pers.netname);
				sprintf(elsemessage, "%s captured the %s flag.\n",
					other->client->pers.netname,
					flagcolor);

				Team_cprint(otherflag->flagteam, message, elsemessage);

				// Award Assists for captures
				assister = ctf_findplayer(NULL, NULL, other->client->ctf.teamnum);  // LM_Hati NULL second argument allows assisting yourself
				while (assister)
				{
					if (level.time < assister->client->kill_carrier_time + 6) //surt was 60 ... (this is seconds, not tenths)
					{
						sprintf(message, "%s assisted the capture by killing the flag carrier.\n",
							assister->client->pers.netname);
						ctf_BSafePrint(PRINT_HIGH, message);

						stats_add(assister, STATS_SCORE, 1);
						assister->client->resp.score += 1;
						assister->client->kill_carrier_time = 0;

						stats_add(assister, STATS_ASSISTS, 1); // STATS - LM_Hati

						// STDLog Flag Capture Frag Carrier - Surt
						sl_LogScore( &gi,
							assister->client->pers.netname,
							NULL,
							"FC Frag Assist",
							NULL,
							1, //FIXME symbolic surt
							level.time );
						
					}
					if (level.time < assister->client->return_flag_time + 3) //surt
					{
						sprintf(message, "%s assisted the capture by returning the flag.\n",
							assister->client->pers.netname);
						ctf_BSafePrint(PRINT_HIGH, message);
						stats_add(assister, STATS_SCORE, 1);
						assister->client->resp.score += 1;
						assister->client->return_flag_time = 0;

						stats_add(assister, STATS_ASSISTS, 1); // STATS - LM_Hati

						// STDLog Flag Capture Return Bonus - Surt
						sl_LogScore( &gi,
							assister->client->pers.netname,
							NULL,
							"F Return Assist",
							NULL,
							1, //bonus frags, should be symbolic, fixme surt
							level.time );

					}
					if	(level.time < assister->client->defend_flag_time + 2) //surt
					{
						sprintf(message, "%s assisted the capture by defending the flag.\n",
							assister->client->pers.netname);
						ctf_BSafePrint(PRINT_HIGH, message);
						stats_add(assister, STATS_SCORE, 1);
						assister->client->resp.score += 1;
						assister->client->defend_flag_time = 0;

						stats_add(assister, STATS_ASSISTS, 1); // STATS - LM_Hati

						// STDLog Flag Capture Defend Bonus - Surt
						sl_LogScore( &gi,
							assister->client->pers.netname,
							NULL,
							"F Defend Assist",
							NULL,
							1, //bonus frags, should be symbolic, fixme surt
							level.time );

					}

					assister = ctf_findplayer(assister, NULL, other->client->ctf.teamnum);
				}


				//gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/tele_up.wav"), 1, ATTN_NORM, 0);	//FIXME capture sound
				
				if (other->client->ctf.teamnum == CTF_TEAM_RED) // Red team
					sprintf(sound, "ctf/redscore%d.wav", (int)(skinset->value)+1);
				else if (other->client->ctf.teamnum == CTF_TEAM_BLUE) // Blue team
					sprintf(sound, "ctf/bluescore%d.wav", (int)(skinset->value)+1);
				else
					strcpy(sound, "misc/tele_up");
					
				gi.sound(ent, CHAN_CTF, gi.soundindex(sound), 1, ATTN_NONE, 0);

				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_BFG_EXPLOSION);
				gi.WritePosition (ent->s.origin);
				gi.multicast (ent->s.origin, MULTICAST_PVS);


				//  Add score to team
				stats_add(other, STATS_SCORE, CTF_CAPTURE_BONUS_CARRIER);
				other->client->resp.score += CTF_CAPTURE_BONUS_CARRIER; //5 for being the actual capturer
				stats_add(other, STATS_CAPTURES, 1);
				// STDLog Flag Capture - Surt
				sl_LogScore( &gi,
					other->client->pers.netname,
					NULL,
					"F Capture",
					NULL,
					CTF_CAPTURE_BONUS_CARRIER,
					level.time );
			
				scorebonus = CTF_CAPTURE_BONUS_TEAM;
				//surt code to give a scoring bonus for a small team capturing vs a large team

				if ( ((int)ctfflags->value) & CTF_SCORE_BALANCE)
				{
					teammate = ctf_findplayer(NULL, NULL, CTF_TEAM_ANYTEAM);
					redcount = bluecount = 1;
					while (teammate != NULL)
					{
						if (teammate->client->ctf.teamnum == CTF_TEAM_RED)
							redcount++;
						else if (teammate->client->ctf.teamnum == CTF_TEAM_BLUE)
							bluecount++;
						teammate = ctf_findplayer(teammate, NULL, CTF_TEAM_ANYTEAM);
					}
					if (other->client->ctf.teamnum == CTF_TEAM_RED)
					{
						scorebonus *= bluecount;
						scorebonus /= redcount;
					}
					else if (other->client->ctf.teamnum == CTF_TEAM_BLUE)
					{
						scorebonus *= redcount;
						scorebonus /= bluecount;
					}
				}



				//surt
				teammate = NULL;
				// MJD Parens are suggested here, but is this
				// correct?
				//surt cleaned up structure
				teammate = ctf_findplayer(NULL, NULL, other->client->ctf.teamnum);
				while (teammate)
				{
					stats_add(teammate, STATS_SCORE, scorebonus);
					teammate->client->resp.score += scorebonus;
					// STDLog Flag Capture Team Score - Surt
					sl_LogScore( &gi,
						teammate->client->pers.netname,
						NULL,
						"Team Score",
						NULL,
						scorebonus,
						level.time );

					teammate = ctf_findplayer(teammate, NULL, other->client->ctf.teamnum);
					
				}

				if (!otherflag)
				{
					ctf_validateflags();
				}
				else
				{
					ctf_resetflagandplayer(otherflag,otherflag->owner);
				}
			}
			return false; // Can't pickup your own flag
		}
		else
		{
			// Return flag to origin
			switch (ent->flagteam)
			{
			case CTF_TEAM_RED:
				gi.sound(ent, CHAN_CTF, gi.soundindex("ctf/r_returned.wav"), 0.8, ATTN_NONE, 0);	
				break;
			case CTF_TEAM_BLUE: 
				gi.sound(ent, CHAN_CTF, gi.soundindex("ctf/b_returned.wav"), 0.8, ATTN_NONE, 0);	
				break;
			}

			sprintf(message, "%s returned your flag!\n",
				other->client->pers.netname);
			sprintf(elsemessage, "%s returned the %s flag.\n",
				other->client->pers.netname,
				flagcolor);

			stats_add(other, STATS_RETURNS, 1); // STATS - LM_Hati

			stats_add(other, STATS_SCORE, 1);

        // STDLog Flag Recover - Surt
        sl_LogScore( &gi,
                     other->client->pers.netname,
                     NULL,
                     "F Return",
                     NULL,
                     1, //fixme surt symbolic
                     level.time );

			other->client->resp.score += 1;
			other->client->return_flag_time = level.time;
			
			Team_cprint(other->client->ctf.teamnum, message, elsemessage);

			// Award Assists for returns
			assister = ctf_findplayer(NULL, other, other->client->ctf.teamnum);
			while (assister)
			{
				if (level.time < assister->client->kill_carrier_time + 6) //surt was 60 ... (this is seconds, not tenths)
				{
					sprintf(message, "%s helped %s return the %s flag.\n",
						assister->client->pers.netname, other->client->pers.netname, flagcolor);
					ctf_BSafePrint(PRINT_HIGH, message);
					stats_add(assister, STATS_SCORE, 1);

					// Log Flag Recover - MarkDavies
					sl_LogScore( &gi,
						assister->client->pers.netname,
						NULL,
						"F Return Assist",
						NULL,
						1,
						level.time );

					assister->client->resp.score += 1;
					assister->client->kill_carrier_time = 0;
					
					stats_add(assister, STATS_ASSISTS, 1); // STATS - LM_Hati
				}
				assister = ctf_findplayer(assister, other, other->client->ctf.teamnum);
			}

			ctf_resetflagandplayer(ent,NULL);
			return false;
		}
	}
	else // Enemy flag
	{
		// Can't pick up the flag if it is frozen
//		if (ent->entprops & CTF_PROPS_FROZEN)
//			return false;
		if (ent->flagteam == CTF_TEAM_RED && ((int)refset->value & CTF_RED_FLAG_FROZEN) )
			return false;
		else if (ent->flagteam == CTF_TEAM_BLUE && ((int)refset->value & CTF_BLUE_FLAG_FROZEN) )
			return false;
	
		// Give us a glowing shell
		other->s.effects |= EF_COLOR_SHELL;
		if (ent->flagteam == CTF_TEAM_BLUE)
			other->s.renderfx |= RF_SHELL_RED;
		else if (ent->flagteam == CTF_TEAM_RED)
			other->s.renderfx |= RF_SHELL_BLUE;


		sprintf(message, "%s stole your flag!\n",
			other->client->pers.netname);
		sprintf(elsemessage, "%s stole the %s flag.\n",
			other->client->pers.netname,
			flagcolor);

		stats_add(other, STATS_OFFENSE_FLAG, 1); // STATS - LM_Hati
		// Log Flag Pickup - MarkDavies
		sl_LogScore( &gi,
			other->client->pers.netname,
			NULL,
			"F Pickup",
			NULL,
			0, //surt fixme symbolic
			level.time );
		
		Team_cprint(ent->flagteam, message, elsemessage);
		
		//gi.sound(ent, CHAN_AUTO, gi.soundindex("world/klaxon2.wav"), 1, ATTN_NONE, 0);	//FIXME powering up sound
//		if (FlagCompare (ent->homeposition, ent->s.origin))
		if (ctf_flagathome(ent))
		{
			gi.sound(ent, CHAN_AUTO, gi.soundindex("ctf/flagtk.wav"), 0.7, ATTN_NORM, 0);	
			switch (ent->flagteam)
			{
			case CTF_TEAM_RED:
				gi.sound(ent, CHAN_CTF, gi.soundindex("ctf/r_stolen.wav"), 0.8, ATTN_NONE, 0);	
				break;
			case CTF_TEAM_BLUE: 
				gi.sound(ent, CHAN_CTF, gi.soundindex("ctf/b_stolen.wav"), 0.8, ATTN_NONE, 0);	
				break;
			default:
				break;
			}
		}

		//surt volume was slightly too loud at 1.0
		//so always plays when stolen from home base
		//surt also irritating if dropped/stolen repeatedly
		else
		{
			//only plays every 5 seconds if not at home base
			if (level.time > (last_flagtktime + 8) )
			{
				last_flagtktime = level.time;
				switch (ent->flagteam)
				{
				case CTF_TEAM_RED:
					gi.sound(ent, CHAN_CTF, gi.soundindex("ctf/r_stolen.wav"), 0.8, ATTN_NORM, 0);	
					break;
				case CTF_TEAM_BLUE: 
					gi.sound(ent, CHAN_CTF, gi.soundindex("ctf/b_stolen.wav"), 0.8, ATTN_NORM, 0);	
					break;
				default:
					break;
				}
			}
		}

		
		ent->owner = other;
		ent->flags |= FL_RESPAWN;
		//ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
		gi.linkentity(ent); //make it touchable

		ent->nextthink = level.time + FRAMETIME;	
		ent->think = ctf_flagwave;

		ent->owner->s.modelindex3 = ent->s.modelindex; // Move mode to player
		ent->s.modelindex = 0; // Turn off model
		
		
		other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	}

	return true;

}

void ctf_deletespawnpointsnearflag (edict_t *flag)
{
	edict_t *spot;
	float	dist;		  // CTF CODE -- LM JORM
	vec3_t	v;			  // CTF CODE -- LM JORM


	spot = NULL;
	if (deathmatch->value)
	{
                // MJD This should have parentheses, given that you DO want
		// to do an asignment and not a ==
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
		while(spot)
		{
			VectorSubtract (spot->s.origin, flag->homeposition, v);
			dist = VectorLength (v);
			if (dist <= 256)			
			{
				G_FreeEdict (spot);
			}
			spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
		}
	}
}

//hook_abort
void ctf_hook_abort(edict_t *ent)
{
	if (ent && ent->client)
	{
//		ent->client->fall_time = 0;
//		ent->client->fall_value = 0;
//		ent->client->bobtime = 0.0625;
//		ent->pain_debounce_time = 0;
//		ent->client->ps.pmove.velocity[2] = 0;
//		if (ent->velocity[2] < 0)
//			ent->velocity[2] = (ent->velocity[2] * -1) /10;
		//surt, none of the above is responsible for the problem with
		//the bouncing view.

		// If we are on the ground, don't take falling damage
		if (ent->groundentity)
		{
			ent->client->oldvelocity[2] = 0;
			ent->velocity[2] = 0;
		}

		if (ent->client->pers.weapon &&
			!strcmp (ent->client->pers.weapon->pickup_name, "Grappling Hook"))
		{
			// Only stop firing grapple
			if (ent->client->weaponstate == WEAPON_FIRING)
				ent->client->weaponstate = WEAPON_READY;
		}
		ent->client->hookstate = 0; //Surt: this used to be ==, obviously broken
		ent->client->hooklength = 0;
		if (ent->client->hook)
		{
			ent->client->hook->think = NULL;
			ent->client->hook->nextthink = 0;
			ent->client->hook->hook_target = NULL;
			G_FreeEdict (ent->client->hook);
			ent->client->hook = NULL;
		}
	}
}

char *
ctf_facing(edict_t *ent)
{
	int angle;
	
	angle = (int)ent->client->v_angle[1];

	// Turn the angle positive
	angle = (angle + 720) % 360;

	if (angle < 22 || angle >= 338)
		return "s";
	else if (angle < 67)
		return "se";
	else if (angle < 112)
		return "e";
	else if (angle < 157)
		return "ne";
	else if (angle < 202)
		return "n";
	else if (angle < 247)
		return "nw";
	else if (angle < 292)
		return "w";
	else // MJD - Requires a default state. Logic removed
	     // MJD - Would a (int)(angle/67) and case statment
	     // MJD - work better?
		return "sw";
}

char *
ctf_faceNorth(edict_t *ent)
{
	int angle;
	
	angle = (int)ent->client->v_angle[1];

	// Turn the angle positive
	angle = (angle + 720) % 360;

	if (angle < 22 || angle >= 338)
		return "south";
	else if (angle < 67)
		return "southwest";
	else if (angle < 112)
		return "west";
	else if (angle < 157)
		return "northwest";
	else if (angle < 202)
		return "north";
	else if (angle < 247)
		return "northeast";
	else if (angle < 292)
		return "east";
	else // MJD See Previous Error - Requires Default
		return "southeast";
}

char *
ctf_faceEnemyFlag(edict_t *ent)
{
	int angle;
	edict_t *flag;
	vec3_t distvector, angles;

	if (ent->client->ctf.teamnum == CTF_TEAM_RED)
		flag = blueflag;
	else if (ent->client->ctf.teamnum == CTF_TEAM_BLUE)
		flag = redflag;
	else return "north";

	if (!flag) //surt no flags mode always points north.
		return "north";

	if (flag->owner)
		flag = flag->owner;

	if (flag == ent)
	{
		if (ent->client->ctf.teamnum == CTF_TEAM_RED)
			return "blueflaggone";		
		else if (ent->client->ctf.teamnum == CTF_TEAM_BLUE)
			return "redflaggone";
	}

	VectorSubtract(flag->s.origin, ent->s.origin, distvector);

	/*
	if (abs(distvector[0]) > abs(distvector[1]) &&		// 0 Biggest
		abs(distvector[0]) > abs(distvector[2]))
	{
		if (distvector[0] > 0)
			strcpy(temp, "north of ");
		else
			strcpy(temp, "south of ");
	}
	else if (abs(distvector[1]) > abs(distvector[2]))	// 1 biggest
	{
		if (distvector[1] > 0)
			strcpy(temp, "west of ");
		else
			strcpy(temp, "east of ");
	}
	*/
	vectoangles(distvector, angles);
	//angle = angles[1];
	angle = (int)ent->client->v_angle[1] - angles[1];

	// Turn the angle positive
	angle = (angle + 720) % 360;

	if (angle < 22 || angle >= 338)
		return "north";
	else if (angle < 67)
		return "northeast";
	else if (angle < 112)
		return "east";
	else if (angle < 157)
		return "southeast";
	else if (angle < 202)
		return "south";
	else if (angle < 247)
		return "southwest";
	else if (angle < 292)
		return "west";
	else // MJD See Previous Error - Requires Default
		return "northwest";
}

qboolean ctf_SpamCheck(edict_t *ent)
{
	qboolean result = false;

	if (ent->client->ctf.extra_flags & CTF_EXTRAFLAGS_REFEREE)
		result = true;

	else if (	(ent->client->spam_band_count <= 0) || //no bandwidth available
					(ent->client->spam_freq_count > CTF_SPAM_FREQ_MAX_ALLOWED) || //no frequency available
					(level.time - ent->client->spam_lock_time < CTF_SPAM_LOCKOUT_TIME)    ) //already in penalty box
	{
		ctf_SafePrint(ent, PRINT_HIGH, "That action has been blocked by spam control.\n");
		ent->client->spam_lock_time = level.time; //you triggered spam lock
	}
	else
	{
		result = true;
		
		//here we have a player who has done too many long says, or some such
		if (ent->client->spam_band_count < CTF_SPAM_BAND_WARN_LEVEL)
		{
			ctf_SafePrint(ent, PRINT_HIGH, "Warning: Approaching spam bandwidth limits.\n");
		}
		
	}

	//here we have a player who has done a few actions in a short time
	if (ent->client->spam_freq_time - level.time < CTF_SPAM_FREQ_EXTRA_PENALTY_TIME)
	{
		ent->client->spam_freq_count += CTF_SPAM_FREQ_EXTRA_PENALTY;
		if (ent->client->spam_band_count < CTF_SPAM_FREQ_BAND_EXTRA_PENALTY_LEVEL) //compound penalty for bandwidth
			ent->client->spam_freq_count += CTF_SPAM_FREQ_BAND_EXTRA_PENALTY;
	}


	ent->client->spam_freq_time = level.time; //see test of this above

/*	gi.dprintf("Time: %f Freq-Time: %f Freq-Count: %ld Lock-Time: %f Band: %ld", 
		level.time,
		ent->client->spam_freq_time,
		ent->client->spam_freq_count,
		ent->client->spam_lock_time,
		ent->client->spam_band_count
		); */

	return result;
}

void ctf_ClientDisconnect(edict_t *ent)
{
	if ((ctf_findplayer(NULL, NULL, CTF_TEAM_IGNORETEAM)) == NULL)
	{
		if (level.time > 60)
		{
			char serverstring[60];
			stats_cleanup();
			sprintf(serverstring, "exec %s\n", server_file->string);
			gi.AddCommandString("echo Server Empty, Resetting Server as precaution.\n");
			gi.AddCommandString("set refset 0\n");
			gi.AddCommandString(serverstring);
		}
	}
}

void ctf_SetEntTeam(edict_t* ent, int whatteam)
{
	ctf_SetEntTeamEx(ent, whatteam, 0);
}


void spectator_respawn (edict_t *ent);

void ctf_SetEntTeamEx(edict_t* ent, int whatteam, int nopenalty)
{
	char buf[MAX_INFO_STRING];
	edict_t *player;
	int red, blue;
	char message[MAX_INFO_STRING];
//	int Old_Team;


	//Old_Team = ent->client->ctf.teamnum;
	//
	//if(Old_Team <= CTF_TEAM_OBSERVER && whatteam >= CTF_TEAM_RED)
	//{
	//	ForceCommand(ent, "spectator 0");
	//	//ent->client->ctf.teamnum = whatteam;
	//	return;
	//}


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

	//validate the team
	//if (whatteam != CTF_TEAM_RED && whatteam != CTF_TEAM_BLUE &&
	//	whatteam != CTF_TEAM_OBSERVER)
	//	whatteam = CTF_TEAM_UNDEFINED;
	//bat
	if(whatteam >= CTF_TEAM_LIMIT || whatteam <= CTF_TEAM_MIN_LIMIT)
		whatteam = CTF_TEAM_UNDEFINED;

	ent->client->ctf.teamnum = whatteam;

	if (ent->client->p_stats_player)
		ent->client->p_stats_player->info.teamnum = whatteam;

	if (!ent || !ent->inuse || !ent->client)
		return; //can't use validate player because this gets called
	//before the ent's classname is set to player.
	
	// STDLog Team Change - Surt
	strcpy(buf,"");
	ctf_teamstring(buf, ent->client->ctf.teamnum, CTF_TEAM_MATCHING);
	sl_LogPlayerTeamChange( &gi,
		ent->client->pers.netname,
		buf,
		level.time );

	//bat
	//Let them keep their score all the time.

	//if (((ent->client->ctf.teamnum == CTF_TEAM_BLUE) && (red > blue) && (redscore > bluescore)) ||
	//	((ent->client->ctf.teamnum == CTF_TEAM_RED) && (blue > red) && (bluescore > redscore)) ||
	//	nopenalty)
	//{
	//	// No penalty for changing teams
	//}
	//else
	//{
	//	//set the client score to 0
	//	stats_clear(ent);
	//}
	

	sprintf(message,"%s is now on the %s team.\n", ent->client->pers.netname, buf);
	ctf_BSafePrint(PRINT_HIGH, message);
}

void ctf_SetLogName()
{
	//surt automated logfile renaming here
	if ( strcmp(logrename->string, "") != 0)
	{
		time_t t;
		struct tm  *ptm;
		qboolean lognamechange = false;
		
		/* Get the time */
		t = time(NULL);
		
//		sl_CloseLogFile();
		
		ptm = localtime( &t );
		if( NULL != ptm )
		{
			char timestring[MAX_INFO_STRING];
			char stdlogstring[MAX_INFO_STRING];
			
			strftime( &timestring[0],
				(sizeof(timestring)/sizeof(timestring[0]))-1,
				"%d%b%Y.log",
				ptm );
			strcpy(stdlogstring, logrename->string);
			strcat(stdlogstring, "/");
			strcat(stdlogstring, timestring);
			if (strcmp(logrename->string, stdlogstring) != 0)
				lognamechange = true;
			if (lognamechange == true)
			{
				sl_GameEnd( &gi, level); //closes the previous logfile
				gi.cvar_set("stdlogname", stdlogstring); //set the new name
				sl_Logging( &gi, GAMEVERSION );	// StdLog - Surt, print the standard header
			}
		}
	}


	//surt end of automated logfile renaming
}

void ctf_SafePrint(edict_t * ent, long print_priority, char * buf)
{
	long i,printsize;
	char *cptr = NULL;
	char tempbuf[MAX_INFO_STRING];

	if (!ctf_validateplayer(ent, CTF_TEAM_IGNORETEAM))
		return;

	if (buf != NULL) //this is a queue request
	{
		ent->client->ctf.printready = true;
		printsize = strlen(ent->client->ctf.printdata[print_priority]);
		if ( printsize < (MAX_PRINT_BUF - 2048) )
			strncat(ent->client->ctf.printdata[print_priority], buf, 2000);
	}
	else
	{
		ent->client->ctf.printready = false;
		for (i=PRINT_LOW; i<=PRINT_CHAT; i++)
		{
			printsize = strlen(ent->client->ctf.printdata[i]);
			if (printsize <= 0)
				continue;
			else if (printsize < 50) //print it all
			{
				gi.cprintf(ent, i, "%s", ent->client->ctf.printdata[i]);
				strcpy(ent->client->ctf.printdata[i], "");
				ent->client->ctf.printready = true;
			}
			else
			{
				//just print up to the nearest return
				cptr = strchr(&ent->client->ctf.printdata[i][1], '\n');
				cptr++; //include the return
				memset(tempbuf, 0, 250);
				strncpy(tempbuf, ent->client->ctf.printdata[i], (long) (cptr - ent->client->ctf.printdata[i]) );
				strcpy(ent->client->ctf.printdata[i], cptr);
				gi.cprintf(ent, i, "%s", tempbuf);
				ent->client->ctf.printready = true;
			}
		}
	}
	return;
}

void ctf_BSafePrint(long print_priority, char * buf)
{
	edict_t *ent = NULL;
//	int i;

//	for (i=0 ; i<game.maxclients ; i++)
//	{
//		ent = g_edicts + 1 + i;
//		ctf_SafePrint(ent, print_priority, buf);
//	}

	gi.dprintf(buf);
	
	ent = ctf_findplayer(ent, NULL, CTF_TEAM_IGNORETEAM);
	while (ent)
	{
		ctf_SafePrint(ent, print_priority, buf); 
		ent = ctf_findplayer(ent, NULL, CTF_TEAM_IGNORETEAM);
	}

}

void ctf_ChangeMap(char *mapname, qboolean startmatch)
{
	extern int matchstate;
    char command[MAX_INFO_STRING];
    Com_sprintf (command, sizeof(command), "gamemap \"%s\"\n", mapname);
    gi.AddCommandString (command);
    level.changemap = NULL;
    level.exitintermission = 0;
    level.intermissiontime = 0;
    stats_cleanup(); // STATS - LM_Hati
    KillMatch();
    if (startmatch)
		matchstate = MATCH_COUNTDOWN;
	else
		matchstate = MATCH_NONE;
}
