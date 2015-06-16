/*
** object-model-serializer
** A tiny C++ library for 'soft serialization' of object models.
** written by Raymond Jacobs
** distributed by Ethereal Darkness Interactive
** under MIT license for private and commercial works.
*/

#include "oms.h"
#include <fstream>
#include <algorithm>

void oms::open_context(oms::context* ctx, oms::environment* env, std::iostream* ios){
	ctx->env=env;
	ctx->ios=ios;
}

void oms::close_context(oms::context* ctx){
	ctx->env=0;
}

void oms::declare_object_info(oms::environment* env, const std::string& class_name, oms::read_fn r, oms::write_fn w, oms::create_fn c){
	oms::object_info td={r,w,c};
	env->oim[class_name]=td;
}

oms::object_info* oms::get_object_info(oms::environment* env, const std::string& class_name){
	std::map<std::string, object_info>::iterator it=env->oim.find(class_name);
	//note: yes... returning pointer to object_info; assume no ownership and don't modify!
	if(it!=env->oim.end())return &it->second;
	return 0;//type not found!
}

void oms::write_property(oms::context* ctx, const std::string& name){
	oms::io::write_uint8(ctx->ios,1);//write go-byte
	oms::io::write_string(ctx->ios, name);
}

void oms::write_null(oms::context* ctx){
	oms::io::write_uint8(ctx->ios, type_null);
}

void oms::write_boolean(oms::context* ctx, bool v){
	oms::io::write_uint8(ctx->ios,oms::type_boolean);
	oms::io::write_bool(ctx->ios,v);
}

void oms::write_integer(oms::context* ctx, int v){
	oms::io::write_uint8(ctx->ios, type_integer);
	oms::io::write_int32(ctx->ios, v);
}

void oms::write_number(oms::context* ctx, double v){
	oms::io::write_uint8(ctx->ios, type_number);
	oms::io::write_double(ctx->ios, v);
}

void oms::write_string(oms::context* ctx, const std::string& v){
	oms::io::write_uint8(ctx->ios, type_string);
	oms::io::write_string(ctx->ios, v);
}

void oms::write_object(oms::context* ctx, void* o, const std::string& class_name){
	oms::io::write_uint8(ctx->ios, type_object);//write object type
	object_info* oi=oms::get_object_info(ctx->env, class_name);
	if(oi&&o){//got an instance and object info?
		//first time writing this instance?
		std::vector<void*>::iterator it=std::find(ctx->object_table.begin(), ctx->object_table.end(), o);
		if(it!=ctx->object_table.end()){
			//nope, let's write a ref instead
			oms::io::write_uint8(ctx->ios,2);//2 a reference
			oms::io::write_uint32(ctx->ios,ctx->object_table.size()-1);//index of object
		}else{//yep, write the actual object
			oms::io::write_uint8(ctx->ios,1);//1 a real object
			oms::io::write_string(ctx->ios, class_name);//write type name
			oi->w(ctx, o);//delegate to writer
			oms::io::write_uint8(ctx->ios,0);//no-go byte
			ctx->object_table.push_back(o);//push into object_table for ref/cycle detection
		}
	}else{
		oms::io::write_uint8(ctx->ios,0);//0 a null object
	}
}

void oms::write_array(oms::context* ctx, uint32_t count){
	oms::io::write_uint8(ctx->ios, type_array);
	oms::io::write_uint32(ctx->ios, count);
}

void oms::write_map(oms::context* ctx, uint32_t count){
	oms::io::write_uint8(ctx->ios, type_map);
	oms::io::write_uint32(ctx->ios, count);
}

bool oms::read_type(oms::context* ctx, uint8_t type){
	uint8_t t = oms::io::read_uint8(ctx->ios);
	return t==type;
}

uint8_t oms::peek_type(oms::context* ctx){
	return ctx->ios->peek();
}

