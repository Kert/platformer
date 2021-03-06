#include "state.h"
#include "config.h"
#include "input.h"
#include "physics.h"
#include "sound.h"
#include "utils.h"

extern std::vector<Machinery*> machinery;

const double SHOOTING_ANIM_DURATION = SecToTicks(0.4);
CreatureState* CreatureState::HandleInput(int input, int type)
{
	// if creature == player do
	Player *p = (Player*)cr;
	switch(type)
	{
		case 0: // press
		{
			switch(input)
			{
				case BIND_FIRE:
				{
					if(p->IsOnlyAbility(ABILITY_ROCK))
					{
						if(!p->pickedBlock)
						{
							p->pickedBlock = PickBlock(p, p->direction);
							if(p->pickedBlock)
								break;
						}
						else
						{
							ProcessShot(WEAPON_BLOCK, *cr);
							p->pickedBlock = nullptr;
							break;
						}
					}
					if(!p->HasAbilities())
						break;
					WEAPONS weapon = cr->GetWeapon();
					if(weapon != WEAPON_NONE)
					{
						if(p->shottime == 0)
						{
							p->shottime = p->fireDelay[weapon];

							switch(weapon)
							{
								case WEAPON_GRENADE:
									if(p->ammo[weapon])
									{
										p->ammo[weapon]--;
										ProcessShot(weapon, *cr);
									}
									break;
								case WEAPON_LIGHTNING: case WEAPON_EMP: case WEAPON_ICETRIPLE:
									ProcessShot(weapon, *cr);
									break;
								case WEAPON_FIREBALL:
									if(p->ammo[weapon])
									{
										p->ammo[weapon]--;
										ProcessShot(weapon, *cr);
										if(p->IsUltraAbility(ABILITY_FIRE))
										{
											p->charging = true;
											p->charge_time = SecToTicks(0.7);
										}
									}
									break;
								case WEAPON_AIRGUST:
									if(p->ammo[weapon])
									{
										p->ammo[weapon]--;
										ProcessShot(weapon, *cr);
									}
									break;
							}							
							p->sprite->shootingAnimTimer = SHOOTING_ANIM_DURATION;
						}
						break;
					}
					break;
				}
				case BIND_SWITCH:
					//if(!p->hasState(STATE_SHOOTING))
					//	p->SwitchWeapon();
					break;
			}
			break;
		}
		case 1: // hold
		{
			switch(input)
			{
				case BIND_FIRE:
					/*if (pl->shottime == 0)
					{
						if (pl->weapon == WEAPON_MGUN)
							pl->setState(STATE_SHOOTING);
					}*/
					//if (pl->hasState(STATE_SHOOTING))
					//{
					//	if (pl->ammo[pl->weapon] && (pl->weapon != WEAPON_GRENADE))
					//	{
					//		pl->ammo[pl->weapon]--;
					//		ProcessShot(pl->weapon, *pl);
					//		pl->removeState(STATE_SHOOTING);
					//		pl->shottime = pl->fireDelay[pl->weapon];
					//		if (pl->weapon >= 2)
					//			pl->shotLocked = true; // don't let player move while firing fire
					//	}
					//}
					break;
			}
			break;
		}
		case 2: // unpress
		{
			switch(input)
			{
				case BIND_FIRE:
					if(cr->GetWeapon() == WEAPON_FIREBALL)
					{
						if(p->IsUltraAbility(ABILITY_FIRE) && cr->charging && cr->charge_time == 0)
						{
							if(!IsInRain(*p))
							{
								ProcessShot(WEAPON_ROCKETL, *cr); // charged
								p->DisableChargedColor();
								p->sprite->shootingAnimTimer = SHOOTING_ANIM_DURATION;
							}
						}
					}
					cr->charge_time = 0;
					cr->charging = false;
					break;
			}
			break;
		}
	}
	return nullptr;
}

CreatureState* CreatureState::HandleIdle()
{
	return nullptr;
}

CreatureState::CreatureState(Creature *cr)
{
	this->cr = cr;
}

CreatureState::~CreatureState()
{

}

