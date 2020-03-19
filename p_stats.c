#include <string.h>
#include "g_local.h"
#include "g_tourney.h"
#include "g_ctffunc.h"

stats_player_s *p_start_player = NULL;

void stats_log_init()
{
	p_start_player = NULL;
}

void stats_log_reset()
{
	stats_player_s * p_current_player;

	p_current_player = p_start_player;
	while (p_current_player != NULL)
	{
		p_start_player = p_start_player->pNext;
		gi.TagFree(p_current_player);
		p_current_player = p_start_player;
	}

	stats_log_init();
}

stats_player_s* stats_find_dropped_player(char* name)
{
	stats_player_s *p_current_player = p_start_player;
	
	while (p_current_player != NULL)
	{
		if ((strcmp(p_current_player->info.name, name) == 0) &&
			p_current_player->dropped)
			break;
		else
			p_current_player = p_current_player->pNext;
	}

	return p_current_player;
}

void stats_init_player(stats_player_s *p_player)
{
	// set up initial stats for player
	int i;
	for (i=0; i < MAX_PLAYER_STATS; i++)
		p_player->stats[i] = 0;
//	p_player->dropped = false;
}

stats_player_s* stats_new_player(char* name)
{
	stats_player_s* p_player;
	
	p_player = (stats_player_s *) gi.TagMalloc(sizeof(stats_player_s), TAG_GAME);
	if (!p_player) {
		gi.error(ERR_FATAL, "LMCTF: allocation failed in %s", __func__);
		return NULL;
	}

	stats_init_player(p_player);
	p_player->dropped = false;
	p_player->info.teamnum = CTF_TEAM_UNDEFINED;
	strcpy(p_player->info.name, name);

	//attach the player to the front of the list
	p_player->pNext = p_start_player;
	p_start_player = p_player;

	return p_player;
}

void stats_set_name(edict_t *ent, char *name)
{
	
	if (ent->client->p_stats_player)
	{
		/* here should check for duplicate name
		   if duplicate name is found, disallow change,
		   and force back to original name  */
		strcpy(ent->client->p_stats_player->info.name, name);
	}
	return;
}

void stats_cleanup()
{
	stats_player_s *p_current_player, *p_prev_player;

	// clear out dropped players from start and
	// adjust start if needed
	p_current_player = p_start_player;
	while ( p_current_player && (p_current_player->dropped))
	{
		p_start_player = p_current_player->pNext;
		gi.TagFree(p_current_player);
		p_current_player = p_start_player;

	}

	// case when everyone is gone
	if (p_start_player == NULL)
	{
		return;
	}

	stats_init_player(p_start_player);

	// clear out dropped players and reinitialize stats
	p_current_player = p_start_player->pNext; // When the start is valid
	p_prev_player = p_start_player;

	while (p_current_player)
	{
		if (p_current_player->dropped)
		{
			p_prev_player->pNext = p_current_player->pNext;
			gi.TagFree(p_current_player);
		}
		else
		{
			stats_init_player(p_current_player);
			p_prev_player = p_current_player;
		}
		p_current_player = p_prev_player->pNext;

	}
	
	if (p_current_player)
		p_current_player->pNext = NULL;
}

void stats_add(edict_t *ent, int stat, long amount)
{
	if (Match_CanScore() && ent->client->p_stats_player)
		ent->client->p_stats_player->stats[stat] += amount;
}

void stats_set(edict_t *ent, int stat, long amount)
{
	if (Match_CanScore() && ent->client->p_stats_player)
		ent->client->p_stats_player->stats[stat] = amount;
}

long stats_get(edict_t *ent, int stat)
{
	if (ent->client && ent->client->p_stats_player)
		return (ent->client->p_stats_player->stats[stat]);
	else
		return 0;
}


void stats_clear(edict_t *ent)
{
	if (!ent || !ent->client || !ent->client->p_stats_player)
		return;

	stats_init_player(ent->client->p_stats_player);
	ent->client->resp.score = 0;
}


void stats_output(edict_t *ent, stats_player_s *p_player)
{
	int total_encounters;
	char teambuf[MAX_INFO_STRING];
	char *conbuf;
	char outbuf[512];
	char tmpbuf[512];

	strcpy(teambuf,"");
	ctf_teamstring(teambuf, p_player->info.teamnum, CTF_TEAM_MATCHING);

	if (p_player->dropped)
		conbuf = "quit";
	else
		conbuf = "active";

	
	
	total_encounters = p_player->stats[STATS_FRAGS] + p_player->stats[STATS_DEATHS];

	strcpy(outbuf,"");


	sprintf (tmpbuf, "\n(%s) [%s] %s\n", teambuf, conbuf, p_player->info.name);
	strcat(outbuf,tmpbuf);

	sprintf (tmpbuf,
					"Score=%ld "
					"Frags=%ld "
					"Deaths=%ld "
					"Eff=%ld%%\n",
					p_player->stats[STATS_SCORE], p_player->stats[STATS_FRAGS], p_player->stats[STATS_DEATHS],
					total_encounters == 0 ? 0 : 100 * p_player->stats[STATS_FRAGS] / total_encounters);
	strcat(outbuf,tmpbuf);

	sprintf (tmpbuf, "Def Base=%ld Def Flag=%ld Def Carrier=%ld\nGot Flag=%ld Lost Flag=%ld Captures=%ld\n", 
				p_player->stats[STATS_DEFENSE_BASE],
				p_player->stats[STATS_DEFENSE_FLAG],
				p_player->stats[STATS_DEFENSE_CARRIER],
				p_player->stats[STATS_OFFENSE_FLAG],
				p_player->stats[STATS_OFFENSE_FLAGLOST],
				p_player->stats[STATS_CAPTURES]);
	strcat(outbuf,tmpbuf);

	sprintf (tmpbuf, "Kill Carrier=%ld Flag Returns=%ld Assists=%ld\nAverage Ping=%ld Samples=%ld\n",
					p_player->stats[STATS_OFFENSE_CARRIER],
					p_player->stats[STATS_RETURNS],
					p_player->stats[STATS_ASSISTS],
					p_player->stats[STATS_PING_TOTAL] / (p_player->stats[STATS_PING_SAMPLES] > 0 ? p_player->stats[STATS_PING_SAMPLES] : 1),
					p_player->stats[STATS_PING_SAMPLES]);
	strcat(outbuf,tmpbuf);
	ctf_SafePrint(ent, PRINT_HIGH, outbuf);

}

void Cmd_PlayerStats_f (edict_t *ent)
{
	edict_t *temp, *target;
	stats_player_s *p_player;
	int i;
	char *p;
	char lowerstr[MAX_INFO_STRING];

	p = gi.args();

	if (p && strlen(p))
	{
		LowerCase(p);
		target = NULL;
		for (i=0 ; i<game.maxclients ; i++)
		{
			temp = g_edicts + 1 + i;
			strcpy(lowerstr, temp->client->pers.netname);
			LowerCase(lowerstr);
			if (strstr(lowerstr, p))
			{
				target = temp;
				break;
			}
		}
	}
	else
		target = ent;

	if (!target)
	{
		ctf_SafePrint(ent, PRINT_HIGH, "Cannot find a matching player.\n");
		return;
	}

	p_player = target->client->p_stats_player;
	stats_output(ent, p_player);

}

void Cmd_StatsAll_f (edict_t *ent)
{
	stats_player_s *p_player;

	p_player = p_start_player;
	while (p_player != NULL)
	{
		stats_output(ent, p_player);
		p_player = p_player->pNext;
	}
}

