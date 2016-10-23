#include <lion/assets.hpp>

#include <memory>

lion::asset_library_t::asset_library_t()
	//: types_{256}
{
	
}

lion::asset_library_t::asset_library_t(vfs_t* vfs)
	: vfs_{vfs}
{}

auto lion::asset_library_t::store(asset_ptr const& a) -> base_asset_handle_t
{
	auto h = table_.construct(a);
	return base_asset_handle_t{this, h};
}

auto lion::asset_library_t::retain_copy(base_asset_handle_t const& h) -> base_asset_handle_t
{
	auto h2 = table_.construct(asset_ptr::null);
	auto os = h.library()->find(h.id());
	auto& n = find(h2)->asset;
	n = os->asset;
	return base_asset_handle_t{this, h2};
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
	
	for (auto const& pattern : r.first->patterns)
	{
		if (!pattern.reload)
			continue;

		vfs_->add_filewatch(pattern.path, [&](rose::path_t const& path, rose::file_change_t change, lion::input_stream_ptr const& stream)
		{
			if (change != rose::file_change_t::changed)
				return;

			auto candidate = pathed_assets_.find(path);
			if (candidate == pathed_assets_.end())
				return;

			auto old_handle = candidate->second;

			if (auto asset = pattern.reload(path, stream))
			{
				auto h = store(asset);
				table_.swap(old_handle, h.id());
			}
		});
	}

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
				atma::string filepath;

				auto stream = vfs_->open(path, &filepath);
				if (stream && stream->stream_status() != atma::stream_status_t::error)
				{
					if (auto istream = atma::stream_cast<atma::input_bytestream_t>(stream))
					{
						auto a = p.load(filepath, istream);
						auto h = store(a);
						auto r = pathed_assets_.insert(std::make_pair(filepath, h.id()));
						ATMA_ASSERT(r.second);
						return h;
					}
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
			return lhs.load < rhs.load;
		});
}
