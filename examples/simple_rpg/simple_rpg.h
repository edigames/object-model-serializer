
/*
** an example of using oms with a 'simple rpg' object model
*/

#ifndef SIMPLE_RPG_H
#define SIMPLE_RPG_H

#include <string>
#include <vector>
#include <map>
#include "../../oms.h"

class World;
class Character;
class Hero;
class Item;

//function decl.
void* instantiation_provider(oms::context* ctx, const std::string& type);
void save(World* world);
World* load(void);

//Factory Functions
World* createWorld();

class World{
public:
	bool initialized;
	Hero* hero;
	std::vector<Character*> npcs;
	World();
	virtual ~World();
	static void write(oms::context* ctx, World* o);
	static void read(oms::context* ctx, World* o);
};

class Character{
public:
	std::vector<Item*> inventoryItems;
	Character();
	virtual ~Character();
	virtual std::string getTypeName(void);
	static void write(oms::context* ctx, Character* o);
	static void read(oms::context* ctx, Character* o);
};

class Hero: public Character{
public:
	int xp;
	std::string name;
	Item* leftHandItem;
	Hero();
	virtual ~Hero();
	std::string getTypeName(void);
	static void write(oms::context* ctx, Hero* o);
	static void read(oms::context* ctx, Hero* o);
};

class Item{
public:
	Item();
	virtual ~Item();
	static void write(oms::context* ctx, Item* o);
	static void read(oms::context* ctx, Item* o);
};


#endif /* SIMPLE_RPG_H */