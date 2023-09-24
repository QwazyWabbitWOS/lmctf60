#ifndef __SKINS_H__
#define __SKINS_H__

qboolean SkinValid(edict_t *ent, char *input);
void SkinsReadFile();
qboolean SkinListInUse();
char *SkinRandom(edict_t *ent);
char **SkinGetList(edict_t *ent);
void SetEnemySkin(edict_t *target);
void SetTeamSkin(edict_t *target);
void SendSkinToClients(edict_t *player, char *skin);

#endif
