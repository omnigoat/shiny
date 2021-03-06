#pragma once

#include <lion/filesystem.hpp>

#include <atma/intrusive_ptr.hpp>
#include <atma/vector.hpp>
#include <atma/atomic.hpp>
#include <atma/handle_table.hpp>
#include <atma/enable_if.hpp>
#include <atma/function.hpp>
#include <atma/streams.hpp>
#include <atma/hash_map.hpp>

#include <tuple>
#include <type_traits>
#include <regex>
#include <set>
#include <typeindex>


// forward declares
namespace lion
{
	struct asset_t;
	struct asset_library_t;
	template <typename> struct asset_handle_t;
	template <typename> struct asset_weak_handle_t;
}


// asset_t
namespace lion
{
	struct asset_t : atma::ref_counted
	{
		asset_t()
		{}

		asset_t(path_t const& path)
			: path_(path)
		{}

		virtual ~asset_t()
		{}

		auto path() const -> path_t const& { return path_; }
		auto set_path(path_t const& p) { ATMA_ASSERT(path_.string().empty()); path_ = p; }

	private:
		path_t path_;
	};

	using asset_ptr = atma::intrusive_ptr<asset_t>;
}


// asset_handle_t / asset_weak_handle_t
namespace lion
{
	template <typename T>
	struct asset_handle_t
	{
		constexpr asset_handle_t() = default;
		asset_handle_t(asset_handle_t const& rhs);
		asset_handle_t(asset_handle_t&& rhs);
		~asset_handle_t();

		explicit asset_handle_t(asset_weak_handle_t<T> const&);

		template <typename Y, typename = std::enable_if_t<std::is_convertible<Y*, T*>::value>>
		asset_handle_t(asset_handle_t<Y> const& rhs);
		template <typename Y, typename = atma::enable_if<std::is_convertible<Y*, T*>>>
		asset_handle_t(asset_handle_t<Y>&& rhs);


		auto operator = (asset_handle_t const& rhs) -> asset_handle_t&;
		auto operator = (asset_handle_t&& rhs) -> asset_handle_t&;

		template <typename Y, typename = std::enable_if_t<std::is_convertible<Y*, T*>::value>>
		auto operator = (asset_handle_t<Y> const& rhs) -> asset_handle_t&;
		template <typename Y, typename = std::enable_if_t<std::is_convertible<Y*, T*>::value>>
		auto operator = (asset_handle_t<Y>&& rhs) -> asset_handle_t&;

		//auto operator -> () const -> T* { return (T      *)library_->find(id_)->asset.get(); }
		auto operator -> () const -> atma::intrusive_ptr<T> { return atma::ptr_cast_static<T>(library_->find(id_)->asset); }
		auto operator *() const -> T& { return *(T*)library_->find(id_)->asset.get(); }

		operator bool() const { return id_ != 0; }

		auto library() const -> asset_library_t* { return library_; }
		auto id() const -> uint32 { return id_; }
		auto address() const -> T const* { return operator ->(); }

	private:
		asset_handle_t(asset_library_t* library, uint32 id, bool incref = false)
			: library_{library}, id_{id}
		{
			if (incref)
				library_->table_.retain(id_);
		}

	private:
		asset_library_t* library_ = nullptr;
		uint32 id_ = 0;

		friend struct asset_library_t;
		template <typename> friend struct asset_handle_t;
		template <typename> friend struct asset_weak_handle_t;

		template <typename Y, typename Y2>
		friend auto polymorphic_asset_cast(asset_handle_t<Y2> const&) -> asset_handle_t<Y>;
	};

	using base_asset_handle_t = asset_handle_t<asset_t>;
}


namespace lion
{
	template <typename T>
	struct asset_weak_handle_t
	{
		constexpr asset_weak_handle_t();
		asset_weak_handle_t(asset_weak_handle_t const&);
		asset_weak_handle_t(asset_weak_handle_t&&);
		~asset_weak_handle_t();

