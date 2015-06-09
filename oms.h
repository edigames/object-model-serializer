/*
** object-model-serializer
** A tiny C++ library for 'soft serialization' of object models.
** written by Raymond Jacobs
** distributed by Ethereal Darkness Interactive
** under MIT license for private and commercial works.
*/

#ifndef OMS_H
#define OMS_H

#include <iostream>
#include <sstream>
#include <stack>
#include <map>
#include <vector>

//-std=c++98

namespace oms{
	const uint8_t type_unknown=0;
	const uint8_t type_object=1;
	const uint8_t type_boolean=2;
	const uint8_t type_null=3;
	const uint8_t type_integer=4;
	const uint8_t type_number=5;
	const uint8_t type_string=6;
	const uint8_t type_array=7;
	const uint8_t type_map=8;

	struct context;

	typedef void* (*inst_fn)(oms::context*,const std::string& type);
	typedef void (*write_fn)(oms::context*,void*);
	typedef void (*read_fn)(oms::context*,void*);

	struct context{
		inst_fn ifn;
		std::iostream* ios;
		std::vector<void*> object_table;
		std::stack<std::iostream*> ioss;//iostream stack
		std::stack<std::map<std::string,std::streampos>*> ps;//property stack
	};

	context* open_context(std::iostream* ios, inst_fn ifn);
	void close_context(oms::context* ctx);
	void write_property(oms::context* ctx, const std::string& name);
	void write_null(oms::context* ctx);
	void write_boolean(oms::context* ctx, bool v);
	void write_integer(oms::context* ctx, int v);
	void write_number(oms::context* ctx, double v);
	void write_string(oms::context* ctx, const std::string& v);
	void write_object(oms::context* ctx, void* o, const std::string& type, write_fn wfn);

	bool read_boolean(oms::context* ctx);
	int read_integer(oms::context* ctx);
	double read_number(oms::context* ctx);
	std::string read_string(oms::context* ctx);
	void* read_object(oms::context* ctx, read_fn rfn);

	bool check_property(oms::context* ctx, const std::string& name, uint8_t type);
	bool check_type(oms::context* ctx, uint8_t type);
	uint32_t check_size(oms::context* ctx, uint8_t type);

	//primitive IO
	namespace io{

		//writers
		void write_int8(std::ostream* os, int8_t v);
		void write_int16(std::ostream* os, int16_t v);
		void write_int32(std::ostream* os, int32_t v);
		void write_int64(std::ostream* os, int64_t v);
		void write_uint8(std::ostream* os, uint8_t v);
		void write_uint16(std::ostream* os, uint16_t v);
		void write_uint32(std::ostream* os, uint32_t v);
		void write_uint64(std::ostream* os, uint64_t v);
		void write_float(std::ostream* os, float v);
		void write_double(std::ostream* os, double v);
		void write_string(std::ostream* os, const std::string& v);
		void write_bool(std::ostream* os, bool v);

		//readers
		int8_t read_int8(std::istream* is);
		int16_t read_int16(std::istream* is);
		int32_t read_int32(std::istream* is);
		int64_t read_int64(std::istream* is);
		uint8_t read_uint8(std::istream* is);
		uint16_t read_uint16(std::istream* is);
		uint32_t read_uint32(std::istream* is);
		uint64_t read_uint64(std::istream* is);
		float read_float(std::istream* is);
		double read_double(std::istream* is);
		std::string read_string(std::istream* is);
		bool read_bool(std::istream* is);
	}
}

#endif /* OMS_H */