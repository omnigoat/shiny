#include <lion/console_log_handler.hpp>


auto lion::console_log_handler_t::handle(atma::log_level_t level, atma::unique_memory_t const& data) -> void
{
	
	auto handle_spacing = [&](atma::log_style_t LS)
	{
		if (last_log_style_ == atma::log_style_t::pretty_print || LS == atma::log_style_t::pretty_print)
			console_.write("\n", 1);
		last_log_style_ = LS;
	};

	atma::decode_logging_data(data,
		handle_spacing,
		atma::curry(&rose::console_t::set_color, &console_),
		atma::curry(&rose::console_t::write, &console_));
}

