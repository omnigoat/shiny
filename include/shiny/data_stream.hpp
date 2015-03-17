#pragma once

#include <shiny/dust_fwd.hpp>
#include <shiny/element_format.hpp>

#include <atma/string.hpp>


namespace shiny
{
	enum class data_stream_stage_t
	{
		input_assembly,
		vertex,
	};

	struct data_stream_t
	{
		data_stream_t(atma::string const&, uint index, element_format_t);
		data_stream_t(atma::string const&, uint index, element_format_t, data_stream_stage_t);
		auto semantic() const -> atma::string const&;
		auto index() const -> uint;
		auto element_format() const -> element_format_t;
		auto size() const -> uint;

	private:
		atma::string semantic_;
		uint index_;
		element_format_t element_format_;
	};

	auto operator < (data_stream_t const&, data_stream_t const&) -> bool;
}
