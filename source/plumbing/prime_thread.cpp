#include <shiny/plumbing/prime_thread.hpp>
#include <shiny/plumbing/device.hpp>
	
auto shiny::plumbing::detail::spawn_prime_thread() -> void
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

auto shiny::plumbing::detail::join_prime_thread() -> void
{
	ATMA_ASSERT(prime_thread_.get_id() != std::thread::id());
	prime_thread_running_.store(false);
	prime_thread_.join();
}
