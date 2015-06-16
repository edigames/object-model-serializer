#include <iostream>
#include <fstream>
#include <sstream>
#include "../../oms.h"

class ClassB{
public:
	int poodle;
	ClassB(){
		poodle=0;
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
	std::vector<int> int_array;
	std::string my_str;
	ClassB* my_firstb;
	double my_num;
	ClassB* my_b;
	std::map<std::string,std::string> my_map;


	ClassA(){
		my_int=0;
		my_b=0;
		my_firstb=0;
		my_nullb=0;
		my_num=0;
		my_b=0;
	}

	virtual ~ClassA(){

	}

	static bool read(oms::context* ctx, const std::string& prop_name, ClassA* o){
		//note: purposefully not reading back, my_firstb
		//to demonstrate first reference drop out
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
		}else if(prop_name=="int_array"){
			uint32_t count=oms::read_array(ctx);
			for(uint32_t i=0;i<count;++i){
				oms::read_type(ctx, oms::type_integer);
				int j=oms::read_integer(ctx);
				o->int_array.push_back(j);
			}
			return true;
		}else if(prop_name=="my_map"){
			uint32_t count=oms::read_map(ctx);
			for(uint32_t i=0;i<count;++i){
				oms::read_type(ctx, oms::type_string);
				std::string key=oms::read_string(ctx);
				oms::read_type(ctx, oms::type_string);
				std::string value=oms::read_string(ctx);
				o->my_map[key]=value;
			}
			return true;
		}
		return false;
	}

	static void write(oms::context* ctx, ClassA* o){
		oms::write_property(ctx, "my_int");
		oms::write_integer(ctx, o->my_int);

		oms::write_property(ctx, "my_nullb");
		oms::write_object(ctx, o->my_nullb, "ClassB");

		oms::write_property(ctx, "my_firstb");
		oms::write_object(ctx, o->my_firstb, "ClassB");

		oms::write_property(ctx, "my_num");
		oms::write_number(ctx, o->my_num);

		oms::write_property(ctx, "my_b");
		oms::write_object(ctx, o->my_b, "ClassB");

		oms::write_property(ctx, "int_array");
		oms::write_array(ctx, o->int_array.size());
		std::vector<int>::iterator it=o->int_array.begin();
		while(it!=o->int_array.end()){
			int j=*it;
			oms::write_integer(ctx,j);
			++it;
		}

		oms::write_property(ctx, "my_str");
		oms::write_string(ctx, o->my_str);

		oms::write_property(ctx, "never_read");
		oms::write_string(ctx, "never read back");

		oms::write_property(ctx, "my_map");
		oms::write_map(ctx, o->my_map.size());
		std::map<std::string,std::string>::iterator it2=o->my_map.begin();
		while(it2!=o->my_map.end()){
			oms::write_string(ctx,it2->first);
			oms::write_string(ctx,it2->second);
			++it2;
		}
	}

	static void* create(oms::context* ctx){
		return new ClassA();
	}
};

ClassA* make_model_a(){
	ClassA* a=new ClassA();
	a->my_int=1983;
	a->my_num=3.141592654;
	a->my_str="hello world!";
	for(int i=0;i<6;++i){
		a->int_array.push_back(i*i);
	}
	a->my_b=new ClassB();
	a->my_b->poodle=778;
	a->my_firstb=a->my_b;
	a->my_map["larry"]="one";
	a->my_map["moe"]="two";
	a->my_map["curly"]="three";
	return a;
}

bool test_deep_copy(oms::environment* env){
	ClassA* a1=make_model_a();
	ClassA* a2=(ClassA*)oms::util::deep_copy(env, a1, "ClassA");
	return a2->my_str.compare("hello world!")==0;
}

bool test_write_to_file(oms::environment* env){
	ClassA* a=make_model_a();
	oms::util::write_to_file(env, "file1.bin", a, "ClassA");
	return true;
}

void wrap_test(const char* name, bool b){
	std::cout << name << ": " << (b ? "PASS!" : "FAIL!") << std::endl;
}

int main(void){
	oms::environment env;

	oms::declare_object_info(&env, "ClassA", (oms::read_fn)ClassA::read, (oms::write_fn)ClassA::write, ClassA::create);
	oms::declare_object_info(&env, "ClassB", (oms::read_fn)ClassB::read, (oms::write_fn)ClassB::write, ClassB::create);

	wrap_test("test_deep_copy", test_deep_copy(&env));
	wrap_test("test_write_to_file", test_write_to_file(&env));

	std::cout << "all tests complete." << std::endl;
	return 0;
}