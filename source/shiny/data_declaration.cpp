#include <shiny/data_declaration.hpp>

using namespace shiny;
using shiny::data_stream_t;


data_declaration_t::data_declaration_t(data_streams_t const& streams)
	: streams_(streams), stride_()
{
	for (auto const& x : streams_)
		stride_ += x.size();
}

auto data_declaration_t::streams() const -> data_streams_t const&
{
	return streams_;
}

auto data_declaration_t::stride() const -> size_t
{
	return stride_;
}
