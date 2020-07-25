#include "g_local.h"
#include "g_ctffunc.h"

void string_replace(edict_t *person, char *inmsg, char *outmsg, int outsize)
{
	char * result;
	char * msgptr;
	char * subptr;
	char * tmpstr;
	char * substr;
	
	
	strcpy(outmsg,inmsg); //in case anything fails, you'll say something
	substr = (char *)gi.TagMalloc(outsize * 2, TAG_LEVEL);
	result = (char *)gi.TagMalloc(outsize * 2, TAG_LEVEL);
	tmpstr = (char *)gi.TagMalloc(outsize * 2, TAG_LEVEL);
	
	//initialize strings empty
	tmpstr[0] = 0;
	result[0] = 0;
	substr[0] = 0;
	
	if ( (substr != NULL) && (result != NULL) && (tmpstr != NULL) )
	{
		msgptr = &inmsg[0]; //point to beginning of inmsg
		while (*msgptr != 0) //while not at end of the inmsg
		{
			if (*msgptr == '%')
			{
				//we are dealing with a substr
				msgptr++; //advance to start with character after %
				subptr = &substr[0]; //point to beginning of string
				do
				{
					*subptr = *msgptr;
					subptr++;
					msgptr++;
				}
				while ( (*msgptr != 0) && (*msgptr != '%') && (*msgptr != ' ') );

				msgptr--; //back off because we went past the end
				
				//now in theory we have the block of chars after %
				*subptr = 0; //should already be pointing to end of substr
				
				if (strlen(substr) == 1)
				{
					switch(tolower(substr[0])) //switch on the single character
					{
					case 'l' :
						replace_location(person, tmpstr, false, true);
						break;
					case 'a' :
						replace_armor(person, tmpstr);
						break;
					case 'h' :
						replace_health(person, tmpstr);
						break;
					case 't' :
						replace_artifact(person, tmpstr);
						break;
					case 'w' :
						replace_weapon(person, tmpstr);
						break;
					case 'n' :
						replace_team(person, tmpstr);
						break;
					case 'p' :
						replace_location(person, tmpstr, true, false);
						break;
					case 'f' :
						replace_flaginfo(person, tmpstr);
						break;
					case 'v' :
						replace_viewinfo(person, tmpstr);
						break;
					case 'c' :
						replace_carrierinfo(person, tmpstr);
						break;
					default:
						tmpstr[0] = substr[0];
						tmpstr[1] = 0;
						break;
					}
				}
				else
				{
					//use infovalueforkey feature
					strcpy(tmpstr, Info_ValueForKey(person->client->pers.userinfo, substr));
				}
				
			}
			else
			{
				tmpstr[0] = *msgptr;
				tmpstr[1] = 0;
				//above should cause us to strcat the single char
			}
			strcat(result, tmpstr);
			msgptr++;
		}
		result[outsize-2] = 0;
		strcpy(outmsg, result); //copy the resulting string
	}

	gi.TagFree(tmpstr);
	gi.TagFree(result);
	gi.TagFree(substr);
}




qboolean visibility_test(edict_t *one, edict_t *two)
{
	trace_t	traceresult;
	
	//movetype push obs have bogus origins
	if (one->movetype == MOVETYPE_PUSH)
		return false; //pretend we can't see it
	if (two->movetype == MOVETYPE_PUSH)
		return false; //pretend we can't see it
	
	traceresult = gi.trace(one->s.origin, NULL, NULL, two->s.origin, one, MASK_SOLID);
	if (traceresult.fraction == 1.0)
		return true;
	return false;
}