		asset_weak_handle_t(asset_handle_t<T> const&);

		template <typename Y, typename = std::enable_if_t<std::is_convertible<Y*, T*>::value>>
		asset_weak_handle_t(asset_weak_handle_t<Y> const& rhs);
		template <typename Y, typename = atma::enable_if<std::is_convertible<Y*, T*>>>
		asset_weak_handle_t(asset_weak_handle_t<Y>&& rhs);

		auto operator = (asset_weak_handle_t const&) -> asset_weak_handle_t&;
		auto operator = (asset_weak_handle_t&&) -> asset_weak_handle_t&;

		template <typename Y, typename = std::enable_if_t<std::is_convertible<Y*, T*>::value>>
		auto operator = (asset_weak_handle_t<Y> const& rhs) -> asset_weak_handle_t&;
		template <typename Y, typename = std::enable_if_t<std::is_convertible<Y*, T*>::value>>
		auto operator = (asset_weak_handle_t<Y>&& rhs) -> asset_weak_handle_t&;

		auto library() const -> asset_library_t* { return library_; }
		auto id() const -> uint32 { return id_; }
		auto address() const -> T const* { return operator ->(); }

		auto expired() const -> bool;
		auto lock() const -> asset_handle_t<T>;

	private:
		asset_library_t* library_ = nullptr;
		uint32 id_ = 0;
	};
}


namespace lion
{
	template <typename T>
	struct asset_raw_handle_t
	{
		constexpr asset_raw_handle_t();
		asset_raw_handle_t(asset_raw_handle_t const&);
		asset_raw_handle_t(asset_raw_handle_t&&);

		asset_raw_handle_t(asset_handle_t<T> const&);
		asset_raw_handle_t(asset_weak_handle_t<T> const&);

		template <typename Y, typename = std::enable_if_t<std::is_convertible<Y*, T*>::value>>
		asset_raw_handle_t(asset_raw_handle_t<Y> const& rhs);
		template <typename Y, typename = atma::enable_if<std::is_convertible<Y*, T*>>>
		asset_raw_handle_t(asset_raw_handle_t<Y>&& rhs);

		auto operator = (asset_raw_handle_t const&) -> asset_raw_handle_t&;
		auto operator = (asset_raw_handle_t&&) -> asset_raw_handle_t&;

		template <typename Y, typename = std::enable_if_t<std::is_convertible<Y*, T*>::value>>
		auto operator = (asset_raw_handle_t<Y> const& rhs) -> asset_raw_handle_t&;
		template <typename Y, typename = std::enable_if_t<std::is_convertible<Y*, T*>::value>>
		auto operator = (asset_raw_handle_t<Y>&& rhs) -> asset_raw_handle_t&;

		auto library() const -> asset_library_t* { return library_; }
		auto id() const -> uint32 { return id_; }
		auto address() const -> T const* { return operator ->(); }

	private:
		asset_library_t* library_ = nullptr;
		uint32 id_ = 0;
	};
}


namespace lion
{

	template <typename T>
	inline asset_handle_t<T>::asset_handle_t(asset_handle_t const& rhs)
		: library_{rhs.library_}
		, id_{rhs.id_}
	{
		library_->table_.retain(id_);
	}

	template <typename T>
	inline asset_handle_t<T>::asset_handle_t(asset_handle_t&& rhs)
		: library_{rhs.library_}
		, id_{rhs.id_}
	{
		rhs.id_ = 0;
	}

	template <typename T>
	inline asset_handle_t<T>::asset_handle_t(asset_weak_handle_t<T> const& rhs)
		: library_{rhs.library_}
		, id_{rhs.expired() ? 0 : rhs.id_}
	{
		library_->table_.retain(id_);
	}

	template <typename T>
	template <typename Y, typename>
	inline asset_handle_t<T>::asset_handle_t(asset_handle_t<Y> const& rhs)
		: library_{rhs.library_}
		, id_{rhs.id_}
	{
		library_->table_.retain(id_);
	}

