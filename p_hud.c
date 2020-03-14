#include "g_local.h"
#include "g_ctffunc.h" //surt for some nice wrapper functions
#include "g_tourney.h"
#include "bat.h"

int MvpDisp;


extern int Time_Left;

edict_t *Declare_Railgun_Victor(void);


/*
======================================================================

INTERMISSION

======================================================================
*/

void MoveClientToIntermission (edict_t *ent)
{
	
	if (deathmatch->value || coop->value)
		ent->client->showscores = true;
	VectorCopy (level.intermission_origin, ent->s.origin);
	ent->client->ps.pmove.origin[0] = level.intermission_origin[0]*8;
	ent->client->ps.pmove.origin[1] = level.intermission_origin[1]*8;
	ent->client->ps.pmove.origin[2] = level.intermission_origin[2]*8;
	VectorCopy (level.intermission_angle, ent->client->ps.viewangles);
	ent->client->ps.pmove.pm_type = PM_FREEZE;
	ent->client->ps.gunindex = 0;
	ent->client->ps.blend[3] = 0;
	ent->client->ps.rdflags &= ~RDF_UNDERWATER;

	// clean up powerup info
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	ent->client->grenade_blew_up = false;
	ent->client->grenade_time = 0;

	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	ent->s.modelindex3 = 0;
	ent->s.modelindex = 0;
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->solid = SOLID_NOT;

	// add the layout

	if (deathmatch->value || coop->value)
	{
		DeathmatchScoreboardMessage (ent, NULL);
		gi.unicast (ent, true);
	}

}

void BeginIntermission (edict_t *targ)
{
	int		i, n;
	edict_t	*ent, *client;

	if (level.intermissiontime)
		return;		// already activated

	MvpDisp = 1;

    // LM_JORM -- Proclaim a victory!
    Victory();
    // END LM_JORM

	game.autosaved = false;

	// respawn any dead clients
	//bat
	//Too Many overflows!!!!!
	//for (i=0 ; i<maxclients->value ; i++)
	//{
	//	client = g_edicts + 1 + i;
	//	if (!client->inuse)
	//		continue;
	//	if (client->health <= 0)
	//	respawn(client);
	//}

	level.intermissiontime = level.time;
	level.changemap = targ->map;


	if (strstr(level.changemap, "*"))
	{
		if (coop->value)
		{
			for (i=0 ; i<maxclients->value ; i++)
			{
				client = g_edicts + 1 + i;
				if (!client->inuse)
					continue;
				// strip players of all keys between units
				for (n = 0; n < MAX_ITEMS; n++)
				{
					if (itemlist[n].flags & IT_KEY)
						client->client->pers.inventory[n] = 0;
				}
			}
		}
	}
	else
	{
		if (!deathmatch->value)
		{
			level.exitintermission = 1;		// go immediately to the next level
			return;
		}
	}

	level.exitintermission = 0;

	// find an intermission spot
	ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
	if (!ent)
	{	// the map creator forgot to put in an intermission point...
		ent = G_Find (NULL, FOFS(classname), "info_player_start");
		if (!ent)
			ent = G_Find (NULL, FOFS(classname), "info_player_deathmatch");
	}
	else
	{	// chose one of four spots
		i = rand() & 3;
		while (i--)
		{
			ent = G_Find (ent, FOFS(classname), "info_player_intermission");
			if (!ent)	// wrap around the list
				ent = G_Find (ent, FOFS(classname), "info_player_intermission");
		}
	}

	VectorCopy (ent->s.origin, level.intermission_origin);
	VectorCopy (ent->s.angles, level.intermission_angle);


	// move all clients to the intermission point
	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		MoveClientToIntermission (client);
	}

}


void Show_String(int x, int y, char *string, char *Text)
{
	sprintf(DBuffer, "xv %i yv %i string2 \"%s\" ", x,  y, Text);
	strcat(string, DBuffer);
}


extern edict_t *Railgun_Victor;


