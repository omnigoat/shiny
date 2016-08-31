#pragma once

#include <atma/logging.hpp>


namespace shiny { namespace logging {

	using level_t = atma::log_level_t;
	using runtime_t = atma::logging_runtime_t;

	
	namespace detail
	{
		inline auto current_runtime() -> runtime_t*&
		{
			static runtime_t* R = nullptr;
			return R;
		}
	}

	inline auto set_runtime(runtime_t* R) -> void
	{
		detail::current_runtime() = R;
	}


	template <typename... Args>
	inline auto send_log(level_t level, char const* filename, int line, Args&&... args) -> bool
	{
		auto* R = detail::current_runtime();
		if (R == nullptr)
			return false;

		// allocate static buffer & encoder
		size_t const bufsize = 8 * 1024;
		char buf[bufsize];
		auto memstream = atma::intrusive_ptr<atma::memory_bytestream_t>::make(buf, bufsize);
		atma::logging_encoder_t encoder{memstream};

		atma::log_style_t styles[] = {
			atma::log_style_t::oneline,
			atma::log_style_t::oneline,
			atma::log_style_t::oneline,
			atma::log_style_t::pretty_print,
			atma::log_style_t::pretty_print,
		};

		atma::colorbyte colors[] = {
			atma::colorbyte{0x08},
			atma::colorbyte{0x1f},
			atma::colorbyte{0x8f},
			atma::colorbyte{0xe4},
			atma::colorbyte{0xcf},
		};

		atma::colorbyte location_colors[] = {
			atma::colorbyte{0x07},
			atma::colorbyte{0x07},
			atma::colorbyte{0x07},
			atma::colorbyte{0x0e},
			atma::colorbyte{0x0c},
		};

		char const* caption[] = {
			"Trace:",
			"[info]",
			"[debug]",
			"[Warning]",
			"[ERROR]"
		};


		size_t p = 0;
		p += encoder.encode_header(styles[(int)level]);
		p += encoder.encode_color(colors[(int)level]);
		p += encoder.encode_sprintf("%s", caption[(int)level]);
		p += encoder.encode_color(location_colors[(int)level]);

		if ((int)level >= (int)level_t::warn)
		{
			p += encoder.encode_sprintf("\n%s:%d\n", filename, line);
			p += encoder.encode_color(atma::colorbyte{0x07});
		}
		else
		{
			p += encoder.encode_cstr(" ", 1);
		}

		p += encoder.encode_all(std::forward<Args>(args)...);

		R->log(level, buf, (uint32)p);

		return true;
	}

#define SHINY_LOG(level, ...) \
	::shiny::logging::send_log(level, __FILE__, __LINE__, __VA_ARGS__, "\n")

#define SHINY_TRACE(...) \
	::shiny::logging::send_log(::shiny::logging::level_t::verbose, __FILE__, __LINE__, __VA_ARGS__, "\n")

#define SHINY_INFO(...) \
	::shiny::logging::send_log(::shiny::logging::level_t::info, __FILE__, __LINE__, __VA_ARGS__, "\n")

#define SHINY_DEBUG(...) \
	::shiny::logging::send_log(::shiny::logging::level_t::debug, __FILE__, __LINE__, __VA_ARGS__, "\n")

#define SHINY_WARN(...) \
	::shiny::logging::send_log(::shiny::logging::level_t::warn, __FILE__, __LINE__, __VA_ARGS__, "\n")

#define SHINY_ERROR(...) \
	::shiny::logging::send_log(::shiny::logging::level_t::error, __FILE__, __LINE__, __VA_ARGS__, "\n")

}}

namespace shiny { namespace log { namespace color {

	namespace
	{
		atma::colorbyte error_text{0xcf};
		atma::colorbyte warn_text{0xe4};
		atma::colorbyte debug_text{0x8f};
	}

} } }

