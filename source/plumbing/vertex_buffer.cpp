#include <shiny/plumbing/vertex_buffer.hpp>


shiny::plumbing::vertex_buffer_t::vertex_buffer_t( usage use, void* data, unsigned int data_size, bool shadow )
 : use_(use), data_(data), data_size_(data_size), shadowing_(shadow)
{
	D3D11_BUFFER_DESC buffer_desc;

	switch (use_) {
		case usage::general: buffer_desc.Usage = D3D11_USAGE_DEFAULT; break;
		case usage::immutable: buffer_desc.Usage = D3D11_USAGE_IMMUTABLE; break;
		case usage::updated_often: buffer_desc.Usage = D3D11_USAGE_DYNAMIC; break;
		case usage::staging: buffer_desc.Usage = D3D11_USAGE_STAGING; break;
	}

	buffer_desc.ByteWidth = data_size_;
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;
}




shiny::plumbing::vertex_buffer_t::lock_t::lock_t(lock_t&& rhs)
 : owner_(rhs.owner_), data_(rhs.data_), data_size_(rhs.data_size_)
{
	rhs.owner_ = nullptr;
	rhs.data_ = nullptr;
	rhs.data_size_ = 0;
}

shiny::plumbing::vertex_buffer_t::lock_t::lock_t( vertex_buffer_t* owner, void* data, unsigned int data_size )
 : owner_(owner), data_(data), data_size_(data_size)
{
/*
	if (device->Map(d3d_buffer_, 0, D3D11_MAP_WRITE, 0, &d3d_resource_) != S_OK) {
		data_ = nullptr;
	}
*/
}

bool shiny::plumbing::vertex_buffer_t::lock_t::valid() const
{
	return data_ != nullptr;
}
