#ifndef SHINY_VOODOO_PRIME_THREAD_HPP
#define SHINY_VOODOO_PRIME_THREAD_HPP
//======================================================================
#include <thread>
#include <atomic>
//======================================================================
#include <d3d11.h>
//======================================================================
#include <atma/assert.hpp>
#include <atma/lockfree/queue.hpp>
#include <shiny/voodoo/device.hpp>
#include <shiny/voodoo/command.hpp>
//======================================================================
namespace shiny {
namespace voodoo {
namespace prime_thread {
//======================================================================
	
	//======================================================================
	// here be dragons.
	//======================================================================
	namespace detail
	{
		extern std::thread handle_;
		extern std::atomic_bool is_running_;
	
		typedef atma::lockfree::queue_t<std::function<void()>> command_queue_t;
		extern command_queue_t command_queue_;
		extern command_queue_t shutdown_queue_;

		auto reenter(std::atomic_bool const&) -> void;
	}

	auto spawn() -> void;
	auto join() -> void;

	auto enqueue_block() -> void;
	auto enqueue(std::function<void()> const& fn) -> void;

//======================================================================
} // namespace prime_thread
} // namespace voodoo
} // namespace shiny
//======================================================================
#endif
//======================================================================
