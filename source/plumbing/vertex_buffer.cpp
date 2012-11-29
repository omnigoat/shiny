#include <shiny/plumbing/vertex_buffer.hpp>


shiny::plumbing::vertex_buffer_t::vertex_buffer_t( ID3D11Device* device, usage use, unsigned int data_size, bool shadow )
 : use_(use), data_(data), data_size_(data_size), shadowing_(shadow), locked_(), d3d_device_(device), d3d_buffer_()
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




