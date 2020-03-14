#include "g_local.h"
#include "g_ctffunc.h"
#include "bat.h"
#include "g_tourney.h"

extern int matchstate;

int Num_Of_Players(edict_t *ent, int Ctf_Team);

void UpdateChaseCam(edict_t *ent)
{
	vec3_t o, ownerv, goal;
	edict_t *targ;
	vec3_t forward, right;
	trace_t trace;
	int i;
	vec3_t angles;

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

	VectorCopy(targ->s.origin, ownerv);

	ownerv[2] += targ->viewheight;

	VectorCopy(targ->client->v_angle, angles);
	if (angles[PITCH] > 56)
		angles[PITCH] = 56;
	AngleVectors (angles, forward, right, NULL);
	VectorNormalize(forward);
	VectorMA(ownerv, -30, forward, o);

	if (o[2] < targ->s.origin[2] + 20)
		o[2] = targ->s.origin[2] + 20;

	// jump animation lifts
	if (!targ->groundentity)
		o[2] += 16;

	trace = gi.trace(ownerv, vec3_origin, vec3_origin, o, targ, MASK_SOLID);

	VectorCopy(trace.endpos, goal);

	VectorMA(goal, 2, forward, goal);

	// pad for floors and ceilings
	VectorCopy(goal, o);
	o[2] += 6;
	trace = gi.trace(goal, vec3_origin, vec3_origin, o, targ, MASK_SOLID);
	if (trace.fraction < 1) {
		VectorCopy(trace.endpos, goal);
		goal[2] -= 6;
	}

	VectorCopy(goal, o);
	o[2] -= 6;
	trace = gi.trace(goal, vec3_origin, vec3_origin, o, targ, MASK_SOLID);
	if (trace.fraction < 1) {
		VectorCopy(trace.endpos, goal);
		goal[2] += 6;
	}

	if (targ->deadflag)
		ent->client->ps.pmove.pm_type = PM_DEAD;
	else
		ent->client->ps.pmove.pm_type = PM_FREEZE;

	VectorCopy(goal, ent->s.origin);
	for (i=0 ; i<3 ; i++)
		ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(targ->client->v_angle[i] - ent->client->resp.cmd_angles[i]);

	if (targ->deadflag) {
		ent->client->ps.viewangles[ROLL] = 40;
		ent->client->ps.viewangles[PITCH] = -15;
		ent->client->ps.viewangles[YAW] = targ->client->killer_yaw;
	} else {
		VectorCopy(targ->client->v_angle, ent->client->ps.viewangles);
		VectorCopy(targ->client->v_angle, ent->client->v_angle);
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

