#include <shiny/plumbing/vertex_buffer.hpp>
#include <atma/assert.hpp>

using shiny::plumbing::vertex_buffer_t;
using shiny::plumbing::gpu_access_t;
using shiny::plumbing::cpu_access_t;

vertex_buffer_t::vertex_buffer_t(gpu_access_t gpua, cpu_access_t cpua, bool shadow, unsigned int data_size)
: vertex_buffer_t(gpua, cpua, shadow, data_size, nullptr)
{
}

vertex_buffer_t::vertex_buffer_t(gpu_access_t gpua, cpu_access_t cpua, bool shadow, unsigned int data_size, void* data)
: d3d_buffer_(), gpu_access_(gpua), cpu_access_(cpua), data_size_(data_size), shadowing_(shadow), locked_()
{
	voodoo::create_buffer(&d3d_buffer_, gpu_access_, cpu_access_, data_size, data);
	
	if (shadowing_) {
		ATMA_ASSERT(data);
		data_.assign(reinterpret_cast<char*>(data), reinterpret_cast<char*>(data) + data_size_);
	}
}

vertex_buffer_t::~vertex_buffer_t()
{
	if (d3d_buffer_) {
		d3d_buffer_->Release();
	}
}

auto vertex_buffer_t::is_shadowing() const -> bool
{
	return shadowing_;
}

auto vertex_buffer_t::reload_from_shadow_buffer() -> void
{
	ATMA_ASSERT(!locked_);
	ATMA_ASSERT(shadowing_);
	
	auto L = lock<char>(lock_type_t::write_discard);
	std::copy_n(&data_.front(), data_size_, L.begin());
}

auto vertex_buffer_t::release_shadow_buffer() -> void
{
	ATMA_ASSERT(!locked_);
	ATMA_ASSERT(shadowing_);
	
	data_.clear();
	data_.shrink_to_fit();
	shadowing_ = false;
}

auto vertex_buffer_t::aquire_shadow_buffer(bool pull_from_hardware) -> void
{
	ATMA_ASSERT(!locked_);
	ATMA_ASSERT(!shadowing_);
	
	data_.resize(data_size_);

	if (pull_from_hardware) {
		auto L = lock<char>(lock_type_t::read);
		std::copy(L.begin(), L.end(), data_.begin());
	}

	shadowing_ = true;
}

auto vertex_buffer_t::rebase_from_buffer(vertex_buffer_t::data_t&& buffer, bool upload_to_hardware) -> void
{
	ATMA_ASSERT(!locked_);
	ATMA_ASSERT(data_size_ == buffer.size());

	data_.swap(buffer);
	if (upload_to_hardware)
		reload_from_shadow_buffer();
}

auto vertex_buffer_t::rebase_from_buffer(vertex_buffer_t::data_t const& buffer, bool upload_to_hardware) -> void
{
	ATMA_ASSERT(!locked_);
	ATMA_ASSERT(data_size_ == buffer.size());

	std::copy(buffer.begin(), buffer.end(), data_.begin());

	if (upload_to_hardware)
		reload_from_shadow_buffer();
}


