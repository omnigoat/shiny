#ifndef SHINY_VOODOO_DEVICE_HPP
#define SHINY_VOODOO_DEVICE_HPP
//======================================================================
#include <shiny/voodoo/prime_thread.hpp>
//======================================================================
#include <fooey/widgets/window.hpp>
//======================================================================
#include <atma/intrusive_ptr.hpp>
//======================================================================
#include <thread>
#include <atomic>
#include <mutex>
//======================================================================
#include <d3d11.h>
//======================================================================
namespace shiny {
namespace voodoo {
//======================================================================
	
	template <typename T>
	class com_ptr
	{
	public:
		com_ptr()
			: x_(nullptr)
		{}

		// raw constructor takes ownership
		com_ptr(T* x)
			: x_(x)
		{}

		com_ptr(com_ptr&& rhs)
			: x_(rhs.x_)
		{
			rhs.x_ = nullptr;
		}

		com_ptr(com_ptr const& rhs)
		{
			if (x_) x_->Release();
			x_ = rhs.x_;
			if (x_) x_->AddRef();
		}

		~com_ptr()
		{
			if (x_) x_->Release();
		}

		auto operator -> () -> T* {
			return x_;
		}

		auto operator = (com_ptr const& rhs) -> com_ptr&
		{
			if (x_) x_->Release();
			x_ = rhs.x_;
			if (x_) x_->AddRef();
			return *this;
		}

		auto operator & () -> T** {
			return &x_;
		}

		operator bool () {
			return x_ != nullptr;
		}

		auto get() const -> T* { return x_; }
	private:
		T* x_;
	};

	template <typename T>
	auto make_com_ptr(T* t) -> com_ptr<T>
	{
		return com_ptr<T>(t);
	}

	//======================================================================
	// the devil lies here.
	//======================================================================
	namespace detail
	{
		// there is one global d3d device, and one immeidate context
		extern ID3D11Device* d3d_device_;
		extern ID3D11DeviceContext* d3d_immediate_context_;
		extern std::mutex immediate_context_mutex_;

		// this is the device context for this thread
		extern __declspec(thread) ID3D11DeviceContext* d3d_local_context_;


		// dxgi factory
		extern com_ptr<IDXGIFactory1> dxgi_factory_;

		// dxgi device for the d3d-device
		extern com_ptr<IDXGIDevice1> dxgi_device_;

		// dxgi adapters
		extern std::vector<com_ptr<IDXGIAdapter1>> dxgi_adapters_;
		extern com_ptr<IDXGIAdapter1> dxgi_primary_adapter_;

		// outputs/surface for primary adapter
		extern std::vector<com_ptr<IDXGIOutput>> dxgi_primary_adaptor_outputs_;
		extern com_ptr<IDXGIOutput> dxgi_primary_output_;
		extern com_ptr<IDXGISurface> dxgi_primary_surface_;




		inline auto is_prime_thread() -> bool { return d3d_local_context_ == d3d_immediate_context_; }

		struct scoped_async_immediate_context_t
		{
			scoped_async_immediate_context_t();
			~scoped_async_immediate_context_t();

			auto operator -> () const -> ID3D11DeviceContext*;
		};
	}

	auto setup_d3d_device() -> void;
	auto teardown_d3d_device() -> void;
	
//======================================================================
} // namespace voodoo
} // namespace shiny
//======================================================================
#endif
//======================================================================
