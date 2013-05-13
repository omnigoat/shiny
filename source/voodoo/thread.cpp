#include <shiny/voodoo/thread.hpp>
#include <shiny/voodoo/device.hpp>

//======================================================================
// externs
//======================================================================
std::thread shiny::voodoo::detail::prime_thread_;
std::atomic_bool shiny::voodoo::detail::prime_thread_running_;



//======================================================================
// prime_thread
//======================================================================
auto shiny::voodoo::prime_thread::spawn() -> void
{
	using detail::prime_thread_;
	using detail::prime_thread_running_;

	ATMA_ASSERT(prime_thread_.get_id() == std::thread::id());
	prime_thread_running_ = true;

	#if 0
	prime_thread_ = std::thread([]{
		while (prime_thread_running_.load()) {
			command_ptr x(nullptr);
			while (command_queue_.pop(x)) {
				(**x)();
				x->processed.store(true);
			}
		}
	});
	#endif
}

auto shiny::voodoo::prime_thread::join() -> void
{
	using detail::prime_thread_;
	using detail::prime_thread_running_;

	ATMA_ASSERT(prime_thread_.get_id() != std::thread::id());
	prime_thread_running_.store(false);
	prime_thread_.join();
}
