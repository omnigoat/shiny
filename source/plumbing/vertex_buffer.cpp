#include <shiny/plumbing/vertex_buffer.hpp>
#include <atma/assert.hpp>

shiny::plumbing::vertex_buffer_t::vertex_buffer_t( usage use, unsigned int data_size, bool shadow )
: use_(use), data_size_(data_size), shadowing_(shadow), locked_(), d3d_buffer_()
{
	D3D11_USAGE buffer_usage;
	switch (use_) {
		case usage::general: buffer_usage = D3D11_USAGE_DEFAULT; break;
		case usage::immutable: buffer_usage = D3D11_USAGE_IMMUTABLE; break;
		case usage::updated_often: buffer_usage = D3D11_USAGE_DYNAMIC; break;
		case usage::staging: buffer_usage = D3D11_USAGE_STAGING; break;
	}

	D3D11_BUFFER_DESC buffer_desc { data_size_, buffer_usage, D3D11_BIND_VERTEX_BUFFER, 0, 0, 0 };

	detail::d3d_device_->CreateBuffer(&buffer_desc, NULL, &d3d_buffer_);
	
	if (shadowing_) {
		data_.resize(data_size_);
	}
}

auto shiny::plumbing::vertex_buffer_t::reload_from_shadow_buffer() -> void
{
	ATMA_ASSERT(shadowing_);

	auto L = lock<char>(lock_type_t::write_discard);
	std::copy_n(&data_.front(), data_size_, L.begin());
}

auto shiny::plumbing::vertex_buffer_t::release_shadow_buffer() -> void
{
	ATMA_ASSERT(shadowing_);

	data_.clear();
	data_.shrink_to_fit();
	shadowing_ = false;
}

auto shiny::plumbing::vertex_buffer_t::aquire_shadow_buffer(bool pull_from_hardware) -> void
{
	ATMA_ASSERT(!shadowing_);
	
	data_.resize(data_size_);

	/*if (pull_from_hardware) {
		ATMA_ASSERT(use_ == usage::general || use_ == usage::updated_often);
	}*/

	shadowing_ = true;
}
