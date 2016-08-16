#include <lion/assets.hpp>

#include <memory>

lion::asset_library_t::asset_library_t()
	//: types_{256}
{
	
}

auto lion::asset_library_t::store(asset_t* a) -> base_asset_handle_t
{
	auto h = table_.construct(a);

	return base_asset_handle_t{this, h};
}

auto lion::asset_library_t::find(base_asset_handle_t const& h) -> storage_t*
{
	auto st = table_.get(h.id_);
	return st;
}

auto lion::asset_library_t::find(base_asset_handle_t const& h) const -> storage_t const*
{
	auto st = table_.get(h.id_);
	return st;
}
