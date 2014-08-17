#pragma once
//======================================================================
#include <dust/dust_fwd.hpp>
#include <dust/vertex_stream.hpp>

#include <vector>
#include <map>
#include <memory>
#include <initializer_list>
//======================================================================
namespace dust
{
	struct vertex_declaration_t
	{
		friend struct runtime_t;

		auto streams() const -> vertex_streams_t const&;
		auto stride() const -> uint;

		// non-default-constructable, non-copyable/movable
		vertex_declaration_t() = delete;
		vertex_declaration_t(vertex_declaration_t const*) = delete;
		vertex_declaration_t(vertex_declaration_t&&) = delete;

	private:
		vertex_declaration_t(vertex_streams_t const&);

	private:
		vertex_streams_t streams_;
		uint stride_;

		platform::d3d_input_layout_ptr d3d_layout_;
	};
}