void replace_location(edict_t *person, char *temp, 
									  qboolean positionvalid, qboolean lineofsight)
{
	edict_t *cur = NULL;
	edict_t *fav = NULL;
	float favrating = 0;
	float currating;
	vec3_t distvector;
	float curdist, reddist, bluedist;
	char *name, *showname, *favname = NULL ; //MJD Uninitilized - Might
	                                          //be used this way. This
	qboolean	noprefix = false;
	strcpy(temp,"");

	while ((cur = findallradius(cur, person->s.origin, 1024)) != NULL)
	{
		if 	(cur->item && cur->item->classname)
			name = cur->item->classname;
		else
			name = cur->classname;
		
		currating = 1;
		showname = NULL;

		if (!strcmp(name, "flag"))
		{
			if ((redflag && (cur == redflag) && (person == redflag->owner)) ||
				(blueflag && (cur == blueflag) && (person == blueflag->owner)))
				continue;
			showname = "Flag";
			currating *= 10;
		}
		else if ((!strcmp(name, "info_flag_red") ||
				 !strcmp(name, "info_flag_blue")) && positionvalid) 
		{
			showname = "Base";
			currating *= 12;
		}
		else if (!strcmp(name, "info_position") && positionvalid)
		{
			// Skip this if spawnflags is 1, and it is not visible.
			if (!visibility_test(person,cur) && (cur->spawnflags & 1))
				continue;
			showname = cur->message;
			currating *= 1000;
			if (cur->spawnflags & 2)
				noprefix = true;
		}

		else if (!strcmp(name, "item_quad") ||
			!strcmp(name, "item_invulnerability") )
		{
			currating *= 8;
		}
		else if (!strcmp(name, "item_health_mega") )
		{
			showname = "Mega Health";
			currating *= 7;
		}
		else if (!strcmp(name, "weapon_bfg") ||
			!strcmp(name, "weapon_railgun") ||
			!strcmp(name, "weapon_rocketlauncher") ||
			!strcmp(name, "weapon_hyperblaster") )
		{
			currating *= 6;
		}
		else if (!strcmp(name, "weapon_chaingun") ||
			!strcmp(name, "weapon_grenadelauncher") )
		{
			currating *= 5;
		}
		else if (!strcmp(name, "weapon_machinegun") ||
			!strcmp(name, "weapon_supershotgun") ||
			!strcmp(name, "weapon_shotgun") )
		{
			currating *= 4;
		}
		else if (!strcmp(name, "item_power_screen") ||
			!strcmp(name, "item_power_shield") )
		{
			currating *= 5;
		}
		else if (!strcmp(name, "item_armor_body") ||
			!strcmp(name, "item_armor_combat") ||
			!strcmp(name, "item_armor_jacket") )
		{
			currating *= 4;
		}
		else if (!strcmp(name, "item_silencer") ||
			!strcmp(name, "item_breather") ||
			!strcmp(name, "item_enviro") ||
			!strcmp(name, "item_adrenaline") ||
			!strcmp(name, "item_bandolier") ||
			!strcmp(name, "item_pack") )
		{
			currating *= 3;
		}
		else if (!cur->item)
			continue;
		else
			currating *= 1;
		//ok, we now have a preliminary rating based on the type of object.
		//now lets decide a bit based on the distance to the ob
	
		VectorSubtract(person->s.origin, cur->s.origin, distvector);
		curdist = VectorLength(distvector);
	
		if (curdist < 64)
			curdist = 64; 
		//don't count distances less than a playerwidth away
		//as being significantly different
	
		currating *= (12000/curdist); //score increases for small distances
	
		//and lets adjust the rating further by whether or not we 
		//have line of sight to the object in question.

		// Don't tell us about powerup if they are not in sight
		if ( !visibility_test(person,cur) && lineofsight)
			continue;
	
		if (currating > favrating) //new winner
		{
			fav = cur;
			favrating = currating;
			favname = showname;  // MJD If you fail this test,
			                     // favname may stay uninitilized!
		}
	}
	
	//test distances to the flags
	if (fav == NULL)
		strcpy(temp, "");
	else if (noprefix)
	{
		if (favname != NULL) // MJD Because of this line, favname should
		             // be null, I think... (surt agrees ... initialized above now)
		{
			strcat(temp, favname);
		}
		else if (fav->item)
		{
			if (fav->item->pickup_name)
				strcat(temp, fav->item->pickup_name);
			else if (fav->item->classname)
				strcat(temp, fav->item->classname);
		}
		else
			strcat(temp, fav->classname);
	}
	else
	{
		VectorSubtract(fav->s.origin, person->s.origin, distvector);
		if (VectorLength(distvector) < 128)
			strcpy(temp, "at ");
		else
		{
			if (fabsf(distvector[0]) > fabsf(distvector[1]) &&		// 0 Biggest
				fabsf(distvector[0]) > fabsf(distvector[2]))
			{
				if (distvector[0] > 0)
					strcpy(temp, "north of ");
				else
					strcpy(temp, "south of ");
			}
			else if (fabsf(distvector[1]) > fabsf(distvector[2]))	// 1 biggest
			{
				if (distvector[1] > 0)
					strcpy(temp, "west of ");
				else
					strcpy(temp, "east of ");
			}
			else												// 2 biggest
			{
				if (distvector[2] > 0)
					strcpy(temp, "below ");
				else
					strcpy(temp, "above ");
			}

		}
		

		if (!positionvalid)
			strcat(temp, "the ");

		cur = G_Find(NULL, FOFS(classname), "info_flag_red");
		VectorSubtract(fav->s.origin, cur->s.origin, distvector);
		reddist = VectorLength(distvector);
		cur = G_Find(NULL, FOFS(classname), "info_flag_blue");
		VectorSubtract(fav->s.origin, cur->s.origin, distvector);
		bluedist = VectorLength(distvector);
		
		if (!positionvalid)
		{
			if (reddist > 2 * bluedist ) //closer to blue
				strcat(temp, "blue ");
			else if (bluedist > 2 * reddist ) //closer to red
				strcat(temp, "red ");
		}

		if (favname)
		{
			strcat(temp, favname);
		}
		else if (fav->item)
		{
			if (fav->item->pickup_name)
				strcat(temp, fav->item->pickup_name);
			else if (fav->item->classname)
				strcat(temp, fav->item->classname);
		}
		else
			strcat(temp, fav->classname);

	}
}


