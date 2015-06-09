#include "simple_rpg.h"
#include <iostream>
#include <fstream>
#include <sstream>

int main(void){
	//create world
	World* world=createWorld();

	//save object model
	oms::util::write_to_file("file.bin",world,"World",(oms::write_fn)World::write);

	std::cout << std::endl << std::endl; //gimme some space

	//reload object model
	World* newWorld=(World*)oms::util::read_from_file("file.bin","World",(oms::read_fn)World::read,(oms::inst_fn)instantiation_provider);

	delete world;
	world=0;
	return 0;
}

void* instantiation_provider(oms::context* ctx, const std::string& type){
	std::cout << "providing type '" << type << "'" << std::endl;
	if(type=="World")return new World();
	if(type=="Character")return new Character();
	if(type=="Hero")return new Hero();
	if(type=="Item")return new Item();
	std::cout << "type '" << type << "' not found!" << std::endl;
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

void World::write(oms::context* ctx, World* o){
	std::cout << "World::write called." << std::endl;

	oms::write_property(ctx, "initialized");
	oms::write_boolean(ctx, o->initialized);

	oms::write_property(ctx, "hero");
	oms::write_object(ctx, o->hero, o->hero->getTypeName(), (oms::write_fn)Hero::write);

	oms::write_property(ctx, "writtenButNeverRead");
	oms::write_boolean(ctx, false);
}

void World::read(oms::context* ctx, World* o){
	std::cout << "World::read called." << std::endl;

	if(oms::check_property(ctx, "initialized", oms::type_boolean)){
		o->initialized=oms::read_boolean(ctx);
	}

	if(oms::check_property(ctx, "hero", oms::type_object)){
		o->hero=(Hero*)oms::read_object(ctx,(oms::read_fn)Hero::read);
	}
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

void Character::write(oms::context* ctx, Character* o){
	std::cout << "Character::write called." << std::endl;
	oms::write_property(ctx,"inventoryItemsZero");
	oms::write_object(ctx, o->inventoryItems[0], "Item", (oms::write_fn)Item::write);

}

void Character::read(oms::context* ctx, Character* o){
	std::cout << "Character::read called." << std::endl;
	if(oms::check_property(ctx,"inventoryItemsZero", oms::type_object)){
		Item* it=(Item*)oms::read_object(ctx, (oms::read_fn)Item::read);
		o->inventoryItems.push_back(it);
	}

}

Hero::Hero():Character(){
	xp=100;
}

Hero::~Hero(){

}

std::string Hero::getTypeName(){
	return "Hero";
}

void Hero::write(oms::context* ctx, Hero* o){
	Character::write(ctx, o);//call to super
	std::cout << "Hero::write called." << std::endl;
	oms::write_property(ctx,"xp");
	oms::write_integer(ctx,o->xp);
	oms::write_property(ctx,"name");
	oms::write_string(ctx,o->name);
	oms::write_property(ctx,"leftHandItem");
	oms::write_object(ctx, o->leftHandItem, "Item", (oms::write_fn)Item::write);
}

void Hero::read(oms::context* ctx, Hero* o){
	Character::read(ctx, o);//call to super
	std::cout << "Hero::read called." << std::endl;

	if(oms::check_property(ctx,"imNewHere",oms::type_integer)){
			o->xp=oms::read_integer(ctx);
	}

	if(oms::check_property(ctx,"name",oms::type_string)){
		o->name=oms::read_string(ctx);
	}

	if(oms::check_property(ctx,"xp",oms::type_integer)){
		o->xp=oms::read_integer(ctx);
	}

	if(oms::check_property(ctx,"leftHandItem",oms::type_object)){
		o->leftHandItem=(Item*)oms::read_object(ctx,(oms::read_fn)Item::read);
	}
}

Item::Item(){

}

Item::~Item(){

}

void Item::write(oms::context* ctx, Item* o){
	std::cout << "Item::write called." << std::endl;
}

void Item::read(oms::context* ctx, Item* o){
	std::cout << "Item::read called." << std::endl;
}

