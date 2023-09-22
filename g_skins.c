#include "g_local.h"
#include "g_skins.h"
#include "g_ctffunc.h"

#define	SKIN_RED	0
#define SKIN_BLUE	1
#define	SKIN_MAXCOLOR	2

#define SKIN_MODELNAME	0
#define SKIN_SKINNAME	1
#define SKIN_MAXNAME	2

#define SKIN_MAXSKINS	256

qboolean skinlistinuse = false;
char *skinlist[SKIN_MAXCOLOR][SKIN_MAXSKINS][SKIN_MAXNAME];

char* CopyString(char* in)
{
	char* out;

	out = gi.TagMalloc((int)strlen(in) + 1, TAG_GAME);
	strcpy(out, in);
	return out;
}

void SkinsReadFile()
{
	FILE	*fp;

	char	name[MAX_INFO_STRING], *newb = NULL;
	size_t	size;
	char    *tempbuf;
	char	line[MAX_INFO_STRING] = { 0 };
	int		colorindex;
	char	skin[MAX_INFO_STRING] = { 0 };
	char	model[MAX_INFO_STRING] = { 0 };
	int		red, blue;
	int		count, ch;

	strcpy(name, gamedir->string);
	strcat(name, "/");
	strcat(name, skin_file->string);

	
	// Open the skins file
	fp = fopen (name, "rt");
	if (!fp)
		return;

	// Prescan to get size we need.
	for (count = 0; (ch = fgetc(fp)) != EOF; count++);
	fseek(fp, 0, SEEK_SET);  //rewind

	// Read the whole skins file and place it in a spot of memory
	
	tempbuf = (char *) gi.TagMalloc(count + 2, TAG_GAME);
	if (tempbuf)
	{
		size = fread(tempbuf, 1, count, fp);
		fclose(fp);

		if (size)
		{
			newb = tempbuf;
			newb[size] = 0;
		}
		else
		{
			if (size >= (size_t)count + 1)
				gi.dprintf("Error: skins file truncated.\n");
			else
				gi.dprintf("Error: skins file empty.\n");
			return;
		}

	}
	else
	{
		gi.dprintf("Error: unable to allocate memory for entities.\n");
		return;
	}

	// Parse our input
	// One list of red skins, one of blue
	// Each list contains model and skin name
	
	red = blue = 0;
	skinlist[SKIN_RED][red][SKIN_MODELNAME] = NULL;
	skinlist[SKIN_RED][red][SKIN_SKINNAME] = NULL;
	skinlist[SKIN_BLUE][blue][SKIN_MODELNAME] = NULL;
	skinlist[SKIN_BLUE][blue][SKIN_SKINNAME] = NULL;

	if (!newb)
		return; // No valid file

	skinlistinuse = true;
	colorindex = SKIN_RED;

	while (tempbuf - newb < (ptrdiff_t)size && sscanf(tempbuf,"%[^\n]", line))
	{
		tempbuf += strlen(line);
		
		// Step over newline
		while (*tempbuf == '\n')
			tempbuf++;

		// Ignore lines with # or / or ;
		if (line[0] == '#' || line[0] == '/' || line[0] == ';')
			continue;

		// Determine which skinset we are dealing with
		if (!strcmp(line, "[red]"))
			colorindex = SKIN_RED;
		if (!strcmp(line, "[blue]"))
			colorindex = SKIN_BLUE;

		// If it is a skin, add it to the list
		if (strlen(line) && sscanf(line, "%[^/]/%s", model, skin) == 2)
		{
			// Add to the red list?
			if (colorindex == SKIN_RED)
			{
				skinlist[SKIN_RED][red][SKIN_MODELNAME] = CopyString(model);
				skinlist[SKIN_RED][red][SKIN_SKINNAME] = CopyString(skin);
				red++;
				skinlist[SKIN_RED][red][SKIN_MODELNAME] = NULL;
				skinlist[SKIN_RED][red][SKIN_SKINNAME] = NULL;
			}
			else if (colorindex == SKIN_BLUE)
			{
				skinlist[SKIN_BLUE][blue][SKIN_MODELNAME] = CopyString(model);
				skinlist[SKIN_BLUE][blue][SKIN_SKINNAME] = CopyString(skin);
				blue++;
				skinlist[SKIN_BLUE][blue][SKIN_MODELNAME] = NULL;
				skinlist[SKIN_BLUE][blue][SKIN_SKINNAME] = NULL;
			}

		}
	}

	// Free the memory
	gi.TagFree(newb);

	if (skin_debug->value)
	{
		red = 0;
		gi.dprintf("[red]\n");
		while (skinlist[SKIN_RED][red][SKIN_MODELNAME])
		{
			gi.dprintf("MODEL: [%s]  SKIN: [%s]\n",
				skinlist[SKIN_RED][red][SKIN_MODELNAME],
				skinlist[SKIN_RED][red][SKIN_SKINNAME]);
			red++;
		}
		red = 0;
		gi.dprintf("\n[blue]\n");
		while (skinlist[SKIN_BLUE][red][SKIN_MODELNAME])
		{
			gi.dprintf("MODEL: [%s]  SKIN: [%s]\n",
				skinlist[SKIN_BLUE][red][SKIN_MODELNAME],
				skinlist[SKIN_BLUE][red][SKIN_SKINNAME]);
			red++;
		}
	}
}