	template <typename T>
	template <typename Y, typename>
	inline asset_handle_t<T>::asset_handle_t(asset_handle_t<Y>&& rhs)
		: library_{rhs.library_}
		, id_{rhs.id_}
	{
		rhs.id_ = 0;
	}

	template <typename T>
	inline asset_handle_t<T>::~asset_handle_t()
	{
		library_->table_.release(id_);
	}

	template <typename T>
	inline auto asset_handle_t<T>::operator = (asset_handle_t const& rhs) -> asset_handle_t&
	{
		this->~asset_handle_t();
		new (this) asset_handle_t{rhs};
		return *this;
	}

	template <typename T>
	inline auto asset_handle_t<T>::operator = (asset_handle_t&& rhs) -> asset_handle_t&
	{
		this->~asset_handle_t();
		new (this) asset_handle_t{std::move(rhs)};
		return *this;
	}

	template <typename T>
	template <typename Y, typename>
	inline auto asset_handle_t<T>::operator = (asset_handle_t<Y> const& rhs) -> asset_handle_t&
	{
		this->~asset_handle_t();
		new (this) asset_handle_t{rhs};
		return *this;
	}

	template <typename T>
	template <typename Y, typename>
	inline auto asset_handle_t<T>::operator = (asset_handle_t<Y>&& rhs) -> asset_handle_t&
	{
		this->~asset_handle_t();
		new (this) asset_handle_t{std::move(rhs)};
		return *this;
	}




	template <typename Y, typename T>
	inline auto polymorphic_asset_cast(asset_handle_t<T> const& x) -> asset_handle_t<Y>
	{
		if (x.library_ == nullptr || x.id_ == 0)
			return asset_handle_t<Y>{x.library_, 0};

		static_assert(std::is_base_of<T, Y>::value, "bad cast");
		ATMA_ASSERT(&*x == nullptr || nullptr != dynamic_cast<Y const*>(&*x), "bad cast");
		return asset_handle_t<Y>{x.library_, x.id_, true};
	}
}


namespace lion
{
	template <typename T>
	inline constexpr asset_weak_handle_t<T>::asset_weak_handle_t()
	{}

	template <typename T>
	inline asset_weak_handle_t<T>::asset_weak_handle_t(asset_weak_handle_t const& rhs)
		: library_{rhs.library_}, id_{rhs.id_}
	{
		library_->table_.weak_retain(id_);
	}

	template <typename T>
	inline asset_weak_handle_t<T>::asset_weak_handle_t(asset_weak_handle_t&& rhs)
		: library_{rhs.library_}, id_{rhs.id_}
	{
		rhs.id_ = 0;
	}

	template <typename T>
	inline asset_weak_handle_t<T>::asset_weak_handle_t(asset_handle_t<T> const& rhs)
		: library_{rhs.library_}, id_{rhs.id_}
	{
		library_->table_.weak_retain(id_);
	}

	template <typename T>
	template <typename Y, typename>
	inline asset_weak_handle_t<T>::asset_weak_handle_t(asset_weak_handle_t<Y> const& rhs)
		: library_{rhs.library_}, id_{rhs.id_}
	{
		static_assert(std::is_convertible<Y*, T*>::value, "bad cast");
		library_->table_.weak_retain(id_);
	}

	template <typename T>
	template <typename Y, typename>
	inline asset_weak_handle_t<T>::asset_weak_handle_t(asset_weak_handle_t<Y>&& rhs)
		: library_{rhs.library_}, id_{rhs.id_}
	{
		rhs.id_ = 0;
	}

	template <typename T>
	inline asset_weak_handle_t<T>::~asset_weak_handle_t()
	{
		library_->table_.release(id_);
	}

