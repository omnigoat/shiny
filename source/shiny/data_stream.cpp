#include <shiny/data_stream.hpp>

using namespace shiny;
using shiny::data_stream_t;


data_stream_t::data_stream_t(atma::string const& semantic, uint index, format_t element_format, data_stream_stage_t stage)
	: stage_(stage), semantic_(semantic), index_(index), element_format_(element_format)
{
}

auto data_stream_t::stage() const -> data_stream_stage_t const&
{
	return stage_;
}

auto data_stream_t::semantic() const -> atma::string const&
{
	return semantic_;
}

auto data_stream_t::index() const -> uint32
{
	return index_;
}

auto data_stream_t::element_format() const -> format_t
{
	return element_format_;
}

auto data_stream_t::size() const -> size_t
{
	return element_size(element_format_);
}

auto shiny::operator < (data_stream_t const& lhs, data_stream_t const& rhs) -> bool
{
	return
		(lhs.semantic() < rhs.semantic() || (!(rhs.semantic() < lhs.semantic()) &&
		(lhs.index() < rhs.index() || (!(rhs.index() < lhs.index()) &&
		(lhs.element_format() < rhs.element_format())))));
}
