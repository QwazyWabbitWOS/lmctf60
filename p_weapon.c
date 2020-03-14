// g_weapon.c

#include "g_local.h"
#include "m_player.h"
#include "g_tourney.h"

// SKWiD MOD
#include "plasma.h"
extern void fire_plasma (edict_t *ent, vec3_t start, vec3_t dir, int mode);
extern void Weapon_PLASMA_Generic (edict_t *,int,int,int,int,int *,int *,void(*fire)(edict_t *));
// END
#include "g_ctffunc.h"

#define GRAPPLE_FIRE_HOOK_SPEED        800
#define GRAPPLE_PULL_SPEED             800
#define GRAPPLE_PULL_BALANCED_SPEED    800

static qboolean	is_quad;
static byte		is_silenced;

qboolean CheckTeamDamage (edict_t *targ, edict_t *attacker); // CTF CODE -- LM_JORM

void weapon_grenade_fire (edict_t *ent, qboolean held);
void Show_Plasma_Mode(gitem_t *New_Weapon, edict_t *ent);


static void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy (distance, _distance);
	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= -1;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;
	G_ProjectSource (point, _distance, forward, right, result);
}


/*
===============
PlayerNoise

Each player can have two noise objects associated with it:
a personal noise (jumping, pain, weapon firing), and a weapon
target noise (bullet wall impacts)

Monsters that don't directly see the player can move
to a noise in hopes of seeing the player from there.
===============
*/
void PlayerNoise(edict_t *who, vec3_t where, int type)
{
	edict_t		*noise;

	if (type == PNOISE_WEAPON)
	{
		if (who->client->silencer_shots)
		{
			who->client->silencer_shots--;
			return;
		}
	}

	if (deathmatch->value)
		return;

	if (who->flags & FL_NOTARGET)
		return;


	if (!who->mynoise)
	{
		noise = G_Spawn();
		noise->classname = "player_noise";
		VectorSet (noise->mins, -8, -8, -8);
		VectorSet (noise->maxs, 8, 8, 8);
		noise->owner = who;
		noise->svflags = SVF_NOCLIENT;
		who->mynoise = noise;

		noise = G_Spawn();
		noise->classname = "player_noise";
		VectorSet (noise->mins, -8, -8, -8);
		VectorSet (noise->maxs, 8, 8, 8);
		noise->owner = who;
		noise->svflags = SVF_NOCLIENT;
		who->mynoise2 = noise;
	}

	if (type == PNOISE_SELF || type == PNOISE_WEAPON)
	{
		noise = who->mynoise;
		level.sound_entity = noise;
		level.sound_entity_framenum = level.framenum;
	}
	else // type == PNOISE_IMPACT
	{
		noise = who->mynoise2;
		level.sound2_entity = noise;
		level.sound2_entity_framenum = level.framenum;
	}

	VectorCopy (where, noise->s.origin);
	VectorSubtract (where, noise->maxs, noise->absmin);
	VectorAdd (where, noise->maxs, noise->absmax);
	noise->teleport_time = level.time;
	gi.linkentity (noise);
}


qboolean Pickup_Weapon (edict_t *ent, edict_t *other)
{
	int			index;
	gitem_t		*ammo;


	if(matchstate == MATCH_RAILGUN_INPLAY)
		return(false);

	index = ITEM_INDEX(ent->item);

	if ( ( ((int)(dmflags->value) & DF_WEAPONS_STAY) || coop->value) 
		&& other->client->pers.inventory[index])
	{
		if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM) ) )
			return false;	// leave the weapon for others to pickup
	}

	other->client->pers.inventory[index]++;

	if (!(ent->spawnflags & DROPPED_ITEM) )
	{
		// give them some ammo with it
		ammo = FindItem (ent->item->ammo);
		if ( (int)dmflags->value & DF_INFINITE_AMMO )
			Add_Ammo (other, ammo, 1000);
		else
			Add_Ammo (other, ammo, ammo->quantity);

		if (! (ent->spawnflags & DROPPED_PLAYER_ITEM) )
		{
			if (deathmatch->value)
			{
				if ((int)(dmflags->value) & DF_WEAPONS_STAY)
					ent->flags |= FL_RESPAWN;
				else
					SetRespawn (ent, 30);
			}
			if (coop->value)
				ent->flags |= FL_RESPAWN;
		}
	}

	if (other->client->pers.weapon != ent->item && 
		(other->client->pers.inventory[index] == 1) &&
		( !deathmatch->value || other->client->pers.weapon == FindItem("blaster") ) )
		other->client->newweapon = ent->item;

	return true;
}


/*
===============
ChangeWeapon

The old weapon has been dropped all the way, so make the new one
current
===============
*/
void ChangeWeapon (edict_t *ent)
{
	int i;

	if(matchstate == MATCH_RAILGUN_INPLAY && ent->health > 0)
	{
		ent->client->newweapon = FindItem ("railgun");
		return;
	}



	if (ent->client->grenade_time)
	{
		ent->client->grenade_time = level.time;
		ent->client->weapon_sound = 0;
		weapon_grenade_fire (ent, false);
		ent->client->grenade_time = 0;
	}

	ent->client->pers.lastweapon = ent->client->pers.weapon;
	ent->client->pers.weapon = ent->client->newweapon;
	ent->client->newweapon = NULL;
	ent->client->machinegun_shots = 0;

	// set visible model
	if (ent->s.modelindex == 255) {
		if (ent->client->pers.weapon)
			i = ((ent->client->pers.weapon->weapmodel & 0xff) << 8);
		else
			i = 0;
		ent->s.skinnum = (ent - g_edicts - 1) | i;
	}

	if (ent->client->pers.weapon && ent->client->pers.weapon->ammo)
		ent->client->ammo_index = ITEM_INDEX(FindItem(ent->client->pers.weapon->ammo));
	else
		ent->client->ammo_index = 0;

	if (!ent->client->pers.weapon)
	{	// dead
		ent->client->ps.gunindex = 0;
		return;
	}

	ent->client->weaponstate = WEAPON_ACTIVATING;
	ent->client->ps.gunframe = 0;
	ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);

	ent->client->anim_priority = ANIM_PAIN;
	if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
			ent->s.frame = FRAME_crpain1;
			ent->client->anim_end = FRAME_crpain4;
	}
	else
	{
			ent->s.frame = FRAME_pain301;
			ent->client->anim_end = FRAME_pain304;
			
	}
}

