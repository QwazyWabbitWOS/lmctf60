/*
 * M82 Plasma Rifle Source
 *
 * Copyright (C) 1999  Team HOSTILE
 *
 * Copyright (C) 1999  LMCTF 5.0
 *
 * created by James "SWKiD" Tomaschke
 */


#include "g_local.h"
#include "plasma.h"
#include "m_player.h"
#include "bat.h"

#define FRAME_FIRE_FIRST		(FRAME_ACTIVATE_LAST + 1)
#define FRAME_IDLE_FIRST		(FRAME_FIRE_LAST + 1)
#define FRAME_DEACTIVATE_FIRST		(FRAME_IDLE_LAST + 1)

void NoAmmoWeaponChange (edict_t *ent);

//bat
#define PLASMA_INDEX	18

int quadmeister = 0;

//== plasma_reflect_touch ======================================================
// If touched a hurtable object, hurt it.  Otherwise bounce with splash damage.
//==============================================================================
void
plasma_reflect_touch
(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf) 
{
int damage;
	
	// If hit the sky, remove from world
	if (surf && (surf->flags & SURF_SKY)) {
		G_FreeEdict(self);
		return;
	}

	// Damage Decay
	// determine damage (max 3.0s * 7 = max 21 damage)
	// if time is adjusted, must also adjust the constant
	
	//bat.  This think stuff is messing me up, so I am taking it out.
	//self->dmg= 7 + (self->nextthink - level.time) * 7 * PLASMA_DAMAGE_MULTIPLIER;

	//bat
	if(quadmeister)
		damage = PLASMA_BOUNCE_DAMAGE * 4;
	else
		damage = PLASMA_BOUNCE_DAMAGE;

	if( self->owner->client )
		PlayerNoise(self->owner,self->s.origin,PNOISE_IMPACT);

	if( other->takedamage ) 
	{
		T_Damage(other, self, self->owner, self->velocity, self->s.origin, plane->normal, damage, 1, DAMAGE_ENERGY, MOD_PLASMA);
			
		///T_Damage (other, self, self->owner, self->velocity,
		 //         self->s.origin, plane->normal, self->dmg, 1,
		 //	  DAMAGE_ENERGY, MOD_PLASMA);
			  
		// play hit sound
		gi.sound( self,CHAN_BODY, gi.soundindex(PLASMA_SOUND_HIT), 1,
		          ATTN_IDLE,0 );

		self->solid=	SOLID_NOT;
		self->touch=	NULL;
		VectorMA (self->s.origin,-1*FRAMETIME,self->velocity,self->s.origin);
		VectorClear (self->velocity);

		// Run Plasma Hit Animation
		self->s.modelindex= 	gi.modelindex (PLASMA_SPRITE_HIT);
		self->s.frame= 		0;
		self->s.sound=		0;
		self->think=		G_FreeEdict;
		self->nextthink=	level.time + 0.1;
	}
	else {	// fx to do when it bounces off something
		// play reflection sound
		gi.sound( self,CHAN_BODY, gi.soundindex(PLASMA_SOUND_BOUNCE), 1,
		          ATTN_STATIC,0 );

		// Draw blue sparks
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_LASER_SPARKS);
		gi.WriteByte (32);		// ammount
		gi.WritePosition (self->s.origin);
		gi.WriteDir (plane->normal);
		gi.WriteByte (176);		// stecki's choice of id's blue
		gi.multicast (self->s.origin, MULTICAST_PVS);

		//T_RadiusDamage(self,self->owner,32,NULL,32,MOD_PLASMA);
		//-bat
		T_RadiusDamage(self,self->owner,damage,NULL, damage+PLASMA_SPLASH_RADIUS,MOD_PLASMA);
	}
}


//== plasma_spread_touch =======================================================
// If hits an entity, do some damage, otherwise do some splash damage.
//==============================================================================
void
plasma_spread_touch
(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf) 
{
int damage;

	// If hit the sky, remove from world
	if (surf && (surf->flags & SURF_SKY)) {
		G_FreeEdict(self);
		return;
	}

	// Don't collide with other plasma goops
	if (Q_stricmp(other->classname,"goop") == 0)
		return;
	
	//bat
	if(quadmeister)
		damage = PLASMA_SPREAD_DAMAGE * 4;
	else
		damage = PLASMA_SPREAD_DAMAGE;

	if( self->owner->client )
		PlayerNoise(self->owner,self->s.origin,PNOISE_IMPACT);

	// If can damage, hurt it
	if( other->takedamage )
	{
		//T_Damage (other, self, self->owner, self->velocity,
		//          self->s.origin, plane->normal, self->dmg, 1,
		//	  DAMAGE_ENERGY, MOD_PLASMA);
		T_Damage(other, self, self->owner, self->velocity, self->s.origin, plane->normal, damage, 1, DAMAGE_ENERGY, MOD_PLASMA);
	}		  
	else	// otherwise, splash damage
	{
		//-bat
		//T_RadiusDamage(self,self->owner,self->dmg,NULL, self->dmg+PLASMA_SPLASH_RADIUS,MOD_PLASMA);
		//T_RadiusDamage(self,self->owner,32,NULL,32,MOD_PLASMA);
		
		//-bat added sparks to this too.
		// Draw blue sparks
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_LASER_SPARKS);
		gi.WriteByte (32);		// ammount
		gi.WritePosition (self->s.origin);
		gi.WriteDir (plane->normal);
		gi.WriteByte (176);		// stecki's choice of id's blue
		gi.multicast (self->s.origin, MULTICAST_PVS);



		T_RadiusDamage(self,self->owner,damage,NULL, damage+PLASMA_SPLASH_RADIUS,MOD_PLASMA);
	}		  
	// play hit sound
	gi.sound( self,CHAN_BODY, gi.soundindex(PLASMA_SOUND_HIT), 1,
	          ATTN_IDLE,0 );	// idle static none

	self->solid=	SOLID_NOT;
	self->touch=	NULL;
	VectorMA (self->s.origin,-1*FRAMETIME,self->velocity,self->s.origin);
	VectorClear (self->velocity);

	// Run Plasma Hit Animation
	self->s.modelindex= 	gi.modelindex (PLASMA_SPRITE_HIT);
	self->s.frame= 		0;
	self->s.sound=		0;
	self->think=		G_FreeEdict;
	self->nextthink=	level.time + 0.1;
}


