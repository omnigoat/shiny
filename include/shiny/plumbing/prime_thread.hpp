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
#include <atma/intrusive_ptr.hpp>
//======================================================================
namespace shiny {
namespace plumbing {
//======================================================================
	
	//======================================================================
	//
	//======================================================================
	struct command_t;
	typedef atma::intrusive_ptr<command_t> command_ptr;
	typedef atma::lockfree::queue_t<command_ptr> command_queue_t;

	namespace detail {
		 // this is the command-queue which through all commands reach the prime thread.
		extern command_queue_t command_queue_;
	}

//======================================================================
namespace prime_thread {
//======================================================================
	
	auto spawn() -> void;
	auto join() -> void;

	auto submit_command(command_ptr const&) -> void;
	auto submit_command_queue(command_queue_t&) -> void;
	
//======================================================================
} // namespace prime_thread
} // namespace plumbing
} // namespace shiny
//======================================================================
#endif
//======================================================================