/*
=================
NoAmmoWeaponChange
=================
*/
void NoAmmoWeaponChange (edict_t *ent)
{
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("railgun"))] )
	{
		ent->client->newweapon = FindItem ("railgun");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("hyperblaster"))] )
	{
		ent->client->newweapon = FindItem ("hyperblaster");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("chaingun"))] )
	{
		ent->client->newweapon = FindItem ("chaingun");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("machinegun"))] )
	{
		ent->client->newweapon = FindItem ("machinegun");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))] > 1
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("super shotgun"))] )
	{
		ent->client->newweapon = FindItem ("super shotgun");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("shotgun"))] )
	{
		ent->client->newweapon = FindItem ("shotgun");
		return;
	}
	ent->client->newweapon = FindItem ("blaster");
}

/*
=================
Think_Weapon

Called by ClientBeginServerFrame and ClientThink
=================
*/
void Think_Weapon (edict_t *ent)
{
	is_quad = (ent->client->quad_framenum > level.framenum);

	// if just died, put the weapon away
	if (ent->health < 1)
	{
		ent->client->newweapon = NULL;
		ChangeWeapon (ent);
	}

	// call active weapon think routine
	if (ent->client->pers.weapon && ent->client->pers.weapon->weaponthink)
	{
		if (ent->client->silencer_shots)
			is_silenced = MZ_SILENCED;
		else
			is_silenced = 0;
		ent->client->pers.weapon->weaponthink (ent);
		RuneWeaponThinkHook (ent);
		
		// LM_JORM -- Switch to next weapon if out of ammo

		/*
		if (ent->client->ammo_index && 
			ent->client->pers.inventory[ent->client->ammo_index] < ent->client->pers.weapon->quantity)
		{
			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			//NoAmmoWeaponChange (ent);
			Cmd_WeapNext_f(ent);
		}
		*/

		// LM_JORM -- Speed up all weapons!

		/*
		if (!(level.framenum % 4))
		{
			if (ent->client->ps.gunframe)
				ent->client->pers.weapon->weaponthink (ent);
		}
		*/
		// END -- LM_JORM
	}
}


/*
================
Use_Weapon

Make the weapon ready if there is ammo
================
*/
void Use_Weapon (edict_t *ent, gitem_t *item)
{
	int			ammo_index;
	gitem_t		*ammo_item;
	char message[MAX_INFO_STRING];

	// see if we're already using it
	if (item == ent->client->pers.weapon)
		return;

	if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO))
	{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);

		if (!ent->client->pers.inventory[ammo_index])
		{
			sprintf(message, "No %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			ctf_SafePrint(ent, PRINT_HIGH, message);
			return;
		}

		if (ent->client->pers.inventory[ammo_index] < item->quantity)
		{
			sprintf(message, "Not enough %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			ctf_SafePrint(ent, PRINT_HIGH, message);
			return;
		}
	}

	// change to this weapon when down
	ent->client->newweapon = item;
}



/*
================
Drop_Weapon
================
*/
void Drop_Weapon (edict_t *ent, gitem_t *item)
{
	int		index;

	if ((int)(dmflags->value) & DF_WEAPONS_STAY)
		return;

	index = ITEM_INDEX(item);
	// see if we're already using it
	if ( ((item == ent->client->pers.weapon) || (item == ent->client->newweapon))&& (ent->client->pers.inventory[index] == 1) )
	{
		ctf_SafePrint(ent, PRINT_HIGH, "Can't drop current weapon\n");
		return;
	}

	Drop_Item (ent, item);
	ent->client->pers.inventory[index]--;
}


/*
================
Weapon_Generic

A generic function to handle the basics of weapon thinking
================
*/
#define FRAME_FIRE_FIRST		(FRAME_ACTIVATE_LAST + 1)
#define FRAME_IDLE_FIRST		(FRAME_FIRE_LAST + 1)
#define FRAME_DEACTIVATE_FIRST	(FRAME_IDLE_LAST + 1)

