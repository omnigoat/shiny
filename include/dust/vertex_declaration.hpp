#pragma once
//======================================================================
#include <dust/context.hpp>

#include <atma/assert.hpp>

#include <vector>
#include <thread>
#include <initializer_list>
//======================================================================
namespace dust {
//======================================================================
	
	//======================================================================
	// vertex_stream_t
	//======================================================================
	struct vertex_stream_t
	{
		enum class usage_t
		{
			position,
			color,
			texcoord,
		};

		enum class element_type_t
		{
			float32,
			uint8,
			uint16,
			uint32,
			int32,
			int16,
			int8
		};

		vertex_stream_t(usage_t, uint32 index, element_type_t, uint32 element_count);

		auto usage() const -> usage_t;
		auto index() const -> uint32;
		auto element_type() const -> element_type_t;
		auto element_count() const -> uint32;
		auto size() const -> uint;

	private:
		usage_t usage_;
		uint32 index_;
		element_type_t element_type_;
		uint32 element_count_;
	};


	struct vertex_shader_t;
	typedef atma::intrusive_ptr<vertex_shader_t> vertex_shader_ptr;


	//======================================================================
	// vertex_declaration_t
	//======================================================================
	struct vertex_declaration_t
	{
		typedef std::vector<vertex_stream_t> streams_t;
		
		vertex_declaration_t(context_ptr const&, vertex_shader_ptr const&, std::initializer_list<vertex_stream_t> streams);
		
		auto streams() const -> streams_t const&;
		auto stride() const -> uint;

		auto d3d_input_layout() const -> platform::d3d_input_layout_ptr const&;

	private:
		auto build(vertex_shader_ptr const&) -> void;

		context_ptr context_;

		streams_t streams_;
		uint stride_;
		platform::d3d_input_layout_ptr d3d_input_layout_;
		bool built_;
	};


//======================================================================
} // namespace dust
//======================================================================
