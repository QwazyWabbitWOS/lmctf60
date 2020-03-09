#include "g_local.h"
#include "g_ctffunc.h"
#include "stdlog.h"


//bat
#ifdef OLDOBSERVERCODE

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

// Turn us into an observer
//    And make us incapable of affecting the game
void Observer_Start (edict_t *ent)
{
	char message[MAX_INFO_STRING];

	// If we are STILL on a team yet, don't START observer mode
	if (ent->client->ctf.teamnum > CTF_TEAM_UNDEFINED)
		return;

	// remove powerups
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
 
	// clear inventory
	memset(ent->client->pers.inventory, 0, sizeof(ent->client->pers.inventory));

	if (ent->deadflag == DEAD_DEAD)
		PutClientInServer (ent);

	ent->solid = SOLID_NOT;
	gi.linkentity(ent); //always when changing solid
	ent->movetype = MOVETYPE_NOCLIP;
	ent->s.modelindex = 0;
	ent->client->ps.gunindex = 0;
	ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;

	//surt
	//your score set to 0 when you become observer
	stats_clear(ent);

	//STDLog Changed to team observer.
	sl_LogPlayerTeamChange( &gi,
		ent->client->pers.netname,
		"OBSERVER",
		level.time );

	sprintf(message, "%s is now an observer.\n", ent->client->pers.netname);
	ctf_BSafePrint(PRINT_HIGH, message);
	gi.centerprintf (ent, "You are now an observer.");

}

void Observer_Stop (edict_t *ent)
{
	char newteam[MAX_INFO_STRING];
	char message[MAX_INFO_STRING];
	
	ent->client->camera_target = 0; // clear possible chase target
	
	// If we are not on a team yet, don't stop observer mode
	if (ent->client->ctf.teamnum <= CTF_TEAM_UNDEFINED)
		return;

	if (ent->client->ctf.teamnum == CTF_TEAM_RED)
		strcpy(newteam, "the red");
	else
		strcpy(newteam, "the blue");

	sprintf(message, "%s stopped observing and joined the %s team.\n", 
		ent->client->pers.netname, newteam);
	ctf_BSafePrint(PRINT_HIGH, message);
	gi.centerprintf (ent, "You are on %s team.", newteam);

	// reset score after leaving observer mode
	stats_clear(ent);

	// locate ent at a spawn point

	PutClientInServer (ent);

	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGIN);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
}

void Camera_Start(edict_t *ent)
{
	edict_t	*other;

	if (ent->client->ctf.teamnum != CTF_TEAM_OBSERVER)
		return;

	other = ctf_findplayer(NULL, ent, CTF_TEAM_ANYTEAM); //surt
	ent->client->camera_target = other; //surt
	ent->client->ctf.extra_flags |= CTF_EXTRAFLAGS_CAMERA_LOCKED; // default to locked
	ent->client->last_popup_frame = 0;

	if (ent->client->camera_target)
		gi.centerprintf (ent, "Chase Camera ON");
	else
		gi.centerprintf (ent, "No one to chase");

}

void Camera_Stop(edict_t *ent)
{
	ent->client->camera_target = 0;
	gi.centerprintf (ent, "Chase Camera OFF");
}

