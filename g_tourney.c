#include "g_local.h"
#include "g_ctffunc.h"
#include "stdlog.h"
#include "g_tourney.h"
#include "bat.h"

int matchstate = MATCH_NONE;
edict_t *tourneyclock = NULL;
qboolean match_pause = false;
edict_t *omvp = NULL, *dmvp = NULL;

edict_t *Reg_Clock = NULL;

extern stats_player_s *p_start_player;

extern edict_t *Railgun_Victor;
int One_Man_Left(void);
edict_t *Declare_Railgun_Victor(void);

void Use_Quad (edict_t *ent, gitem_t *item);


void Reset_MVP()
{
    omvp = dmvp = NULL;
}

edict_t *Query_OMVP()
{
    return omvp;
}

edict_t *Query_DMVP()
{
    return dmvp;
}

void Match_Start(edict_t *ent)
{
    int i;
    edict_t *player;
    char message[MAX_INFO_STRING];
    int player_count = 0;
    gitem_t *Item;
    //edict_t    *Item_ent;

    // Blank all players' stats, and respawn them
    for (i=0 ; i < game.maxclients ; i++) {
        player = g_edicts + 1 + i;

        // Don't bother killing them if they are an observer
        if (player->inuse && !player->client->resp.spectator) {
            player_count++;

            if(matchstate != MATCH_RAILGUN_COUNTDOWN) {
                player->health = 0;
                player_die (player, player, player, 100000, vec3_origin);
                // don't even bother waiting for death frames

                // Start our time over so we respawn on team spawn point
                player->client->resp.enterframe = level.framenum;

                //bat force them to hit the button to respawn.
                //took out respawn() and added a second to the respawn time
                //Too Many overflows!!!!!
                //respawn (player);
                //This is in seconds!!!

                //Let's try this in the respawn instead;
                //player->client->respawn_time = level.time + (0.2 * player_count);

                stats_clear(player); // Blank all our stats, whether we are here or not
            } else {
                player->health = 100;
                player->client->pers.weapon = FindItem ("railgun");;
                player->client->newweapon = player->client->pers.weapon;
                Item = FindItem("slugs");
                Add_Ammo(player, Item, 1000);
                ChangeWeapon(player);
            }
        }
    }

    if(matchstate == MATCH_RAILGUN_COUNTDOWN) {
        matchstate = MATCH_RAILGUN_INPLAY;
        ent->count = railtime->value;
        sprintf(message, "%d seconds. %d men enter 1 man leaves \n", ent->count, player_count);
        ctf_BSafePrint(PRINT_HIGH, message);
    } else {
        matchstate = MATCH_INPLAY;
        ent->count = ((unsigned short)timelimit->value) * 60;
        sprintf(message, "%d minutes until match ends.\n", ent->count/60);
        ctf_BSafePrint(PRINT_HIGH, message);
    }
}

