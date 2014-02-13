#pragma once
//======================================================================
#include <dust/platform/win32/dxgi_fwd.hpp>

#include <atma/com_ptr.hpp>
#include <atma/intrusive_ptr.hpp>
//======================================================================
namespace dust {
//======================================================================
	
	//======================================================================
	// output_t (win32/dxgi)
	// ------------------------
	//   represents one display device. wrapper around IDXGIOutput.
	//======================================================================
	struct output_t;
	typedef atma::intrusive_ptr<output_t> output_ptr;

	struct output_t : atma::ref_counted
	{
		auto native_resolution() const -> std::pair<uint32, uint32>;
		
		auto dxgi_output() const -> platform::dxgi_output_ptr const& { return dxgi_output_; }

	private:
		platform::dxgi_output_ptr dxgi_output_;
	};




	struct runtime_t;

	namespace platform
	{
		auto output_at(runtime_t const&, dxgi_adapter_ptr const&, uint32 output_index) -> dxgi_output_ptr;
	}

//======================================================================
} // namespace dust
//======================================================================
