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
namespace prime_thread {
//======================================================================
	
	namespace detail {
		auto spawn_prime_thread() -> void;
		auto join_prime_thread() -> void;
	}

	auto submit_command(command_t*) -> void;
	auto submit_command_queue(command_queue_t&) -> void;
	
//======================================================================
} // namespace prime_thread
} // namespace plumbing
} // namespace shiny
//======================================================================
#endif
//======================================================================
