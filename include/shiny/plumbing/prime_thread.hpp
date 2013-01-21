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
	
	//======================================================================
	// forward declares
	//======================================================================
	namespace detail {
		extern atma::lockfree::queue_t<command_t*> command_queue_;
		extern std::thread prime_thread_;
		extern std::atomic_bool prime_thread_running_;
	}

	namespace detail {
		auto spawn_prime_thread() -> void
		{
			ATMA_ASSERT(prime_thread_.get_id() == std::thread::id());
			prime_thread_running_ = true;

			prime_thread_ = std::thread([]{
				command_t* x = nullptr;
				while (prime_thread_running_.load() && command_queue_.pop(x)) {
					(*x)();
				}
			});
		}

		auto join_prime_thread() -> void
		{
			ATMA_ASSERT(prime_thread_.get_id() != std::thread::id());
			prime_thread_running_.store(false);
			prime_thread_.join();
		}
	}
	
//======================================================================
} // namespace plumbing
} // namespace shiny
//======================================================================
#endif
//======================================================================
