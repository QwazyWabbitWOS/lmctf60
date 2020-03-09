#include "g_local.h"
#include "bat.h"



char DBuffer[512];

///////----Put all Debug functions here---------
#ifdef BAT_DEBUG


void Cmd_DanMan(edict_t *ent)
{

//	gi.centerprintf(ent, "Danman\n");
//	sprintf(DBuffer, "Danman");
//	gi.centerprintf(ent, DBuffer);

	Debug_Show("I am the Bat");
}


void Debug_Show(char *Text)
{
	gi.dprintf("%s\n", Text);
}



#endif
///////----Put all Debug functions here---------
