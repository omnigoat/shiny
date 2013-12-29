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
				#if 0
				ATMA_ASSERT(detail::d3d_device_);
				ATMA_ASSERT(detail::d3d_local_context_ == nullptr);
				detail::d3d_device_->CreateDeferredContext(0, &detail::d3d_local_context_);
				ATMA_ASSERT(detail::d3d_local_context_);

				fn_(args...);

				ATMA_ASSERT(detail::d3d_local_context_);
				detail::d3d_local_context_->Release();
				detail::d3d_local_context_ = nullptr;
				#endif
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
} // namespace voodoo
} // namespace shiny
//======================================================================
#endif
//======================================================================
