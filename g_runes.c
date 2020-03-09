#include "g_local.h"
#include "g_ctffunc.h"
#include "bat.h"

// RUNES
void Rune_Think (edict_t *self);

#define RUNETHINKTIME 30 //time before a rune relocates itself

static void tossruneset(edict_t *ent)
{
	ent->svflags &= ~SVF_NOCLIENT;
	ent->flags |= FL_RESPAWN;
	ent->spawnflags = DROPPED_ITEM;
	
	if (ent->model)
		gi.setmodel (ent, ent->model);
	else
		gi.setmodel (ent, ent->item->world_model);
	ent->movetype = MOVETYPE_TOSS;  
}

static void tossrune (edict_t *ent, vec3_t dir)
{	
	float		*v;
	vec3_t		temp;
	edict_t     *owner;
	trace_t	tr;
	
	VectorSet(ent->mins, -15,-15,-15);
	VectorSet(ent->maxs, 15, 15, 15);
	tossruneset(ent);
	ent->touch = Touch_Item;
	owner = ent->owner;
	ent->owner = NULL;
	ent->groundentity = NULL;
	
	VectorCopy(ent->s.origin, temp);
	
	if (!dir || !dir[0] ) // Random toss
	{
		
		v = tv(0, 0, 48);
		VectorAdd (ent->s.origin, v, ent->s.origin);
		ent->velocity[0] = -2000 + (random() * 4000);
		ent->velocity[1] = -2000 + (random() * 4000);
		ent->velocity[2] = 800 + (random() * 200);
		
	}
	else
	{
		VectorScale(dir, 64, dir);
		VectorAdd (ent->s.origin, dir, ent->s.origin);
		VectorScale(dir, 4, dir);
		VectorCopy(dir, ent->velocity);
	}
	
	tr = gi.trace (temp, ent->mins, ent->maxs, ent->s.origin, owner, MASK_SOLID);
	if (tr.fraction < 1.0 || tr.allsolid)
	{
		VectorCopy(temp, ent->s.origin);
	}
	
	ent->think = Rune_Think;
	ent->nextthink = level.time + FRAMETIME;//RUNETHINKTIME;

	// We just moved.  Remember this time
	ent->last_move_time = level.time;

	//ent->solid = SOLID_BBOX;
	ent->solid = SOLID_TRIGGER;
	gi.linkentity (ent); //always pair with changes to solid
}



edict_t *SelectRuneSpawnPoint (void)
{
	edict_t	*spot = NULL;
	int		count = 0;
	int		selection;
	
	spot = NULL;
	
	while ((spot = G_Find (spot, FOFS(classname), "item_health_small")) != NULL)
	{
		count++;
	}
	
	if (count>0)
	{
		
		selection = random() * count;
		
		if (selection < 0)
			selection = 0;
		if (selection > 20) //arbitrary, lets not wait too long
			selection = 20;
		
		spot = NULL;
		do
		{
			spot = G_Find (spot, FOFS(classname), "item_health_small");
			selection--;
		} 
		while(selection > 0);
	}
	else
	{
		while ((spot = G_Find (spot, FOFS(classname), "item_health_large")) != NULL)
		{
			count++;
		}
		if (count>0)
		{
			
			selection = random() * count;
			
			if (selection < 0)
				selection = 0;
			if (selection > 20) //arbitrary, lets not wait too long
				selection = 20;
			
			spot = NULL;
			do
			{
				spot = G_Find (spot, FOFS(classname), "item_health_large");
				selection--;
			} 
			while(selection > 0);
		}
		else
		{
			while ((spot = G_Find (spot, FOFS(classname), "item_health")) != NULL)
			{
				count++;
			}
			if (count>0)
			{
				
				selection = random() * count;
				
				if (selection < 0)
					selection = 0;
				if (selection > 20) //arbitrary, lets not wait too long
					selection = 20;
				
				spot = NULL;
				do
				{
					spot = G_Find (spot, FOFS(classname), "item_health");
					selection--;
				} 
				while(selection > 0);
			}
		}
		
	}
	
	return spot;
}