	template <typename T>
	inline auto asset_weak_handle_t<T>::operator = (asset_weak_handle_t const& rhs) -> asset_weak_handle_t&
	{
		this->~asset_weak_handle_t();
		new (this) asset_weak_handle_t{rhs};
		return *this;
	}

	template <typename T>
	inline auto asset_weak_handle_t<T>::operator = (asset_weak_handle_t&& rhs) -> asset_weak_handle_t&
	{
		this->~asset_weak_handle_t();
		new (this) asset_weak_handle_t{std::move(rhs)};
		return *this;
	}

	template <typename T>
	template <typename Y, typename>
	inline auto asset_weak_handle_t<T>::operator = (asset_weak_handle_t<Y> const& rhs) -> asset_weak_handle_t&
	{
		this->~asset_weak_handle_t();
		new (this) asset_weak_handle_t{rhs};
		return *this;
	}

	template <typename T>
	template <typename Y, typename>
	inline auto asset_weak_handle_t<T>::operator = (asset_weak_handle_t<Y>&& rhs) -> asset_weak_handle_t&
	{
		this->~asset_weak_handle_t();
		new (this) asset_weak_handle_t{std::move(rhs)};
		return *this;
	}

	template <typename T>
	inline auto asset_weak_handle_t<T>::expired() const -> bool
	{
		return library_->table_.expired(id_);
	}

	template <typename T>
	inline auto asset_weak_handle_t<T>::lock() const -> asset_handle_t<T>
	{
		if (expired())
			return asset_handle_t<T>{library_, 0};
		else
			return asset_handle_t<T>{library_, id_};
	}
}


namespace lion
{
	template <typename T>
	inline constexpr asset_raw_handle_t<T>::asset_raw_handle_t()
	{}

	template <typename T>
	inline asset_raw_handle_t<T>::asset_raw_handle_t(asset_raw_handle_t const& rhs)
		: library_{rhs.library_}, id_{rhs.id_}
	{}

	template <typename T>
	inline asset_raw_handle_t<T>::asset_raw_handle_t(asset_raw_handle_t&& rhs)
		: library_{rhs.library_}, id_{rhs.id_}
	{}

	template <typename T>
	inline asset_raw_handle_t<T>::asset_raw_handle_t(asset_handle_t<T> const& rhs)
		: library_{rhs.library_}, id_{rhs.id_}
	{}

	template <typename T>
	inline asset_raw_handle_t<T>::asset_raw_handle_t(asset_weak_handle_t<T> const& rhs)
		: library_{rhs.library_}, id_{rhs.id_}
	{}

	template <typename T>
	template <typename Y, typename>
	inline asset_raw_handle_t<T>::asset_raw_handle_t(asset_raw_handle_t<Y> const& rhs)
		: library_{rhs.library_}, id_{rhs.id_}
	{}

	template <typename T>
	template <typename Y, typename>
	inline asset_raw_handle_t<T>::asset_raw_handle_t(asset_raw_handle_t<Y>&& rhs)
		: library_{rhs.library_}, id_{rhs.id_}
	{
		rhs.id_ = 0;
	}

	template <typename T>
	inline auto asset_raw_handle_t<T>::operator = (asset_raw_handle_t const& rhs) -> asset_raw_handle_t&
	{
		library_ = rhs.library_;
		id_ = rhs.id_;
		return *this;
	}

	template <typename T>
	inline auto asset_raw_handle_t<T>::operator = (asset_raw_handle_t&& rhs) -> asset_raw_handle_t&
	{
		library_ = rhs.library_;
		id_ = rhs.id_;
		rhs.id_ = 0;
		return *this;
	}

	template <typename T>
	template <typename Y, typename>
	inline auto asset_raw_handle_t<T>::operator = (asset_raw_handle_t<Y> const& rhs) -> asset_raw_handle_t&
	{
		library_ = rhs.library_;
		id_ = rhs.id_;
		return *this;
	}

