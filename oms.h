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
	const uint8_t type_true=2;
	const uint8_t type_false=3;
	const uint8_t type_null=4;
	const uint8_t type_integer=5;
	const uint8_t type_number=6;
	const uint8_t type_string=7;
	const uint8_t type_array=8;
	const uint8_t type_map=9;

	struct object_info;
	struct environment;
	struct context;

	typedef bool (*read_fn)(oms::context*, const std::string&, void*);
	typedef void (*write_fn)(oms::context*, void*);
	typedef void* (*create_fn)(oms::context*);

	struct object_info{
		read_fn r;
		write_fn w;
		create_fn c;
	};

	struct environment{
		std::map<std::string, object_info> oim;
	};

	struct context{
		oms::environment* env;
		std::iostream* ios;
		std::vector<void*> object_table;
		//todo: table strings too
	};

	void open_context(oms::context* ctx, oms::environment* env, std::iostream* ios);
	void close_context(oms::context* ctx);

	void declare_object_info(oms::environment* env, const std::string& class_name, oms::read_fn r, oms::write_fn w, oms::create_fn c);
	object_info* get_object_info(oms::environment* env, const std::string& class_name);

	void write_property(oms::context* ctx, const std::string& name);
	void write_null(oms::context* ctx);
	void write_boolean(oms::context* ctx, bool v);
	void write_integer(oms::context* ctx, int v);
	void write_number(oms::context* ctx, double v);
	void write_string(oms::context* ctx, const std::string& v);
	void write_object(oms::context* ctx, void* o, const std::string& class_name);
	void write_array(oms::context* ctx, uint32_t count);
	void write_map(oms::context* ctx, uint32_t count);

	bool read_boolean(oms::context* ctx);
	int read_integer(oms::context* ctx);
	double read_number(oms::context* ctx);
	std::string read_string(oms::context* ctx);
	void* read_object(oms::context* ctx);
	uint32_t read_array(oms::context* ctx);
	uint32_t read_map(oms::context* ctx);

	void consume(oms::context* ctx, uint8_t type);

	bool read_type(oms::context* ctx, uint8_t type);
	uint8_t peek_type(oms::context* ctx);
	uint32_t read_size(oms::context* ctx, uint8_t type);

	//utility functions
	namespace util{
		void* deep_copy(oms::environment* env, void* o, const std::string& class_name);
		std::string write_to_string(oms::environment* env, void* o, const std::string& class_name);
		void* read_from_string(oms::environment* env, const std::string& data);
		void write_to_file(oms::environment* env, const std::string& file, void* o, const std::string& class_name);
		void* read_from_file(oms::environment* env, const std::string& file);
	}

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