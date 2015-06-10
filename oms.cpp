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

/*
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
*/
void oms::write_object(oms::context* ctx, void* o, const std::string& class_name){
	object_info* oi=oms::get_object_info(ctx->env, class_name);
	if(oi){
		oms::io::write_uint8(ctx->ios, type_object);//write object type
		std::streampos size_pos=ctx->ios->tellp();//record our location for the size header
		oms::io::write_uint32(ctx->ios,0);//block-out space for size
		//first time writing this instance?
		std::vector<void*>::iterator it=std::find(ctx->object_table.begin(), ctx->object_table.end(), o);
		if(it!=ctx->object_table.end()){
			//nope, let's write a ref instead
			std::cout << "saving object ref." << std::endl;
			oms::io::write_bool(ctx->ios,false);//false, not real object
			oms::io::write_uint32(ctx->ios,ctx->object_table.size()-1);//index of object
		}else{//yep, write the actual object
			oms::io::write_bool(ctx->ios,true);//true, a real object
			ctx->object_table.push_back(o);//push into object_table for ref/cycle detection
			oms::io::write_string(ctx->ios, class_name);//write type name
			oi->w(ctx, o);//delegate to writer
			oms::io::write_uint8(ctx->ios,0);//no-go byte
		}
		//all done writing, first record our position
		std::streampos end_pos=ctx->ios->tellp();
		//now jump-back to our blocked out size space
		ctx->ios->seekp(size_pos);
		//now, over-write our difference in length, minus 4 bytes for the size header
		uint32_t size=(end_pos-size_pos)-4;
		std::cout << "object write size is: " << size << std::endl;
		oms::io::write_uint32(ctx->ios, size);


		//finally jump back to the end
		ctx->ios->seekp(end_pos);
	}else{//no type? write a null
		oms::write_null(ctx);
	}
}

bool oms::check_property(oms::context* ctx, const std::string& name, uint8_t type){
	std::map<std::string,std::streampos>::iterator it=ctx->ps.top()->find(name);
	if(it!=ctx->ps.top()->end()){
		ctx->ios->seekg(it->second);
		return oms::read_type(ctx, type);//check/reject type and eat size data
	}
	std::cout << "requested property '" << name <<"' of type " << (int)type << " not found!" << std::endl;
	return false;
}

bool oms::read_type(oms::context* ctx, uint8_t type){
	uint8_t t = oms::io::read_uint8(ctx->ios);
	return t==type;
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
	case oms::type_object:
		size=oms::io::read_uint32(ctx->ios);
		ctx->ios->seekg(-4, std::ios::cur);//rewind
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
	uint32_t size=oms::io::read_uint32(ctx->ios);
	std::cout << "object read size is: " << size << std::endl;
	//is this a real object, or a reference?
	if(oms::io::read_bool(ctx->ios)){
		//we have a real object!
		//read class name
		std::string class_name=oms::io::read_string(ctx->ios);
		//get object info
		object_info* oi=oms::get_object_info(ctx->env, class_name);
		if(oi){
			std::map<std::string, std::streampos>* pm=new std::map<std::string, std::streampos>();
			//scan and collect names and positions of properties
			//poll go/nogo byte
			while(oms::io::read_uint8(ctx->ios)){
				std::string prop_name=oms::io::read_string(ctx->ios);
				pm->insert(std::pair<std::string,std::streampos>(prop_name, ctx->ios->tellg()));
				uint8_t type=oms::io::read_uint8(ctx->ios);
				uint32_t jump_size=oms::check_size(ctx, type);
				ctx->ios->seekg(jump_size, std::ios::cur);
			}

			//record where we are now
			std::streampos oend=ctx->ios->tellg();

			//class exists, create an instance
			o=oi->c(ctx);
			if(o){//did we get an instance?
				oi->r(ctx, o);//delegate to reader
			}
		}
		//push o into the object table, might be null
		//if class doesnt exist, but this will keep the table balanced
		ctx->object_table.push_back(o);
	}else{//we have a reference
		//read index
		uint32_t index=oms::io::read_uint32(ctx->ios);
		o=ctx->object_table[index];//set o to reference in table
	}
	return o;

	//todo: first we would read the real object or ref signal here
	//std::cout << "ios pos " << ctx->ios->tellg() << std::endl;

	/*
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
	*/
}

/*
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
*/

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