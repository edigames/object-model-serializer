
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
	static bool read(oms::context* ctx, const std::string& prop_name, World* o);
	static void write(oms::context* ctx, World* o);
	static void* create(oms::context* ctx);
};

class Character{
public:
	std::vector<Item*> inventoryItems;
	Character();
	virtual ~Character();
	virtual std::string getTypeName(void);
	static bool read(oms::context* ctx, const std::string& prop_name, Character* o);
	static void write(oms::context* ctx, Character* o);
	static void* create(oms::context* ctx);
};

class Hero: public Character{
public:
	int xp;
	std::string name;
	Item* leftHandItem;
	Hero();
	virtual ~Hero();
	std::string getTypeName(void);
	static bool read(oms::context* ctx, const std::string& prop_name, Hero* o);
	static void write(oms::context* ctx, Hero* o);
	static void* create(oms::context* ctx);
};

class Item{
public:
	Item();
	virtual ~Item();
	static bool read(oms::context* ctx, const std::string& prop_name, Item* o);
	static void write(oms::context* ctx, Item* o);
	static void* create(oms::context* ctx);
};


#endif /* SIMPLE_RPG_H */