//== Spawn_Goop ================================================================
// Spawns the plasma entities, and defines values global to both weapon modes.
//==============================================================================
edict_t *
Spawn_Goop
(edict_t *ent, vec3_t start) {
	edict_t *goop = G_Spawn();

	goop->owner=	ent;
	goop->clipmask=	MASK_SHOT;
	goop->solid=	SOLID_BBOX;
	goop->svflags=	SVF_DEADMONSTER;
	
	VectorCopy (start, goop->s.origin);
	goop->classname= "goop";

	//goop->s.effects|=	EF_BLUEHYPERBLASTER | EF_ANIM_ALLFAST;
	//bat to get rid of the blue flag effect
	goop->s.effects|=	EF_IONRIPPER | EF_ANIM_ALLFAST;
	goop->s.renderfx= 	RF_TRANSLUCENT;
	goop->s.modelindex=	gi.modelindex(PLASMA_SPRITE_FLY);
	goop->s.sound= 		gi.soundindex(PLASMA_SOUND_FLYBY);

	return goop;
}


//== fire_plasma_reflect =======================================================
// Unique code to fire a bouncy plasma goob.  Uses 'MOVETYPE_REFLECT' which has
// been defined in 'g_phys.c'.
//==============================================================================
void
fire_plasma_reflect
(edict_t *self, vec3_t start, vec3_t dir) {
	self->movetype = MOVETYPE_REFLECT;	// new movetype (MOVETYPE_BOUNCE
						// without gravity and friction,
						// and does not stop projectile
						// when it hits a ground plane)

	VectorScale(dir, PLASMA_REFLECT_SPEED, self->velocity);
	VectorCopy(self->velocity, self->s.angles);	// needed for post touch

	//Not sure what this even does????
	//if (deathmatch->value)
	//	self->dmg = 15 * PLASMA_BOUNCE_DAMAGE;
	//else
	//	self->dmg = 20 * PLASMA_BOUNCE_DAMAGE;
	
	//-bat
	self->dmg = PLASMA_BOUNCE_DAMAGE;
	
	self->touch = plasma_reflect_touch;
	
	self->think = G_FreeEdict;			// change this to handle
	//self->nextthink = level.time + 3.0;		//  sprite animation?
	self->nextthink = level.time + 1.5;

	gi.linkentity(self);
}