/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage (edict_t *ent, edict_t *killer)
{
    char    entry[MAX_MSGLEN];
    char    string[MAX_MSGLEN];
    char    string2[MAX_MSGLEN];  // TEAM PLAY -- LM_JORM
	char	mvpstring[64];
	int     bluescore, redscore;  // TEAM PLAY -- LM_JORM
    int     bluecaps, redcaps;  // TEAM PLAY -- LM_JORM
    int     blue, red;  // TEAM PLAY -- LM_JORM
    int     bluepings, redpings;  //bat
    int     Avg_Bluepings, Avg_Redpings;  //bat
    int     stringlength;
    int     i, j, k;
    //int       sortedscores[MAX_CLIENTS];
    int     redsorted[MAX_CLIENTS];
    int     redsortedscores[MAX_CLIENTS];
    int     bluesorted[MAX_CLIENTS];
    int     bluesortedscores[MAX_CLIENTS];
#ifdef OLDOBSERVERCODE
    int     sorted[MAX_CLIENTS];
    int     observers;
#endif
    //bat
    int     sorted_reg_observers[MAX_CLIENTS];
	int     sorted_red_observers[MAX_CLIENTS];
	int     sorted_blue_observers[MAX_CLIENTS];
    int     reg_observers = 0;
    int     red_observers = 0;
    int     blue_observers = 0;

    
	//int     score, total;
	int     score;
 //   int     picnum;
    int     x, y;
    gclient_t   *cl;
    edict_t     *cl_ent;
    char    *tag;
    qboolean    showsmall;

    showsmall = false;
//    showsmall = true;

    bluescore = bluecaps = blue = 0; // TEAM PLAY -- LM_JORM
    redscore = redcaps = red = 0;  // TEAM PLAY -- LM_JORM
#ifdef OLDOBSERVERCODE
    observers = 0;  // TEAM PLAY -- LM_JORM
#endif

	//bat
	redpings = 0;
	bluepings = 0;



	//
	// This function really should be rewritten, but I probably 
	// won't get around to it.
	// -bat
	//

	// sort the clients by score
    //total = 0;
    for (i=0; i<game.maxclients; i++)
    {
        cl_ent = g_edicts + 1 + i;
        cl = &game.clients[i];

        //if (!cl_ent->inuse || game.clients[i].resp.spectator)
        //bat allow spectators to be on teams.
		if(!cl_ent->inuse)
            continue;

		//sprintf(DBuffer, "hud t %d p %d r %d", ent->client->ctf.teamnum,
		//	ent->client->pers.spectator, ent->client->resp.spectator);
		//Debug_Show(DBuffer);


		//sprintf(DBuffer, "hud h %d", cl_ent->health);
		//Debug_Show(DBuffer);

		score = stats_get(cl_ent, STATS_SCORE);

        if (cl_ent->client->ctf.teamnum == CTF_TEAM_RED) // RED TEAM
        {
            redscore += score;
            redcaps += stats_get(cl_ent, STATS_CAPTURES); 
			redpings += cl->ping;

            for (j=0 ; j<red ; j++)
            {
                if (score > redsortedscores[j])
                    break;
            }
            for (k=red ; k>j ; k--)
            {
                redsorted[k] = redsorted[k-1];
                redsortedscores[k] = redsortedscores[k-1];
            }
            redsorted[j] = i;
            redsortedscores[j] = score;
            red++;
        }
        else if (cl_ent->client->ctf.teamnum == CTF_TEAM_BLUE) // BLUE TEAM
        {
            bluescore += score;
            bluecaps  += stats_get(cl_ent, STATS_CAPTURES);
			bluepings += cl->ping;

            for (j=0 ; j<blue ; j++)
            {
                if (score > bluesortedscores[j])
                    break;
            }
            for (k=blue ; k>j ; k--)
            {
                bluesorted[k] = bluesorted[k-1];
                bluesortedscores[k] = bluesortedscores[k-1];
            }
            bluesorted[j] = i;
            bluesortedscores[j] = score;    
            blue++;
        }
        else if (cl_ent->client->ctf.teamnum == CTF_TEAM_OBSERVER_BLUE)
		{
			sorted_blue_observers[blue_observers] = i;
			blue_observers++;
		}
        else if (cl_ent->client->ctf.teamnum == CTF_TEAM_OBSERVER_RED)
		{
			sorted_red_observers[red_observers] = i;
			red_observers++;
		}
//bat - put this back in
//#ifdef OLDOBSERVERCODE
		else
		{
			sorted_reg_observers[reg_observers] = i;
			reg_observers++;
		}
//#endif
        //total++;

        /*
        for (j=0 ; j<total ; j++)
        {
            if (score > sortedscores[j])
                break;
        }
        for (k=total ; k>j ; k--)
        {
            sorted[k] = sorted[k-1];
            sortedscores[k] = sortedscores[k-1];
        }
        sorted[j] = i;
        sortedscores[j] = score;
        total++;
        */
    }

    // print level name and exit rules
    string[0] = 0;

    //stringlength = strlen(string);
    stringlength = 0;


    // add the clients in sorted order
    if (red > 6)
    {
        showsmall = true;
        if (red > 21)
            red = 21;
    }
    
	if (blue > 6)
    {
        showsmall = true;
        if (blue > 21)
            blue = 21;
    }

   
	if(red == 0)
		Avg_Redpings = 0;
	else
		Avg_Redpings = redpings/red;
	
	if(blue == 0)
		Avg_Bluepings =	0;
	else
		Avg_Bluepings = bluepings/blue;

	
    if (showsmall)
    {
        x = 0;
        y = 32;
        Com_sprintf (string2, sizeof(string2),
            "xv 0 yv 32 string2 \"Scr Png Name        \" "
            "xv 0 yv 40 string2 \"------------------- \" "
            "xv 160 yv 32 string2 \"Scr Png Name        \" "
            "xv 160 yv 40 string2 \"------------------- \" "
            );

        j = strlen(string2);
        if (stringlength + j <= 1024)
        {
            strcpy (string + stringlength, string2);
            stringlength += j;
        }
    }


    for (i=0 ; i<red ; i++)
    {
        cl = &game.clients[redsorted[i]];
        cl_ent = g_edicts + 1 + redsorted[i];   

    
        if (showsmall)
        {
            x = 0;
            y = 48 + 8 * i;

            if (cl_ent == Query_DMVP())
			{
				sprintf(mvpstring, "D%3d %3d %s", cl->resp.score, cl->ping, cl->pers.netname);
				mvpstring[19] = 0;
				Show_String(x, y, string2, mvpstring);
			}
            else if (cl_ent == Query_OMVP())
			{
				sprintf(mvpstring, "O%3d %3d %s", cl->resp.score, cl->ping, cl->pers.netname);
				mvpstring[19] = 0;
				Show_String(x, y, string2, mvpstring);
			}
			else
			{
				sprintf(string2, "ctf %d %d %d %ld %d ", x, y, redsorted[i],
					stats_get(cl_ent, STATS_SCORE),	cl->ping > 999 ? 999 : cl->ping);
			}

            
            j = strlen(string2);
            if (stringlength + j > 1024)
                break;
            strcpy (string + stringlength, string2);
            stringlength += j;
        }
        else
        {
 //           picnum = gi.imageindex ("i_fixme");
            x = 0;
            y = 32 + 32 * (i%6);

            // add a dogtag
            if (cl_ent == ent)
                tag = "tag1";
            else if (cl_ent == killer)
                tag = "tag2";
            else
                tag = NULL;
            if (tag)
            {
                Com_sprintf (entry, sizeof(entry),
                    "xv %i yv %i picn %s ",x+32, y, tag);
                j = strlen(entry);
                if (stringlength + j > 1024)
                    break;
                strcpy (string + stringlength, entry);
                stringlength += j;
            }

        // send the layout
            Com_sprintf (entry, sizeof(entry),
                "client %i %i %i %i %i %i ",
                x, y, redsorted[i], stats_get(cl_ent, STATS_SCORE), 
                cl->ping,
                (level.framenum - cl->resp.enterframe)/600);    

            j = strlen(entry);
            if (stringlength + j > 1024)
                break;
            strcpy (string + stringlength, entry);
            stringlength += j;

            if (stats_get(cl_ent, STATS_CAPTURES))
            {
                Com_sprintf (string2, sizeof(string2),
                    "xv %i yv %i string2 \"C:%i\" ",        // teamname
                    x+32+80, y+24,
                    stats_get(cl_ent, STATS_CAPTURES));

                j = strlen(string2);
                if (stringlength + j > 1024)
                    break;
                strcpy (string + stringlength, string2);
                stringlength += j;
            }

            if (cl_ent == Query_DMVP())
            {
                Com_sprintf (string2, sizeof(string2),
                    "xv %d yv %d picn dmvpicon ",
                    x, y);
                j = strlen(string2);
                if (stringlength + j > 1024)
                    break;
                strcpy (string + stringlength, string2);
                stringlength += j;
            }
            else if (cl_ent == Query_OMVP())
            {
                Com_sprintf (string2, sizeof(string2),
                    "xv %d yv %d picn omvpicon ",
                    x, y);
                j = strlen(string2);
                if (stringlength + j > 1024)
                    break;
                strcpy (string + stringlength, string2);
                stringlength += j;
            }

        }
        // END PLAY -- LM JORM

	}

    for (i=0 ; i<blue ; i++)
    {
        cl = &game.clients[bluesorted[i]];
        cl_ent = g_edicts + 1 + bluesorted[i];  

        if (showsmall)
        {
            x = 160;
            y = 48 + 8 * i;

            if (cl_ent == Query_DMVP())
			{
				sprintf(mvpstring, "D%3d %3d %s", cl->resp.score, cl->ping, cl->pers.netname);
				mvpstring[19] = 0;
				Show_String(x, y, string2, mvpstring);
			}
            else if (cl_ent == Query_OMVP())
			{
				sprintf(mvpstring, "O%3d %3d %s", cl->resp.score, cl->ping, cl->pers.netname);
				mvpstring[19] = 0;
				Show_String(x, y, string2, mvpstring);
			}
			else
			{

				sprintf(string2,
					"ctf %d %d %d %ld %d ",
					x, y,
					bluesorted[i],
					stats_get(cl_ent, STATS_SCORE),
					cl->ping > 999 ? 999 : cl->ping);
			}

            j = strlen(string2);
            if (stringlength + j > 1024)
                break;
            strcpy (string + stringlength, string2);
            stringlength += j;
        }
        else
        {
 //           picnum = gi.imageindex ("i_fixme");
            //x = (i>=6) ? 160 : 0;
            x = 160;
            y = 32 + 32 * (i%6);

            // add a dogtag
            if (cl_ent == ent)
                tag = "tag1";
            else if (cl_ent == killer)
                tag = "tag2";
            else
                tag = NULL;
            if (tag)
            {
                Com_sprintf (entry, sizeof(entry),
                    "xv %i yv %i picn %s ",x+32, y, tag);
                j = strlen(entry);
                if (stringlength + j > 1024)
                    break;
                strcpy (string + stringlength, entry);
                stringlength += j;
            }

        // send the layout
            Com_sprintf (entry, sizeof(entry),
                "client %i %i %i %i %i %i ",
                x, y, bluesorted[i], stats_get(cl_ent, STATS_SCORE),
                cl->ping, (level.framenum - cl->resp.enterframe)/600);  

            j = strlen(entry);
            if (stringlength + j > 1024)
                break;
            strcpy (string + stringlength, entry);
            stringlength += j;

            if (stats_get(cl_ent, STATS_CAPTURES))
            {
                Com_sprintf (string2, sizeof(string2),
                    "xv %i yv %i string2 \"C:%i\" ",        // teamname
                    x+32+80, y+24,
                    stats_get(cl_ent, STATS_CAPTURES));

                j = strlen(string2);
                if (stringlength + j > 1024)
                    break;
                strcpy (string + stringlength, string2);
                stringlength += j;
            }
            if (cl_ent == Query_DMVP())
            {
                Com_sprintf (string2, sizeof(string2),
                    "xv %d yv %d picn dmvpicon ",
                    x, y);
                j = strlen(string2);
                if (stringlength + j > 1024)
                    break;
                strcpy (string + stringlength, string2);
                stringlength += j;
            }
            else if (cl_ent == Query_OMVP())
            {
                Com_sprintf (string2, sizeof(string2),
                    "xv %d yv %d picn omvpicon ",
                    x, y);
                j = strlen(string2);
                if (stringlength + j > 1024)
                    break;
                strcpy (string + stringlength, string2);
                stringlength += j;
            }

        }
        // END PLAY -- LM JORM
    }


    y = 32 * 8;
	string2[0] = 0;


	if(MvpDisp)
	{
		sprintf(mvpstring, "*** %s MVPs ***", level.mapname);
		Show_String(80, y, string2, mvpstring);
		y += 8;

    	if(Railgun_Victor)
		{
			sprintf(mvpstring, "Railgod -> %s", Railgun_Victor->client->pers.netname);
			Show_String(100, y, string2, mvpstring);
			y += 8;
		}

		sprintf(mvpstring, "1) %s %4ld", Highscore_Table[0].Player, Highscore_Table[0].Score);
		Show_String(130, y, string2, mvpstring);
		y += 8;

		x = 0;

		for(i = 1; i < MAX_HIGHSCORE_ENTRIES; i++)
		{
			if(i == 4)
			{
				x = 220;
				y = 272;
    			if(Railgun_Victor)
					y += 8;
			}
			
			sprintf(mvpstring, "%d) %15s %4ld", i + 1, Highscore_Table[i].Player, Highscore_Table[i].Score);
				
			Show_String(x, y, string2, mvpstring);
			y += 8;
		}

		j = strlen(string2);
		if (stringlength + j <= 1024)
		{
			strcpy (string + stringlength, string2);
			stringlength += j;
		}

	}
	else
	{
		if(red_observers)
		{
			x = 0;
			Show_String(x, y, string2, "Red Observers:");
			y += 8;

			for (i = 0; i < red_observers ; i++)
			{
				cl = &game.clients[sorted_red_observers[i]];
				cl_ent = g_edicts + 1 + sorted_red_observers[i];  
				Show_String(x, y, string2, cl->pers.netname);
				y += 8;
			}

			j = strlen(string2);
			if (stringlength + j <= 1024)
			{
				strcpy (string + stringlength, string2);
				stringlength += j;
			}
		}

		if(blue_observers)
		{
			x = 160;
			Show_String(x, y, string2, "Blue Observers:");
			y += 8;

			for (i = 0; i < blue_observers ; i++)
			{
				cl = &game.clients[sorted_blue_observers[i]];
				cl_ent = g_edicts + 1 + sorted_blue_observers[i];  
				//strcat(entry, cl->pers.netname);
				Show_String(x, y, string2, cl->pers.netname);
				y += 8;
			}

			j = strlen(string2);
			if (stringlength + j <= 1024)
			{
				strcpy (string + stringlength, string2);
				stringlength += j;
			}
		}



		if(reg_observers)
		{
			//give more space for the reg observers
			if(red_observers == 0 && blue_observers == 0)
			{
				x = 80;
				Show_String(x, y, string2, "Observers:");
				y += 8;

				//Do 2 obs per line

				for(i = 0; i < reg_observers ; i++)
				{
					//x = (i & 0x01) * 160;
					x = (i % 3) * 130;
				
					cl = &game.clients[sorted_reg_observers[i]];
					cl_ent = g_edicts + 1 + sorted_reg_observers[i];  
					Show_String(x, y, string2, cl->pers.netname);
				
					if((i % 3) == 2)
						y += 8;
				}

				j = strlen(string2);
				if (stringlength + j <= 1024)
				{
					strcpy (string + stringlength, string2);
					stringlength += j;
				}
		
			}
			else
			{
				x = 80;
				Show_String(x, y, string2, "Observers:");
				y += 8;

				for (i = 0; i < reg_observers ; i++)
				{
					cl = &game.clients[sorted_reg_observers[i]];
					cl_ent = g_edicts + 1 + sorted_reg_observers[i];  
					//strcat(entry, cl->pers.netname);
					Show_String(x, y, string2, cl->pers.netname);
					y += 8;
				}

				j = strlen(string2);
				if (stringlength + j <= 1024)
				{
					strcpy (string + stringlength, string2);
					stringlength += j;
				}
			}
		}
	}



#ifdef OLDOBSERVERCODE
    if (observers)
    {
        strcpy(entry, "Observers: ");
        for (i=0 ; i<observers ; i++)
        {
            cl = &game.clients[sorted[i]];
            cl_ent = g_edicts + 1 + sorted[i];  

            strcat(entry, cl->pers.netname);
            if (i+1 < observers)
                strcat(entry, ", ");
        }

        x = 0;
        y = 32 + 32 * 7;

        // send the layout
        Com_sprintf (string2, sizeof(string2),
            "xv %i yv %i string2 \"%s\" ",      // teamname
            x, y, entry
            );

        j = strlen(string2);
        if (stringlength + j <= 1024)
        {
            strcpy (string + stringlength, string2);
            stringlength += j;
        }
    }
#endif
    // END PLAY -- LM JORM


    // Don't show captures graphic if TEAMS and FLAGS turned off (DM MODE)
    //Show them - bat
	//if (!((int)ctfflags->value & CTF_TEAM_NOTEAMS) ||
    //    !((int)ctfflags->value & CTF_FLAGS_NOFLAGS))
    {
        Com_sprintf (string2, sizeof(string2),
        "xv %i yv %i picn %s "
        "xv %i yv %i picn %s "
        "xv %i yv %i picn %s "
        "xv %i yv %i picn %s "
        
		//Just caps - bat
		//"xv %i yv %i string2 \"P:%3i\" "  // players
        "xv %i yv %i string2 \"C:%3i\" "    // captures
        
		"xv %i yv %i string2 \"AP:%3i\" "    // bat AVG PING
        
		//Just caps - bat
		//"xv %i yv %i string2 \"P:%3i\" "
        "xv %i yv %i string2 \"C:%3i\" "
		"xv %i yv %i string2 \"AP:%3i\" "    // bat AVG PING
        
		"xv %i yv %i num 4 19 "
        "xv %i yv %i num 4 20 ",

        0, 0, "redlion_i",
        160, 0, "bluewolf_i",
        32, 0, "redtag",
        192, 0, "bluetag",
        //36, 4,  red, 
        //36, 20, redcaps, 

        36, 4,  redcaps, 
		36, 20, Avg_Redpings,

        //196, 4,  blue, 
        196, 4, bluecaps,
		196, 20, Avg_Bluepings,
        90, 4,
        250, 4
        );

        j = strlen(string2);
        if (stringlength + j < 1024)
        {       
            strcpy (string + stringlength, string2);
            stringlength += j;
        }
    }

    // END PLAY -- LM JORM


	gi.WriteByte (svc_layout);
	gi.WriteString (string);

}


