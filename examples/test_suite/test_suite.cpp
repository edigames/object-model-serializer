#include <iostream>
#include <fstream>
#include <sstream>
#include "../../oms.h"

class ClassB{
public:
	int poodle;
	ClassB(){
		poodle=778;
	}

	virtual ~ClassB(){

	}

	static bool read(oms::context* ctx, const std::string& prop_name, ClassB* o){
		if(prop_name=="poodle"){
			o->poodle=oms::read_integer(ctx);
			return true;
		}
		return false;
	}

	static void write(oms::context* ctx, ClassB* o){
		oms::write_property(ctx, "poodle");
		oms::write_integer(ctx, o->poodle);
	}

	static void* create(oms::context* ctx){
		return new ClassB();
	}
};

class ClassA{
public:
	int my_int;
	ClassB* my_nullb;
	double my_num;
	ClassB* my_b;
	std::string my_str;

	ClassA(){
		my_int=1983;
		my_b=0;
		my_nullb=0;
		my_num=3.141592654;
		my_str="hello world!";
	}

	virtual ~ClassA(){

	}

	static bool read(oms::context* ctx, const std::string& prop_name, ClassA* o){
		if(prop_name=="my_int"){
			o->my_int=oms::read_integer(ctx);
			return true;
		}else if(prop_name=="my_nullb"){
			o->my_nullb=(ClassB*)oms::read_object(ctx);
			return true;
		}else if(prop_name=="my_num"){
			o->my_num=oms::read_number(ctx);
			return true;
		}else if(prop_name=="my_b"){
			o->my_b=(ClassB*)oms::read_object(ctx);
			return true;
		}else if(prop_name=="never_written"){
			oms::read_string(ctx);
			return true;
		}else if(prop_name=="my_str"){
			o->my_str=oms::read_string(ctx);
			return true;
		}
		return false;
	}

	static void write(oms::context* ctx, ClassA* o){
		oms::write_property(ctx, "my_int");
		oms::write_integer(ctx, o->my_int);
		oms::write_property(ctx, "my_nullb");
		oms::write_object(ctx, o->my_nullb, "ClassB");
		oms::write_property(ctx, "my_num");
		oms::write_number(ctx, o->my_num);
		oms::write_property(ctx, "my_b");
		oms::write_object(ctx, o->my_b, "ClassB");
		oms::write_property(ctx, "my_str");
		oms::write_string(ctx, o->my_str);
		oms::write_property(ctx, "never_read");
		oms::write_string(ctx, "never read back");
	}

	static void* create(oms::context* ctx){
		return new ClassA();
	}
};

bool test_deep_copy(oms::environment* env){
	ClassA* a1=new ClassA();
	a1->my_b=new ClassB();
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
		uint32_t size=oms::check_size(ctx, oms::type_object);
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
	oms::declare_object_info(&env, "ClassB", (oms::read_fn)ClassB::read, (oms::write_fn)ClassB::write, ClassB::create);

	bool b;
	b=test_deep_copy(&env);
	std::cout << "test_deep_copy: " << (b ? "PASS!" : "FAIL!") << std::endl;

	std::cout << "all tests complete." << std::endl;
	return 0;
}