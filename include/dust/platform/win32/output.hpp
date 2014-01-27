#ifndef DUST_PLATFORM_WIN32_OUTPUT_HPP
#define DUST_PLATFORM_WIN32_OUTPUT_HPP
//======================================================================
#include <dust/platform/win32/dxgi_fwd.hpp>
//======================================================================
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

	struct output_t : atma::ref_counted<output_t>
	{
		auto native_resolution() const -> std::pair<uint32_t, uint32_t>;
		
		auto dxgi_output() const -> platform::dxgi_output_ptr const& { return dxgi_output_; }

	private:
		platform::dxgi_output_ptr dxgi_output_;
	};
	

	//======================================================================
	// primary-output
	//======================================================================
	uint32_t const primary_output = 0;

//======================================================================
} // namespace dust
//======================================================================
#endif
//======================================================================
