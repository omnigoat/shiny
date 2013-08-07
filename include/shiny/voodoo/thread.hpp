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
	#if 0
	struct scoped_blocking_queue_t
	{
		scoped_blocking_queue_t();
		~scoped_blocking_queue_t();

		template <typename... Args>
		auto add(void (*fn)(Args...), Args... args) -> void
		{
			queue_.push(make_command(fn, args...));
		}

	private:
		atma::lockfree::queue_t<command_ptr> queue_;
	};
	#endif
	
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
		typedef atma::lockfree::queue_t<shiny::voodoo::command_ptr> command_queue_t;
		extern command_queue_t command_queue;

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
	}

	auto spawn() -> void;
	auto join() -> void;


	// enqueues a list of commands, and then blocks until their completion
	template <typename... Commands>
	inline auto enqueue_blocking(Commands... commands) -> void
	{
		detail::command_queue_t::batch_t block;
		detail::enqueue_blocking_impl(block, commands...);
		std::atomic_bool blocked{true};
		block.push(make_command([&blocked] {blocked = false;}));

		detail::command_queue.push(block);

		while (blocked)
			;
	}

	

//======================================================================
} // namespace prime_thread
//======================================================================

	struct scoped_command_batch_t
	{
		template <typename R, typename... Params, typename... Args>
		auto push(R(*fn)(Params...), Args&&... args) -> scoped_command_batch_t&
		{
			batch_.push(make_command(fn, std::forward<Args>(args)...));
			return *this;
		}

		auto block() -> scoped_command_batch_t&
		{
			batch_.push(make_command([&] {blocked_ = false;}));
			return *this;
		}

		~scoped_command_batch_t()
		{
			prime_thread::detail::command_queue.push(batch_);

			while (blocked_)
				;
		}

	private:
		std::atomic_bool blocked_;
		prime_thread::detail::command_queue_t::batch_t batch_;
	};

//======================================================================
} // namespace voodoo
} // namespace shiny
//======================================================================
#endif
//======================================================================
