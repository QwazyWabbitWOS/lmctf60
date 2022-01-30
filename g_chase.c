#include "g_local.h"
#include "g_ctffunc.h"
#include "bat.h"
#include "g_tourney.h"

int Num_Of_Players(edict_t *ent, int Ctf_Team);

void UpdateChaseCam(edict_t *ent)
{
	vec3_t goal, forward, right, angles;
	edict_t *targ;
	trace_t trace;
	int i;

	// is our chase target gone?
	if (!ent->client->chase_target->inuse
		|| ent->client->chase_target->client->resp.spectator) {
		edict_t *old = ent->client->chase_target;
		ChaseNext(ent);
		if (ent->client->chase_target == old) {
			ent->client->chase_target = NULL;
			ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
			return;
		}
	}

	targ = ent->client->chase_target;
	VectorCopy(targ->client->v_angle, angles);
	VectorCopy (targ->s.origin, goal);
	goal[2] += targ->viewheight;

	vec3_t	targorigin;

	VectorCopy (goal, targorigin);

	AngleVectors (angles, forward, right, NULL);
	VectorMA (goal, 30, forward, goal);

	// trace from targorigin to final chase origin goal
	trace = gi.trace (targorigin, vec3_origin, vec3_origin, goal, targ, MASK_SOLID);

	// test for hit so we don't go out of the map!
	if (trace.fraction < 1) {
		vec3_t	temp;

		// we hit something, need to do a bit of avoidance

		// take real end point
		VectorCopy (trace.endpos, goal);

		// real dir vector
		VectorSubtract (goal, targorigin, temp);

		// scale it back bit more
		VectorMA (targorigin, 0.9f, temp, goal);
	}

	VectorCopy(goal, ent->s.origin);
	for (i=0 ; i<3 ; i++) {
		ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(targ->client->v_angle[i] - ent->client->resp.cmd_angles[i]);
	}

	if (targ->deadflag) {
		ent->client->ps.viewangles[ROLL] = 40;
		ent->client->ps.viewangles[PITCH] = -15;
		ent->client->ps.viewangles[YAW] = targ->client->killer_yaw;
		ent->client->ps.pmove.pm_type = PM_DEAD;
	} else {
		VectorCopy(targ->client->v_angle, ent->client->ps.viewangles);
		VectorCopy(targ->client->v_angle, ent->client->v_angle);
		ent->client->ps.pmove.pm_type = PM_FREEZE;
	}

	ent->viewheight = 0;
	ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
	gi.linkentity(ent);
}

void ChaseNext(edict_t *ent)
{
int i;
edict_t *e;
int Chase_Team; //-bat

	if(!ent->client->chase_target)
		return;

	Chase_Team = ent->client->ctf.teamnum;

	i = ent->client->chase_target - g_edicts;
	do 
	{
		i++;
		if (i > maxclients->value)
			i = 1;
		e = g_edicts + i;
		if(!e->inuse)
			continue;

		//bat
		if(((Chase_Team == CTF_TEAM_OBSERVER_RED) && (e->client->ctf.teamnum != CTF_TEAM_RED))
			|| ((Chase_Team == CTF_TEAM_OBSERVER_BLUE) && (e->client->ctf.teamnum != CTF_TEAM_BLUE)))
			continue;


		if (!e->client->resp.spectator)
			break;
	} while (e != ent->client->chase_target);

	ent->client->chase_target = e;
	ent->client->update_chase = true;
}

void ChasePrev(edict_t *ent)
{
int i;
edict_t *e;
int Chase_Team; //-bat
int Team_Observe_OK;

	if(!ent->client->chase_target)
		return;

	Chase_Team = ent->client->ctf.teamnum;

	if(Chase_Team == CTF_TEAM_OBSERVER_RED || Chase_Team == CTF_TEAM_OBSERVER_BLUE)
		Team_Observe_OK = false;
	else
		Team_Observe_OK = true;

	i = ent->client->chase_target - g_edicts;
	do 
	{
		i--;
		if(i < 1)
			i = maxclients->value;
		e = g_edicts + i;
		if(!e->inuse)
			continue;

		//bat
		if(((Chase_Team == CTF_TEAM_OBSERVER_RED) && (e->client->ctf.teamnum != CTF_TEAM_RED))
			|| ((Chase_Team == CTF_TEAM_OBSERVER_BLUE) && (e->client->ctf.teamnum != CTF_TEAM_BLUE)))
			continue;

		if(!e->client->resp.spectator)
		{
			Team_Observe_OK = true;
			break;
		}
	} while (e != ent->client->chase_target);

	if(Team_Observe_OK)
	{
		ent->client->chase_target = e;
		ent->client->update_chase = true;
	}
}


//bat
int Team_Observer_OK(int Team_To_View, edict_t *ent)
{
	if(Num_Of_Players(ent, Team_To_View) > 0)
		return(true);
	
	if(Team_To_View == CTF_TEAM_RED)
		gi.centerprintf(ent, "No red players to chase.");
	else
		gi.centerprintf(ent, "No blue players to chase.");

	return(false);
}


void GetChaseTarget(edict_t *ent)
{
int i;
edict_t *other;
int Chase_Team; //-bat

	Chase_Team = ent->client->ctf.teamnum;

	for(i = 1; i <= maxclients->value; i++) 
	{
		//other is the guy we are chasing
		other = g_edicts + i;

		//-bat
		if(((Chase_Team == CTF_TEAM_OBSERVER_RED) && (other->client->ctf.teamnum != CTF_TEAM_RED))
			|| ((Chase_Team == CTF_TEAM_OBSERVER_BLUE) && (other->client->ctf.teamnum != CTF_TEAM_BLUE)))
			continue;

		if(other->inuse && !other->client->resp.spectator) 
		{
			ent->client->chase_target = other;
			ent->client->update_chase = true;
			UpdateChaseCam(ent);
			return;
		}
	}

	gi.centerprintf(ent, "No other players to chase.");
}