OnGroundState::OnGroundState(Creature *cr) : CreatureState(cr)
{
	state = CREATURE_STATES::ONGROUND;
	PrintLog(LOG_DEBUG, "Switched to ONGROUND");
	cr->doubleJumped = false;
}

CreatureState* OnGroundState::HandleInput(int input, int type)
{
	// if creature == player do
	Player *p = (Player*)cr;
	switch(type)
	{
		case 0:
		{
			switch(input)
			{
				case BIND_DOWN:
					//if (p->state->Is(CREATURE_STATES::ONGROUND))
					//	return new DuckingState();
					if(IsOnIce(*p))
						if((p->GetVelocity().x > (p->move_vel - 0.1) && p->direction) || (p->GetVelocity().x < -(p->move_vel - 0.1) && !p->direction))// && (IsBindPressed(BIND_LEFT) || IsBindPressed(BIND_RIGHT)))
							return new SlidingState(cr);
					break;
				case BIND_UP:
				{
					if(p->interactTarget > -1)
					{
						for(auto &dy : machinery)
						{
							if(dy->type == MACHINERY_TYPES::MACHINERY_BUTTON)
							{
								Button *btn = (Button*)dy;
								if(p->interactTarget == btn->pairID && btn->solid == true)
									btn->Activate();
							}
							
						}
					}
					break;
				}
				case BIND_JUMP:
					if(IsBindPressed(BIND_DOWN) && IsOnPlatform(*p))
					{
						p->SetY(p->GetY() + 3);
						if(p->attached)
						{
							p->SetY(p->GetY() + 2);
							p->onMachinery = false;
							p->Detach();
						}
					}						
					else
						return new JumpingState(cr);
					break;
				default:
					CreatureState::HandleInput(input, type);
			}
			break;
		}
		case 1: // hold
		{
			switch(input)
			{
				/*case BIND_DOWN:
					if (p->state->Is(CREATURE_STATES::ONGROUND))
						return new DuckingState();
					break;*/
				case BIND_RIGHT:
					if(IsBindPressed(BIND_DOWN) && IsOnIce(*p))
						if((p->GetVelocity().x > (p->move_vel - 0.1) && p->direction) || (p->GetVelocity().x < -(p->move_vel - 0.1) && !p->direction))// && (IsBindPressed(BIND_LEFT) || IsBindPressed(BIND_RIGHT)))
							return new SlidingState(cr);
					if(cr->status == STATUS_DYING)
						break;
					cr->SetDirection(DIRECTION_RIGHT);
					cr->Walk();
					break;
				case BIND_LEFT:
					if(IsBindPressed(BIND_DOWN) && IsOnIce(*p))
						if((p->GetVelocity().x > (p->move_vel - 0.1) && p->direction) || (p->GetVelocity().x < -(p->move_vel - 0.1) && !p->direction))// && (IsBindPressed(BIND_LEFT) || IsBindPressed(BIND_RIGHT)))
							return new SlidingState(cr);
					if(cr->status == STATUS_DYING)
						break;
					cr->SetDirection(DIRECTION_LEFT);
					cr->Walk();
					break;
				default:
					CreatureState::HandleInput(input, type);
			}
			break;
		}
		case 2: //unpress
		{
			switch(input)
			{
				case BIND_RIGHT: case BIND_LEFT:
					p->accel.x = 0;
					break;
				default:
					CreatureState::HandleInput(input, type);
			}
		}
	}
	return nullptr;
}

InAirState::InAirState(Creature *cr) : CreatureState(cr)
{
	state = CREATURE_STATES::INAIR;
	PrintLog(LOG_DEBUG, "Switched to IN AIR");
}