void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent))
{
	int		n;
	ent->client->isfiring = 0; // By default, we aren't firing;

	if(ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
	{
		return;
	}

	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
		{
			ChangeWeapon (ent);
			return;
		}
		else if ((FRAME_DEACTIVATE_LAST - ent->client->ps.gunframe) == 4)
		{
			ent->client->anim_priority = ANIM_REVERSE;
			if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->s.frame = FRAME_crpain4+1;
				ent->client->anim_end = FRAME_crpain1;
			}
			else
			{
				ent->s.frame = FRAME_pain304+1;
				ent->client->anim_end = FRAME_pain301;
				
			}
		}

		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		if (ent->client->ps.gunframe == FRAME_ACTIVATE_LAST)
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
			return;
		}

		ent->client->ps.gunframe++;
		return;
	}

	if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING))
	{
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;

		if ((FRAME_DEACTIVATE_LAST - FRAME_DEACTIVATE_FIRST) < 4)
		{
			ent->client->anim_priority = ANIM_REVERSE;
			if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->s.frame = FRAME_crpain4+1;
				ent->client->anim_end = FRAME_crpain1;
			}
			else
			{
				ent->s.frame = FRAME_pain304+1;
				ent->client->anim_end = FRAME_pain301;
				
			}
		}
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if ((!ent->client->ammo_index) || 
				( ent->client->pers.inventory[ent->client->ammo_index] >= ent->client->pers.weapon->quantity))
			{
				ent->client->ps.gunframe = FRAME_FIRE_FIRST;
				ent->client->weaponstate = WEAPON_FIRING;

				// start the animation
				ent->client->anim_priority = ANIM_ATTACK;
				if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
				{
					ent->s.frame = FRAME_crattak1-1;
					ent->client->anim_end = FRAME_crattak9;
				}
				else
				{
					ent->s.frame = FRAME_attack1-1;
					ent->client->anim_end = FRAME_attack8;
				}
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
		}
		else
		{
			if (ent->client->ps.gunframe == FRAME_IDLE_LAST)
			{
				ent->client->ps.gunframe = FRAME_IDLE_FIRST;
				return;
			}

			if (pause_frames)
			{
				for (n = 0; pause_frames[n]; n++)
				{
					if (ent->client->ps.gunframe == pause_frames[n])
					{
						if (rand()&15)
							return;
					}
				}
			}

			ent->client->ps.gunframe++;
			return;
		}
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		for (n = 0; fire_frames[n]; n++)
		{
			if (ent->client->ps.gunframe == fire_frames[n])
			{
				if (ent->client->quad_framenum > level.framenum)
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);

				ent->client->isfiring = 1; // We are firing this frame
				fire (ent);
				break;
			}
		}

		if (!fire_frames[n])
			ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == FRAME_IDLE_FIRST+1)
			ent->client->weaponstate = WEAPON_READY;

		//bat - Change the weapon right away!
		if ((ent->client->ammo_index) && 
			( ent->client->pers.inventory[ent->client->ammo_index] < ent->client->pers.weapon->quantity))
		{
			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			NoAmmoWeaponChange (ent);
		}

	}
}


/*
======================================================================

GRENADE

======================================================================
*/

#define GRENADE_TIMER		3.0f
#define GRENADE_MINSPEED	400.0f
#define GRENADE_MAXSPEED	800.0f

void weapon_grenade_fire (edict_t *ent, qboolean held)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 125;
	float	timer;
	int		speed;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	timer = ent->client->grenade_time - level.time;
	speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);
	fire_grenade2 (ent, start, forward, damage, speed, timer, radius, held);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->grenade_time = level.time + 1.0;

	if(ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
	{
		return;
	}

	if (ent->health <= 0)
		return;

	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame = FRAME_crattak1-1;
		ent->client->anim_end = FRAME_crattak3;
	}
	else
	{
		ent->client->anim_priority = ANIM_REVERSE;
		ent->s.frame = FRAME_wave08;
		ent->client->anim_end = FRAME_wave01;
	}
}

void Weapon_Grenade (edict_t *ent)
{
	if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY))
	{
		ChangeWeapon (ent);
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if (ent->client->pers.inventory[ent->client->ammo_index])
			{
				ent->client->ps.gunframe = 1;
				ent->client->weaponstate = WEAPON_FIRING;
				ent->client->grenade_time = 0;
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
			return;
		}

		if ((ent->client->ps.gunframe == 29) || (ent->client->ps.gunframe == 34) || (ent->client->ps.gunframe == 39) || (ent->client->ps.gunframe == 48))
		{
			if (rand()&15)
				return;
		}

		if (++ent->client->ps.gunframe > 48)
			ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		if (ent->client->ps.gunframe == 5)
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);

		if (ent->client->ps.gunframe == 11)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
				ent->client->weapon_sound = gi.soundindex("weapons/hgrenc1b.wav");
			}

			// they waited too long, detonate it in their hand
			if (!ent->client->grenade_blew_up && level.time >= ent->client->grenade_time)
			{
				ent->client->weapon_sound = 0;
				weapon_grenade_fire (ent, true);
				ent->client->grenade_blew_up = true;
			}

			if (ent->client->buttons & BUTTON_ATTACK)
				return;

			if (ent->client->grenade_blew_up)
			{
				if (level.time >= ent->client->grenade_time)
				{
					ent->client->ps.gunframe = 15;
					ent->client->grenade_blew_up = false;
				}
				else
				{
					return;
				}
			}
		}

		if (ent->client->ps.gunframe == 12)
		{
			ent->client->weapon_sound = 0;
			weapon_grenade_fire (ent, false);
			ent->client->grenade_time = 0;
		}

		if ((ent->client->ps.gunframe == 15) && (level.time < ent->client->grenade_time))
			return;

		ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == 16)
		{
			ent->client->grenade_time = 0;
			ent->client->weaponstate = WEAPON_READY;
		}
	}
}

/*
======================================================================

GRENADE LAUNCHER

======================================================================
*/

void weapon_grenadelauncher_fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 120;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

#ifdef WEAP_BALANCE_OK	

	if ((int)ctfflags->value & CTF_WEAP_BALANCE)
	{
		radius*=1.5;
		damage-=10;
	}
#endif


	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_grenade (ent, start, forward, damage, 600, 2.5, radius);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_GrenadeLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_grenadelauncher_fire);
}

/*
======================================================================

ROCKET

======================================================================
*/

void Weapon_RocketLauncher_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	damage = 100 + (int)(random() * 20.0);

#ifdef WEAP_BALANCE_OK
	if ((int)ctfflags->value & CTF_WEAP_BALANCE)
	{
		radius_damage = 75; //SURT was 120
		damage_radius = 240; //LM_JORM was 360 //SURT was 120
	}
	else
	{
		radius_damage = 120;
		damage_radius = 120;
	}
#else
	radius_damage = 120;
	damage_radius = 120;
#endif	
	
	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

#ifdef WEAP_BALANCE_OK	
	if ((int)ctfflags->value & CTF_WEAP_BALANCE)
		fire_rocket (ent, start, forward, damage, 750, damage_radius, radius_damage); //SURT 750 was 550 then 650 by id
	else
		fire_rocket (ent, start, forward, damage, 650, damage_radius, radius_damage); //SURT 
#else
	fire_rocket (ent, start, forward, damage, 650, damage_radius, radius_damage); //SURT 
