#include "simple_rpg_trinity.h"
#include <iostream>
#include <fstream>
#include <sstream>

int main(void){
	//create runtime model and state model
	Engine* engine=createEngine();
	WorldState* worldState=new WorldState();
	engine->world=createWorld(engine, worldState);

	//save object model state
	save(worldState, "test1.bin");

	std::cout << std::endl << std::endl; //gimme some space

	//reload object model state
	WorldState* newWorldState=load("test1.bin");

	save(newWorldState, "test2.bin");

	delete engine;
	engine=0;
	return 0;
}

void* instantiation_provider(oms::context* ctx, const std::string& type){
	std::cout << "providing type '" << type << "'" << std::endl;
	if(type=="WorldState")return new WorldState();
	if(type=="CharacterState")return new CharacterState();
	if(type=="HeroState")return new HeroState();
	if(type=="ItemState")return new ItemState();
	std::cout << "type '" << type << "' not found!" << std::endl;
	return 0;
}

void save(WorldState* worldState, const std::string& file){
	std::fstream s;
	s.open(file.c_str(),std::ios::out | std::ios::binary | std::ios::trunc);
	oms::context* ctx=oms::open_context(&s,0);
	std::cout << "writing worldstate" << std::endl;
	oms::write_object(ctx, worldState, "WorldState", (oms::write_fn)WorldState::write);
	oms::close_context(ctx);
	ctx=0;
}

WorldState* load(const std::string& file){
	std::fstream s;
	s.open(file.c_str(),std::ios::in | std::ios::binary);
	oms::context* ctx=oms::open_context(&s,(oms::inst_fn)instantiation_provider);
	std::cout << "reading worldstate" << std::endl;
	WorldState* worldState=0;
	if(oms::check_type(ctx, oms::type_object)){
		worldState = (WorldState*)oms::read_object(ctx, (oms::read_fn)WorldState::read);
	}
	oms::close_context(ctx);
	ctx=0;
	return worldState;
}



//create the engine environment
//todo: this function should be fed data about static resources to load or they may be hard-coded
Engine* createEngine(){
	Engine* e=new Engine();

	//load in or statically create templates here
	CharacterTemplate* ct;
	ct=new CharacterTemplate();
	e->characterTemplates["hero"]=ct;

	return e;
}

//create the world instance (an instance of a game, new or loaded)
//todo: this function should be fed data for staging the world, or it may be hard-coded
World* createWorld(Engine* engine, WorldState* s){
	World* world=new World(engine, s);

	//check if this is our first run (new game)
	if(!s->initialized){
		//set up some default data as needed
	}
	s->initialized=true;//mark initialized

	//create our hero with get or create pattern
	HeroState* hs=s->heroState;
	if(!hs){
		s->heroState=new HeroState();
		hs=s->heroState;
	}
	world->hero=createHero(engine, hs);

	//create NPCs and other world objects here

	return world;
}

Hero* createHero(Engine* engine, HeroState* s){
	//we assume a template id of 'hero'
	CharacterTemplate* t=engine->getCharacterTemplate("hero");
	if(!t)return 0;//no hero template no hero
	Hero* h=new Hero(t, s);

	//create some inventory
	ItemTemplate* itt1=new ItemTemplate();
	ItemState* its1=new ItemState();
	Item* item1=new Item(itt1,its1);
	h->leftHandItem=item1;
	((HeroState*)h->s)->leftHandItemState=item1->s;

	h->inventoryItems.push_back(item1);
	h->s->inventoryItemsState.push_back(item1->s);


	return h;
}


Engine::Engine(){

}

Engine::~Engine(){
	if(world)delete world;
	world=0;

	std::map<std::string, CharacterTemplate*>::iterator it1=characterTemplates.begin();
	while(it1!=characterTemplates.end()){
		delete it1->second;
		++it1;
	}
	characterTemplates.clear();
}

//standard template retrival example, using null '0' for signal of non-existance or removal
CharacterTemplate* Engine::getCharacterTemplate(const std::string& id){
	std::map<std::string,CharacterTemplate*>::iterator it=characterTemplates.find(id);
	if(it==characterTemplates.end())return 0;
	return it->second;
}

WorldState::WorldState(){
	initialized=false;
	heroState=0;
}

WorldState::~WorldState(){

}