/*
==================
DeathmatchScoreboard

Draw instead of help message.
Note that it isn't that hard to overflow the 1400 byte message limit!
==================
*/
void DeathmatchScoreboard (edict_t *ent)
{
	DeathmatchScoreboardMessage (ent, ent->enemy);
	gi.unicast (ent, true);
}

// ADC
/*
==================
SquadboardMessage

==================
*/
void SquadboardMessage (edict_t *ent, edict_t *killer)
{
	CTFSquadboardMessage (ent, killer);
}
// ADC

// ADC
/*
==================
Squadboard

Draw instead of help message.
Note that it isn't that hard to overflow the 1400 byte message limit!
==================
*/
void Squadboard (edict_t *ent)
{
	SquadboardMessage (ent, ent->enemy);
	gi.unicast (ent, true);
}
// ADC

/*
==================
CTFSquadboardMessage
==================
*/
void CTFSquadboardMessage (edict_t *ent, edict_t* killer) // ADC
{
	char	entry[1024];
	char	string[1400];
	int		len, i, j, team, ready;
	edict_t		*cl_ent;
	int maxsize = 1000;

	gclient_t* clients [MAX_CLIENTS];
	int clientCount = 0;
	gclient_t* sortedClients [MAX_CLIENTS];
	int sortedCount = 0;

	int teamOfInterest = 0;

	char* squad = 0;
	int numCategoryLines = 0;

	char readyString [] = "string2"; // green string
	char notReadyString [] = "string"; // white string

	char statusStart [MAX_STATUS_LEN];
	int greenStatusLen = strlen (GREEN_STATUS_STR);

	int widestName = 0; // in chars

	for (i = 0; i< MAX_CLIENTS; i++)
		clients [i] = sortedClients [i] = 0;

	teamOfInterest = (ent->client->ctf.teamnum == CTF_TEAM_RED) ? 0 : 1;

	for (i = 0; i< game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (game.clients[i].ctf.teamnum == CTF_TEAM_RED)
			team = 0;
		else if (game.clients[i].ctf.teamnum == CTF_TEAM_BLUE)
			team = 1;
		else
			continue; // unknown team?

		if (team == teamOfInterest)
		{
			len = strlen(game.clients[i].pers.netname);
			clients [clientCount++] = &game.clients [i];

			if (len > widestName)
				widestName = len;
		}
	}

	// We want to put the predefined categories first
	// in the list, then any remaining ones.

	for (i = 0, ready = 1; ready; i++) // squad loop
	{
		ready = 0;

		switch (i)
		{
		case 0:	 squad = "Offense"; break;
		case 1:	 squad = "Middle";  break;
		case 2:	 squad = "Defense"; break;
		default: squad = 0;         break;
		}

		for (j = 0; j< game.maxclients; j++) // client loop
		{
			if (clients[j])
			{
				ready = 1;

				if (squad == 0)
					squad = clients[j]->pers.squad;

				if (!Q_stricmp (clients[j]->pers.squad, squad))
				{
					sortedClients [sortedCount++] = clients [j];
					clients [j] = 0;
				}
			}
		}
	}

	// print level name and exit rules
	// add the clients in sorted order
	*string = 0;
	len = 0;

	if (teamOfInterest == 0) // red
		strcpy (string, "xv 0 yv 0 picn redlion_i xv 32 yv 0 picn redtag ");
	else  // blue
		strcpy (string, "xv 0 yv 0 picn bluewolf_i xv 32 yv 0 picn bluetag ");

	strcat (string, "xv 48 yv 10 string \"Squad Board\" ");

	squad = 0;

	for (i = 0; i< 16 ; i++)
	{
		if (i >= sortedCount)
			break; // we're done

		*entry = 0;

		if (!squad || Q_stricmp (squad, sortedClients[i]->pers.squad))
		{
			squad = sortedClients[i]->pers.squad;

			sprintf(entry+strlen(entry),
				"xv 0 yv %d string \"%s\" ",
				42 + i * 8 + numCategoryLines * 8,
				sortedClients[i]->pers.squad);

			numCategoryLines++;
		}

		// If the status starts with "Ready", then it should be shown
		// in green.

		strncpy (statusStart, sortedClients[i]->pers.squadStatus, 
			greenStatusLen);
		statusStart[greenStatusLen] = 0;

		ready = !Q_stricmp (statusStart, GREEN_STATUS_STR);

		// Note that the width %*s below is the widest chars
		// for a netname. We want the names padded with spaces
		// to make the status line up.

		sprintf(entry+strlen(entry),
			"xv 0 yv %d %s \"   %-*s %s\" ",
			42 + i * 8 + numCategoryLines * 8,
			ready ? readyString : notReadyString,
			widestName, sortedClients[i]->pers.netname,
			sortedClients[i]->pers.squadStatus);

		if (maxsize - len > strlen(entry)) {
			strcat(string, entry);
			len = strlen(string);
		}
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

/*
==================
Cmd_Score_f

Display the scoreboard
==================
*/
void Cmd_Score_f (edict_t *ent)
{
	ent->client->showinventory = false;
	ent->client->showhelp = false;
    ent->client->showctfhud = false;
    ent->client->showmod = false;
    ent->client->showmenu = false;
	ent->client->showsquadboard = false; // ADC

	if (!deathmatch->value && !coop->value)
		return;

	if (ent->client->showscores)
	{
		ent->client->showscores = false;
		return;
	}

	ent->client->showscores = true;
	DeathmatchScoreboard (ent);
}

// ADC
/*
==================
Cmd_Squadboard_f

Display the squadboard
==================
*/
void Cmd_Squadboard_f (edict_t *ent)
{
	ent->client->showhelp = false;
	ent->client->showinventory = false;
	ent->client->showctfhud = false;
	ent->client->showmod = false;
	ent->client->showmenu = false;
	ent->client->showscores = false;

	if (!deathmatch->value && !coop->value)
		return;

	if (ent->client->showsquadboard)
	{
		ent->client->showsquadboard = false;
		return;
	}

	ent->client->showsquadboard = true;
	Squadboard (ent);
}
// ADC

/*
==================
HelpComputer

Draw help computer.
==================
*/
void HelpComputer (edict_t *ent)
{
	char	string[1024];
	char	*sk;

	if (skill->value == 0)
		sk = "easy";
	else if (skill->value == 1)
		sk = "medium";
	else if (skill->value == 2)
		sk = "hard";
	else
		sk = "hard+";

	// send the layout
	Com_sprintf (string, sizeof(string),
		"xv 32 yv 8 picn help "			// background
		"xv 202 yv 12 string2 \"%s\" "		// skill
		"xv 0 yv 24 cstring2 \"%s\" "		// level name
		"xv 0 yv 54 cstring2 \"%s\" "		// help 1
		"xv 0 yv 110 cstring2 \"%s\" "		// help 2
		"xv 50 yv 164 string2 \" kills     goals    secrets\" "
		"xv 50 yv 172 string2 \"%3i/%3i     %i/%i       %i/%i\" ", 
		sk,
		level.level_name,
		game.helpmessage1,
		game.helpmessage2,
		level.killed_monsters, level.total_monsters, 
		level.found_goals, level.total_goals,
		level.found_secrets, level.total_secrets);

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}


/*
==================
Cmd_Help_f

Display the current help message
==================
*/
void Cmd_Help_f (edict_t *ent)
{
	// this is for backwards compatability
	if (deathmatch->value)
	{
		Cmd_Score_f (ent);
		return;
	}

	ent->client->showinventory = false;
	ent->client->showscores = false;
    ent->client->showctfhud = false;
    ent->client->showmod = false;
    ent->client->showmenu = false;
	ent->client->showsquadboard = false; // ADC

	if (ent->client->showhelp && (ent->client->pers.game_helpchanged == game.helpchanged))
	{
		ent->client->showhelp = false;
		return;
	}

	ent->client->showhelp = true;
	ent->client->pers.helpchanged = 0;
	HelpComputer (ent);
}


//=======================================================================

/*
===============
G_SetStats
===============
*/
void G_SetStats (edict_t *ent)
{
    gitem_t     *item;
    int         index, cells=0;
    int         power_armor_type;

    // TEAM PLAY -- LM_JORM
    char        *s;
    char        portrait[MAX_INFO_STRING];

int Red_Caps = 0;
int Blue_Caps = 0;
int i;
edict_t     *cl_ent;

#ifdef OLDOBSERVERCODE   
    if (ent->client->camera_target &&
        ent->client->camera_target->client)
    {
        memcpy(&ent->client->ps.stats, &ent->client->camera_target->client->ps.stats, sizeof(short)*MAX_STATS);
        ent->client->ps.stats[STAT_FRAGS] = 0;
    }
    else
#endif
    {
        ent->client->ps.stats[STAT_RED_FRAGS] = redscore;
        ent->client->ps.stats[STAT_BLUE_FRAGS] = bluescore;
		
		//bat - This should be global. :/
		for(i = 0 ; i < game.maxclients; i++)
		{
	        cl_ent = g_edicts + 1 + i;
			if(!cl_ent->inuse)
			    continue;

			if(cl_ent->client->ctf.teamnum == CTF_TEAM_RED)
				Red_Caps  += stats_get(cl_ent, STATS_CAPTURES);
	        else if(cl_ent->client->ctf.teamnum == CTF_TEAM_BLUE)
				Blue_Caps  += stats_get(cl_ent, STATS_CAPTURES);
		}
        
		ent->client->ps.stats[STAT_RED_CAPS] = Red_Caps;
		ent->client->ps.stats[STAT_BLUE_CAPS] = Blue_Caps;
		ent->client->ps.stats[STAT_MATCH_TIME] = Time_Left;
        


        //s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
        
        // decide gender
        
        /*
        if (s[0] == 'f' || s[0] == 'F') // Female
        {
        if (ent->client->teamnum == 1)
        ent->client->ps.stats[STAT_TEAM_ICON] = gi.imageindex ("../players/female/red_i");
        
         if (ent->client->teamnum == 2)
         ent->client->ps.stats[STAT_TEAM_ICON] = gi.imageindex ("../players/female/blue_i");
         }
         else // male
         {
         if (ent->client->teamnum == 1)
         ent->client->ps.stats[STAT_TEAM_ICON] = gi.imageindex ("../players/male/red_i");
         
          if (ent->client->teamnum == 2)
          ent->client->ps.stats[STAT_TEAM_ICON] = gi.imageindex ("../players/male/blue_i");
          }
        */
        
        
        s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
        s = strchr(s, '/');
        if (s && strlen(s) > 0)
        {
            s++;
            strcpy(portrait, s);
            strcat(portrait, "_i");
        }
        else
        {
            strcpy(portrait, "redlion_i");
        }
        
        
        switch (ent->client->ctf.compass)
        {
        default:
        case 0:
            if (redflag && (redflag->owner == ent))
                ent->client->ps.stats[STAT_TEAM_ICON] = gi.imageindex ("redflaggone");
            else if (blueflag && (blueflag->owner == ent))
                ent->client->ps.stats[STAT_TEAM_ICON] = gi.imageindex ("blueflaggone");
            else
                ent->client->ps.stats[STAT_TEAM_ICON] = gi.imageindex (portrait);
            break;
        case 1:
            ent->client->ps.stats[STAT_TEAM_ICON] = gi.imageindex (ctf_facing(ent));
            break;
        case 2:
            ent->client->ps.stats[STAT_TEAM_ICON] = gi.imageindex (ctf_faceNorth(ent));
            break;
        case 3:
            ent->client->ps.stats[STAT_TEAM_ICON] = gi.imageindex (ctf_faceEnemyFlag(ent));
            break;
        }
        
        
        // Show status of the red flag
        if (redflag)
        {
            if (redflag->owner)
                ent->client->ps.stats[STAT_RED_ICON] = gi.imageindex ("redflaggone");
            else if (!ctf_flagathome(redflag))
                ent->client->ps.stats[STAT_RED_ICON] = gi.imageindex ("redflagdown");
            else
                ent->client->ps.stats[STAT_RED_ICON] = gi.imageindex ("redlion_i");
        }
        
        if (blueflag)
        {
            if (blueflag->owner)
                ent->client->ps.stats[STAT_BLUE_ICON] = gi.imageindex ("blueflaggone");
            else if (!ctf_flagathome(blueflag))
                ent->client->ps.stats[STAT_BLUE_ICON] = gi.imageindex ("blueflagdown");
            else
                ent->client->ps.stats[STAT_BLUE_ICON] = gi.imageindex ("bluewolf_i");
        }
        
        if (ent->client->rune)
        {
            if (ent->client->rune->runetype == RUNE_DAMAGE)
                ent->client->ps.stats[STAT_RUNE_ICON] = gi.imageindex ("strength");
            if (ent->client->rune->runetype == RUNE_RESIST)
                ent->client->ps.stats[STAT_RUNE_ICON] = gi.imageindex ("resist");
            if (ent->client->rune->runetype == RUNE_HASTE)
                ent->client->ps.stats[STAT_RUNE_ICON] = gi.imageindex ("haste");
            if (ent->client->rune->runetype == RUNE_REGEN)
                ent->client->ps.stats[STAT_RUNE_ICON] = gi.imageindex ("regen");
			if (ent->client->rune->runetype == RUNE_VAMP)                          //added by Vampire
                ent->client->ps.stats[STAT_RUNE_ICON] = gi.imageindex ("k_redkey");
                //ent->client->ps.stats[STAT_RUNE_ICON] = gi.imageindex ("resist");    
        }
        else
            ent->client->ps.stats[STAT_RUNE_ICON] = 0;
        
        // END CTF CODE -- LM_JORM
        
        //
        // health
        //
        ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
        ent->client->ps.stats[STAT_HEALTH] = ent->health;
        
        //
        // ammo
        //
        if (!ent->client->ammo_index /* || !ent->client->pers.inventory[ent->client->ammo_index] */)
        {
            ent->client->ps.stats[STAT_AMMO_ICON] = 0;
            ent->client->ps.stats[STAT_AMMO] = 0;
        }
        else
        {
            item = &itemlist[ent->client->ammo_index];
            ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex (item->icon);
            ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[ent->client->ammo_index];
        }
        
        //
        // armor
        //
        power_armor_type = PowerArmorType (ent);
        if (power_armor_type)
        {
            cells = ent->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))];
            if (cells == 0)
            {   // ran out of cells for power armor
                ent->flags &= ~FL_POWER_ARMOR;
                gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
                power_armor_type = 0;;
            }
        }
        
        index = ArmorIndex (ent);
        if (power_armor_type && (!index || (level.framenum & 8) ) )
        {   // flash between power armor and other armor icon
            ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex ("i_powershield");
            ent->client->ps.stats[STAT_ARMOR] = cells;
        }
        else if (index)
        {
            item = GetItemByIndex (index);
            ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex (item->icon);
            ent->client->ps.stats[STAT_ARMOR] = ent->client->pers.inventory[index];
        }
        else
        {
            ent->client->ps.stats[STAT_ARMOR_ICON] = 0;
            ent->client->ps.stats[STAT_ARMOR] = 0;
        }
        
        //
        // pickup message
        //
        if (level.time > ent->client->pickup_msg_time)
        {
            ent->client->ps.stats[STAT_PICKUP_ICON] = 0;
            ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
        }
        
        //
        // timers
        //
        if (ent->client->quad_framenum > level.framenum)
        {
            ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_quad");
            ent->client->ps.stats[STAT_TIMER] = (ent->client->quad_framenum - level.framenum)/10;
        }
        else if (ent->client->invincible_framenum > level.framenum)
        {
            ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_invulnerability");
            ent->client->ps.stats[STAT_TIMER] = (ent->client->invincible_framenum - level.framenum)/10;
        }
        else if (ent->client->enviro_framenum > level.framenum)
        {
            ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_envirosuit");
            ent->client->ps.stats[STAT_TIMER] = (ent->client->enviro_framenum - level.framenum)/10;
        }
        else if (ent->client->breather_framenum > level.framenum)
        {
            ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_rebreather");
            ent->client->ps.stats[STAT_TIMER] = (ent->client->breather_framenum - level.framenum)/10;
        }
        else
        {
            ent->client->ps.stats[STAT_TIMER_ICON] = 0;
            ent->client->ps.stats[STAT_TIMER] = 0;
        }
        
        //
        // selected item
        //
        
        // Show proper flag item
        if (ent->client->pers.selected_item == -1)
            ent->client->ps.stats[STAT_SELECTED_ICON] = 0;
        else
        {
            if (blueflag && blueflag->item == &itemlist[ent->client->pers.selected_item])
            {
                ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ("a_blueflag");
            }
            else if (redflag && redflag->item == &itemlist[ent->client->pers.selected_item])
            {
                ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex ("a_redflag");            
            }
            else
            {
                ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex (itemlist[ent->client->pers.selected_item].icon);
                
            }
        }
        
        ent->client->ps.stats[STAT_SELECTED_ITEM] = ent->client->pers.selected_item;
        
        //
        // frags
        //
        ent->client->ps.stats[STAT_FRAGS] = stats_get(ent, STATS_SCORE);
        
        //
        // help icon / current weapon if not shown
        //
        if (ent->client->pers.helpchanged && (level.framenum&8) )
            ent->client->ps.stats[STAT_HELPICON] = gi.imageindex ("i_help");
        else if ( (ent->client->pers.hand == CENTER_HANDED || ent->client->ps.fov > 91)
            && ent->client->pers.weapon)
            ent->client->ps.stats[STAT_HELPICON] = gi.imageindex (ent->client->pers.weapon->icon);
        else
            ent->client->ps.stats[STAT_HELPICON] = 0;
    }

    //
    // layouts
    //
    ent->client->ps.stats[STAT_LAYOUTS] = 0;

    if (deathmatch->value)
    {
        if (ent->client->pers.health <= 0 || level.intermissiontime
            || ent->client->showscores || ent->client->showctfhud 
            || ent->client->showmod || ent->client->showmenu
			|| ent->client->showsquadboard) // ADC
            ent->client->ps.stats[STAT_LAYOUTS] |= 1;
        if (ent->client->showinventory && ent->client->pers.health > 0)
            ent->client->ps.stats[STAT_LAYOUTS] |= 2;
    }
    else
    {
        if (ent->client->showscores || ent->client->showhelp
            || ent->client->showctfhud || ent->client->showmod
			|| ent->client->showsquadboard) // ADC
            ent->client->ps.stats[STAT_LAYOUTS] |= 1;
        if (ent->client->showinventory && ent->client->pers.health > 0)
            ent->client->ps.stats[STAT_LAYOUTS] |= 2;
    }

    // LM_JORM -- Turn CTF HUD back on automatically
    /*
    if ((level.framenum - ent->client->awayframe > 10) &&
        !ent->client->showmenu)
        ent->client->showctfhud = true;
    */

	ent->client->ps.stats[STAT_SPECTATOR] = 0;
}

