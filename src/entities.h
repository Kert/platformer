#ifndef _entities_h_
#define _entities_h_ 

#include <SDL.h>
#include <algorithm>
#include <vector>
#include "ai.h"
#include "globals.h"
#include "sprite.h"

class CreatureState;
class Player;
class Machinery;

struct Path
{
	std::vector<SDL_Point> points;
	bool loopable = false;
};

struct Velocity
{
	double x;
	double y;
};

struct Accel
{
	double x;
	double y;
};

struct DamageSource
{
	int id;
	double immunity;
};

class Hitbox
{
	private:
		double x;
		double y;
		double h;
		double w;

	public:
		Hitbox(double x, double y, double h, double w);
		bool HasCollision(Hitbox *hitbox);
		SDL_Rect GetRect();
		PrecisionRect GetPRect();
		void SetPos(double x, double y);
		void SetRect(SDL_Rect rect);
		void SetSize(int w, int h);
};

class Entity
{
	private:
		double x, y; // used for physics engine processing
	public:
		Hitbox *hitbox;
		Sprite *sprite;
		int entityID;
		int status; // dying state can be useful on static entities
		double statusTimer; // Status timer time in game ticks
		DIRECTIONS direction;
		double yNew;
		double xNew;
		bool blinkDamaged;

		bool attachToScreen;
		int attachScreenX;
		int attachScreenY;

		bool REMOVE_ME = false;

	public:
		double GetX() { return x; };
		double GetY() { return y; };
		void SetX(double x) { SetPos(x, GetY()); };
		void SetY(double y) { SetPos(GetX(), y); };
		void GetPos(int &x, int &y);
		void GetPos(double &x, double &y);
		void SetPos(int x, int y);
		void SetPos(double x, double y);
		void SwitchDirection();
		void SetDirection(DIRECTIONS direction);
		int AssignEntityID(int vectorID);
		double GetDistanceToEntity(Entity *e);
		double GetXDistanceToEntity(Entity *e);
		double GetYDistanceToEntity(Entity *e);
};

class StaticEntity : public Entity
{

};

class DynamicEntity : public Entity
{
	private:
		Velocity velocity;

	public:
		Accel accel;
		bool ignoreWorld = 0;
		bool ignoreGravity = 0;
		double gravityMultiplier = 1;
		DynamicEntity *attached;
		double attX;
		double attY;
	public:
		~DynamicEntity();
		Velocity GetVelocity();
		bool isMoving(bool onlyX);
		void SetVelocity(Velocity vel);
		void SetVelocity(double x, double y);
		void AttachTo(DynamicEntity *e);
		void Detach();
		virtual void Remove();
};

class Tripod : public DynamicEntity
{
	public:
		Tripod();
		~Tripod();
};

class Bullet : public DynamicEntity
{
	public:
		DynamicEntity *owner;
		WEAPONS origin;
		double lifetime; // Time in ticks
		bool piercing;
		int damage;

	public:
		Bullet();
		~Bullet();
		Bullet(WEAPONS firedFrom, Creature &shooter);
		void Remove();
};

class Lightning : public DynamicEntity
{
	public:
		DynamicEntity *owner;
		Velocity vel;
		double lifetime; // Time in ticks
		bool piercing;
		SDL_Texture *tex;

	public:
		~Lightning();
		Lightning(DynamicEntity &shooter);
		void Remove();
};

struct LightningBranch
{
	int offset;
	int going;
	int forkEscape; // how long to run away from other branches
	bool forkPending;
	int timeUntilBranchable;
	int lifetime;
};

class Creature : public DynamicEntity
{
	public:
		int health;
		double jumptime;
		double jump_accel;
		double term_vel;
		double move_vel;
		double shottime;
		double charge_time;
		bool nearhookplatform;
		bool lefthook;
		int interactTarget;
		BaseAI *AI = nullptr;
		WEAPONS weapon;
		// still leaks apparently?
		std::vector<DamageSource> hitFrom;
		CreatureState* state;
		bool shotLocked;
		bool charging;
		bool onMachinery;
		bool doubleJumped = false; // air ability specific
		Bullet *pickedBlock = nullptr;
		struct
		{
			bool left;
			bool right;
			bool top;
			bool bottom;
		} pushedFrom;

