#include "simple_rpg.h"
#include <iostream>
#include <fstream>
#include <sstream>

int main(void){
	//should only need one shared env per application
	oms::environment env;

	//declare object info, with linkage to readers, writers and creators
	oms::declare_object_info(&env,"World",(oms::read_fn)World::read, (oms::write_fn)World::write, (oms::create_fn)World::create);
	oms::declare_object_info(&env,"Character",(oms::read_fn)Character::read, (oms::write_fn)Character::write, (oms::create_fn)Character::create);
	oms::declare_object_info(&env,"Hero",(oms::read_fn)Hero::read, (oms::write_fn)Hero::write, (oms::create_fn)Hero::create);
	oms::declare_object_info(&env,"Item",(oms::read_fn)Item::read, (oms::write_fn)Item::write, (oms::create_fn)Item::create);

	//create world
	World* world=createWorld();

	//save object model
	oms::util::write_to_file(&env, "file.bin", world, "World");

	//reload object model
	World* newWorld=(World*)oms::util::read_from_file(&env, "file.bin");

	//sanity check
	std::string s1=oms::util::write_to_string(&env, world, "World");
	std::string s2=oms::util::write_to_string(&env, newWorld, "World");

	std::cout << "sanity check " << (s1.compare(s2)==0 ? "PASS" : "FAIL!") << std::endl;

	delete world; world=0;
	delete newWorld; newWorld=0;
	return 0;
}

//create the world
World* createWorld(){
	World* world=new World();

	//check if this is our first run (new game)
	if(!world->initialized){
		//set up some default data as needed
	}
	world->initialized=true;//mark initialized

	//create our hero with get or create pattern
	Hero* hero=new Hero();

	Item* item1=new Item();
	hero->leftHandItem=item1;
	hero->inventoryItems.push_back(item1);

	world->hero=hero;

	return world;
}

bool World::read(oms::context* ctx, const std::string& prop_name, World* o){
	std::cout << "World::read called" << std::endl;
	if(prop_name=="initialized"){
		o->initialized=oms::read_boolean(ctx);
		return true;
	}else if(prop_name=="hero"){
		o->hero=(Hero*)oms::read_object(ctx);
		return true;
	}
	return false;
}

void World::write(oms::context* ctx, World* o){
	std::cout << "World::write called." << std::endl;

	oms::write_property(ctx, "initialized");
	oms::write_boolean(ctx, o->initialized);

	oms::write_property(ctx, "hero");
	oms::write_object(ctx, o->hero, o->hero->getTypeName());

	oms::write_property(ctx, "writtenButNeverRead");
	oms::write_boolean(ctx, false);
}

void* World::create(oms::context* ctx){
	return new World();
}

World::World(){
	hero=0;
}

World::~World(){

}

Character::Character(){

}

Character::~Character(){

}

std::string Character::getTypeName(){
	return "Character";
}

bool Character::read(oms::context* ctx, const std::string& prop_name, Character* o){
	std::cout << "Character::read called." << std::endl;
	if(prop_name=="inventoryItemsZero"){
		Item* it=(Item*)oms::read_object(ctx);
		o->inventoryItems.push_back(it);
		return true;
	}
	return false;
}

void Character::write(oms::context* ctx, Character* o){
	std::cout << "Character::write called." << std::endl;
	oms::write_property(ctx,"inventoryItemsZero");
	oms::write_object(ctx, o->inventoryItems[0], "Item");

}

void* Character::create(oms::context* ctx){
	return new Character();
}

Hero::Hero():Character(){
	xp=100;
}

Hero::~Hero(){

}

std::string Hero::getTypeName(){
	return "Hero";
}

bool Hero::read(oms::context* ctx, const std::string& prop_name, Hero* o){
	//call to super, if true, propegate true
	if(Character::read(ctx, prop_name, o))return true;
	std::cout << "Hero::read called." << std::endl;
	if(prop_name=="name"){
		o->name=oms::read_string(ctx);
		return true;
	}else if(prop_name=="xp"){
		o->xp=oms::read_integer(ctx);
		return true;
	}else if(prop_name=="leftHandItem"){
		o->leftHandItem=(Item*)oms::read_object(ctx);
		return true;
	}
	return false;
}

void Hero::write(oms::context* ctx, Hero* o){
	Character::write(ctx, o);//call to super
	std::cout << "Hero::write called." << std::endl;
	oms::write_property(ctx,"xp");
	oms::write_integer(ctx,o->xp);
	oms::write_property(ctx,"name");
	oms::write_string(ctx,o->name);
	oms::write_property(ctx,"leftHandItem");
	oms::write_object(ctx, o->leftHandItem, "Item");
}

void* Hero::create(oms::context* ctx){
	return new Hero();
}

Item::Item(){

}

Item::~Item(){

}

bool Item::read(oms::context* ctx, const std::string& prop_name, Item* o){
	std::cout << "Item::read called." << std::endl;
}

void Item::write(oms::context* ctx, Item* o){
	std::cout << "Item::write called." << std::endl;
}

void* Item::create(oms::context* ctx){
	return new Item();
}
