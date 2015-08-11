#pragma once

#include <shiny/shiny_fwd.hpp>
#include <shiny/format.hpp>

#include <atma/string.hpp>


namespace shiny
{
	struct data_stream_t
	{
		data_stream_t(atma::string const&, uint index, format_t, data_stream_stage_t = data_stream_stage_t::vertex);
		auto stage() const -> data_stream_stage_t const&;
		auto semantic() const -> atma::string const&;
		auto index() const -> uint;
		auto element_format() const -> format_t;
		auto size() const -> size_t;

	private:
		data_stream_stage_t stage_;
		atma::string semantic_;
		uint index_;
		format_t element_format_;
	};

	auto operator < (data_stream_t const&, data_stream_t const&) -> bool;
}
