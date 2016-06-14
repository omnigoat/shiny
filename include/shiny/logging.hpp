#pragma once

#include <atma/logging.hpp>


namespace shiny { namespace logging
{
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



	template <typename... Args>
	inline auto log(level_t level) -> bool
	{
		auto* R = detail::current_runtime();
		if (R == nullptr)
			return false;

		size_t p = 0;
		size_t const bufsize = 8 * 1024;
		char buf[bufsize];

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

		auto memstream = atma::intrusive_ptr<atma::memory_stream_t>::make(buf, bufsize);
		atma::logging_encoder_t encoder{memstream};

		p += encoder.encode_header(atma::log_style_t::pretty_print);
		p += encoder.encode_color(colors[(int)level]);
		p += encoder.encode_sprintf("%s\n", caption[(int)level]);
		p += encoder.encode_color(0x07);
		p += encoder.encode_sprintf("%s:%d\n%s\n", filename, line, message);

		R->log(level_t::error, buf, (uint32)p);
		return true;
	}

	inline auto log(level_t level, char const* message) -> bool
	{
		auto* R = detail::current_runtime();
		if (R == nullptr)
			return false;

		size_t p = 0;
		char buf[8 * 1024];

		buf[0] = (byte)atma::log_style_t::oneline;
		p += 1;
		p += atma::logging_encode_string(buf + p, "%s\n", message);
		R->log(level, buf, (uint32)p);
		return true;
	}

#define SHINY_LOG(level, ...) \
	::shiny::logging::log(level, __VA_ARGS__)

#define SHINY_TRACE(msg) \
	::shiny::logging::log(::shiny::logging::level_t::verbose, msg)

#define SHINY_INFO(msg) \
	::shiny::logging::log(::shiny::logging::level_t::info, msg)

#define SHINY_DEBUG(msg) \
	::shiny::logging::log(::shiny::logging::level_t::debug, __FILE__, __LINE__, msg)

#define SHINY_WARN(msg) \
	::shiny::logging::log(::shiny::logging::level_t::warn, __FILE__, __LINE__, msg)

#define SHINY_ERROR(msg) \
	::shiny::logging::log(::shiny::logging::level_t::error, __FILE__, __LINE__, msg)

}}