uint32_t oms::read_size(oms::context* ctx, uint8_t type){
	uint32_t size=0;
	switch(type){
	case oms::type_boolean:
		size=1;
		break;
	case oms::type_integer:
		size=4;
		break;
	case oms::type_number:
		size=8;
		break;
	case oms::type_string:
		size=oms::io::read_uint32(ctx->ios);
		break;
	}
	return size;
}

bool oms::read_boolean(oms::context* ctx){
	return oms::io::read_bool(ctx->ios);
}

int oms::read_integer(oms::context* ctx){
	return oms::io::read_int32(ctx->ios);
}

double oms::read_number(oms::context* ctx){
	return oms::io::read_double(ctx->ios);
}

std::string oms::read_string(oms::context* ctx){
	return oms::io::read_string(ctx->ios);
}

void* oms::read_object(oms::context* ctx){
	void* o=0;
	//is this a real object, or a reference?
	uint8_t nor=oms::io::read_uint8(ctx->ios);
	if(nor==1){
		//we have a real object!
		//read class name
		std::string class_name=oms::io::read_string(ctx->ios);
		//get object info
		object_info* oi=oms::get_object_info(ctx->env, class_name);
		//class exists, create an instance
		if(oi)o=oi->c(ctx);
		while(oms::io::read_uint8(ctx->ios)){
			std::string prop_name=oms::io::read_string(ctx->ios);
			uint8_t type=oms::io::read_uint8(ctx->ios);
			//delegate to reader
			bool read=false;
			if(oi)read=oi->r(ctx, prop_name, o);
			if(!read)oms::consume(ctx, type);
		}
		//push o into the object table, might be null
		//if class doesnt exist, but this will keep the table balanced
		ctx->object_table.push_back(o);
	}else if(nor==2){//we have a reference
		//read index
		uint32_t index=oms::io::read_uint32(ctx->ios);
		o=ctx->object_table[index];//set o to reference in table
	}else{
		//a null, no-op
	}
	return o;
}

uint32_t oms::read_array(oms::context* ctx){
	uint32_t count=oms::io::read_uint32(ctx->ios);
	return count;
}

uint32_t oms::read_map(oms::context* ctx){
	uint32_t count=oms::io::read_uint32(ctx->ios);
	return count;
}

void oms::consume(oms::context* ctx, uint8_t type){
	switch(type){
	case oms::type_object:
		oms::read_object(ctx);
		break;
	case oms::type_array:
		{
			uint32_t count=oms::read_array(ctx);
			for(int i=0;i<count;++i){
				uint8_t type2=oms::io::read_uint8(ctx->ios);
				oms::consume(ctx, type2);
			}
		}
		break;
	case oms::type_map:
		{
			uint32_t count=oms::read_array(ctx);
			for(int i=0;i<count;++i){
				uint8_t ktype=oms::io::read_uint8(ctx->ios);
				oms::consume(ctx, ktype);
				uint8_t vtype=oms::io::read_uint8(ctx->ios);
				oms::consume(ctx, vtype);
			}
		}
		break;
	default://skip type based on length
		uint32_t size=oms::read_size(ctx, type);
		ctx->ios->seekg(size, std::ios::cur);
		break;
	}
}

//perform a deep copy of an object model by serialization
void* oms::util::deep_copy(oms::environment* env, void* o, const std::string& class_name){
	std::string data=oms::util::write_to_string(env, o, class_name);
	return oms::util::read_from_string(env, data);
}

std::string oms::util::write_to_string(oms::environment* env, void* o, const std::string& class_name){
	std::stringstream ss;
	oms::context ctx;
	oms::open_context(&ctx, env, &ss);
	oms::write_object(&ctx, o, class_name);
	oms::close_context(&ctx);
	return ss.str();
}

void* oms::util::read_from_string(oms::environment* env, const std::string& data){
	std::stringstream ss(data);
	oms::context ctx;
	oms::open_context(&ctx, env, &ss);
	void* o=0;
	if(oms::read_type(&ctx, oms::type_object)){
		o=oms::read_object(&ctx);
	}
	oms::close_context(&ctx);
	return o;
}