	template <typename T>
	template <typename Y, typename>
	inline auto asset_raw_handle_t<T>::operator = (asset_raw_handle_t<Y>&& rhs) -> asset_raw_handle_t&
	{
		library_ = rhs.library_;
		id_ = rhs.id_;
		rhs.id_ = 0;
		return *this;
	}
}


// asset_collection_t
namespace lion
{
	struct asset_collection_t
	{
		template <typename T> auto retain(asset_handle_t<T> const&) -> asset_raw_handle_t<T>;

	private:
		atma::hash_set<uint32> handles_;
	};
}

// asset_library_t
namespace lion
{
	struct asset_pattern_t
	{
		using load_callback_t   = atma::function<asset_ptr(rose::path_t const&, lion::input_stream_ptr const&)>;
		using reload_callback_t = atma::function<asset_ptr(rose::path_t const&, lion::input_stream_ptr const&)>;

		asset_pattern_t(atma::string const& regex, load_callback_t const& load)
			: path{rose::path_t{regex}.directory()}
			, regex{regex.c_str()}, load{load}
		{}

		asset_pattern_t(atma::string const& regex, load_callback_t const& load, reload_callback_t const& reload)
			: path{rose::path_t{regex}.directory()}
			, regex{regex.c_str()}, load{load}, reload{reload}
		{}

		rose::path_t path;
		std::regex regex;
		load_callback_t load;
		reload_callback_t reload;
	};

	struct asset_library_t
	{
		struct asset_type_t;

		using asset_patterns_t = atma::vector<asset_pattern_t>;
		using asset_types_t = std::set<asset_type_t>;
		using asset_type_handle_t = asset_types_t::iterator;

		asset_library_t();
		asset_library_t(vfs_t*);

		template <typename T>
		auto register_asset_type(asset_patterns_t patterns) -> asset_type_handle_t
		{
			return register_asset_type(std::type_index{typeid(T)}, std::move(patterns));
		}

		auto register_asset_type(std::type_index, asset_patterns_t) -> asset_type_handle_t;

		auto store(asset_ptr const&) -> base_asset_handle_t;
		auto retain_copy(base_asset_handle_t const&) -> base_asset_handle_t;

		auto load(rose::path_t const&) -> base_asset_handle_t;
		//auto load(asset_collection_t, atma::string const& path) -> base_asset_handle_t;

		template <typename T> auto load_as(rose::path_t const&) -> asset_handle_t<T>;


	private: // table management
		struct storage_t;

		auto find(uint32) -> storage_t*;
		auto find(uint32) const -> storage_t const*;

		atma::handle_table_t<storage_t> table_;

	private: // vfs
		vfs_t* vfs_ = nullptr;

	private: // assets
		asset_types_t asset_types_;
		std::map<rose::path_t, std::tuple<std::type_index, uint32>> pathed_assets_;

	private:
		template <typename> friend struct asset_handle_t;
		template <typename> friend struct asset_weak_handle_t;

		friend void swap(storage_t& lhs, storage_t&);
	};


	struct asset_library_t::asset_type_t
	{
		std::type_index typeidx;
		asset_patterns_t patterns;
	};

	auto operator < (asset_library_t::asset_type_t const&, asset_library_t::asset_type_t const&) -> bool;

	struct asset_library_t::storage_t
	{
		storage_t(asset_ptr const& a)
			: asset{a}
			, generation{}
			, semaphore{}
			, next_generation_handle{}
		{}

		~storage_t() {}

		union {
			struct {
				asset_ptr asset;
				uint16 generation;
				uint16 semaphore;
				uint32 next_generation_handle;
			};

			atma::atomic128_t atom;
		};
	};

	void swap(asset_library_t::storage_t& lhs, asset_library_t::storage_t&);

	template <typename T>
	auto asset_library_t::load_as(rose::path_t const& path) -> asset_handle_t<T>
	{
		auto h = load(path);
		return polymorphic_asset_cast<T>(h);
	}

}
