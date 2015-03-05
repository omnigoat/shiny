#include <shiny/vertex_declaration.hpp>


using namespace shiny;
using shiny::vertex_stream_t;
using shiny::vertex_declaration_t;


vertex_stream_t::vertex_stream_t(vertex_stream_semantic_t semantic, uint index, element_format_t element_format)
: semantic_(semantic), index_(index), element_format_(element_format)
{
}

auto vertex_stream_t::semantic() const -> vertex_stream_semantic_t {
	return semantic_;
}

auto vertex_stream_t::index() const -> uint32 {
	return index_;
}

auto vertex_stream_t::element_format() const -> element_format_t
{
	return element_format_;
}

auto vertex_stream_t::size() const -> uint
{
	return element_size(element_format_);
}

auto shiny::operator < (vertex_stream_t const& lhs, vertex_stream_t const& rhs) -> bool
{
	return
		(lhs.semantic() < rhs.semantic() || (!(rhs.semantic() < lhs.semantic()) &&
		(lhs.index() < rhs.index() || (!(rhs.index() < lhs.index()) &&
		(lhs.element_format() < rhs.element_format())))));
}