	public:
		Creature();
		Creature(std::string type);
		~Creature();
		void ProcessBulletHit(Bullet *b);
		void TakeDamage(int damage);
		void SetInvulnerability(double sec);
		void SetStun(double sec);
		// TODO: Make it player specific
		void ToggleDucking(bool enable);
		void Walk();
		void Walk(DIRECTIONS direction);
		void MoveUp();
		void MoveDown();
		void Shoot();
		void Die();
		bool IsAI();
		// specify one of AI classes as type, like AI_Chaser
		template<typename T>
		void SetAI()
		{
			if(this->IsAI())
				delete this->AI;
			this->AI = new T((Creature*)this);
		};
		void SetState(CREATURE_STATES state);
		void SetState(CreatureState *newState);
		void HandleInput(int input, int type);
		void HandleStateIdle();
		void Crush();
		void TouchSpikes();
		WEAPONS GetWeapon();
};

struct CreatureData
{
	bool blinkDamaged = false;
	bool ignoreWorld = 0;
	bool ignoreGravity = 0;
	double gravityMultiplier = 1;
	int health;
	double term_vel;
	double move_vel;
	WEAPONS weapon;
	std::string graphicsName;
};

struct EntityGraphicsData
{
	std::string textureFile;
	Hitbox hitbox = { 0,0,0,0 };
	Sprite sprite = {};
	// spriteRect, offsets
	// and animations
	// offsetX, offsetY, frames, interval, fps, loopType
};

void ReadCreatureData();
void ReadPlatformData();

class Player : public Creature
{
	public:
		bool ownedWeapons[NUMWEAPONS];
		int ammo[NUMWEAPONS];
		double fireDelay[NUMWEAPONS]; // Time in ms
		bool chargedColored = false;
		int idleTimer;
		ABILITIES abilities[MAX_ABILITIES];
	public:
		Player();
		~Player();
		void SwitchWeapon();
		void SwitchWeapon(WEAPONS newWeap);
		void ResetWeapons();
		void GiveWeapon(WEAPONS weap);
		bool CanMoveWhileFiring();
		void ToggleChargedColor();
		void DisableChargedColor();
		bool HasAbility(ABILITIES a);
		bool HasAbilities();
		void SetAbilities(ABILITIES first, ABILITIES second = ABILITY_NONE);
		ABILITIES GetAbility(int index);
		bool IsUltraAbility(ABILITIES a);
		bool IsOnlyAbility(ABILITIES a);
};

class Pickup : public StaticEntity
{
	public:
		PICKUP_TYPES type;

	public:
		Pickup();
		Pickup(PICKUP_TYPES spawnType);
		void OnPickup();
		void Remove();
		~Pickup();
};

class Effect : public StaticEntity
{
	public:
		EFFECT_TYPES type;

	public:
		~Effect();
		Effect(EFFECT_TYPES type);
		void Remove();
};

enum MACHINERY_TYPES
{
	MACHINERY_PLATFORM,
	MACHINERY_DOOR,
	MACHINERY_BUTTON,
	MACHINERY_LAVAFLOOR
};

class Machinery : public DynamicEntity
{
	public:
		MACHINERY_TYPES type;
		bool enabled;
		bool solid;
		bool destructable;
		SDL_Rect default_pos;
	public:
		void Activate();
		~Machinery();
};

class Button : public Machinery
{
	public:
		int pairID;
	public:
		Button(int x, int y, int doorID);
		void Activate();
		void Remove();
		~Button();
};

class Door : public Machinery
{
	public:
		Button *leftButton;
		Button *rightButton;
		int pairID;
	public:
		Door(int x, int y, bool spawnButtons);
		void Open();
		void Close();
		void Remove();
		~Door();
};

class Platform : public Machinery
{
	public:
		bool standable = true;
		bool hookable = false;
		double speed;
		int pathID = -1;
		int currentPathPoint = 0;
	public:
		Platform(int x, int y, std::string platformType, int pathID, double speed);
		void Remove();
		~Platform();
};

class Lava_Floor : public Machinery
{
	public:
		Lava_Floor(int x, int y);

		void Activate();
		void Remove();
		~Lava_Floor();
};

enum ENTITY_LISTS
{
	LIST_BULLETS,
	LIST_CREATURES,
	LIST_MACHINERY,
	LIST_PICKUPS,
	LIST_EFFECTS
};

void TestMemory();
void DeleteAllEntities();
void EntityCleanup();

#endif