CreatureState* InAirState::HandleInput(int input, int type)
{
	// if creature == player do
	Player *p = (Player*)cr;
	switch(type)
	{
		case 0:
		{
			switch(input)
			{
				case BIND_JUMP:
					if(p->HasAbility(ABILITY_WIND) && !cr->doubleJumped)
					{
						cr->doubleJumped = true;
						return new JumpingState(cr);
					}
					break;
				default:
					CreatureState::HandleInput(input, type);
			}
			break;
		}
		case 1: // hold
		{
			switch(input)
			{
				/*case BIND_DOWN:
				if (p->hasState(STATE_ONGROUND))
				return new DuckingState();
				break;*/
				case BIND_RIGHT:
					if(cr->status == STATUS_DYING)
						break;
					cr->SetDirection(DIRECTION_RIGHT);
					cr->Walk();
					break;
				case BIND_LEFT:
					if(cr->status == STATUS_DYING)
						break;
					cr->SetDirection(DIRECTION_LEFT);
					cr->Walk();
					break;
				default:
					CreatureState::HandleInput(input, type);
			}
			break;
		}
		case 2: //unpress
		{
			switch(input)
			{
				case BIND_RIGHT: case BIND_LEFT:
					p->accel.x = 0;
					break;
				default:
					CreatureState::HandleInput(input, type);
			}
		}
	}
	return nullptr;
}

DuckingState::DuckingState(Creature *cr) : CreatureState(cr)
{
	state = CREATURE_STATES::DUCKING;
	PrintLog(LOG_DEBUG, "Switched to DUCKING");
	cr->ToggleDucking(true);
	cr->accel.x = 0;
}

DuckingState::~DuckingState()
{
	cr->ToggleDucking(false);
}

CreatureState* DuckingState::HandleInput(int input, int type)
{
	// if creature == player do
	Player *p = (Player*)cr;
	switch(type)
	{
		case 0: // press
		{
			switch(input)
			{
				case BIND_RIGHT:	case BIND_LEFT: case BIND_UP:
					p->ToggleDucking(false);
					return new OnGroundState(cr);
					break;
				case BIND_JUMP:
					p->ToggleDucking(false);
					return new JumpingState(cr);
					break;
				default:
					CreatureState::HandleInput(input, type);
			}
			break;
		}
		case 2:
		{
			switch(input)
			{
				case BIND_DOWN:
					return new OnGroundState(cr);
					break;
				default:
					CreatureState::HandleInput(input, type);
			}
		}
	}
	return nullptr;
}

JumpingState::JumpingState(Creature *cr) : CreatureState(cr)
{
	state = CREATURE_STATES::JUMPING;
	PrintLog(LOG_DEBUG, "Switched to JUMPING");
	cr->jumptime = 14;
	cr->SetVelocity(cr->GetVelocity().x, -1.75);
	cr->Detach();
	cr->onMachinery = false;
	//Play the jump sound
	if(!cr->IsAI())
		Sound::PlaySfx("jump");
	if(cr->doubleJumped)
	{
		Effect *eff = new Effect(EFFECT_DOUBLE_JUMP);
		eff->SetPos(cr->GetX(), cr->GetY() + eff->hitbox->GetRect().h);
	}		
}

JumpingState::~JumpingState()
{
	cr->jumptime = 0;
	//pl->accel.y = 0;
}

CreatureState* JumpingState::HandleInput(int input, int type)
{
	// if creature == player do
	Player *p = (Player*)cr;
	switch(type)
	{
		case 0:
		{
			switch(input)
			{
				default:
					CreatureState::HandleInput(input, type);
			}
			break;
		}
		case 1:
		{
			switch(input)
			{
				case BIND_JUMP:
				{
					if(p->jumptime <= 0)
					{
						return new InAirState(cr);
					}
					break;
				}
				case BIND_RIGHT:
					cr->SetDirection(DIRECTION_RIGHT);
					cr->Walk();
					break;
				case BIND_LEFT:
					cr->SetDirection(DIRECTION_LEFT);
					cr->Walk();
					break;
				default:
					CreatureState::HandleInput(input, type);
			}
			break;
		}
		case 2:
		{
			switch(input)
			{
				case BIND_JUMP:
					return new InAirState(cr);
				case BIND_LEFT: case BIND_RIGHT:
					p->accel.x = 0;
				default:
					CreatureState::HandleInput(input, type);
			}
		}
	}
	return nullptr;
}

