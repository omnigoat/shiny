#pragma once

#include <atma/logging.hpp>


namespace shiny { namespace logging
{
	using level_t = atma::log_level_t;

	struct runtime_t;

	auto current_runtime() -> runtime_t*&;

	
	struct runtime_t : atma::logging_runtime_t
	{
		runtime_t()
		{
			ATMA_ASSERT(current_runtime() == nullptr);
			current_runtime() = this;
		}

		~runtime_t()
		{
			ATMA_ASSERT(current_runtime() != nullptr);
			current_runtime() = nullptr;
		}
	};

	
	inline auto current_runtime() -> runtime_t*&
	{
		static runtime_t* R = nullptr;
		return R;
	}

	inline auto log(level_t level, char const* filename, uint32 line, char const* message) -> bool
	{
		auto* R = current_runtime();
		if (R == nullptr)
			return false;

		size_t p = 0;
		char buf[8 * 1024];

		char const* caption[] = {
			"Trace:",
			"Info:",
			"Debug:",
			"Warning!",
			"Error!"
		};

		byte colors[] = {
			0x01,
			0x07,
			0x07,
			0xe1,
			0xcf,
		};

		buf[0] = (byte)atma::log_style_t::pretty_print;
		p += 1;
		p += atma::logging_encode_color(buf + p, colors[(int)level]);
		p += atma::logging_encode_string(buf + p, "%s\n", caption[(int)level]);
		p += atma::logging_encode_color(buf + p, 0x07);
		p += atma::logging_encode_string(buf + p, " %s:%d\n %s\n", filename, line, message);

		R->log(level_t::error, buf, (uint32)p);
		return true;
	}

	inline auto log(level_t level, char const* message) -> bool
	{
		auto* R = current_runtime();
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


#define SHINY_TRACE(msg) \
	::shiny::logging::log(::shiny::logging::level_t::warn, msg)

#define SHINY_INFO(msg) \
	::shiny::logging::log(::shiny::logging::level_t::warn, msg)

#define SHINY_DEBUG(msg) \
	::shiny::logging::log(::shiny::logging::level_t::warn, __FILE__, __LINE__, msg)

#define SHINY_WARN(msg) \
	::shiny::logging::log(::shiny::logging::level_t::warn, __FILE__, __LINE__, msg)

#define SHINY_ERROR(msg) \
	::shiny::logging::log(::shiny::logging::level_t::error, __FILE__, __LINE__, msg)

}}
