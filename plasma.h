//==============================================================================
// Plasma Rifle Configuration File
//==============================================================================

//Bat
//release 5.0 was at 2.8

//#define PLASMA_DAMAGE_MULTIPLIER	2.8
//#define PLASMA_SPLASH_RADIUS		70

#define PLASMA_SPREAD_DAMAGE		28
#define PLASMA_BOUNCE_DAMAGE		39
#define PLASMA_SPLASH_RADIUS		70



// Plasma Spread Range (max angle off line of sight) 
#define	PLASMA_SPREAD_RANGE	0.1745	// 10 degrees (radians)

// Plasma Projectile Speeds
//#define	PLASMA_REFLECT_SPEED	650
//#define	PLASMA_SPREAD_SPEED	650

//-bat
#define	PLASMA_REFLECT_SPEED	1200
#define	PLASMA_SPREAD_SPEED		1200

#define PLASMA_CELLS_PER_SHOT	10


// Model/Sprite Information
#define	PLASMA_SPRITE_FLY	"sprites/s_plasma1.sp2"
#define	PLASMA_SPRITE_HIT	"sprites/s_plasma2.sp2"
#define	PLASMA_MODEL_VIEW	"models/weapons/v_plasma/tris.md2"
#define	PLASMA_MODEL_WORLD	"models/weapons/g_plasma/tris.md2"

// Sound
#define	PLASMA_SOUND_BOUNCE	"weapons/plasma/bounce.wav"
#define	PLASMA_SOUND_EMPTY	"weapons/plasma/empty.wav"
#define	PLASMA_SOUND_FIRE1	"weapons/plasma/fire1.wav"
#define	PLASMA_SOUND_FIRE2	"weapons/plasma/fire2.wav"
#define	PLASMA_SOUND_FLYBY	"weapons/plasma/flyby.wav"
#define	PLASMA_SOUND_HIT	"weapons/plasma/hit.wav"
#define	PLASMA_SOUND_IDLE	"weapons/plasma/idle.wav"
#define	PLASMA_SOUND_PICKUP	"misc/w_pkup.wav"
#define	PLASMA_SOUND_VENT	"weapons/plasma/vent.wav"

// Misc
#define	PLASMA_ICON		"w_plasma"
#define	PLASMA_PICKUP		"Plasma Rifle"
#define	MOD_PLASMA		34
//#define MOD_PLASMA2		35

// Precache
#define	PLASMA_PRECACHE		PLASMA_SOUND_BOUNCE PLASMA_SOUND_EMPTY	\
				PLASMA_SOUND_FIRE1 PLASMA_SOUND_FIRE2	\
				PLASMA_SOUND_FLYBY PLASMA_SOUND_HIT	\
				PLASMA_SOUND_IDLE PLASMA_SOUND_PICKUP	\
				PLASMA_SOUND_VENT			\
				PLASMA_MODEL_VIEW PLASMA_MODEL_WORLD	\
				PLASMA_SPRITE_FLY PLASMA_SPRITE_HIT 



