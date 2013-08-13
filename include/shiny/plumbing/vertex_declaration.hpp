#ifndef SHINY_PLUMBING_VERTEX_DECLARATION_HPP
#define SHINY_PLUMBING_VERTEX_DECLARATION_HPP
//======================================================================
#include <shiny/plumbing/lock.hpp>
#include <shiny/plumbing/prime_thread.hpp>
#include <shiny/plumbing/commands/map_unmap_copy.hpp>
//======================================================================
#include <shiny/voodoo/resources.hpp>
//======================================================================
#include <atma/assert.hpp>
//======================================================================
#include <d3d11.h>
//======================================================================
#include <vector>
#include <thread>
#include <initializer_list>
//======================================================================
namespace shiny {
namespace plumbing {
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
			real32,
			uint8,
			uint16,
			uint32,
			int32,
			int16,
			int8
		};

		vertex_stream_t(usage_t, uint32_t index, element_type_t, uint32_t element_count);

		auto usage() const -> usage_t;
		auto index() const -> uint32_t;
		auto element_type() const -> element_type_t;
		auto element_count() const -> uint32_t;
		auto size() const -> uint32_t;

	private:
		usage_t usage_;
		uint32_t index_;
		element_type_t element_type_;
		uint32_t element_count_;
	};



	//======================================================================
	// vertex_declaration_t
	//======================================================================
	struct vertex_declaration_t
	{
		typedef std::vector<vertex_stream_t> streams_t;
		
		vertex_declaration_t( std::initializer_list<vertex_stream_t> streams );
		
		auto streams() const -> streams_t const&;
		auto stride() const -> uint32_t;

	private:
		auto build() -> void;

		streams_t streams_;
		uint32_t stride_;
		ID3D11InputLayout* d3d_layout_;
	};


//======================================================================
} // namespace plumbing
} // namespace shiny
//======================================================================
#endif
//======================================================================