#endif

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_RocketLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_RocketLauncher_Fire);
}


/*
======================================================================

BLASTER / HYPERBLASTER

======================================================================
*/

void Blaster_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	if (is_quad)
		damage *= 4;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_blaster (ent, start, forward, damage, 1000, effect, hyper);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	if (hyper)
		gi.WriteByte (MZ_HYPERBLASTER | is_silenced);
	else
		gi.WriteByte (MZ_BLASTER | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);
}


void Weapon_Blaster_Fire (edict_t *ent)
{
	int		damage;

	if (deathmatch->value)
		damage = 15;
	else
		damage = 10;
	Blaster_Fire (ent, vec3_origin, damage, false, EF_BLASTER);

#ifdef WEAP_BALANCE_OK	
	if ((int)ctfflags->value & CTF_WEAP_BALANCE)
	{
		if (ent->client->ps.gunframe == 5) // First frame
			ent->client->ps.gunframe+=2;
		else
		{
			ent->client->ps.gunframe++;
		}
	}
	else
	{
		ent->client->ps.gunframe++;
	}
#else
	ent->client->ps.gunframe++;
#endif

}

void Weapon_Blaster (edict_t *ent)
{
	static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Blaster_Fire);
}


void Weapon_HyperBlaster_Fire (edict_t *ent)
{
	float	rotation;
	vec3_t	offset;
	int		effect;
	int		damage;

	ent->client->weapon_sound = gi.soundindex("weapons/hyprbl1a.wav");

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
	}
	else
	{
		if (! ent->client->pers.inventory[ent->client->ammo_index] )
		{
			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			NoAmmoWeaponChange (ent);
		}
		else
		{
			rotation = (ent->client->ps.gunframe - 5.0) * 2.0 * M_PI / 6.0;
			offset[0] = -4 * sin(rotation);
			offset[1] = 0;
			offset[2] = 4 * cos(rotation);

			if ((ent->client->ps.gunframe == 6) || (ent->client->ps.gunframe == 9))
				effect = EF_HYPERBLASTER;
			else
				effect = 0;
			if (deathmatch->value)
				damage = 15;
			else
				damage = 20;

#ifdef WEAP_BALANCE_OK	
			if ((int)ctfflags->value & CTF_WEAP_BALANCE) //surt, a little less damage
				damage = 12; //these things come out pretty fast
#endif

			Blaster_Fire (ent, offset, damage, true, effect);
			if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
				ent->client->pers.inventory[ent->client->ammo_index]--;

			ent->client->anim_priority = ANIM_ATTACK;
			if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->s.frame = FRAME_crattak1 - 1;
				ent->client->anim_end = FRAME_crattak9;
			}
			else
			{
				ent->s.frame = FRAME_attack1 - 1;
				ent->client->anim_end = FRAME_attack8;
			}
		}

		ent->client->ps.gunframe++;
		if (ent->client->ps.gunframe == 12 && ent->client->pers.inventory[ent->client->ammo_index])
			ent->client->ps.gunframe = 6;
	}

	if (ent->client->ps.gunframe == 12)
	{
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/hyprbd1a.wav"), 1, ATTN_NORM, 0);
		ent->client->weapon_sound = 0;
	}

}

void Weapon_HyperBlaster (edict_t *ent)
{
	static int	pause_frames[]	= {0};
	static int	fire_frames[]	= {6, 7, 8, 9, 10, 11, 0};

	Weapon_Generic (ent, 5, 20, 49, 53, pause_frames, fire_frames, Weapon_HyperBlaster_Fire);
}

/*
======================================================================

MACHINEGUN / CHAINGUN

======================================================================
*/

void Machinegun_Fire (edict_t *ent)
{
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 8;
	int			kick = 2;
	vec3_t		offset;

#ifdef WEAP_BALANCE_OK	
	if ((int)ctfflags->value & CTF_WEAP_BALANCE)
	{
		damage = 9; // 9/8 = 12.5% additional damage per bullet
		//we'll lose accuracy for our mod however (extra spread)
	}
#endif

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->ps.gunframe == 5)
		ent->client->ps.gunframe = 4;
	else
		ent->client->ps.gunframe = 5;

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 6;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	for (i=1 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}
	ent->client->kick_origin[0] = crandom() * 0.35;
	ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;

	// raise the gun as it is firing
	if (!deathmatch->value)
	{
		ent->client->machinegun_shots++;
		if (ent->client->machinegun_shots > 9)
			ent->client->machinegun_shots = 9;
	}

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

#ifdef WEAP_BALANCE_OK	
	if ((int)ctfflags->value & CTF_WEAP_BALANCE) //loss of accuracy
		fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD + 100, DEFAULT_BULLET_VSPREAD + 100, MOD_MACHINEGUN);
	else //id original code
		fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);
#else
	fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);
#endif

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_attack8;
	}
}

void Weapon_Machinegun (edict_t *ent)
{
	static int	pause_frames[]	= {23, 45, 0};
	static int	fire_frames[]	= {4, 5, 0};

	Weapon_Generic (ent, 3, 5, 45, 49, pause_frames, fire_frames, Machinegun_Fire);
}

