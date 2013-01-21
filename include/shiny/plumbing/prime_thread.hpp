#ifndef SHINY_PLUMBING_PRIME_THREAD_HPP
#define SHINY_PLUMBING_PRIME_THREAD_HPP
//======================================================================
#include <thread>
#include <map>
#include <mutex>
//======================================================================
#include <d3d11.h>
//======================================================================
#include <atma/assert.hpp>
#include <atma/lockfree/queue.hpp>
//======================================================================
#include <shiny/plumbing/command.hpp>
//======================================================================
namespace shiny {
namespace plumbing {
//======================================================================
	
	namespace detail {
		auto spawn_prime_thread() -> void;
		auto join_prime_thread() -> void;
	}
	
//======================================================================
} // namespace plumbing
} // namespace shiny
//======================================================================
#endif
//======================================================================
