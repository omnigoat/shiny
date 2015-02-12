#pragma once
//======================================================================
#include <dust/dust_fwd.hpp>
#include <dust/vertex_stream.hpp>
//======================================================================
namespace dust
{
	struct vertex_declaration_t
	{
		vertex_declaration_t() = delete;
		vertex_declaration_t(vertex_declaration_t const*) = delete;
		vertex_declaration_t(vertex_declaration_t&&) = delete;

		auto streams() const -> vertex_streams_t const&;
		auto stride() const -> uint;

	private:
		vertex_declaration_t(vertex_streams_t const&);

	private:
		vertex_streams_t streams_;
		uint stride_;

		friend struct runtime_t;
	};
}
