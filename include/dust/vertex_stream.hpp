#pragma once
//======================================================================
#include <dust/dust_fwd.hpp>
#include <dust/element_format.hpp>
//======================================================================
namespace dust
{
	enum class vertex_stream_semantic_t
	{
		position,
		color,
		texcoord,
	};

	struct vertex_stream_t
	{
		vertex_stream_t(vertex_stream_semantic_t, uint index, element_format_t);

		auto semantic() const -> vertex_stream_semantic_t;
		auto index() const -> uint;
		auto element_format() const -> element_format_t;
		auto size() const -> uint;

	private:
		vertex_stream_semantic_t semantic_;
		uint index_;
		element_format_t element_format_;
	};

	auto operator < (vertex_stream_t const&, vertex_stream_t const&) -> bool;
}
