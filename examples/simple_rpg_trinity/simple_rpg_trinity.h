
/*
** an example of using oms, and the trinity pattern with a 'simple rpg' object model
** more on the trinity pattern:
** https://docs.google.com/document/d/1TgCIzKuxiraa2sHC7QDrSHGwDspUE6C9OUN4Pjoihcw/edit?usp=sharing
*/

#ifndef SIMPLE_RPG_H
#define SIMPLE_RPG_H

#include <string>
#include <vector>
#include <map>
#include "../../oms.h"

class Engine;
class WorldState;
class World;
class CharacterTemplate;
class CharacterState;
class Character;
class HeroState;
class Hero;
class ItemTemplate;
class ItemState;
class Item;

//function decl.
void* instantiation_provider(oms::context* ctx, const std::string& type);
void save(WorldState* worldState);
WorldState* load(void);

//Factory Functions
Engine* createEngine();
World* createWorld(Engine* engine, WorldState* s);
Hero* createHero(Engine* engine, HeroState* s);

class Engine{
public:
	World* world;
	std::map<std::string,CharacterTemplate*> characterTemplates;
	Engine();
	virtual ~Engine();
	CharacterTemplate* getCharacterTemplate(const std::string& id);
};

class WorldState{
public:
	bool initialized;
	HeroState* heroState;
	WorldState();
	virtual ~WorldState();
	static void write(oms::context* ctx, WorldState* o);
	static void read(oms::context* ctx, WorldState* o);
};

class World{
public:
	Engine* engine;
	WorldState* s;
	Hero* hero;
	std::vector<Character*> npcs;
	World(Engine* engine, WorldState* s);
	virtual ~World();
};

class CharacterTemplate{
public:
	CharacterTemplate();
	virtual ~CharacterTemplate();

};

class CharacterState{
public:
	std::vector<ItemState*> inventoryItemsState;
	CharacterState();
	virtual ~CharacterState();
	virtual std::string getTypeName(void);
	static void write(oms::context* ctx, CharacterState* o);
	static void read(oms::context* ctx, CharacterState* o);
};

class Character{
public:
	CharacterTemplate* t;
	CharacterState* s;
	std::vector<Item*> inventoryItems;
	Character(CharacterTemplate* t, CharacterState* s);
	virtual ~Character();
};

class HeroState: public CharacterState{
public:
	HeroState();
	virtual ~HeroState();
	std::string getTypeName(void);
	int xp;
	std::string name;
	ItemState* leftHandItemState;
	static void write(oms::context* ctx, HeroState* o);
	static void read(oms::context* ctx, HeroState* o);
};

class Hero: public Character{
public:
	Item* leftHandItem;
	Hero(CharacterTemplate* t, HeroState* s);
	virtual ~Hero();
};

class ItemTemplate{
public:
	ItemTemplate();
	virtual ~ItemTemplate();
};

class ItemState{
public:
	ItemState();
	virtual ~ItemState();
	static void write(oms::context* ctx, ItemState* o);
	static void read(oms::context* ctx, ItemState* o);
};


class Item{
public:
	ItemTemplate* t;
	ItemState* s;
	Item(ItemTemplate* t, ItemState* s);
	virtual ~Item();
};


#endif /* SIMPLE_RPG_H */