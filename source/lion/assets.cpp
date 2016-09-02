#include <lion/assets.hpp>

#include <memory>

lion::asset_library_t::asset_library_t()
	//: types_{256}
{
	
}

lion::asset_library_t::asset_library_t(vfs_t* vfs)
	: vfs_{vfs}
{}

auto lion::asset_library_t::store(asset_t* a) -> base_asset_handle_t
{
	auto h = table_.construct(a);

	return base_asset_handle_t{this, h};
}

auto lion::asset_library_t::find(uint32 h) -> storage_t*
{
	auto st = table_.get(h);
	return st;
}

auto lion::asset_library_t::find(uint32 h) const -> storage_t const*
{
	auto st = table_.get(h);
	return st;
}

auto lion::asset_library_t::register_asset_type(asset_patterns_t patterns) -> asset_type_handle_t
{
	auto r = asset_types_.insert(asset_type_t{std::move(patterns)});
	ATMA_ASSERT(r.second, "bad insert");
	return r.first;
}

auto lion::asset_library_t::load(path_t const& path) -> base_asset_handle_t
{
	for (auto const& at : asset_types_)
	{
		for (auto const& p : at.patterns)
		{
			if (std::regex_match(path.c_str(), p.regex))
			{
				auto stream = vfs_->open(path);
				auto istream = atma::stream_cast<atma::input_bytestream_t>(stream);
				if (stream->stream_status() != atma::stream_status_t::error)
				{
					p.callback(path, istream);
				}
			}
		}
	}

	return {};
}

auto lion::operator < (lion::asset_library_t::asset_type_t const& lhs, lion::asset_library_t::asset_type_t const& rhs) -> bool
{
	return std::lexicographical_compare(
		lhs.patterns.begin(), lhs.patterns.end(),
		rhs.patterns.begin(), rhs.patterns.end(),
		[](lion::asset_pattern_t const& lhs, lion::asset_pattern_t const& rhs) {
			return lhs.callback < rhs.callback;
		});
}