//== fire_plasma_spread ========================================================
// Unique code to fire a spread of three bullets, each with 1/3 the damage of
// one initial bouncy bullet.
//==============================================================================
void
fire_plasma_spread
(edict_t *goop_c, vec3_t start, vec3_t dir) {
	edict_t	*goop_l = Spawn_Goop(goop_c->owner, start);
	edict_t	*goop_r = Spawn_Goop(goop_c->owner, start);
	vec3_t	angles;
	
	goop_l->movetype = MOVETYPE_FLYMISSILE;
	goop_c->movetype = MOVETYPE_FLYMISSILE;
	goop_r->movetype = MOVETYPE_FLYMISSILE;

	VectorClear (goop_l->mins);
	VectorClear (goop_l->maxs);
	VectorClear (goop_c->mins);
	VectorClear (goop_c->maxs);
	VectorClear (goop_r->mins);
	VectorClear (goop_r->maxs);

	goop_l->dmg= 1;
	goop_c->dmg= 1;
	goop_r->dmg= 1;

	//bat  this really shouldn't even matter???
	//if(is_quad)
	//{
	//	goop_l->dmg=	7 + 9 * PLASMA_SPREAD_DAMAGE * 4;
	//	goop_c->dmg=	7 + 9 * PLASMA_SPREAD_DAMAGE * 4;
	//	goop_r->dmg=	7 + 9 * PLASMA_SPREAD_DAMAGE * 4;
	//}
	//else
	//{
	//	goop_l->dmg=	7 + 9 * PLASMA_SPREAD_DAMAGE;
	//	goop_c->dmg=	7 + 9 * PLASMA_SPREAD_DAMAGE;
	//	goop_r->dmg=	7 + 9 * PLASMA_SPREAD_DAMAGE;
	//}



	// center spread, line of sight
	VectorScale(dir, PLASMA_SPREAD_SPEED, goop_c->velocity);
	vectoangles(dir,angles);

	// right spread, has 10+ in yaw
	angles[YAW] -= 10;
	AngleVectors( angles, dir, NULL, NULL );
	VectorScale(dir, PLASMA_SPREAD_SPEED, goop_r->velocity);

	// left spread, has 10- in yaw
	angles[YAW] += 20;
	AngleVectors( angles, dir, NULL, NULL );
	VectorScale(dir, PLASMA_SPREAD_SPEED, goop_l->velocity);

	goop_l->touch = plasma_spread_touch;
	goop_c->touch = plasma_spread_touch;
	goop_r->touch = plasma_spread_touch;
	
	goop_l->think = G_FreeEdict;
	goop_c->think = G_FreeEdict;
	goop_r->think = G_FreeEdict;
	goop_l->nextthink = level.time + 3.0;
	goop_c->nextthink = level.time + 3.0;
	goop_r->nextthink = level.time + 3.0;

	gi.linkentity(goop_l);
	gi.linkentity(goop_c);
	gi.linkentity(goop_r);
}


//== fire_plasma ===============================================================
// If "reflect" is > 0, it will fire a bouncy shot, else, it will fire a spread.
// "start" and "dir" are not set here, but in 'p_weapon.c', where it is setup to
// handle weapon firing.
//==============================================================================
void
fire_plasma
(edict_t *ent, vec3_t start, vec3_t dir, int reflect) {
	edict_t *goop= Spawn_Goop(ent, start);

	// give it some thickness for the bounce
	VectorSet (goop->mins, -12,-12,-12);
	VectorSet (goop->maxs, 12,12,12);

	if(reflect)
	{
		fire_plasma_reflect (goop,start,dir);
		//bat
		ent->client->pers.inventory[ent->client->ammo_index] -= (PLASMA_CELLS_PER_SHOT -1);
	}
	else
	{
		fire_plasma_spread (goop,start,dir);
		//bat
		ent->client->pers.inventory[ent->client->ammo_index] -= (PLASMA_CELLS_PER_SHOT - 1);
	}


}


//== Use_PLASMA ================================================================
// This is just a copy of 'Use_Weapon' but instead it allows us to toggle
// between the two weapon modes.  This function should be used in place of
// 'Use_Weapon' within 'g_items.c' for the plasma rifle.
//==============================================================================
void
Use_PLASMA
(edict_t *ent, gitem_t *item)
{
	int		ammo_index;
	gitem_t		*ammo_item;
///////// FIXME /////////
	// if we're already using it, switch modes
	if(item == ent->client->pers.weapon) {
		// see if you have the other weapon
			// if yes, switch
			// else return
// simple hack for demo
		ent->client->plasma_mode = (!ent->client->plasma_mode);

		if(ent->client->plasma_mode)
			gi.cprintf(ent, PRINT_HIGH, "bounce plasma\n");
		else
			gi.cprintf(ent, PRINT_HIGH, "spread plasma\n");

		return;
	}
/////////////////////////

	if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO))
	{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);

		if (!ent->client->pers.inventory[ammo_index])
		{
			gi.cprintf (ent, PRINT_HIGH, "No %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		}

		if (ent->client->pers.inventory[ammo_index] < item->quantity)
		{
			gi.cprintf (ent, PRINT_HIGH, "Not enough %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		}
	}

	// change to this weapon when down
	ent->client->newweapon = item;
}



//== Weapon_PLASMA_Generic =====================================================
// Plasma Rifle Frame Code
//==============================================================================
void Weapon_PLASMA_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent))
{
int		n;

	if(ent->client->quad_framenum > level.framenum)
		quadmeister = true;
	else
		quadmeister = false;

	// VWep animations screw up corpses
	if(ent->deadflag || ent->s.modelindex != 255)
		return;

	//bat
	ent->client->isfiring = 0;


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
			
			//-bat
			if(ent->client->plasma_mode)
				gi.cprintf(ent, PRINT_HIGH, "bounce plasma\n");
			else
				gi.cprintf(ent, PRINT_HIGH, "spread plasma\n");

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

			if ( ent->client->ps.gunframe == 35 )
			gi.sound(ent,CHAN_WEAPON,
			  gi.soundindex(PLASMA_SOUND_VENT), 1, ATTN_NORM,0 );

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
				//bat
				if(ent->client->quad_framenum > level.framenum)
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);

				
				//bat
				ent->client->isfiring = 1;
				fire (ent);
				break;
			}
		}

		if (!fire_frames[n])
			ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == FRAME_IDLE_FIRST+1)
			ent->client->weaponstate = WEAPON_READY;
	}
}



// EOF
