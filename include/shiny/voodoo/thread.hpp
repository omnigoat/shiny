#ifndef SHINY_VOODOO_THREAD_HPP
#define SHINY_VOODOO_THREAD_HPP
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
//======================================================================
	
	
	//======================================================================
	// here be dragons.
	//======================================================================
	namespace detail
	{
		// the prime thread is the thread upon which @d3d_immediate_context_ resides.
		extern std::thread prime_thread_;
		extern std::atomic_bool prime_thread_running_;
	}





	//======================================================================
	// thread_fn
	// -----------
	//   the function passed to std::thread that maintains thread-local
	//   validity of d3d_local_context_
	//======================================================================
	namespace detail
	{
		template <typename FN, typename... Args>
		struct thread_fn
		{
			thread_fn(FN fn)
				: fn_(fn)
			{
			}

			void operator ()(Args&&... args) const
			{
				ATMA_ASSERT(detail::d3d_device_);
				ATMA_ASSERT(detail::d3d_local_context_ == nullptr);
				detail::d3d_device_->CreateDeferredContext(0, &detail::d3d_local_context_);
				ATMA_ASSERT(detail::d3d_local_context_);

				fn_(args...);

				ATMA_ASSERT(detail::d3d_local_context_);
				detail::d3d_local_context_->Release();
				detail::d3d_local_context_ = nullptr;
			}

		private:
			FN fn_;
		};
	}
	
	
	template <typename FN, typename... Args>
	inline auto spawn_thread(FN fn, Args... args) -> std::thread
	{
		return std::thread(detail::thread_fn<FN, Args...>(fn), args...);
	}
	
//======================================================================
namespace prime_thread {
//======================================================================
	
	namespace detail
	{
		typedef atma::lockfree::queue_t<std::function<void()>> command_queue_t;
		extern command_queue_t command_queue;
		extern command_queue_t shutdown_queue;
#if 0
		inline auto enqueue_blocking_impl(command_queue_t::batch_t& block, command_ptr const& command) -> void
		{
			block.push(command);
		}

		template <typename... Commands>
		auto enqueue_blocking_impl(command_queue_t::batch_t& block, command_ptr const& command, Commands... commands) -> void
		{
			block.push(command);
			enqueue_blocking_impl(block, commands...);
		}
#endif

		auto reenter(std::atomic_bool const&) -> void;
	}

	auto spawn() -> void;
	auto join() -> void;

	inline auto enqueue_block() -> void
	{
		if (!voodoo::detail::prime_thread_running_)
			return;

		// push blocking fn!
		std::atomic_bool blocked{ true };
		detail::command_queue.push([&blocked]{ blocked = false; });

		// don't block if we're the prime thread blocking ourselves.
		if (std::this_thread::get_id() == voodoo::detail::prime_thread_.get_id())
		{
			voodoo::prime_thread::detail::reenter(blocked);
			return;
		}
		else
		{
			while (blocked)
				;
		}
	}

	inline auto enqueue(std::function<void()> const& fn) -> void
	{
		if (!voodoo::detail::prime_thread_running_)
			return;

		// perform straight away if we're the prime thread enqueuing ourselves
		//if (std::this_thread::get_id() == voodoo::detail::prime_thread_.get_id())
			//fn();

		detail::command_queue.push(fn);
	}

	

#if 0
	inline auto enqueue_batch(detail::command_queue_t::batch_t& batch) -> void
	{
		if (std::this_thread::get_id() == voodoo::detail::prime_thread_.get_id()) {
			auto n = batch.begin();
			while (n) {
				command_ptr const& x = *n->value_ptr();
				x->operator()();
				n = n->next;
			}
		}
		else {
			detail::command_queue.push(batch);
		}
	}
#endif

	

//======================================================================
} // namespace prime_thread
} // namespace voodoo
} // namespace shiny
//======================================================================
#endif
//======================================================================