void replace_armor(edict_t *person, char *temp)
{
	gitem_t *item;
	int test, power_armor_type;
	char fill[1024];
	qboolean	hasArmor = false; // MJD Unitialized.  It needs to be


	strcpy(temp, ""); //this will get replaced

	power_armor_type = PowerArmorType(person);

	if (power_armor_type)
	{
		test = person->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))];
		if (test)
		{
			hasArmor = true;
			sprintf(temp, "%i cells of ", test);
			if (power_armor_type == POWER_ARMOR_SCREEN)
				strcat(temp, "power screen ");
			else
				strcat(temp, "power shield ");
		}
	}

	test = ArmorIndex(person);
	if (test)
	{
		item = GetItemByIndex (test);
		if (item)
		{
			hasArmor = true;
			if (strlen(temp) > 1)
				strcat(temp, "and ");
			sprintf(fill, "%i of %s", 
				person->client->pers.inventory[test], item->pickup_name);
			strcat(temp, fill);
		}
	}
	
	if (!hasArmor)
		strcat (temp, "no armor");
}

void replace_health(edict_t *person, char *temp)
{
	edict_t *cur = NULL;
	int sum = 0;
	char str[10] = "";

	strcpy(temp,"");

	if (person->health <= 0)
		strcpy(temp, "dead");
	else
	{
		sprintf(temp, "%i health", person->health);
		while ((cur = findradius(cur, person->s.origin, 256)) != NULL)
		{
			if (!strcmp(cur->classname, "item_health_small") ||
			!strcmp(cur->classname, "item_health_large") ||
			!strcmp(cur->classname, "item_health") ||
			!strcmp(cur->classname, "item_health_mega") )
				sum += cur->count;
		}

		if (person->health < person->max_health)
		{
			strcat(temp," with ");
			if (!sum)
				strcat(temp,"none ");
			else
			{
				sprintf(str, "%i ", sum);
				strcat(temp, str);
			}
			strcat(temp, "nearby");
		}
	}
}

