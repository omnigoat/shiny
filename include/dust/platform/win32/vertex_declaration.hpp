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
		typedef std::vector<vertex_stream_t> streams_t;

		friend auto get_vertex_declaration(streams_t const&) -> vertex_declaration_t const*;
		friend struct context_t;

		auto streams() const -> streams_t const&;
		auto stride() const -> uint;

		// non-default-constructable, non-copyable/movable
		vertex_declaration_t() = delete;
		vertex_declaration_t(vertex_declaration_t const*) = delete;
		vertex_declaration_t(vertex_declaration_t&&) = delete;

	private:
		vertex_declaration_t(streams_t const&);

	private:
		streams_t streams_;
		uint stride_;

		void* platform_impl_;

		static std::map<streams_t, std::unique_ptr<vertex_declaration_t>> cache_;
	};
}
