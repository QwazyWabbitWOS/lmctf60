#ifndef __MENU_H__
#define __MENU_H__

typedef struct
{
	char	*text;
	void	(*func)(edict_t *ent);
} menuitem;

#include "g_local.h"

#define MENU_LOCAL		0
#define MENU_MAIN		1
#define MENU_SKIN		2
#define MENU_HELP		3
#define MENU_HOWTOPLAY	4
#define MENU_COMMAND	5
#define MENU_RADIO		6

typedef struct
{
	menuitem	*menu;
	int			size;
} menudata;

void Ctf_Menu (edict_t *ent);
void Skin_Menu (edict_t *ent);
void SetSkin (edict_t *ent);

#ifdef OLDOBSERVERCODE
void Obs_CamLock_Exec(edict_t *ent);
void Observe (edict_t *ent);
#endif

void Toggle_Radio_Menu (edict_t *ent);
void Main_Menu (edict_t *ent);
void Ref_Main_Menu (edict_t *ent);
void Menu_Free (edict_t *ent);
void Menu_Next (edict_t *ent);
void Menu_Prev (edict_t *ent);
void Menu_Use (edict_t *ent);
void Menu_Draw (edict_t *ent);
void Menu_Set (edict_t *ent, int item, char *text, void	(*func)(edict_t *ent));

#endif