HangingState::HangingState(Creature *cr) : CreatureState(cr)
{
	state = CREATURE_STATES::HANGING;
	PrintLog(LOG_DEBUG, "Switched to HANGING");
	cr->accel.y = 0;
	cr->accel.x = 0;
	cr->SetVelocity(0, 0);
	cr->doubleJumped = false;
}

HangingState::~HangingState()
{

}

CreatureState* HangingState::HandleInput(int input, int type)
{
	// if creature == player do
	Player *p = (Player*)cr;
	switch(type)
	{
		case 0: // press
		{
			switch(input)
			{
				case BIND_RIGHT:
					p->direction = DIRECTION_RIGHT;
					break;
				case BIND_LEFT:
					p->direction = DIRECTION_LEFT;
					break;
				case BIND_DOWN:
					p->lefthook = true;
					p->Detach();
					return new InAirState(cr);
				case BIND_JUMP:
					p->lefthook = true;
					p->Detach();
					if(IsBindPressed(BIND_DOWN))
						return new InAirState(cr);
					else
						return new JumpingState(cr);
				default:
					CreatureState::HandleInput(input, type);
			}
			break;
		}
		case 1:
		{
			switch(input)
			{
				default:
					CreatureState::HandleInput(input, type);
			}
		}
		case 2:
		{
			switch(input)
			{
				default:
					CreatureState::HandleInput(input, type);
			}
		}
	}
	return nullptr;
}

SlidingState::SlidingState(Creature *cr) : CreatureState(cr)
{
	state = CREATURE_STATES::SLIDING;
	PrintLog(LOG_INFO, "slide ON");
	cr->hitbox->SetSize(10, 10);
	cr->accel.x = 0;
}

SlidingState::~SlidingState()
{
	PrintLog(LOG_INFO, "slide OFF");
	cr->hitbox->SetSize(10, 30);
	cr->hitbox->SetPos(cr->GetX(), cr->GetY() - 30);
	cr->accel.x = 0;
}

CreatureState* SlidingState::HandleInput(int input, int type)
{
	// if creature == player do
	Player *p = (Player*)cr;
	switch(type)
	{
		case 0:
		{
			switch(input)
			{
				case BIND_JUMP:
				{
					if(HasCeilingRightAbove(*p))
						break;
					return new JumpingState(cr);
					break;
				}
				case BIND_LEFT:
				{
					if(HasCeilingRightAbove(*p))
					{
						if(p->GetVelocity().x == 0)
						{
							p->SetVelocity(-0.5, 0);
							p->direction = DIRECTION_LEFT;
							CreatureState::HandleInput(input, type);
						}
						break;
					}
					if(p->GetVelocity().x > 0)
						return new OnGroundState(cr);
					break;
				}
				case BIND_RIGHT:
				{
					if(HasCeilingRightAbove(*p))
					{
						if(p->GetVelocity().x == 0)
						{
							p->SetVelocity(0.5, 0);
							p->direction = DIRECTION_RIGHT;
							CreatureState::HandleInput(input, type);
						}
						break;
					}
					if(p->GetVelocity().x < 0)
						return new OnGroundState(cr);
					break;
				}
				default:
					CreatureState::HandleInput(input, type);
			}
		}
		case 1: // hold
		{
			switch(input)
			{
				case BIND_LEFT: case BIND_RIGHT: case BIND_DOWN:
				{
					if(HasCeilingRightAbove(*p))
						break;
					if(!IsOnIce(*p) || p->GetVelocity().x == 0)
						return new OnGroundState(cr);
					break;
				}
				default:
					CreatureState::HandleInput(input, type);
			}
			break;
		}
		case 2: // unpress
		{
			switch(input)
			{
				case BIND_DOWN:
					if(HasCeilingRightAbove(*p))
						break;
					return new OnGroundState(cr);
					break;
				default:
					CreatureState::HandleInput(input, type);
			}
			break;
		}
		case 3: // not holding anything specific
		{
			if(HasCeilingRightAbove(*p))
				break;
			if(!IsBindPressed(BIND_DOWN))
				return new OnGroundState(cr);
		}
	}
	return nullptr;
}

CreatureState* SlidingState::HandleIdle()
{
	return this->HandleInput(NULL, 3);
}