void replace_artifact(edict_t *person, char *temp)
{
	if (person->client->rune)
	{
		if (person->client->rune->item)
		{
			strcpy(temp,person->client->rune->item->pickup_name);
		}
		else
		{
			strcpy(temp, "an unnamed artifact of power");
		}
	}
	else
	{
		strcpy(temp, "no artifact");
	}
}

void replace_weapon(edict_t *person, char *temp)
{
	if (person->client->pers.weapon)
		strcpy(temp, person->client->pers.weapon->pickup_name);
	else
		strcpy(temp, "no weapon");
}

void replace_team(edict_t *person, char *temp)
{
	if (person->client->ctf.teamnum == CTF_TEAM_RED)
		strcpy(temp, "red team");
	else if (person->client->ctf.teamnum == CTF_TEAM_BLUE)
		strcpy(temp, "blue team");
	else
		strcpy(temp, "unassigned team");
}


void replace_visibleplayers(edict_t *person, char *temp)
{
	edict_t *cur = NULL;
	qboolean more_than_one = false;	
	
	strcpy(temp,"");
	while ((cur = findradius(cur, person->s.origin, 2048)) != NULL)
	{
		if (cur->client)
		{
			if (cur->inuse && cur != person && visibility_test(person, cur) )
			{
				if (more_than_one)
					strcat(temp," and");
				strcat(temp, cur->client->pers.netname);
				more_than_one = true;
			}
		}
	}
}

void replace_flaginfo(edict_t *person, char *temp)
{
	int flagcount=CTF_TEAM_UNDEFINED+1;										   
	edict_t *whichflag = NULL;

	if (!ctf_validateplayer(person, CTF_TEAM_IGNORETEAM))
		return;// only works for valid players

	strcpy(temp,"");
	while (flagcount < CTF_TEAM_LIMIT)
	{
		if (flagcount == person->client->ctf.teamnum)
			strcat(temp, "Your ");
		else
			strcat(temp, "The enemy ");
		ctf_teamstring(temp, flagcount, CTF_TEAM_MATCHING);
		strcat(temp, " flag ");
		whichflag = ctf_getteamflag(flagcount, CTF_TEAM_MATCHING);
		if (!whichflag)
			strcat(temp, "is missing.  ");
		else if (whichflag->owner)
		{
			strcat(temp,"is held by ");
			strcat(temp,whichflag->owner->client->pers.netname );
			strcat(temp,".  ");
		}
		else if (!ctf_flagathome(whichflag))
			strcat(temp, "is sitting around.  ");
		else
			strcat(temp, "is at home.  " );

		flagcount++;
	}
	strcat(temp,"\n");
}

void replace_viewinfo(edict_t *person, char *temp)
{
	strcpy(temp, "");
	if (person->client->ctf.popup_ent)
	{
		strcpy(temp, person->client->ctf.popup_ent->client->pers.netname);
		temp[15] = 0;
	}
}

void replace_carrierinfo(edict_t *person, char *temp)
{
	edict_t * teamflag;

	teamflag = ctf_getteamflag(person->client->ctf.teamnum, CTF_TEAM_OPPOSING);

	if (teamflag)
	{
		if (ctf_validateplayer(teamflag->owner, person->client->ctf.teamnum))
		{
			strcpy(temp, teamflag->owner->client->pers.netname);
			strcat(temp, " is at ");
			replace_location(teamflag->owner, temp, true, true);
		}
		else
		{
			strcpy(temp, " no carrier");
		}
	}
}


void LowerCase(char* src)
{
	size_t len;
	size_t i;

	len = strlen(src);
	for (i = 0; i < len; i++)
	{
		src[i] = tolower(src[i]);
	}
}
