#pragma once

#include <rose/console.hpp>

#include <atma/logging.hpp>

namespace lion
{
	struct console_log_handler_t : atma::logging_handler_t
	{
		console_log_handler_t(rose::console_t& console)
			: console_(console)
		{}

		auto handle(atma::log_level_t, atma::unique_memory_t const&) -> void override;

	private:
		rose::console_t& console_;
		atma::log_style_t last_log_style_ = atma::log_style_t::oneline;
	};
}

