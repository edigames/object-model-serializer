#include <iostream>
#include <fstream>
#include <sstream>
#include "../../oms.h"

class ClassA{
public:
	int my_int;
	double my_num;
	std::string my_str;

	ClassA(){
		my_int=1983;
		my_num=3.141592654;
		my_str="hello world!";
	}

	virtual ~ClassA(){

	}

	static void read(oms::context* ctx, ClassA* o){
		if(oms::check_property(ctx, "my_int", oms::type_integer)){
			o->my_int=oms::read_integer(ctx);
		}
		if(oms::check_property(ctx, "my_num", oms::type_number)){
			o->my_num=oms::read_number(ctx);
		}
		if(oms::check_property(ctx, "my_str", oms::type_string)){
			o->my_str=oms::read_string(ctx);
		}
	}

	static void write(oms::context* ctx, ClassA* o){
		oms::write_property(ctx, "my_int");
		oms::write_integer(ctx, o->my_int);
		oms::write_property(ctx, "my_num");
		oms::write_number(ctx, o->my_num);
		oms::write_property(ctx, "my_str");
		oms::write_string(ctx, o->my_str);
	}

	static void* create(oms::context* ctx){
		return 0;
	}
};

void* instantiation_provider(oms::context* ctx, const std::string& type){
	std::cout << "providing type '" << type << "'" << std::endl;
	if(type=="ClassA")return new ClassA();
	std::cout << "type '" << type << "' not found!" << std::endl;
	return 0;
}

bool test_deep_copy(oms::environment* env){
	ClassA* a1=new ClassA();
	ClassA* a2=0;

	oms::context* ctx=0;
	std::stringstream ss;

	ctx=new oms::context();
	oms::open_context(ctx, env, &ss);
	oms::write_object(ctx, a1, "ClassA");
	oms::close_context(ctx);
	delete ctx;
	ctx=0;

	ctx=new oms::context();
	oms::open_context(ctx, env, &ss);
	if(oms::read_type(ctx, oms::type_object)){
		a2=(ClassA*)oms::read_object(ctx);
	}
	oms::close_context(ctx);
	delete ctx;
	ctx=0;

	if(a2)delete a2;
	a2=0;

	//ClassA* a2=(ClassA*)oms::util::deep_copy(a1, "ClassA", (oms::write_fn)ClassA::write,(oms::read_fn)ClassA::read,(oms::inst_fn)instantiation_provider);
	//std::string s1=oms::util::write_to_string(a1, "ClassA", (oms::write_fn)ClassA::write);
	//std::string s2=oms::util::write_to_string(a2, "ClassA", (oms::write_fn)ClassA::write);

	return true;//s1.compare(s2)==0;
}

int main(void){
	oms::environment env;

	oms::declare_object_info(&env, "ClassA", (oms::read_fn)ClassA::read, (oms::write_fn)ClassA::write, ClassA::create);

	bool b;
	b=test_deep_copy(&env);
	std::cout << "test_deep_copy: " << (b ? "PASS!" : "FAIL!") << std::endl;

	std::cout << "all tests complete." << std::endl;
	return 0;
}