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

oms::context* oms::open_context(std::iostream* ios, oms::inst_fn ifn){
	oms::context* ctx=new oms::context;
	ctx->ifn=ifn;
	ctx->ios=ios;
	return ctx;
}

void oms::close_context(oms::context* ctx){
	delete ctx;
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
	oms::io::write_bool(ctx->ios, v);
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
	oms::io::write_uint32(ctx->ios, 4 + v.length());//write variable length (header+strlen)
	oms::io::write_string(ctx->ios, v);
}


void oms::write_object(oms::context* ctx, void* o, const std::string& type, write_fn wfn){

	oms::io::write_uint8(ctx->ios, type_object);
	std::vector<void*>::iterator it=std::find(ctx->object_table.begin(), ctx->object_table.end(), o);
	if(it!=ctx->object_table.end()){
		std::cout << "saving object ref." << std::endl;
		oms::io::write_uint32(ctx->ios,5);//size of 5, signal bool and index
		oms::io::write_bool(ctx->ios,false);//false, not real object
		oms::io::write_uint32(ctx->ios,ctx->object_table.size()-1);//index of object
	}else{
		//push new stream
		std::stringstream* ss=new std::stringstream();
		ctx->ioss.push(ctx->ios);
		ctx->ios=ss;

		std::cout << "saving a real object." << std::endl;
		oms::io::write_bool(ctx->ios,true);//true, a real object follows
		ctx->object_table.push_back(o);

		//write type
		oms::io::write_string(ctx->ios, type);

		//delgate to writer
		wfn(ctx,o);

		//write trailing nogo byte (end of properties)
		oms::io::write_uint8(ctx->ios, 0);

		//pop stream
		std::string str = ss->str();
		delete ctx->ios;
		ctx->ios=ctx->ioss.top();
		ctx->ioss.pop();
		oms::io::write_uint32(ctx->ios, str.length());//write variable length
		ctx->ios->write(str.data(),str.length());//write object data
	}
}

bool oms::check_property(oms::context* ctx, const std::string& name, uint8_t type){
	std::map<std::string,std::streampos>::iterator it=ctx->ps.top()->find(name);
	if(it!=ctx->ps.top()->end()){
		ctx->ios->seekg(it->second);
		return oms::check_type(ctx, type);//check/reject type and eat size data
	}
	std::cout << "requested property '" << name <<"' of type " << (int)type << " not found!" << std::endl;
	return false;
}

bool oms::check_type(oms::context* ctx, uint8_t type){
	std::cout << "checking type, expecting  " << (int)type << std::endl;
	uint8_t t = oms::io::read_uint8(ctx->ios);
	uint32_t jmpsiz=check_size(ctx, t);
	if(t!=type){
		std::cout << "unexpected type, jumping " << jmpsiz << " bytes!" << std::endl;
		ctx->ios->seekg(jmpsiz,std::ios::cur);
	}
	return t==type;
}

uint32_t oms::check_size(oms::context* ctx, uint8_t type){
	std::cout << "checking size for type " << (int)type << std::endl;
	switch(type){
	case oms::type_boolean:
		return 1;
	case oms::type_integer:
		return 4;
	case oms::type_number:
		return 8;
	case oms::type_string:
		return oms::io::read_uint32(ctx->ios);
	case oms::type_object:
		return oms::io::read_uint32(ctx->ios);
	}
	return 0;
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

void* oms::read_object(oms::context* ctx, read_fn rfn){
	//todo: first we would read the real object or ref signal here
	std::cout << "ios pos " << ctx->ios->tellg() << std::endl;
	void* o=0;
	if(oms::io::read_bool(ctx->ios)){
		std::cout << "reading real object" << std::endl;
		//read back type
		std::string type=oms::io::read_string(ctx->ios);

		//instantiate type
		o=ctx->ifn(ctx, type);

		//push into table
		ctx->object_table.push_back(o);

		std::map<std::string, std::streampos>* pm=new std::map<std::string, std::streampos>();
		//scan and collect names and positions of properties
		//poll go/nogo byte
		while(oms::io::read_uint8(ctx->ios)){
			std::string n=oms::io::read_string(ctx->ios);
			pm->insert(std::pair<std::string,std::streampos>(n, ctx->ios->tellg()));
			uint8_t type=oms::io::read_uint8(ctx->ios);
			uint32_t size=oms::check_size(ctx,type);
			ctx->ios->seekg(size,std::ios::cur);
		}

		//record where we are now
		std::streampos oend=ctx->ios->tellg();

		//load stacks
		ctx->ps.push(pm);

		//delegate to reader
		rfn(ctx, o);

		//pop stacks
		ctx->ps.pop();
		delete pm;

		//seek back to our end point
		ctx->ios->seekg(oend);
	}else{
		std::cout << "reading object reference" << std::endl;
		//this is an object reference
		uint32_t index=oms::io::read_uint32(ctx->ios);
		o=ctx->object_table[index];
	}
	return o;
}

//perform a deep copy of an object model by serialization
void* oms::util::deep_copy(void* o, const std::string& type, oms::write_fn wfn, oms::read_fn rfn, oms::inst_fn ifn){
	std::stringstream ss;
	oms::context* ctx=0;
	ctx=oms::open_context(&ss, 0);
	oms::write_object(ctx, o, type, wfn);
	oms::close_context(ctx);
	ctx=oms::open_context(&ss, ifn);
	void* o2=0;
	if(oms::check_type(ctx,oms::type_object)){
		o2=oms::read_object(ctx, rfn);
	}
	oms::close_context(ctx);
	return o2;
}

std::string oms::util::write_to_string(void* o, const std::string& type, oms::write_fn wfn){
	std::stringstream ss;
	oms::context* ctx=0;
	ctx=oms::open_context(&ss, 0);
	oms::write_object(ctx, o, type, wfn);
	oms::close_context(ctx);
	return ss.str();
}

void oms::util::write_to_file(const std::string& file, void* o, const std::string& type, oms::write_fn wfn){
	std::fstream f;
	oms::context* ctx=0;
	f.open(file.c_str(),std::ios::out | std::ios::binary | std::ios::trunc);
	ctx=oms::open_context(&f, 0);
	oms::write_object(ctx, o, type, wfn);
	oms::close_context(ctx);
	f.close();
}

void* oms::util::read_from_file(const std::string& file, const std::string& type, oms::read_fn rfn, oms::inst_fn ifn){
	std::fstream f;
	oms::context* ctx=0;
	f.open(file.c_str(),std::ios::in | std::ios::binary);
	ctx=oms::open_context(&f, ifn);
	void* o=0;
	if(oms::check_type(ctx,oms::type_object)){
		o=oms::read_object(ctx, rfn);
	}
	oms::close_context(ctx);
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