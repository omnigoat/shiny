#pragma once

#include <lion/filesystem.hpp>

#include <atma/intrusive_ptr.hpp>
#include <atma/vector.hpp>
#include <atma/atomic.hpp>
#include <atma/handle_table.hpp>
#include <atma/enable_if.hpp>

#include <tuple>
#include <type_traits>


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
	struct asset_t
	{
		auto path() const -> path_t const& { return path_; }
		virtual ~asset_t() {}
	private:
		path_t path_;
	};
}



// asset_handle_t / asset_weak_handle_t
namespace lion
{
	template <typename T>
	struct asset_handle_t
	{
		constexpr asset_handle_t();
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

		auto operator -> () const -> T const* { return (T const*)library_->find(id_)->asset.get(); }
		auto operator -> () -> T*             { return (T      *)library_->find(id_)->asset.get(); }
		auto operator *() const -> T const&   { return *this->operator ->(); }
		auto operator *() -> T&               { return *this->operator ->(); }

		operator bool() const { return id_ != 0; }

		auto library() const -> asset_library_t* { return library_; }
		auto id() const -> uint32 { return id_; }

	private:
		asset_handle_t(asset_library_t* library, uint32 id)
			: library_{library}, id_{id}
		{}

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
	inline constexpr asset_handle_t<T>::asset_handle_t()
	{}

	template <typename T>
	inline asset_handle_t<T>::asset_handle_t(asset_handle_t const& rhs)
		: library_{rhs.library_}, id_{rhs.id_}
	{
		library_->table_.retain(id_);
	}

	template <typename T>
	inline asset_handle_t<T>::asset_handle_t(asset_handle_t&& rhs)
		: library_{rhs.library_}, id_{rhs.id_}
	{
		rhs.id_ = 0;
	}

	template <typename T>
	inline asset_handle_t<T>::asset_handle_t(asset_weak_handle_t<T> const& rhs)
		: library_{rhs.library_}
		, id_{rhs.expired() ? 0 : rhs.id_}
	{
		library_->table_.release(id_);
	}

	template <typename T>
	template <typename Y, typename>
	inline asset_handle_t<T>::asset_handle_t(asset_handle_t<Y> const& rhs)
		: library_{rhs.library_}, id_{rhs.id_}
	{
		//static_assert(std::is_convertible<Y*, T*>::value, "bad cast");
		library_->table_.retain(id_);
	}

	template <typename T>
	template <typename Y, typename>
	inline asset_handle_t<T>::asset_handle_t(asset_handle_t<Y>&& rhs)
		: library_{rhs.library_}, id_{rhs.id_}
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
		static_assert(std::is_base_of<T, Y>::value, "bad cast");
		ATMA_ASSERT(nullptr != dynamic_cast<Y const*>(&*x), "bad cast");
		return asset_handle_t<Y>{x.library_, x.id_};
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









// asset_library_t
namespace lion
{
	struct asset_library_t
	{
		asset_library_t();
		asset_library_t(vfs_t*);

		auto register_loader(atma::string const& regex) -> void;

		auto store(asset_t*) -> base_asset_handle_t;

	private: // table management
		struct storage_t;

		auto find(uint32) -> storage_t*;
		auto find(uint32) const -> storage_t const*;

		atma::handle_table_t<storage_t> table_;

	private: // vfs
		vfs_t* vfs_ = nullptr;

	private:
		template <typename> friend struct asset_handle_t;
		template <typename> friend struct asset_weak_handle_t;
	};


	struct asset_library_t::storage_t
	{
		storage_t(asset_t* a)
			: asset{a}
			, generation{}
		{}

		std::unique_ptr<asset_t> asset;
		uint8 generation;
	};

}