qboolean
SkinValid(edict_t *ent, char *input)
{
	int teamnum, i;
	qboolean	valid = false;
	char	skin[MAX_INFO_STRING] = { 0 };
	char	model[MAX_INFO_STRING] = { 0 };

	if (!ent->client)
		return false;

	if (!input)
		return false;

	// Parse our input skin and model
	if (sscanf(input, "%[^/]/%s", model, skin) != 2)
		return false; // Doesn't contain skin and model

	if (ent->client->ctf.teamnum == CTF_TEAM_RED)
		teamnum = SKIN_RED;
	else 
		teamnum = SKIN_BLUE;

	i = 0;
	while(skinlist[teamnum][i][SKIN_MODELNAME])
	{
		if (!strcmp(skinlist[teamnum][i][SKIN_SKINNAME], skin) &&
			!strcmp(skinlist[teamnum][i][SKIN_MODELNAME], model))
		{
			valid = true;
			break;
		}
		i++;
	}
	return valid;
}

qboolean SkinListInUse()
{
	return skinlistinuse;
}

char *SkinRandom(edict_t *ent)
{
	int teamnum,i;
	static	char skin[MAX_INFO_STRING]; // Returned and used as a temp

	if (ent->client->ctf.teamnum == CTF_TEAM_RED)
		teamnum = SKIN_RED;
	else 
		teamnum = SKIN_BLUE;

	i = 0;
	while (skinlist[teamnum][i][SKIN_MODELNAME])
	{
		i++;
	}

	i = rand() % i;

	sprintf(skin, "%s/%s", skinlist[teamnum][i][SKIN_MODELNAME],
		skinlist[teamnum][i][SKIN_SKINNAME]);

	return skin;
}

char **SkinGetList(edict_t *ent)
{
	int teamnum, i;
	static	char *skins[SKIN_MAXSKINS]; // Returned and used as a temp
	static  char storage[SKIN_MAXSKINS][30];
	
	if (ent->client->ctf.teamnum == CTF_TEAM_RED)
		teamnum = SKIN_RED;
	else 
		teamnum = SKIN_BLUE;

	i = 0;
	while (skinlist[teamnum][i][SKIN_MODELNAME])
	{
		sprintf(storage[i], "%s/%s", skinlist[teamnum][i][SKIN_MODELNAME],
			skinlist[teamnum][i][SKIN_SKINNAME]);
		skins[i] = storage[i];		
		i++;
	}
	skins[i] = NULL;

	return skins;

}

/**
 * Sets the teamskin property of an player and
 * applies it to all team members. This does not
 * set the global skin for a particular team, but
 * the specific skin one player wants to use for
 * their team. This allows players to override
 * the team default skin for whatever reason.
 *
 * Can be set to ANY skin, regardless of whether
 * it's in the skin list or not.
 *
 * Called when a player manually types "tskin"
 * command.
 */
void SetTeamSkin(edict_t *target)
{
    edict_t *ent;

    if (!target->client->pers.teamskin[0]) {
        return;
    }

    for (ent = g_edicts + 1; ent <= g_edicts + game.maxclients; ent++) {
        if (!ent->inuse) {
            continue;
        }

        if (!TEAMMATES(ent, target)) {
            continue;
        }

        gi.WriteByte(svc_configstring);
        gi.WriteShort(CS_PLAYERSKINS + (ent - g_edicts) - 1);
        gi.WriteString(va("%s\\%s", NAME(ent), target->client->pers.teamskin));
        gi.unicast(target, true);
    }
}

/**
 * Sets the enemyskin property of an player and
 * applies it to all enemy players.
 *
 * Can be set to ANY skin, regardless of whether
 * it's in the skin list or not.
 *
 * Called when a player manually types "eskin"
 * command.
 */
void SetEnemySkin(edict_t *target)
{
    edict_t *ent;

    if (!target->client->pers.enemyskin[0]) {
        return;
    }

    for (ent = g_edicts + 1; ent <= g_edicts + game.maxclients; ent++) {
        if (!ent->inuse) {
            continue;
        }

        if (IS_SPECTATOR(ent)) {
            continue;
        }

        if (TEAMMATES(ent, target)) {
            continue;
        }

        gi.WriteByte(svc_configstring);
        gi.WriteShort(CS_PLAYERSKINS + (ent - g_edicts) - 1);
        gi.WriteString(va("%s\\%s", NAME(ent), target->client->pers.enemyskin));
        gi.unicast(target, true);
    }
}

/**
 * Send skinfor's skin to all players so they know what to display.
 *
 * If another player has custom skins set (tskin/eskin), those skins
 * will be sent to those players instead.
 */
void SetPlayerSkin(edict_t *skinfor)
{
    edict_t *ent;
    char *newskin;
    char *defaultskin;

    if (!skinfor->client) {
        return;
    }

    // pick a team-specific random skin
    defaultskin = SkinRandom(skinfor);

    for (ent = g_edicts + 1; ent <= g_edicts + game.maxclients; ent++) {
        if (!ent->inuse) {
            continue;
        }

        newskin = defaultskin;

        if (TEAMMATES(ent, skinfor) && ent->client->pers.teamskin[0]) {
            newskin = va("%s\\%s", NAME(skinfor), ent->client->pers.teamskin);
        }

        if (!TEAMMATES(ent, skinfor) && ent->client->pers.enemyskin[0]) {
            newskin = va("%s\\%s", NAME(skinfor), ent->client->pers.enemyskin);
        }

        gi.WriteByte(svc_configstring);
        gi.WriteShort(CS_PLAYERSKINS + (skinfor - g_edicts) - 1);
        gi.WriteString(newskin);
        gi.unicast(ent, true);
    }
}
