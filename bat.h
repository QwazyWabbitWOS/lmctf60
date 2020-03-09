#define MALE	0
#define FEMALE	1

typedef enum
{
	CTF_MENU,
} MENU_TYPES;


#undef BAT_DEBUG
//#define BAT_DEBUG

#define MAX_HIGHSCORE_ENTRIES	7

//#define RAILFEST_TIME	60 //In seconds

#define MAX_RAILTIME	1200



typedef struct
{
	char Player[32];
	long Score;
}HIGHSCORE_STATS_TYPE;

extern char DBuffer[];
extern int Observer_Show_Menu;
extern HIGHSCORE_STATS_TYPE Highscore_Table[]; 
extern HIGHSCORE_STATS_TYPE Default_Highscore_Table[]; 
extern int MvpDisp;



void Cmd_DanMan(edict_t *ent);
void Debug_Show(char *Text);

