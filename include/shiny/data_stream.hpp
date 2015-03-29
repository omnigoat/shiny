#pragma once

#include <shiny/shiny_fwd.hpp>
#include <shiny/element_format.hpp>

#include <atma/string.hpp>


namespace shiny
{
	struct data_stream_t
	{
		data_stream_t(atma::string const&, uint index, element_format_t, data_stream_stage_t = data_stream_stage_t::vertex);
		auto stage() const -> data_stream_stage_t const&;
		auto semantic() const -> atma::string const&;
		auto index() const -> uint;
		auto element_format() const -> element_format_t;
		auto size() const -> size_t;

	private:
		data_stream_stage_t stage_;
		atma::string semantic_;
		uint index_;
		element_format_t element_format_;
	};

	auto operator < (data_stream_t const&, data_stream_t const&) -> bool;
}