qboolean Camera_Think(edict_t *ent)
{
	//surt vars
	vec_t goal_dist;
	vec3_t current_origin, goal_origin, alt_goal_origin, goal_vector;
	vec3_t current_view, goal_view;
	vec3_t forward_face, forward_view, up_face;
	edict_t * targ;
	trace_t trace;
	edict_t * player=NULL;
	edict_t * next_player=NULL;
	unsigned long cur_val=0, next_val=999999999; //this could be trouble every billion players



	if (ent->client->camera_target && //they are looking at someone
		!ctf_validateplayer(ent->client->camera_target, CTF_TEAM_ANYTEAM) ) //but that person is not valid
		ent->client->camera_target = NULL; //not watching anyone

	//this indicates a desire to chase someone else
	if (ent->client->latched_buttons & BUTTON_ATTACK)//they are pressing fire
	{
		if (ent->client->camera_target)
			cur_val = ent->client->camera_target->client->ctf.ctfid;


		//find the player with the least-greater ctfid than the current camera target
		player = ctf_findplayer(NULL, NULL, CTF_TEAM_ANYTEAM);
		while (player)
		{
			if (player->client->ctf.ctfid > cur_val)
			{
				if (player->client->ctf.ctfid < next_val)
				{
					next_val = player->client->ctf.ctfid;
					next_player = player; //found a valid player
				}
			}

			player = ctf_findplayer(player, NULL, CTF_TEAM_ANYTEAM);
		}

		if (next_player == NULL) //found nothing, so find least ctfid
		{
			next_val = cur_val = 0;
			player = ctf_findplayer(NULL, NULL, CTF_TEAM_ANYTEAM);
			while (player)
			{
				if (player->client->ctf.ctfid > cur_val)
				{
					if (player->client->ctf.ctfid < next_val)
					{
						next_val = player->client->ctf.ctfid;
						next_player = player;
					}
				}
				player = ctf_findplayer(player, NULL, CTF_TEAM_ANYTEAM);
			}
		}

		ent->client->camera_target = next_player;
		ent->client->last_popup_frame = 0;
			
		if (!ent->client->camera_target)
			gi.centerprintf (ent, "Free movement observer mode.");
		else if (ent->client->camera_target->client)
			gi.centerprintf(ent, "Now observing: [%d] %s", 
			ent->client->camera_target->client->ctf.ctfid,
			ent->client->camera_target->client->pers.netname);
	}
	if (!ent->client->camera_target) //don't need to force camera to move if chasing no one
		return false;
	if (!ctf_validateplayer(ent->client->camera_target,CTF_TEAM_ANYTEAM))
	{
		//how did this happen?
		ent->client->camera_target = NULL;
		return false;
	}

	//at this point we want to position the ent's viewpoint so they are 'watching'
	//the camera_target
	//new surt code for smooth camera
	//we'll try to close an 1/6th of the distance between us and the target per frame
	targ = ent->client->camera_target;

	VectorCopy(ent->s.origin, current_origin); //this is the current position

	VectorCopy(targ->s.origin, goal_origin); //this is a position at the entity in question

	//let's determine which way to move the camera based on the view angle for up/down
	AngleVectors(targ->client->v_angle, forward_view, NULL, NULL); //get the up facing for target

	//let's determine which way to move the camera based on body angle for forward/backward
	AngleVectors(targ->s.angles, forward_face, NULL, up_face); //get the forward facing for target

	VectorNormalize(forward_face); //get its normal
	VectorMA(goal_origin, -32, forward_face, goal_origin); //move the goal_origin behind

	VectorNormalize(forward_view); //get its normal
	VectorMA(goal_origin, -32, forward_view, goal_origin); //and move the goal_origin up

	VectorNormalize(up_face); //get its normal
	VectorMA(goal_origin, 32, up_face, goal_origin); //and move the goal_origin up

	//make sure our 'guess' position isn't out of bounds
	trace = gi.trace(targ->s.origin, vec3_origin, vec3_origin, goal_origin, targ, MASK_SOLID);
	VectorCopy(trace.endpos, goal_origin);

	VectorSubtract(goal_origin, current_origin, goal_vector); //vector to goal
	goal_dist = VectorLength(goal_vector); //distance to goal
	VectorNormalize(goal_vector); //normal vector to goal
	VectorMA(current_origin, goal_dist/6, goal_vector, alt_goal_origin); //1/8th target position

	trace = gi.trace(current_origin, vec3_origin, vec3_origin, alt_goal_origin, targ, MASK_SOLID);
	if ( 
			(trace.fraction < 0.5) || //can't get at least half way, jump position
			(ent->client->ctf.extra_flags & CTF_EXTRAFLAGS_CAMERA_LOCKED)
		) 
	{
		VectorCopy(goal_origin, alt_goal_origin); //copies the guaranteed ok goal_origin to alt
	}

	//alt_goal_origin should now be valid in any case
	trace = gi.trace(targ->s.origin, vec3_origin, vec3_origin, alt_goal_origin, targ, MASK_SOLID);

	ent->client->ps.pmove.pm_type = PM_FREEZE;

	VectorCopy(trace.endpos, ent->s.origin); //now positioned correctly

	//now we need to determine which direction to face


	VectorCopy(ent->client->v_angle, current_view);

	VectorSubtract(targ->s.origin, ent->s.origin, goal_view); //vector from us to targ
	vectoangles(goal_view, goal_view); //angle of normal

	//in the z view, we want to look where the player is looking,
	//so this code copies the up/down view angle from the camera target
	goal_view[0] = targ->client->v_angle[0];

	//and now we put a cap on this because looking at right angle up/down
	//is visually annoying
	if (goal_view[0] > 80)
		goal_view[0] = 80; //cap the angle of view for 3rd person at 80 degrees
	if (goal_view[0] < -80)
		goal_view[0] = -80;


	//now we decide where to look in the xy plane
	//the goal_view unfortunately is in a 360 circle, while the current view
	//is in the -180/180 circle, so we need to do a conversion

	if (ent->client->ctf.extra_flags & CTF_EXTRAFLAGS_CAMERA_REVERSE)
	{
		current_view[1] += 180; //funky circle conversion for vamp's buttcam
	}
	else
	{
		if (current_view[1] < 0)
			current_view[1] += 360; //correct circle conversion
	}


	if (abs(goal_view[1] - current_view[1]) <= 89) //as long as this is less than 90 degrees off
		goal_view[1] = (goal_view[1] + current_view[1] ) / 2; //gradual xy turn
	//the else case causes our goal view to be jerked slightly towards the player
	//this is a bugfix fir the circle wrap at 0 vs 360 degrees
	
	if (ent->client->ctf.extra_flags & CTF_EXTRAFLAGS_CAMERA_REVERSE)
	{
		goal_view[1] -= 180; //funky circle conversion for vamp's buttcam
	}
	else
	{
		if (goal_view[1] > 180)
			goal_view[1] -= 360; //correct circle conversion
	}

	VectorCopy(goal_view, ent->client->ps.viewangles); //face the same direction as player
	VectorCopy(goal_view, ent->client->v_angle); //same thing, i assume

	ent->viewheight = 0;
	ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
	gi.linkentity(ent);

	return true;

	//end surt code

}

#endif