float RunesRangeFromSpot (edict_t *spot)
{
  edict_t *rune;
  float	bestrunedistance;
  vec3_t v;
  float	runedistance;
  
  
  bestrunedistance = 9999999;
  

  //-bat - This should use a function!

  rune = G_Find (spot, FOFS(classname), "damage_rune");
  if (rune)
  {
	  VectorSubtract (spot->s.origin, rune->s.origin, v);
	  runedistance = VectorLength (v);
	  
	  if (runedistance < bestrunedistance)
		  bestrunedistance = runedistance;
  }

  rune = G_Find (spot, FOFS(classname), "haste_rune");
  if (rune)
  {
	  VectorSubtract (spot->s.origin, rune->s.origin, v);
	  runedistance = VectorLength (v);
	  
	  if (runedistance < bestrunedistance)
		  bestrunedistance = runedistance;
  }

  rune = G_Find (spot, FOFS(classname), "resist_rune");
  if (rune)
  {
	  VectorSubtract (spot->s.origin, rune->s.origin, v);
	  runedistance = VectorLength (v);
	  
	  if (runedistance < bestrunedistance)
		  bestrunedistance = runedistance;
  }

  rune = G_Find (spot, FOFS(classname), "regen_rune");
  if (rune)
  {
	  VectorSubtract (spot->s.origin, rune->s.origin, v);
	  runedistance = VectorLength (v);
	  
	  if (runedistance < bestrunedistance)
		  bestrunedistance = runedistance;
  }

  
  rune = G_Find (spot, FOFS(classname), "vampire_rune");   //added by Vampire
  if (rune)                                                  
  {
	  VectorSubtract (spot->s.origin, rune->s.origin, v);
	  runedistance = VectorLength (v);
	  
	  if (runedistance < bestrunedistance)
		  bestrunedistance = runedistance;
  }



  return bestrunedistance;
}
  
/*
================
SelectFarthestDeathmatchSpawnPoint

 ================
*/
edict_t *SelectFarthestRuneSpawnPoint (void)
{
	edict_t	*bestspot;
	float	bestdistance, bestrunedistance;
	edict_t	*spot;


	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	spot = G_Find (spot, FOFS(classname), "item_health");
	while (spot)
	{
		bestrunedistance = RunesRangeFromSpot (spot);
		
		if (bestrunedistance > bestdistance)
		{
			bestspot = spot;
			bestdistance = bestrunedistance;
		}
		spot = G_Find (spot, FOFS(classname), "item_health");
	}

	if (bestspot)
	{
	    return bestspot;
	}

	spot = SelectRuneSpawnPoint();

	return spot;
}
  

void Rune_Think (edict_t *self)
{
	edict_t *spot;
	static qboolean forward = true;
	
	if (self->solid != SOLID_NOT)
	{
		switch (self->runetype)
		{
		case RUNE_DAMAGE:
			//screwy stuff to fix animation due to gimblelock
			if (forward == true)
			{
				self->s.frame++;
				if (self->s.frame >= 5)
					forward = false;
			}
			else
			{
				self->s.frame--;
				if (self->s.frame <= 0)
					forward = true;
			}
			break;
		case RUNE_HASTE:
			//screwy stuff to fix animation due to gimblelock
			switch (self->s.frame)
			{
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
				self->s.frame++;
				break;
			default:
				self->s.frame = 5;
				break;
			}
			break;
		case RUNE_RESIST:
			self->s.frame = ((self->s.frame + 1) % 15);
			break;
		case RUNE_REGEN:
			self->s.frame = ((self->s.frame + 1) % 14);
			break;
		case RUNE_VAMP:                                   //added by Vampire
			self->s.frame = ((self->s.frame + 1) % 15);   
			break;                                        
		default:
			break;
		}
	}
	self->think = Rune_Think;
	self->nextthink = level.time + FRAMETIME;
	
	// Let's reuse last_move_time
	if (self->last_move_time + RUNETHINKTIME < level.time)
	{
		//	spot = SelectRuneSpawnPoint();
		
		spot = SelectRuneSpawnPoint();
		
		if (!spot)
			spot = redflag;
		
		if (spot)
		{
			VectorCopy (spot->s.origin, self->s.origin);
			tossrune (self, 0);
		}
		self->last_move_time = level.time;
	}
}

