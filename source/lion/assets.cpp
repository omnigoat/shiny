#include <lion/assets.hpp>

#include <atma/ranges/filter.hpp>

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
	auto os = h.library()->find(h.id());
	auto h2 = table_.construct(os->asset);
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

auto lion::asset_library_t::register_asset_type(std::type_index idx, asset_patterns_t patterns) -> asset_type_handle_t
{
	auto [asset_type, good_insert] = asset_types_.insert(asset_type_t{idx, std::move(patterns)});
	ATMA_ASSERT(good_insert, "asset type already registered for loading");
	
	for (auto const& pattern : atma::filter(&asset_pattern_t::reload, asset_type->patterns))
	{
		vfs_->add_filewatch(pattern.path, [&, asset_type](rose::path_t const& path, rose::file_change_t change, lion::input_stream_ptr const& stream)
		{
			if (change != rose::file_change_t::changed)
				return;
			else if (auto candidate = pathed_assets_.find(path); candidate == pathed_assets_.end())
				return;
			else if (auto [typeidx, old_handle] = candidate->second; asset_type->typeidx != typeidx)
				return;
			else if (auto asset = pattern.reload(path, stream))
				table_.swap(old_handle, store(asset).id());
			
		});
	}

	return asset_type;
}

auto lion::asset_library_t::load(path_t const& path) -> base_asset_handle_t
{
	for (auto const& at : asset_types_)
	{
		for (auto const& p : at.patterns)
		{
			if (!std::regex_match(path.c_str(), p.regex))
				continue;

			atma::string filepath;

			auto stream = vfs_->open(path, &filepath);
			if (stream && stream->stream_status() != atma::stream_status_t::error)
			{
				if (auto istream = atma::stream_cast<atma::input_bytestream_t>(stream))
				{
					auto a = p.load(filepath, istream);
					auto h = store(a);
					auto r = pathed_assets_.insert(std::make_pair(filepath, std::make_tuple(at.typeidx, h.id())));
					ATMA_ASSERT(r.second);
					return h;
				}
			}
		}
	}

	return {};
}

auto lion::operator < (lion::asset_library_t::asset_type_t const& lhs, lion::asset_library_t::asset_type_t const& rhs) -> bool
{
	return lhs.typeidx < rhs.typeidx || (lhs.typeidx == rhs.typeidx &&
		std::lexicographical_compare(
		lhs.patterns.begin(), lhs.patterns.end(),
		rhs.patterns.begin(), rhs.patterns.end(),
		[](lion::asset_pattern_t const& lhs, lion::asset_pattern_t const& rhs) {
			return lhs.load < rhs.load;
		}));
}


namespace lion
{
	void swap(lion::asset_library_t::storage_t& lhs, lion::asset_library_t::storage_t& rhs)
	{
		static_assert(sizeof(lhs.asset) == sizeof(uintptr), "bad sizes");

		// this could deadlock if two threads are trying to swap
		// the same two assets in the opposite order
		while (!atma::atomic_compare_exchange(&lhs.semaphore, (uint16)0u, (uint16)1u));
		while (!atma::atomic_compare_exchange(&rhs.semaphore, (uint16)0u, (uint16)1u));

		atma::atomic128_t nlhs;
		nlhs.uptr[0] = (uintptr)rhs.asset.get();
		nlhs.ui16[4] = lhs.generation + 1;
		nlhs.ui16[5] = lhs.semaphore;
		nlhs.ui32[3] = lhs.next_generation_handle;

		atma::atomic128_t nrhs;
		nrhs.uptr[0] = (uintptr)lhs.asset.get();
		nrhs.ui16[4] = rhs.generation + 1;
		nrhs.ui16[5] = rhs.semaphore;
		nrhs.ui32[3] = rhs.next_generation_handle;

		atma::atomic_exchange(&lhs.atom, nlhs);
		atma::atomic_exchange(&rhs.atom, nrhs);

		atma::atomic_pre_decrement(&lhs.semaphore);
		atma::atomic_pre_decrement(&rhs.semaphore);
	}
}