void Victory()
{
    long i;
    edict_t *ent=NULL;
    long redscore, bluescore;
    long temp, oscore, dscore;
    char victory_buf[MAX_INFO_STRING];
    char temp_buf[MAX_INFO_STRING];
    char teambuf[MAX_INFO_STRING];

    bluescore = redscore = 0;
    oscore = dscore = 0;
    dmvp = omvp = NULL;

    strcpy(victory_buf,"");

    for (i=0 ; i<game.maxclients ; i++) {
        ent = g_edicts + 1 + i;

        if (!ent->inuse) {
            continue;
        }

        if (ent->client->ctf.teamnum == CTF_TEAM_RED) { // RED TEAM
            redscore += stats_get(ent, STATS_SCORE);
        } else if (ent->client->ctf.teamnum == CTF_TEAM_BLUE) { // BLUE TEAM
            bluescore += stats_get(ent, STATS_SCORE);
        }
    }

    // Find the dmvp
    for (i=0 ; i<game.maxclients ; i++) {
        ent = g_edicts + 1 + i;

        temp = 4*stats_get(ent, STATS_OFFENSE_CARRIER) +
            3*stats_get(ent, STATS_DEFENSE_FLAG) +
            2*stats_get(ent, STATS_RETURNS) +
            stats_get(ent, STATS_DEFENSE_CARRIER) +
            stats_get(ent, STATS_DEFENSE_BASE);

        if (temp > dscore) {
            dscore = temp;
            dmvp = ent;
        }
    }

    // Find the omvp
    for (i=0 ; i<game.maxclients ; i++) {
        ent = g_edicts + 1 + i;

        // Exclude dmvp
        if (ent == dmvp) {
            continue;
        }

        temp = 8*stats_get(ent, STATS_CAPTURES) + 4*stats_get(ent, STATS_DEFENSE_CARRIER)
            + stats_get(ent, STATS_ASSISTS);

        if (temp > oscore) {
            oscore = temp;
            omvp = ent;
        }
    }

    if (dmvp) {
        strcpy(teambuf,"");
        ctf_teamstring(teambuf,dmvp->client->ctf.teamnum,CTF_TEAM_MATCHING);

        Com_sprintf(temp_buf, sizeof temp_buf, "Defense MVP: %s (%s)!\n",
            dmvp->client->pers.netname,
            teambuf);
        strcat(victory_buf,temp_buf);
    }

    if (omvp) {
        strcpy(teambuf,"");
        ctf_teamstring(teambuf,omvp->client->ctf.teamnum,CTF_TEAM_MATCHING);

        Com_sprintf(temp_buf, sizeof temp_buf, "Offense MVP: %s (%s)!\n",
            omvp->client->pers.netname,
            teambuf);
        strcat(victory_buf, temp_buf);
    }

    if (bluescore > redscore) {
        gi.sound (ent, CHAN_CTF, gi.soundindex ("ctf/end_blue.wav"), 1, ATTN_NONE, 0);
        sprintf(temp_buf, "Blue: %ld beats red: %ld!\n", bluescore,redscore);
        strcat(victory_buf, temp_buf);
    } else if (redscore > bluescore) {
        gi.sound (ent, CHAN_CTF, gi.soundindex ("ctf/end_red.wav"), 1, ATTN_NONE, 0);
        sprintf(temp_buf, "Red: %ld beats blue: %ld!\n", redscore,bluescore);
        strcat(victory_buf, temp_buf);
    } else {
        gi.sound (ent, CHAN_CTF, gi.soundindex ("ctf/end_tie.wav"), 1, ATTN_NONE, 0);
        sprintf(temp_buf, "Tie game at %ld!\n", redscore);
        strcat(victory_buf, temp_buf);
    }

    ctf_BSafePrint(PRINT_HIGH, victory_buf);
    ctf_SetLogName(); //automated log rename check each level
    //sl_Logging( gi, NULL );
    //this will cause the name of the log to change when the day number changes
    //which hypothetically would be midnight
}

void Match_End(edict_t *ent)
{
    Victory();

    matchstate = MATCH_OVER;
    ent->count = 300; // five minutes
    game.teamslocked = false;
}

qboolean Match_InCountdown()
{
    return matchstate == MATCH_COUNTDOWN;
}

qboolean Match_InPlay()
{
    return matchstate == MATCH_INPLAY;
}

qboolean Match_Mode()
{
    return matchstate > MATCH_ENDLEVEL;
}

qboolean Match_CanScore()
{
    if ((matchstate == MATCH_OVER) || (matchstate == MATCH_COUNTDOWN) || (matchstate == MATCH_RAILGUN_COUNTDOWN)) {
        return false;
    } else {
        return true;
    }
}

qboolean Match_Over()
{
    return matchstate == MATCH_OVER;
}

qboolean GamePaused()
{
    return match_pause;
}

void SetPause(qboolean state)
{
    edict_t *ent;
    char *message;
    int i;

    match_pause = state;

    if (state) {
    	if ((int)autolock->value) {
    		game.teamslocked = false;
    	}
        message = "Game Paused\n";
    } else {
    	if ((int)autolock->value) {
    		game.teamslocked = true;
    	}
        message = "Game Unpaused\n";
    }

    for (i=0 ; i<game.maxclients ; i++) {  // Go through everyone
        ent = g_edicts + 1 + i;            // Select the client entity from the list of ents.
        if (!ent->inuse) {                 // Not in game yet is what I think this means.
            continue;
        }
        gi.centerprintf(ent, message);
    }
    gi.dprintf(message);
}


char *CTF_Countdown_Table[11] = {
    "ctf/go.wav",
    "ctf/1.wav",
    "ctf/2.wav",
    "ctf/3.wav",
    "ctf/4.wav",
    "ctf/5.wav",
    "ctf/6.wav",
    "ctf/7.wav",
    "ctf/8.wav",
    "ctf/9.wav",
    "ctf/10.wav",
};

short Last_Guy       = 0;
short Position_Count = 0;
int   Time_Left      = 0;