void oms::util::write_to_file(oms::environment* env, const std::string& file, void* o, const std::string& class_name){
	std::fstream f;
	oms::context ctx;
	f.open(file.c_str(),std::ios::out | std::ios::binary | std::ios::trunc);
	oms::open_context(&ctx, env, &f);
	oms::write_object(&ctx, o, class_name);
	oms::close_context(&ctx);
	f.close();
}

void* oms::util::read_from_file(oms::environment* env, const std::string& file){
	std::fstream f;
	oms::context ctx;
	f.open(file.c_str(),std::ios::in | std::ios::binary);
	oms::open_context(&ctx, env, &f);
	void* o=0;
	if(oms::read_type(&ctx, oms::type_object)){
		o=oms::read_object(&ctx);
	}
	oms::close_context(&ctx);
	f.close();
	return o;
}

//primitive IO functions
void oms::io::write_int8(std::ostream* os, int8_t v){
	os->write((char*)&v,1);
}

void oms::io::write_int16(std::ostream* os, int16_t v){
	os->write((char*)&v,2);
}

void oms::io::write_int32(std::ostream* os, int32_t v){
	os->write((char*)&v,4);
}

void oms::io::write_int64(std::ostream* os, int64_t v){
	os->write((char*)&v,8);
}

void oms::io::write_uint8(std::ostream* os, uint8_t v){
	os->write((char*)&v,1);
}

void oms::io::write_uint16(std::ostream* os, uint16_t v){
	os->write((char*)&v,2);
}

void oms::io::write_uint32(std::ostream* os, uint32_t v){
	os->write((char*)&v,4);
}

void oms::io::write_uint64(std::ostream* os, uint64_t v){
	os->write((char*)&v,8);
}


void oms::io::write_float(std::ostream* os, float v){
	os->write((char*)&v,4);
}

void oms::io::write_double(std::ostream* os, double v){
	os->write((char*)&v,8);
}

//todo: employ 7bit leading string length
void oms::io::write_string(std::ostream* os, const std::string& v){
	uint32_t l=v.length();
	os->write((char*)&l,4);
	os->write((char*)v.data(),l);
}

void oms::io::write_bool(std::ostream* os, bool v){
	os->write((char*)&v,1);
}

int8_t oms::io::read_int8(std::istream* is){
	int8_t v;
	is->read((char*)&v,1);
	return v;
}

int16_t oms::io::read_int16(std::istream* is){
	int16_t v;
	is->read((char*)&v,2);
	return v;
}

int32_t oms::io::read_int32(std::istream* is){
	int32_t v;
	is->read((char*)&v,4);
	return v;
}

int64_t oms::io::read_int64(std::istream* is){
	int64_t v;
	is->read((char*)&v,8);
	return v;
}

uint8_t oms::io::read_uint8(std::istream* is){
	uint8_t v;
	is->read((char*)&v,1);
	return v;
}

uint16_t oms::io::read_uint16(std::istream* is){
	uint16_t v;
	is->read((char*)&v,2);
	return v;
}

uint32_t oms::io::read_uint32(std::istream* is){
	uint32_t v;
	is->read((char*)&v,4);
	return v;
}

uint64_t oms::io::read_uint64(std::istream* is){
	uint64_t v;
	is->read((char*)&v,8);
	return v;
}

float oms::io::read_float(std::istream* is){
	float v;
	is->read((char*)&v,4);
	return v;
}

double oms::io::read_double(std::istream* is){
	double v;
	is->read((char*)&v,8);
	return v;
}

//todo: employ 7bit leading string length
std::string oms::io::read_string(std::istream* is){
	uint32_t l=0;
	is->read((char*)&l,4);
	char* buf=new char[l+1];
	is->read(buf,l);
	buf[l]='\0';
	std::string v=buf;
	delete[] buf;
	return v;
}

bool oms::io::read_bool(std::istream* is){
	bool v;
	is->read((char*)&v,1);
	return v;
}