/*
===============
G_CheckChaseStats
===============
*/
void G_CheckChaseStats (edict_t *ent)
{
	int i;
	gclient_t *cl;

	for (i = 1; i <= maxclients->value; i++) {
		cl = g_edicts[i].client;
		if (!g_edicts[i].inuse || cl->chase_target != ent)
			continue;
		memcpy(cl->ps.stats, ent->client->ps.stats, sizeof(cl->ps.stats));
		G_SetSpectatorStats(g_edicts + i);
	}
}

/*
===============
G_SetSpectatorStats
===============
*/
void G_SetSpectatorStats (edict_t *ent)
{
	gclient_t *cl = ent->client;

	if (!cl->chase_target)
		G_SetStats (ent);

	cl->ps.stats[STAT_SPECTATOR] = 1;

	// layouts are independant in spectator
	cl->ps.stats[STAT_LAYOUTS] = 0;
	//if (cl->pers.health <= 0 || level.intermissiontime || cl->showscores)
//	if (cl->pers.health <= 0 || level.intermissiontime || cl->showscores || cl->showmenu)
	
	//bat - I think that cl->pers.health is only supposed to be checked in deathmatch
	
	if(level.intermissiontime || cl->showscores || cl->showmenu)
		cl->ps.stats[STAT_LAYOUTS] |= 1;
	if (cl->showinventory && cl->pers.health > 0)
		cl->ps.stats[STAT_LAYOUTS] |= 2;

	if (cl->chase_target && cl->chase_target->inuse)
		cl->ps.stats[STAT_CHASE] = CS_PLAYERSKINS + 
			(cl->chase_target - g_edicts) - 1;
	else
		cl->ps.stats[STAT_CHASE] = 0;
}