void Tourney_Think(edict_t *ent)
{
    int     minutes;
    edict_t *player;
    int     i;
    char    message[MAX_INFO_STRING];

    ent->nextthink = level.time + 1;

    // If game paused, don't keep counting down
    if (GamePaused()) {
        return;
    }

    minutes = ent->count/60;

    if (matchstate == MATCH_OVER) {
        Time_Left = 0;
    } else if (ent->count < 60) {
        Time_Left = ent->count;
    } else {
        Time_Left = minutes + 1;
    }


    if (matchstate == MATCH_COUNTDOWN || matchstate == MATCH_RAILGUN_COUNTDOWN) {
        if(ent->count <= 10) {
            gi.sound (ent, CHAN_CTF, gi.soundindex (CTF_Countdown_Table[ent->count]), 1, ATTN_NONE, 0);
        }

        switch (ent->count) {
        case 60:
            ctf_BSafePrint(PRINT_HIGH, "60 seconds until match begins.\n");
            break;
        case 30:
            ctf_BSafePrint(PRINT_HIGH, "30 seconds until match begins.\n");
            break;
        case 15:
            if (matchstate == MATCH_RAILGUN_COUNTDOWN) {
                ctf_BSafePrint(PRINT_HIGH, "Prepare to annihilate your enemy...\n");

                gi.sound(ent, CHAN_CTF, gi.soundindex ("weapons/bfg__l1a.wav"), 1, ATTN_NONE, 0);
            } else {
                ctf_BSafePrint(PRINT_HIGH, "15 seconds until match begins.\n");
            }
            break;
        case 0:
        //    gi.sound (ent, CHAN_CTF, gi.soundindex ("ctf/go.wav"), 1, ATTN_NONE, 0);
            Last_Guy = 0;
            Position_Count = 0;
            Match_Start(ent);
            break;
        }
    } else if (matchstate == MATCH_RAILGUN_INPLAY) {
        edict_t *cl_ent;

        if (++Position_Count == 6) {
            cl_ent = g_edicts + 1 + Last_Guy;

            while (cl_ent->health <= 0) {
                Last_Guy++;

                if (Last_Guy == game.maxclients) {
                    Last_Guy = 0;
                    break;
                }
            }

            cl_ent = g_edicts + 1 + Last_Guy;

            if(cl_ent->health > 0) {
                ForceCommand(cl_ent, "say I am %p");
            }

            Last_Guy++;
            Position_Count = 0;
        }


        if (ent->count == 0 || One_Man_Left()) {
            Railgun_Victor = Declare_Railgun_Victor();
            matchstate = MATCH_RAILGUN_OVER;
        } else if(ent->count <= 15) {
            sprintf(message, "%d\n", ent->count);
            ctf_BSafePrint(PRINT_HIGH, message);

            if (ent->count <= 10) {
                gi.sound (ent, CHAN_CTF, gi.soundindex (CTF_Countdown_Table[ent->count]), 1, ATTN_NONE, 0);
            }
        }

    } else if (matchstate == MATCH_INPLAY) {
        // Start the countdown if we hit the fraglimit
        if (ent->count > 10) {
            for (i=0 ; i<maxclients->value ; i++) {
                player = g_edicts + 1 + i;
                if (!player->inuse) {
                    continue;
                }

                if (fraglimit->value && stats_get(player, STATS_SCORE) >= fraglimit->value) {
                    // Fraglimit was hit
                    ent->count = 10;
                }
            }
        }

        if (ent->count <= 0) { // End match
            Match_End(ent);
            return;
        } else if (!(ent->count % 60)) {
            if (minutes > 1) {
                sprintf(message, "%d minutes until match ends.\n", minutes);
            } else if (minutes == 1) {
                sprintf(message, "%d minute until match ends.\n", minutes);
            } else {
                sprintf(message, "%d frags until match ends.\n", (int)fraglimit->value);
            }
            ctf_BSafePrint(PRINT_HIGH, message);
        } else if(ent->count <= 10) {
            gi.sound (ent, CHAN_CTF, gi.soundindex (CTF_Countdown_Table[ent->count]), 1, ATTN_NONE, 0);
        }
    } else if (matchstate == MATCH_OVER) {
        if (ent->count <= 0) {
            matchstate = MATCH_NONE;
            ent->think = G_FreeEdict;
            tourneyclock = NULL;
        }
    }

    ent->count--;
}

void KillMatch()
{
    matchstate = MATCH_NONE;

    if (tourneyclock) {
        tourneyclock->think = G_FreeEdict;
        tourneyclock->nextthink = level.time + 1;
        tourneyclock = NULL;
    }

    if ((int)autolock->value) {
    	game.teamslocked = false;
    }
}

void StartMatch (char *levelname)
{
	if ((int) autolock->value) {
		game.teamslocked = true;
	}

    ctf_ChangeMap(levelname, true);
}

void SpawnTourneyClock ()
{
    edict_t    *ent;

    if (!tourneyclock) {
        ent = G_Spawn();
        tourneyclock = ent;
    } else {
        ent = tourneyclock;
    }

    if (matchstate == MATCH_RAILGUN_COUNTDOWN) {
        ent->count = 16;
    } else {
        ent->count = (int)countdown_time->value;
        matchstate = MATCH_COUNTDOWN;
    }

    ent->think = Tourney_Think;
    ent->nextthink = level.time + 1;
}