void WorldState::write(oms::context* ctx, WorldState* o){
	std::cout << "WorldState::write called." << std::endl;

	oms::write_property(ctx, "initialized");
	oms::write_boolean(ctx, o->initialized);

	oms::write_property(ctx, "heroState");
	oms::write_object(ctx, o->heroState, o->heroState->getTypeName(), (oms::write_fn)HeroState::write);

	oms::write_property(ctx, "writtenButNeverRead");
	oms::write_boolean(ctx, false);
}

void WorldState::read(oms::context* ctx, WorldState* o){
	std::cout << "WorldState::read called." << std::endl;

	if(oms::check_property(ctx, "initialized", oms::type_boolean)){
		o->initialized=oms::read_boolean(ctx);
	}

	if(oms::check_property(ctx, "heroState", oms::type_object)){
		o->heroState=(HeroState*)oms::read_object(ctx,(oms::read_fn)HeroState::read);
	}
}

World::World(Engine* engine, WorldState* s){
	this->engine=engine;
	this->s=s;
	hero=0;
}

World::~World(){
	engine=0;//not owned, no delete
}


CharacterTemplate::CharacterTemplate(){

}

CharacterTemplate::~CharacterTemplate(){

}

CharacterState::CharacterState(){

}

CharacterState::~CharacterState(){

}

std::string CharacterState::getTypeName(){
	return "CharacterState";
}

void CharacterState::write(oms::context* ctx, CharacterState* o){
	std::cout << "CharacterState::write called." << std::endl;
	oms::write_property(ctx,"inventoryItemsStateZero");
	oms::write_object(ctx, o->inventoryItemsState[0], "ItemState", (oms::write_fn)ItemState::write);

}

void CharacterState::read(oms::context* ctx, CharacterState* o){
	std::cout << "CharacterState::read called." << std::endl;
	if(oms::check_property(ctx,"inventoryItemsStateZero", oms::type_object)){
		ItemState* its=(ItemState*)oms::read_object(ctx, (oms::read_fn)ItemState::read);
		o->inventoryItemsState.push_back(its);
	}

}

Character::Character(CharacterTemplate* t, CharacterState* s){
	this->t=t;
	this->s=s;
}

Character::~Character(){

}


HeroState::HeroState():CharacterState(){
	xp=100;
}

HeroState::~HeroState(){

}

std::string HeroState::getTypeName(){
	return "HeroState";
}

void HeroState::write(oms::context* ctx, HeroState* o){
	CharacterState::write(ctx, o);//call to super
	std::cout << "HeroState::write called." << std::endl;
	oms::write_property(ctx,"xp");
	oms::write_integer(ctx,o->xp);
	oms::write_property(ctx,"name");
	oms::write_string(ctx,o->name);
	oms::write_property(ctx,"leftHandItemState");
	oms::write_object(ctx, o->leftHandItemState, "ItemState", (oms::write_fn)ItemState::write);
}

void HeroState::read(oms::context* ctx, HeroState* o){
	CharacterState::read(ctx, o);//call to super
	std::cout << "HeroState::read called." << std::endl;

	if(oms::check_property(ctx,"imNewHere",oms::type_integer)){
			o->xp=oms::read_integer(ctx);
	}

	if(oms::check_property(ctx,"name",oms::type_string)){
		o->name=oms::read_string(ctx);
	}

	if(oms::check_property(ctx,"xp",oms::type_integer)){
		o->xp=oms::read_integer(ctx);
	}

	if(oms::check_property(ctx,"leftHandItemState",oms::type_object)){
		o->leftHandItemState=(ItemState*)oms::read_object(ctx,(oms::read_fn)ItemState::read);
	}
}

Hero::Hero(CharacterTemplate* t, HeroState* s):Character(t,s){

}

Hero::~Hero(){

}

ItemTemplate::ItemTemplate(){

}

ItemTemplate::~ItemTemplate(){

}

ItemState::ItemState(){

}

ItemState::~ItemState(){

}

void ItemState::write(oms::context* ctx, ItemState* o){
	std::cout << "ItemState::write called." << std::endl;
}

void ItemState::read(oms::context* ctx, ItemState* o){
	std::cout << "ItemState::read called." << std::endl;
}

Item::Item(ItemTemplate* t, ItemState* s){
	this->t=t;
	this->s=s;
}

Item::~Item(){

}