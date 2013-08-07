#include <shiny/voodoo/thread.hpp>
#include <shiny/voodoo/device.hpp>
#include <shiny/voodoo/command.hpp>

#include <atma/lockfree/queue.hpp>

//======================================================================
// externs
//======================================================================
std::thread shiny::voodoo::detail::prime_thread_;
std::atomic_bool shiny::voodoo::detail::prime_thread_running_;
shiny::voodoo::prime_thread::detail::command_queue_t shiny::voodoo::prime_thread::detail::command_queue;


//======================================================================
// prime_thread
//======================================================================
auto shiny::voodoo::prime_thread::spawn() -> void
{
	using voodoo::detail::prime_thread_;
	using voodoo::detail::prime_thread_running_;

	ATMA_ASSERT(prime_thread_.get_id() == std::thread::id());
	prime_thread_running_ = true;

	prime_thread_ = std::thread([]{
		while (prime_thread_running_) {
			command_ptr x;
			while (detail::command_queue.pop(x)) {
				(**x)();
			}
		}
	});
}

auto shiny::voodoo::prime_thread::join() -> void
{
	using voodoo::detail::prime_thread_;
	using voodoo::detail::prime_thread_running_;

	ATMA_ASSERT(prime_thread_.get_id() != std::thread::id());
	prime_thread_running_.store(false);
	prime_thread_.join();
}