void SpawnRune (int type)
{
	edict_t		*ent, *spot; 
	char *name = NULL, *model = NULL; //MJD Uninitialized
	int effects = 0, renderfx = 0;    //MJD Uninitialized
	
	
	//	if (!deathmatch->value)
	//		return;
	//surt single player runes support
	
	spot = SelectFarthestRuneSpawnPoint();
	if (!spot)
		spot = redflag;
	
	if (spot)
	{
		switch (type)
		{
		case RUNE_DAMAGE:
			name = "damage_rune";
			effects = 0; //EF_COLOR_SHELL;
			renderfx = 0; //RF_SHELL_RED;
			model = "models/ctf/damage/tris.md2";
			break;
			
		case RUNE_HASTE:
			name = "haste_rune";
			effects = 0;//EF_COLOR_SHELL;
			renderfx = 0;//RF_SHELL_BLUE;
			model = "models/ctf/haste/tris.md2";
			break;
			
		case RUNE_RESIST:
			name = "resist_rune";
			effects = 0;//EF_COLOR_SHELL;
			renderfx = 0;//RF_SHELL_RED | RF_SHELL_BLUE;
			model = "models/ctf/resist/tris.md2";
			break;
			
		case RUNE_REGEN:
			name = "regen_rune";
			effects = 0;//EF_COLOR_SHELL;
			renderfx = 0;//RF_SHELL_GREEN;
			model = "models/ctf/regen/tris.md2";
			break;
			
		case RUNE_VAMP:                                   //added by Vampire
			name = "vampire_rune";
			//effects = EF_TELEPORTER | EF_ANIM01;//EF_COLOR_SHELL;
			effects = EF_ANIM01;//EF_COLOR_SHELL;
			renderfx = RF_SHELL_RED;
			model = "models/ctf/resist/tris.md2";
			break;
			
		default:
			gi.dprintf("Bad rune model selected.\n");
			break;  // MJD If we hit this line, then name,
			// effects, renderfx, and model would
			// stay uninitialized, which would be
			// a bad thing, I think.
		}
		
		ent = G_Spawn();
		ent->classname = ED_NewString (name);
		ED_CallSpawn(ent);
		
		VectorCopy (spot->s.origin, ent->s.origin);
		ent->takedamage = DAMAGE_NO; //no damage on runes
		ent->dontfree = 1;
		gi.soundindex ("items/m_health.wav");
		ent->s.effects |= effects;
		ent->s.renderfx |= renderfx;
		ent->model = model;
		ent->runetype = type; // We store our rune type here
		tossrune(ent, 0);
		//		gi.dprintf("Spawned rune.\n");
	}
}

void SP_damage_rune (edict_t *self)
{
	self->model = "models/items/invulner/tris.md2";
	
	SpawnItem (self, FindItemByClassname ("damage_rune"));
}


qboolean Pickup_Rune (edict_t *ent, edict_t *other)
{
	if (!other->client->rune) // don't already have rune
	{
		// Make sure it will respawn
		ent->flags |= FL_RESPAWN;
		ent->think = NULL;
		ent->nextthink = 0;

		other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
		ent->owner = other;
		other->client->rune = ent;
		ent->solid = SOLID_NOT;
		gi.linkentity (ent); //always pair with solid changes

		ent->svflags |= SVF_NOCLIENT;
		ent->movetype = MOVETYPE_NONE;  // Don't block doors
		gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power1.wav"), 1, ATTN_NORM, 0);
		return true;
	}

	ent->touch = Touch_Item;
	ent->think = Rune_Think;
	ent->nextthink = level.time + FRAMETIME;
	
	return false;
}

void Drop_Rune_Think (edict_t *ent)
{
	ent->touch = Touch_Item;
	ent->owner = NULL;
	ent->think = Rune_Think;
	ent->nextthink = level.time + FRAMETIME;
}

extern void drop_temp_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

