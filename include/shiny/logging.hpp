#pragma once

#include <atma/logging.hpp>


namespace shiny { namespace logging {

	using level_t = atma::log_level_t;
	using runtime_t = atma::logging_runtime_t;

	namespace detail
	{
		inline auto current_runtime() -> runtime_t*& { static runtime_t* R = nullptr; return R; }
		inline auto flush_after_error() -> std::atomic<bool>& { static std::atomic<bool> _{true}; return _; }
	}

	inline auto set_runtime(runtime_t* R) -> void
	{
		detail::current_runtime() = R;
	}

	extern std::atomic<bool> flush_after_error;
	//extern std::atomic<bool> flush_after_warning;

	inline auto set_flush_after_error(bool flush) { detail::flush_after_error() = flush; }

}}

namespace shiny { namespace log { namespace color {

	namespace
	{
		atma::colorbyte reset{0x07};

		// white-on-red for ERRORS
		atma::colorbyte error_text{0xcf};
		// dark-red-on-yellow for WARNINGS
		atma::colorbyte warn_text{0xe4};
		// white-on-blue for highlighting things
		atma::colorbyte highlight_text{0x1f};
	}

} } }

#define SHINY_LOG(level, ...) \
	::atma::send_log(::shiny::logging::detail::current_runtime(), \
		level, nullptr, __FILE__, __LINE__, __VA_ARGS__, "\n")

#define SHINY_TRACE(...) \
	::atma::send_log(::shiny::logging::detail::current_runtime(), \
		::shiny::logging::level_t::verbose, nullptr, __FILE__, __LINE__, __VA_ARGS__, "\n")

#define SHINY_INFO(...) \
	::atma::send_log(::shiny::logging::detail::current_runtime(), \
		::shiny::logging::level_t::info, nullptr, __FILE__, __LINE__, __VA_ARGS__, "\n")

#define SHINY_DEBUG(...) \
	::atma::send_log(::shiny::logging::detail::current_runtime(), \
		::shiny::logging::level_t::debug, nullptr, __FILE__, __LINE__, __VA_ARGS__, "\n")

#define SHINY_WARN(...) \
	::atma::send_log(::shiny::logging::detail::current_runtime(), \
		::shiny::logging::level_t::warn, "shiny", __FILE__, __LINE__, __VA_ARGS__, "\n")

#define SHINY_ERROR(...) \
	do { \
		::atma::send_log(::shiny::logging::detail::current_runtime(), \
			::shiny::logging::level_t::error, "shiny", __FILE__, __LINE__, __VA_ARGS__, "\n"); \
		if (::shiny::logging::detail::flush_after_error()) \
			::shiny::logging::detail::current_runtime()->flush(); \
	} while (0)