void Chaingun_Fire (edict_t *ent)
{
	int			i;
	int			shots;
	vec3_t		start;
	vec3_t		forward, right, up;
	float		r, u;
	vec3_t		offset;
	int			damage;
	int			kick = 2;

	if (deathmatch->value)
		damage = 6;
	else
		damage = 8;

	if (ent->client->ps.gunframe == 5)
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnu1a.wav"), 1, ATTN_IDLE, 0);

	if ((ent->client->ps.gunframe == 14) && !(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe = 32;
		ent->client->weapon_sound = 0;
		return;
	}
	else if ((ent->client->ps.gunframe == 21) && (ent->client->buttons & BUTTON_ATTACK)
		&& ent->client->pers.inventory[ent->client->ammo_index])
	{
		ent->client->ps.gunframe = 15;
	}
	else
	{
		ent->client->ps.gunframe++;
	}

	if (ent->client->ps.gunframe == 22)
	{
		ent->client->weapon_sound = 0;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnd1a.wav"), 1, ATTN_IDLE, 0);
	}
	else
	{
		ent->client->weapon_sound = gi.soundindex("weapons/chngnl1a.wav");
	}

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - (ent->client->ps.gunframe & 1);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - (ent->client->ps.gunframe & 1);
		ent->client->anim_end = FRAME_attack8;
	}

	if (ent->client->ps.gunframe <= 9)
		shots = 1;
	else if (ent->client->ps.gunframe <= 14)
	{
		if (ent->client->buttons & BUTTON_ATTACK)
			shots = 2;
		else
			shots = 1;
	}
	else
		shots = 3;

	if (ent->client->pers.inventory[ent->client->ammo_index] < shots)
		shots = ent->client->pers.inventory[ent->client->ammo_index];

	if (!shots)
	{
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	for (i=0 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}

	for (i=0 ; i<shots ; i++)
	{
		// get start / end positions
		AngleVectors (ent->client->v_angle, forward, right, up);
		r = 7 + crandom()*4;
		u = crandom()*4;
		VectorSet(offset, 0, r, u + ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

		fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_CHAINGUN);
	}

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte ((MZ_CHAINGUN1 + shots - 1) | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= shots;
}


void Weapon_Chaingun (edict_t *ent)
{
	static int	pause_frames[]	= {38, 43, 51, 61, 0};
	static int	fire_frames[]	= {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0};

	Weapon_Generic (ent, 4, 31, 61, 64, pause_frames, fire_frames, Chaingun_Fire);
}


/*
======================================================================

SHOTGUN / SUPERSHOTGUN

======================================================================
*/

void weapon_shotgun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = 4;
	int			kick = 8;
	int			count = 0; //surt weapon balance

	if (ent->client->ps.gunframe == 9)
	{
		ent->client->ps.gunframe++;
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

#ifdef WEAP_BALANCE_OK	
	if ((int)ctfflags->value & CTF_WEAP_BALANCE) //surt
	{
		damage+=1; //slightly more damage, except with quad
		count = 2; //more pellets 
	}
#endif

	if (deathmatch->value)
		fire_shotgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_DEATHMATCH_SHOTGUN_COUNT + count, MOD_SHOTGUN); //surt balance
	else
		fire_shotgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_SHOTGUN_COUNT + count, MOD_SHOTGUN); //surt balance

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_SHOTGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void fire_fieldgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count)
{
	int		i;
	trace_t	tr;
	vec3_t	end, from;
	vec3_t	mins, maxs;
	float		*v;
	edict_t	*target;

	v = tv(-32,-32,-32);
	VectorCopy (v, mins);
	v = tv(32,32,32);
	VectorCopy (v, maxs);

	if ((level.framenum % 3) && self->client->hooklength)
	{
		return;
	}

	//for (i = 0; i < count; i++)
	//	fire_lead (self, start, aimdir, damage, kick, TE_SHOTGUN, hspread, vspread);

	//fire_shotgun (self, start, aimdir, damage, kick, hspread, vspread, count);
	// send gun puff / flash
	if (self->client->hooklength < 640)
	{
		if (self->client->hooklength < 96)
			self->client->hooklength = 96;
		else
			self->client->hooklength += 32;
	}

	VectorMA (start, self->client->hooklength, aimdir, end);
	VectorCopy (start, from);

	target = self;
	while (target)
	{
		tr = gi.trace (from, mins, maxs, end, self, MASK_SHOT);

		if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))
			target = tr.ent;
		else
			target = NULL;

		if (target && (target != self) && (target->takedamage))
			T_Damage (target, self, self, aimdir, tr.endpos, tr.plane.normal, 2, 5, 0, MOD_SHOTGUN);

		VectorCopy (tr.endpos, from);
	}

	for (i=96; i < self->client->hooklength; i+=32)
	{
		VectorMA (start, i, aimdir, end);
		
		tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);
		//T_RadiusDamage(self, self, 10, self, 32);

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_ROCKET_EXPLOSION);
		gi.WritePosition (tr.endpos);
		gi.multicast (self->s.origin, MULTICAST_PHS);
	}
}


void weapon_fieldgun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = 4;
	int			kick = 8;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

#ifdef WEAP_BALANCE_OK	
	if ((int)ctfflags->value & CTF_WEAP_BALANCE) //surt
		damage+=1; //slightly more damage, except with quad
#endif

	if (ent->client->buttons & BUTTON_ATTACK)
	{
		if (deathmatch->value)
			fire_fieldgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_DEATHMATCH_SHOTGUN_COUNT);
		else
			fire_fieldgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_SHOTGUN_COUNT);

		// send muzzle flash
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_SHOTGUN | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

		//ent->client->ps.gunframe++;
		//PlayerNoise(ent, start, PNOISE_WEAPON);

		//ent->client->pers.inventory[ent->client->ammo_index] -= ent->client->pers.weapon->quantity;
	}
	else
	{
		ent->client->ps.gunframe++;
		ent->client->hooklength = 0;
	}
}


void Weapon_Shotgun (edict_t *ent)
{
	static int	pause_frames[]	= {22, 28, 34, 0};
	static int	fire_frames[]	= {8, 9, 0};

	Weapon_Generic (ent, 7, 18, 36, 39, pause_frames, fire_frames, weapon_shotgun_fire);
}


void weapon_supershotgun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	vec3_t		v;
	int			damage = 6;
	int			kick = 12;
	int			count = 0; //for damage balance

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

#ifdef WEAP_BALANCE_OK	
	if ((int)ctfflags->value & CTF_WEAP_BALANCE) //surt
	{
		count = 12;
		damage-=3; //slightly less damage, except with quad
	}