void Drop_Rune (edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;
	float *v;
	
	if (!item)
		return;
	
	ent->client->pers.inventory[ITEM_INDEX(item)]--;

	dropped = ent->client->rune;
	if (!dropped)
		return;
	ent->client->rune = 0;
	
	tossruneset(dropped);

	VectorCopy(ent->s.origin, dropped->s.origin);
	tossruneset(dropped);
	dropped->touch = drop_temp_touch;
	dropped->owner = ent;

	v = tv(0, 0, 48);
	VectorAdd (dropped->s.origin, v, dropped->s.origin);
	dropped->velocity[0] = -2000 + (random() * 4000);
	dropped->velocity[1] = -2000 + (random() * 4000);
	dropped->velocity[2] = 800 + (random() * 200);

	ctf_TossEnt(ent, dropped);
	
	dropped->think = Drop_Rune_Think;
	dropped->nextthink = level.time + 1;

	// We just moved.  Remember this time
	dropped->last_move_time = level.time;

	//ent->solid = SOLID_BBOX;
	dropped->solid = SOLID_TRIGGER;
	gi.linkentity (dropped); //always pair with changes to solid

	gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
	ValidateSelectedItem (ent);
}

int DamageRuneHook(edict_t *targ, edict_t *inflictor, edict_t *attacker, int damage, int knockback, int dflags)
{
	if (attacker && attacker->client && attacker->client->rune)
	{
		if (attacker->client->rune->runetype == RUNE_DAMAGE)
		{
			//damage *=2; // Double damage
			damage *= 1.75f; //-bat
		}
	}
	
	return damage;
}

int ResistRuneHook(edict_t *targ, edict_t *inflictor, edict_t *attacker, int damage, int knockback, int dflags)
{
	if (targ && targ->client && targ->client->rune)
	{
		if (targ->client->rune->runetype == RUNE_RESIST)
		{
			//damage /=2; // Half damage
			damage /= 1.75f; //-bat needs to offset damage rune.
			gi.sound(targ, CHAN_ITEM, gi.soundindex("ctf/resist.wav"), 1, ATTN_NORM, 0);
		}
	}
	
	return damage;
}

void RuneThinkHook(edict_t *ent)
{
	int old_armor_index;
	int sound = 0; // Don't play a sound unless needed.
	int	heartrate;

	if (ent && ent->client && ent->client->rune)
	{
		
		if (ent->client->rune->runetype == RUNE_REGEN)
		{
			heartrate = ent->health / 5;
			if (heartrate < 5)
				heartrate = 5;
			if (heartrate > 25)
				heartrate = 25;
			
			if (level.framenum < ent->client->regentime + heartrate)
				return;
			
			ent->client->regentime = level.framenum;
			
			if (ent->health < ent->max_health + 25)
			{
				//bat
				ent->health+=(heartrate / 3.0f);
				//ent->health+=(heartrate/4);
				if (ent->health > ent->max_health + 25)
					ent->health = ent->max_health + 25;
				sound = 1;
			}
			
			old_armor_index = ArmorIndex (ent);
			if (!old_armor_index)
			{
				ent->client->pers.inventory[ITEM_INDEX(FindItem("Jacket Armor"))] = heartrate/4;
				sound = 1;
			}
			else
			{
				if (ent->client->pers.inventory[old_armor_index] < 200)
				{
					//bat
					ent->client->pers.inventory[old_armor_index] += heartrate / 3.0f;
					//ent->client->pers.inventory[old_armor_index] += heartrate/4;
					if (ent->client->pers.inventory[old_armor_index] > 200)
						ent->client->pers.inventory[old_armor_index] = 200;
					sound = 1;
				}
			}
			if (sound)
				gi.sound(ent, CHAN_ITEM, gi.soundindex("ctf/regen.wav"), 1, ATTN_NORM, 0);
		}
	}
}


void RuneWeaponThinkHook (edict_t *ent)
{
	if (ent && ent->client && ent->client->rune)
	{
		if (ent->client->rune->runetype == RUNE_HASTE)
		{
			if (ent->client->isfiring)
				gi.sound(ent, CHAN_ITEM, gi.soundindex("player/lava1.wav"), 1, ATTN_NORM, 0);
			
			if (ent->client->ps.gunframe)
				ent->client->pers.weapon->weaponthink (ent);
		}
		if (ent->client->rune->runetype == RUNE_DAMAGE)
		{
			if (ent->client->isfiring)
				gi.sound(ent, CHAN_ITEM, gi.soundindex("ctf/strength.wav"), 1, ATTN_NORM, 0);
		}
	}
}
