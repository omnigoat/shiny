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

		inline auto log_impl(atma::logging_encoder_t&) -> void
		{
		}

		template <typename... Args>
		inline auto log_impl(atma::logging_encoder_t& enc, atma::log_style_t style, Args&&... args) -> void
		{
			enc.encode_header(style);
			log_impl(enc, std::forward<Args>(args)...);
		}

		template <typename... Args>
		inline auto log_impl(atma::logging_encoder_t& enc, char const* t, Args&&... args) -> void
		{
			enc.encode_cstr(t);
			log_impl(enc, std::forward<Args>(args)...);
		}

		template <typename... Args>
		inline auto log_impl(atma::logging_encoder_t& enc, byte color, Args&&... args) -> void
		{
			enc.encode_color(color);
			log_impl(enc, std::forward<Args>(args)...);
		}
	}

	inline auto set_runtime(runtime_t* R) -> void
	{
		detail::current_runtime() = R;
	}



	inline auto log_header(atma::logging_encoder_t& encoder, level_t level, char const* filename, int line) -> size_t
	{
		auto* R = detail::current_runtime();
		if (R == nullptr)
			return false;

		char const* caption[] = {
			"Trace:",
			"[info]",
			"[debug]",
			"[Warning]",
			"[ERROR]"
		};

		byte colors[] = {
			0x08,
			0x1f,
			0x8f,
			0xe4,
			0xcf,
		};

		size_t p = 0;
		p += encoder.encode_header(atma::log_style_t::pretty_print);
		p += encoder.encode_color(colors[(int)level]);
		p += encoder.encode_sprintf("%s\n", caption[(int)level]);
		p += encoder.encode_color(0x07);
		p += encoder.encode_sprintf("%s:%d\n", filename, line);

		return p;
	}

	template <typename... Args>
	inline auto log(level_t level, char const* filename, int line, Args&&... args) -> bool
	{
		auto* R = detail::current_runtime();
		if (R == nullptr)
			return false;

		// allocate static buffer & encoder
		size_t const bufsize = 8 * 1024;
		char buf[bufsize];
		auto memstream = atma::intrusive_ptr<atma::memory_bytestream_t>::make(buf, bufsize);
		atma::logging_encoder_t encoder{memstream};

		// encode
		size_t p = 
			log_header(encoder, level, filename, line) +
			encoder.encode_all(std::forward<Args>(args)...);

		R->log(level, buf, (uint32)p);

		return true;
	}

#define SHINY_LOG(level, ...) \
	::shiny::logging::log(level, __VA_ARGS__)

#define SHINY_TRACE(...) \
	::shiny::logging::log(::shiny::logging::level_t::verbose, __FILE__, __LINE__, __VA_ARGS__, "\n")

#define SHINY_INFO(...) \
	::shiny::logging::log(::shiny::logging::level_t::info, __FILE__, __LINE__, __VA_ARGS__, "\n")

#define SHINY_DEBUG(...) \
	::shiny::logging::log(::shiny::logging::level_t::debug, __FILE__, __LINE__, __VA_ARGS__, "\n")

#define SHINY_WARN(...) \
	::shiny::logging::log(::shiny::logging::level_t::warn, __FILE__, __LINE__, __VA_ARGS__, "\n")

#define SHINY_ERROR(...) \
	::shiny::logging::log(::shiny::logging::level_t::error, __FILE__, __LINE__, __VA_ARGS__, "\n")

}}