#endif

	v[PITCH] = ent->client->v_angle[PITCH];
	v[YAW]   = ent->client->v_angle[YAW] - 5;
	v[ROLL]  = ent->client->v_angle[ROLL];
	AngleVectors (v, forward, NULL, NULL);
	fire_shotgun (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2 + count/2, MOD_SSHOTGUN); //surt, balance
	v[YAW]   = ent->client->v_angle[YAW] + 5;
	AngleVectors (v, forward, NULL, NULL);
	fire_shotgun (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2 + count/2, MOD_SSHOTGUN); //surt balance

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_SSHOTGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 2;
}

void Weapon_SuperShotgun (edict_t *ent)
{
	static int	pause_frames[]	= {29, 42, 57, 0};
	static int	fire_frames[]	= {7, 0};

	Weapon_Generic (ent, 6, 17, 57, 61, pause_frames, fire_frames, weapon_supershotgun_fire);
}



/*
======================================================================

RAILGUN

======================================================================
*/

void weapon_railgun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;

	if(matchstate == MATCH_RAILGUN_INPLAY)
	{
	 	damage = 5000;
	 	kick = 5000;
	}
	else if (deathmatch->value)
	{	
#ifdef WEAP_BALANCE_OK	
		// normal damage is too extreme in dm
		if ((int)ctfflags->value & CTF_WEAP_BALANCE)
		{
			damage = 82; //SURT was 100, we have a little formula to compensate later
			kick = 125; //make it a little less irritating to use
		}
		else
		{
			damage = 100;
			kick = 200;
		}
#else
		damage = 100;
		kick = 200;
#endif

	}
	else
	{
		damage = 150;
		kick = 250;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_rail (ent, start, forward, damage, kick);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_RAILGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}


void Weapon_Railgun (edict_t *ent)
{
	static int	pause_frames[]	= {56, 0};
	static int	fire_frames[]	= {4, 0};

	Weapon_Generic (ent, 3, 18, 56, 61, pause_frames, fire_frames, weapon_railgun_fire);
}


/*
======================================================================

BFG10K

======================================================================
*/

void weapon_bfg_fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius = 1000;
	
#ifdef WEAP_BALANCE_OK	
	if ((int)ctfflags->value & CTF_WEAP_BALANCE)
		damage_radius = 1200; //SURT was 1000
#endif

	if (deathmatch->value)
		damage = 200;
	else
		damage = 500;

#ifdef WEAP_BALANCE_OK	
	if ((int)ctfflags->value & CTF_WEAP_BALANCE)
		damage = 180; //SURT was 200
#endif

	if (ent->client->ps.gunframe == 9)
	{
		// send muzzle flash
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_BFG | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

		ent->client->ps.gunframe++;

		PlayerNoise(ent, start, PNOISE_WEAPON);
		return;
	}

	// cells can go down during windup (from power armor hits), so
	// check again and abort firing if we don't have enough now
	if (ent->client->pers.inventory[ent->client->ammo_index] < 50)
	{
		ent->client->ps.gunframe++;
		return;
	}

	if (is_quad)
		damage *= 4;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);

	// make a big pitch kick with an inverse fall
	ent->client->v_dmg_pitch = -40;
	ent->client->v_dmg_roll = crandom()*8;
	ent->client->v_dmg_time = level.time + DAMAGE_TIME;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

#ifdef WEAP_BALANCE_OK	
	if ((int)ctfflags->value & CTF_WEAP_BALANCE)
		fire_bfg (ent, start, forward, damage, 180, damage_radius); //SURT 180 was 400
	else
		fire_bfg (ent, start, forward, damage, 400, damage_radius); //SURT
#else
	fire_bfg (ent, start, forward, damage, 400, damage_radius); //SURT
#endif

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 50;
}

void Weapon_BFG (edict_t *ent)
{
	static int	pause_frames[]	= {39, 45, 50, 55, 0};
	static int	fire_frames[]	= {9, 17, 0};

	Weapon_Generic (ent, 8, 32, 55, 58, pause_frames, fire_frames, weapon_bfg_fire);
}


//======================================================================

// CTF CODE -- LM_JORM



/*
=================
hook_touch

Touch function for the grappling hook
=================
*/
void hook_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t dest;
	if (other == self->owner)
		return; //we hit ourselves, ignore us

	if (self->hook_target && self->hook_target != other) // Already have a target... ignore this new target
		return;

	if (other &&
		(strcmp(other->classname, "bodyque") != 0) &&
		(!ctf_validateplayer(other, CTF_TEAM_ANYTEAM)) && 
		(strcmp(other->classname, "worldspawn") != 0) &&
		(strncmp(other->classname, "func", 4) != 0) &&
		(strncmp(other->classname, "info_flag", 9) != 0)
		)
	{
		ctf_hook_abort(self->owner);
		return;
	}

	//else we hit something else
