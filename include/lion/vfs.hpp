#pragma once

#include <atma/intrusive_ptr.hpp>


namespace lion
{
	enum class path_type_t
	{
		dir,
		file,
		symlink
	};

	struct path_t;
	using  path_ptr  = atma::intrusive_ptr<path_t>;
	using  path_wptr = path_t*;

	struct abstract_filesystem_t : atma::ref_counted
	{
		virtual auto generate_path(atma::string const&) -> path_ptr const& = 0;
	};

	using abstract_filesystem_ptr = atma::intrusive_ptr<abstract_filesystem_t>;

	auto split_on_any(atma::string const& str, atma::string const& delims) -> atma::vector<atma::string>
	{
		atma::vector<atma::string> r;

		auto i = str.begin();
		while (i != str.end())
		{
			auto e = atma::find_first_of(str, i, delims.c_str());

			r.push_back(atma::string{i, e});
			i = ++e;
		}
		
		return r;
	}

	struct physical_filesystem_t : abstract_filesystem_t
	{
		auto generate_path(atma::string const& p) -> path_ptr const& override
		{
			auto things = split_on_any(p, "/\\");
		}

	private:
		path_ptr root_;
	};

	// path
	struct path_t : atma::ref_counted
	{
		auto to_string() const -> atma::string;

		auto is_file() const -> bool;

	private:
		path_t(abstract_filesystem_ptr const&, path_t* parent, atma::string const&);

	private:
		using children_t = atma::vector<path_t>;

		abstract_filesystem_ptr fs_;
		atma::string name_;
		path_type_t type_;

		path_t* parent_;
		children_t children_;
	};



	char const* const delims = "\\/";

	inline auto split_path(atma::string const& str) -> void
	{
	}

	auto path_t::to_string() const -> atma::string
	{
		auto result = atma::string();

		//for (auto t = this; t != nullptr; t = t->child_.get())
		//{
		//	result += t->name_;
		//}

		return result;
	}

	inline auto operator == (path_t const& lhs, path_t const& rhs) -> bool
	{
		return lhs.to_string() == rhs.to_string();
	}

	inline auto operator != (path_t const& lhs, path_t const& rhs) -> bool
	{
		return !operator == (lhs, rhs);
	}


	



	// 
	struct vfs_t
	{
	};
}
