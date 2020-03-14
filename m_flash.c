// m_flash.c

#include "q_shared.h"

// this file is included in both the game dll and quake2,
// the game needs it to source shot locations, the client
// needs it to position muzzle flashes
vec3_t monster_flash_offset [] =
{
// flash 0 is not used
	{0.0f, 0.0f, 0.0f},

// MZ2_TANK_BLASTER_1				1
	{20.7f, -18.5f, 28.7f},
// MZ2_TANK_BLASTER_2				2
	{16.6f, -21.5f, 30.1f},
// MZ2_TANK_BLASTER_3				3
	{11.8f, -23.9f, 32.1f},
// MZ2_TANK_MACHINEGUN_1			4
	{22.9f, -0.7f, 25.3f},
// MZ2_TANK_MACHINEGUN_2			5
	{22.2f, 6.2f, 22.3f},
// MZ2_TANK_MACHINEGUN_3			6
	{19.4f, 13.1f, 18.6f},
// MZ2_TANK_MACHINEGUN_4			7
	{19.4f, 18.8f, 18.6f},
// MZ2_TANK_MACHINEGUN_5			8
	{17.9f, 25.0f, 18.6f},
// MZ2_TANK_MACHINEGUN_6			9
	{14.1f, 30.5f, 20.6f},
// MZ2_TANK_MACHINEGUN_7			10
	{9.3f, 35.3f, 22.1f},
// MZ2_TANK_MACHINEGUN_8			11
	{4.7f, 38.4f, 22.1f},
// MZ2_TANK_MACHINEGUN_9			12
	{-1.1f, 40.4f, 24.1f},
// MZ2_TANK_MACHINEGUN_10			13
	{-6.5f, 41.2f, 24.1f},
// MZ2_TANK_MACHINEGUN_11			14
	{3.2f, 40.1f, 24.7f},
// MZ2_TANK_MACHINEGUN_12			15
	{11.7f, 36.7f, 26.0f},
// MZ2_TANK_MACHINEGUN_13			16
	{18.9f, 31.3f, 26.0f},
// MZ2_TANK_MACHINEGUN_14			17
	{24.4f, 24.4f, 26.4f},
// MZ2_TANK_MACHINEGUN_15			18
	{27.1f, 17.1f, 27.2f},
// MZ2_TANK_MACHINEGUN_16			19
	{28.5f, 9.1f, 28.0f},
// MZ2_TANK_MACHINEGUN_17			20
	{27.1f, 2.2f, 28.0f},
// MZ2_TANK_MACHINEGUN_18			21
	{24.9f, -2.8f, 28.0f},
// MZ2_TANK_MACHINEGUN_19			22
	{21.6f, -7.0f, 26.4f},
// MZ2_TANK_ROCKET_1				23
	{6.2f, 29.1f, 49.1f},
// MZ2_TANK_ROCKET_2				24
	{6.9f, 23.8f, 49.1f},
// MZ2_TANK_ROCKET_3				25
	{8.3f, 17.8f, 49.5f},

// MZ2_INFANTRY_MACHINEGUN_1		26
	{26.6f, 7.1f, 13.1f},
// MZ2_INFANTRY_MACHINEGUN_2		27
	{18.2f, 7.5f, 15.4f},
// MZ2_INFANTRY_MACHINEGUN_3		28
	{17.2f, 10.3f, 17.9f},
// MZ2_INFANTRY_MACHINEGUN_4		29
	{17.0f, 12.8f, 20.1f},
// MZ2_INFANTRY_MACHINEGUN_5		30
	{15.1f, 14.1f, 21.8f},
// MZ2_INFANTRY_MACHINEGUN_6		31
	{11.8f, 17.2f, 23.1f},
// MZ2_INFANTRY_MACHINEGUN_7		32
	{11.4f, 20.2f, 21.0f},
// MZ2_INFANTRY_MACHINEGUN_8		33
	{9.0f, 23.0f, 18.9f},
// MZ2_INFANTRY_MACHINEGUN_9		34
	{13.9f, 18.6f, 17.7f},
// MZ2_INFANTRY_MACHINEGUN_10		35
	{15.4f, 15.6f, 15.8f},
// MZ2_INFANTRY_MACHINEGUN_11		36
	{10.2f, 15.2f, 25.1f},
// MZ2_INFANTRY_MACHINEGUN_12		37
	{-1.9f, 15.1f, 28.2f},
// MZ2_INFANTRY_MACHINEGUN_13		38
	{-12.4f, 13.0f, 20.2f},

// MZ2_SOLDIER_BLASTER_1			39
	{10.6f * 1.2f, 7.7f * 1.2f, 7.8f * 1.2f},
// MZ2_SOLDIER_BLASTER_2			40
	{21.1f * 1.2f, 3.6f * 1.2f, 19.0f * 1.2f},
// MZ2_SOLDIER_SHOTGUN_1			41
	{10.6f * 1.2f, 7.7f * 1.2f, 7.8f * 1.2f},
// MZ2_SOLDIER_SHOTGUN_2			42
	{21.1f * 1.2f, 3.6f * 1.2f, 19.0f * 1.2f},
// MZ2_SOLDIER_MACHINEGUN_1			43
	{10.6f * 1.2f, 7.7f * 1.2f, 7.8f * 1.2f},
// MZ2_SOLDIER_MACHINEGUN_2			44
	{21.1f * 1.2f, 3.6f * 1.2f, 19.0f * 1.2f},

// MZ2_GUNNER_MACHINEGUN_1			45
	{30.1f * 1.15f, 3.9f * 1.15f, 19.6f * 1.15f},
// MZ2_GUNNER_MACHINEGUN_2			46
	{29.1f * 1.15f, 2.5f * 1.15f, 20.7f * 1.15f},
// MZ2_GUNNER_MACHINEGUN_3			47
	{28.2f * 1.15f, 2.5f * 1.15f, 22.2f * 1.15f},
// MZ2_GUNNER_MACHINEGUN_4			48
	{28.2f * 1.15f, 3.6f * 1.15f, 22.0f * 1.15f},
// MZ2_GUNNER_MACHINEGUN_5			49
	{26.9f * 1.15f, 2.0f * 1.15f, 23.4f * 1.15f},
// MZ2_GUNNER_MACHINEGUN_6			50
	{26.5f * 1.15f, 0.6f * 1.15f, 20.8f * 1.15f},
// MZ2_GUNNER_MACHINEGUN_7			51
	{26.9f * 1.15f, 0.5f * 1.15f, 21.5f * 1.15f},
// MZ2_GUNNER_MACHINEGUN_8			52
	{29.0f * 1.15f, 2.4f * 1.15f, 19.5f * 1.15f},
// MZ2_GUNNER_GRENADE_1				53
	{4.6f * 1.15f, -16.8f * 1.15f, 7.3f * 1.15f},
// MZ2_GUNNER_GRENADE_2				54
	{4.6f * 1.15f, -16.8f * 1.15f, 7.3f * 1.15f},
// MZ2_GUNNER_GRENADE_3				55
	{4.6f * 1.15f, -16.8f * 1.15f, 7.3f * 1.15f},
// MZ2_GUNNER_GRENADE_4				56
	{4.6f * 1.15f, -16.8f * 1.15f, 7.3f * 1.15f},

// MZ2_CHICK_ROCKET_1				57
//	-24.8, -9.0, 39.0,
	{24.8f, -9.0f, 39.0f},			// PGM - this was incorrect in Q2

// MZ2_FLYER_BLASTER_1				58
	{12.1f, 13.4f, -14.5f},
// MZ2_FLYER_BLASTER_2				59
	{12.1f, -7.4f, -14.5f},

// MZ2_MEDIC_BLASTER_1				60
	{12.1f, 5.4f, 16.5f},

// MZ2_GLADIATOR_RAILGUN_1			61
	{30.0f, 18.0f, 28.0f},

// MZ2_HOVER_BLASTER_1				62
	{32.5f, -0.8f, 10.0f},

// MZ2_ACTOR_MACHINEGUN_1			63
	{18.4f, 7.4f, 9.6f},

// MZ2_SUPERTANK_MACHINEGUN_1		64
	{30.0f, 30.0f, 88.5f},
// MZ2_SUPERTANK_MACHINEGUN_2		65
	{30.0f, 30.0f, 88.5f},
// MZ2_SUPERTANK_MACHINEGUN_3		66
	{30.0f, 30.0f, 88.5f},
// MZ2_SUPERTANK_MACHINEGUN_4		67
	{30.0f, 30.0f, 88.5f},
// MZ2_SUPERTANK_MACHINEGUN_5		68
	{30.0f, 30.0f, 88.5f},
// MZ2_SUPERTANK_MACHINEGUN_6		69
	{30.0f, 30.0f, 88.5f},
// MZ2_SUPERTANK_ROCKET_1			70
	{16.0f, -22.5f, 91.2f},
// MZ2_SUPERTANK_ROCKET_2			71
	{16.0f, -33.4f, 86.7f},
// MZ2_SUPERTANK_ROCKET_3			72
	{16.0f, -42.8f, 83.3f},

// --- Start Xian Stuff ---
// MZ2_BOSS2_MACHINEGUN_L1			73
	{-32,	-40,	70},
// MZ2_BOSS2_MACHINEGUN_L2			74
	{-32,	-40,	70},
// MZ2_BOSS2_MACHINEGUN_L3			75
	{-32,	-40,	70},
// MZ2_BOSS2_MACHINEGUN_L4			76
	{-32,	-40,	70},
// MZ2_BOSS2_MACHINEGUN_L5			77
	{-32,	-40,	70},
// --- End Xian Stuff

// MZ2_BOSS2_ROCKET_1				78
	{22.0f, 16.0f, 10.0f},
// MZ2_BOSS2_ROCKET_2				79
	{22.0f, 8.0f, 10.0f},
// MZ2_BOSS2_ROCKET_3				80
	{22.0f, -8.0f, 10.0f},
// MZ2_BOSS2_ROCKET_4				81
	{22.0f, -16.0f, 10.0f},

// MZ2_FLOAT_BLASTER_1				82
	{32.5f, -0.8f, 10},

// MZ2_SOLDIER_BLASTER_3			83
	{20.8f * 1.2f, 10.1f * 1.2f, -2.7f * 1.2f},
// MZ2_SOLDIER_SHOTGUN_3			84
	{20.8f * 1.2f, 10.1f * 1.2f, -2.7f * 1.2f},
// MZ2_SOLDIER_MACHINEGUN_3			85
	{20.8f * 1.2f, 10.1f * 1.2f, -2.7f * 1.2f},
// MZ2_SOLDIER_BLASTER_4			86
	{7.6f * 1.2f, 9.3f * 1.2f, 0.8f * 1.2f},
// MZ2_SOLDIER_SHOTGUN_4			87
	{7.6f * 1.2f, 9.3f * 1.2f, 0.8f * 1.2f},
// MZ2_SOLDIER_MACHINEGUN_4			88
	{7.6f * 1.2f, 9.3f * 1.2f, 0.8f * 1.2f},
// MZ2_SOLDIER_BLASTER_5			89
	{30.5f * 1.2f, 9.9f * 1.2f, -18.7f * 1.2f},
// MZ2_SOLDIER_SHOTGUN_5			90
	{30.5f * 1.2f, 9.9f * 1.2f, -18.7f * 1.2f},
// MZ2_SOLDIER_MACHINEGUN_5			91
	{30.5f * 1.2f, 9.9f * 1.2f, -18.7f * 1.2f},
// MZ2_SOLDIER_BLASTER_6			92
	{27.6f * 1.2f, 3.4f * 1.2f, -10.4f * 1.2f},
// MZ2_SOLDIER_SHOTGUN_6			93
	{27.6f * 1.2f, 3.4f * 1.2f, -10.4f * 1.2f},
// MZ2_SOLDIER_MACHINEGUN_6			94
	{27.6f * 1.2f, 3.4f * 1.2f, -10.4f * 1.2f},
// MZ2_SOLDIER_BLASTER_7			95
	{28.9f * 1.2f, 4.6f * 1.2f, -8.1f * 1.2f},
// MZ2_SOLDIER_SHOTGUN_7			96
	{28.9f * 1.2f, 4.6f * 1.2f, -8.1f * 1.2f},
// MZ2_SOLDIER_MACHINEGUN_7			97
	{28.9f * 1.2f, 4.6f * 1.2f, -8.1f * 1.2f},
// MZ2_SOLDIER_BLASTER_8			98
	{34.5f * 1.2f, 9.6f * 1.2f, 6.1f * 1.2f},
// MZ2_SOLDIER_SHOTGUN_8			99
	{34.5f * 1.2f, 9.6f * 1.2f, 6.1f * 1.2f},
// MZ2_SOLDIER_MACHINEGUN_8			100
	{34.5f * 1.2f, 9.6f * 1.2f, 6.1f * 1.2f},

// --- Xian shit below ---
// MZ2_MAKRON_BFG					101
	{17,		-19.5f,	62.9f},
// MZ2_MAKRON_BLASTER_1				102
	{-3.6f,	-24.1f,	59.5f},
// MZ2_MAKRON_BLASTER_2				103
	{-1.6f,	-19.3f,	59.5f},
// MZ2_MAKRON_BLASTER_3				104
	{-0.1f,	-14.4f,	59.5f},		
// MZ2_MAKRON_BLASTER_4				105
	{2.0f,	-7.6f,	59.5f},	
// MZ2_MAKRON_BLASTER_5				106
	{3.4f,	1.3f,	59.5f},
// MZ2_MAKRON_BLASTER_6				107
	{3.7f,	11.1f,	59.5f},	
// MZ2_MAKRON_BLASTER_7				108
	{-0.3f,	22.3f,	59.5f},
// MZ2_MAKRON_BLASTER_8				109
	{-6,		33,		59.5f},
// MZ2_MAKRON_BLASTER_9				110
	{-9.3f,	36.4f,	59.5f},
// MZ2_MAKRON_BLASTER_10			111
	{-7,		35,		59.5f},
// MZ2_MAKRON_BLASTER_11			112
	{-2.1f,	29,		59.5f},
// MZ2_MAKRON_BLASTER_12			113
	{3.9f,	17.3f,	59.5f},
// MZ2_MAKRON_BLASTER_13			114
	{6.1f,	5.8f,	59.5f},
// MZ2_MAKRON_BLASTER_14			115
	{5.9f,	-4.4f,	59.5f},
// MZ2_MAKRON_BLASTER_15			116
	{4.2f,	-14.1f,	59.5f},		
// MZ2_MAKRON_BLASTER_16			117
	{2.4f,	-18.8f,	59.5f},
// MZ2_MAKRON_BLASTER_17			118
	{-1.8f,	-25.5f,	59.5f},
// MZ2_MAKRON_RAILGUN_1				119
	{-17.3f,	7.8f,	72.4f},

// MZ2_JORG_MACHINEGUN_L1			120
	{78.5f,	-47.1f,	96},			
// MZ2_JORG_MACHINEGUN_L2			121
	{78.5f,	-47.1f,	96},			
// MZ2_JORG_MACHINEGUN_L3			122
	{78.5f,	-47.1f,	96},			
// MZ2_JORG_MACHINEGUN_L4			123
	{78.5f,	-47.1f,	96},			
// MZ2_JORG_MACHINEGUN_L5			124
	{78.5f,	-47.1f,	96},			
// MZ2_JORG_MACHINEGUN_L6			125
	{78.5f,	-47.1f,	96},			
// MZ2_JORG_MACHINEGUN_R1			126
	{78.5f,	46.7f,  96},			
// MZ2_JORG_MACHINEGUN_R2			127
	{78.5f,	46.7f,	96},			
// MZ2_JORG_MACHINEGUN_R3			128
	{78.5f,	46.7f,	96},			
// MZ2_JORG_MACHINEGUN_R4			129
	{78.5f,	46.7f,	96},			
// MZ2_JORG_MACHINEGUN_R5			130
	{78.5f,	46.7f,	96},			
// MZ2_JORG_MACHINEGUN_R6			131
	{78.5f,	46.7f,	96},			
// MZ2_JORG_BFG_1					132
	{6.3f,	-9,		111.2f},

// MZ2_BOSS2_MACHINEGUN_R1			73
	{-32,	40,	70},
// MZ2_BOSS2_MACHINEGUN_R2			74
	{-32,	40,	70},
// MZ2_BOSS2_MACHINEGUN_R3			75
	{-32,	40,	70},
// MZ2_BOSS2_MACHINEGUN_R4			76
	{-32,	40,	70},
// MZ2_BOSS2_MACHINEGUN_R5			77
	{-32,	40,	70},

// --- End Xian Shit ---

// end of table
	{0.0f, 0.0f, 0.0f}
};