//	self->s.sound = gi.soundindex ("weapons/grapple/grpull.wav");
//this works, but sounds bad

	// Abort the hook if:
	// [a] we hit the sky
	// [b] we hit a teammate
	// [c] we hit a dead guy
	if ((surf && (surf->flags & SURF_SKY)) ||
		((other->client) && (self->owner->client->ctf.teamnum == other->client->ctf.teamnum)) ||
		other->deadflag)
	{
		ctf_hook_abort(self->owner);
		return;
	}

	VectorClear (self->velocity);

	if (self->owner->client)
	{
		//PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);
		if (self->owner->client->hookstate == 1) // Have we just hit?
		{
//			gi.sound(self, CHAN_AUTO, gi.soundindex("weapons/grapple/ghit.wav"), 1, ATTN_NORM, 0);
			//gi.sound(self->owner, CHAN_AUTO, gi.soundindex("weapons/grapple/grpull.wav"), 1, ATTN_NORM, 0);
		}
		self->owner->client->hookstate = 2;
	}

	if (! ((int)ctfflags->value & CTF_NO_GRAP_DAMAGE) || (!other->client)) //surt, no damage if server says so
	{
		if (self->hook_target == other) 
		{
			if ( (level.framenum % 7) == 0 && (level.framenum != self->hook_lastframe) )
			{
				if (ctf_validateplayer(other,CTF_TEAM_ANYTEAM)) //noise for hitting players
					gi.sound(self, CHAN_AUTO, gi.soundindex("weapons/grapple/gkilling.wav"), 1, ATTN_NORM, 0);
				//every 1.5 seconds
				T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, 1, 1, DAMAGE_ENERGY, MOD_CTF_GRAPPLE); //damage, knockback
				self->hook_lastframe = level.framenum;
			}
		}
		else 
		{
			if (ctf_validateplayer(other,CTF_TEAM_ANYTEAM)) //noise for hitting players
				gi.sound(self, CHAN_AUTO, gi.soundindex("weapons/grapple/ghit.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound(self, CHAN_AUTO, gi.soundindex("weapons/grapple/ghitwall.wav"), 0.8, ATTN_NORM, 0);
			
			// Bonus damage for first hit
			T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, 8, 8, DAMAGE_ENERGY, MOD_CTF_GRAPPLE); //damage, knockback
		}
	}
	
	if (other->deadflag)
	{
		ctf_hook_abort(self->owner);
		return;
	}

		
	if (!self->hook_target)
	{
		self->hook_target = other;
		VectorSubtract(self->s.origin, self->hook_target->absmin, dest);
		VectorCopy(dest, self->hook_offset);
		self->solid = SOLID_TRIGGER;
		gi.linkentity(self);
	}

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BLASTER);
	gi.WritePosition (self->s.origin);
	if (!plane)
		gi.WriteDir (vec3_origin);
	else
		gi.WriteDir (plane->normal);
	gi.multicast (self->s.origin, MULTICAST_PVS);	
}

void Grapple_Bolt_Think(edict_t *self)
{
	if (!self->hook_target && self->owner->client->hooklength > 126)
	{
		//in flight sound
		gi.sound(self, CHAN_AUTO, gi.soundindex("weapons/grapple/gflyair.wav"), 1, ATTN_NORM, 0);
		self->nextthink = level.time + 0.4;
		self->think = Grapple_Bolt_Think;
	}
	else if (self->owner->client->hooklength > 126)
	{
		//retracting sound
		gi.sound(self, CHAN_AUTO, gi.soundindex("weapons/grapple/gpulling.wav"), 1, ATTN_NORM, 0);
		self->nextthink = level.time + 0.8;
		self->think = Grapple_Bolt_Think;
	}
	else
	{
		//no sound
		self->nextthink = 0;
		self->think = NULL;
	}
	
}

void
hook_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	ctf_hook_abort(self->owner);
}

edict_t *fire_hook (edict_t *self, vec3_t start, vec3_t dir, int speed)
{
	edict_t	*bolt;
	trace_t	tr;

	VectorNormalize (dir);

	bolt = G_Spawn();
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	bolt->s.angles[PITCH] += 90;
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	//bolt->s.effects |= effect;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex ("models/objects/ghook/tris.md2");
//	bolt->s.sound = gi.soundindex ("weapons/grapple/grfire.wav");
	bolt->owner = self;
	bolt->touch = hook_touch;
	bolt->die = hook_die;
	bolt->nextthink = level.time + 1;
	bolt->think = Grapple_Bolt_Think;
	bolt->dmg = 2;
	bolt->takedamage = DAMAGE_YES;
	bolt->health = 59;	 // after 59 damage, hook destoyed
	gi.linkentity (bolt);


	//surt the muzzle flash code also causes a shotgun noise!!!!

		// send muzzle flash
//	gi.WriteByte (svc_muzzleflash);
//	gi.WriteShort (self-g_edicts);
//	gi.WriteByte (MZ_SHOTGUN | is_silenced);
//	gi.multicast (self->s.origin, MULTICAST_PVS);

	//surt can that be avoided?

	gi.sound(self, CHAN_AUTO, gi.soundindex("weapons/grapple/grfire.wav"), 0.8, ATTN_NORM, 0);
//	gi.dprintf("Played grapple sound.\n");

//	bolt->s.sound = gi.soundindex ("weapons/grapple/grfire.wav");
//surt the above works, but sounds bad
	/*
	if (self->client)
		check_dodge (self, bolt->s.origin, dir, speed);
	*/

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, self, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
	return bolt;
}	


// Ent is the owner
void Draw_Hook (edict_t *ent, vec3_t start, vec3_t end)
{
	vec3_t	dir, mins, maxs;
	vec3_t	offset;
	float		*v;

	v = tv(-15,-15,-15);
	_VectorCopy (v, mins);
	v = tv(15,15,15);
	_VectorCopy (v, maxs);

	VectorSubtract(end, start, dir);
	VectorSet(offset, 0, 0, 0);

	// Only display the grapple line if it isn't short
	if (VectorLength(dir) > 64)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_GRAPPLE_CABLE);
		gi.WriteShort (ent - g_edicts);
		gi.WritePosition (start);
		gi.WritePosition (end);
		gi.WritePosition (offset);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}	
}


void Weapon_Hook_Fire (edict_t *ent)
{
	vec3_t	offset, mins, maxs, start, forward, right, dir;
	float		*v;
	int speed;
	vec3_t	dest;

	v = tv(-15,-15,-15);
	_VectorCopy (v, mins);
	v = tv(15,15,15);
	_VectorCopy (v, maxs);

	ent->client->isfiring = 0; // We are only "firing" when we start

	if (ent->client->hookstate == 0)
	{
		VectorCopy(ent->client->v_angle, ent->client->hookangle);
	}

	// Set out ending point to our starting point
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	
	switch (ent->client->hookstate)
	{
	case 0: // Starting
		VectorScale (forward, -2, ent->client->kick_origin);
		ent->client->kick_angles[0] = -1;
		ent->client->hookstate++;
		ent->client->isfiring = 1;

		ent->client->hook = fire_hook (ent, start, forward, GRAPPLE_FIRE_HOOK_SPEED);

		//-bat.  Let's see this right away!
		Draw_Hook(ent, start, ent->client->hook->s.origin);

	case 1: // Moving out
		if (ent->client->hook)
			Draw_Hook(ent, start, ent->client->hook->s.origin);
		break;
	case 2: // Pulling us to the hook
		if (ent->client->hook)
		{
			if (ent->client->hook->hook_target)
			{
//				if ( 
//					(!ent->client->hook->hook_target->s.origin[0] == 0) && 
//					(!ent->client->hook->hook_target->s.origin[1] == 0) && 
//					(!ent->client->hook->hook_target->s.origin[2] == 0)
//					)
//						VectorCopy(ent->client->hook->hook_target->s.origin, ent->client->hook->s.origin);
				//surt test code
				VectorAdd(ent->client->hook->hook_target->absmin, ent->client->hook->hook_offset, dest);
				VectorCopy(dest, ent->client->hook->s.origin);

			}
			Draw_Hook(ent, start, ent->client->hook->s.origin);
			VectorSubtract(ent->client->hook->s.origin, start, dir);
			speed = VectorLength(dir);

			if (!ent->client->hooklength)
				ent->client->hooklength = speed;

			//sprintf(message, "Speed %i, length %i\n", speed, ent->client->hooklength);
			//ctf_SafePrint(ent, PRINT_HIGH, message);

			ent->client->hooklength = speed;
			VectorNormalize(dir);

			if (speed > 120)
			{
#ifdef WEAP_BALANCE_OK	
				if ((int)ctfflags->value & CTF_WEAP_BALANCE)
					VectorScale(dir, GRAPPLE_PULL_BALANCED_SPEED, dir);
				else
					VectorScale(dir, GRAPPLE_PULL_SPEED, dir);
#else
	   			VectorScale(dir, GRAPPLE_PULL_SPEED, dir);
#endif
				SV_AddGravity (ent); // Add gravity
			}
			else if (speed > 100)
			{
				VectorScale(dir, speed*5, dir);
			}
			else if (speed > 80)
			{
				VectorScale(dir, speed*4, dir);
			}
			else if (speed > 40)
			{
				VectorScale(dir, speed*3, dir);
			}
			else if (speed > 20)
			{
				VectorScale(dir, speed*2, dir);
			}
			else if (speed > 10)
			{
				VectorScale(dir, speed*1, dir);
			}

			VectorCopy (dir, ent->velocity);
			VectorCopy (ent->velocity, ent->client->oldvelocity); // This prevents falling damage

			//ctf_SafePrint(ent, PRINT_HIGH, "Hook in state 2\n");
		}
		else
			ent->client->hookstate = 0;
		break;
	default: //bug
		ctf_hook_abort(ent);
		break;
	}
}

void Weapon_Hook (edict_t *ent)
{
	static int	pause_frames[]	= {14, 18, 26, 30, 0};
	static int	fire_frames[]	= {8, 9, 10, 11,0};

	//if (ent->client->weaponstate == WEAPON_READY)
	//		ent->client->hookstate = 0;
	
	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		// Speed up activation
		ent->client->ps.gunframe+=1;
	}

	if (ent->client->newweapon && ent->client->weaponstate != WEAPON_DROPPING)
	{
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = 36; //FRAME_DEACTIVATE_FIRST;
		return;
	}
	
	if ( !((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK))
	{
		ctf_hook_abort(ent);
	}
	
	Weapon_Generic (ent, 9, 13, 34, 38, pause_frames, fire_frames, Weapon_Hook_Fire);

}
// END CTF CODE

/*	SKWiD MOD
======================================================================

Plasma Rifle

======================================================================
*/

void weapon_plasma_fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;

	// if outa ammo, don't fire
	if (ent->client->pers.inventory[ent->client->ammo_index] < 1) {
		ent->client->ps.gunframe++;

		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex(PLASMA_SOUND_EMPTY), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		
		NoAmmoWeaponChange (ent);
		return;
	}

	if (ent->client->ps.gunframe == 4) {
		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorScale (forward, -2, ent->client->kick_origin);
		
		// fire weapon
		VectorSet(offset, 8, 8, ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

		if( ent->client->plasma_mode ) {
		gi.sound( ent,CHAN_WEAPON, gi.soundindex(PLASMA_SOUND_FIRE1), 1,
		          ATTN_NORM,0 );
		fire_plasma (ent, start, forward, 1);
		} else {
		gi.sound( ent,CHAN_WEAPON, gi.soundindex(PLASMA_SOUND_FIRE2), 1,
		          ATTN_NORM,0 );
		fire_plasma (ent, start, forward, 0);
		}

		if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
			ent->client->pers.inventory[ent->client->ammo_index] -= 1;

		// make a big pitch kick with an inverse fall
		ent->client->v_dmg_pitch = -2;
		ent->client->v_dmg_roll = crandom()*2;
		ent->client->v_dmg_time = level.time + DAMAGE_TIME;

	}


	//-bat Silence??
	// send muzzle flash
	//gi.WriteByte (svc_muzzleflash);
	//gi.WriteShort (ent-g_edicts);
	//gi.WriteByte (MZ_ROCKET | is_silenced);
	//gi.multicast (ent->s.origin, MULTICAST_PVS);


	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_Plasma (edict_t *ent)
{
	static int	pause_frames[]	= {16, 46, 0};
	static int	fire_frames[]	= {4, 5, 0};
	//Weapon_PLASMA_Generic (ent, 3, 5, 46, 51, pause_frames, fire_frames, weapon_plasma_fire);
	//-bat make the time to fire next shot longer.
	//Weapon_PLASMA_Generic (ent, 3, 8, 46, 51, pause_frames, fire_frames, weapon_plasma_fire);
	Weapon_PLASMA_Generic (ent, 3, 11, 46, 51, pause_frames, fire_frames, weapon_plasma_fire);
}

// END
