#include <atma/string.hpp>
#include <atma/intrusive_ptr.hpp>
#include <atma/math/vector4f.hpp>
#include <atma/algorithm.hpp>
#include <atma/enable_if.hpp>
#include <atma/math/intersection.hpp>
#include <atma/bind.hpp>
#include <atma/math/vector4i.hpp>
#include <atma/filesystem/file.hpp>

#include <filesystem>




namespace shelf
{
	enum class path_type_t
	{
		dir,
		file,
		symlink
	};

	struct path_t;
	using  path_ptr = std::unique_ptr<path_t>; // atma::intrusive_ptr<path_t>;

	// path
	struct path_t
	{
		path_t();
		path_t(atma::string const&);
		path_t(path_t const&);

		auto to_string() const -> atma::string;

		auto is_file() const -> bool;

	private:
		path_t(atma::string const&, atma::string::const_iterator const&);

	private:
		atma::string name_;
		path_type_t type_;
		path_ptr child_;
	};


	path_t::path_t()
	{
	}

	path_t::path_t(atma::string const& str)
		: path_t(str, str.begin())
	{
	}

	path_t::path_t(atma::string const& str, atma::string::const_iterator const& begin)
	{
		char const* delims = "/\\";

		auto end = atma::find_first_of(str, begin, delims);
		if (end == str.end()) {
			type_ = path_type_t::file;
		}
		else {
			type_ = path_type_t::dir;
			++end;
		}

		name_ = atma::string(begin, end);

		if (end == begin)
			return;

		child_ = path_ptr(new path_t(str, end));
	}

	path_t::path_t(path_t const& rhs)
		: name_(rhs.name_), type_(rhs.type_)
	{
		if (rhs.child_)
			child_ = path_ptr(new path_t(*rhs.child_));
	}

	auto path_t::to_string() const -> atma::string
	{
		auto result = atma::string();

		for (auto t = this; t != nullptr; t = t->child_.get())
		{
			result += t->name_;
		}

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


	struct filesystem_t
	{
		auto add_mount(path_t const& virtual_path, path_t const& host_path) -> void;

	private:
		struct node_t;
		using  node_ptr = atma::intrusive_ptr<node_t>;
		using  nodes_t = std::vector<node_ptr>;
		struct mount_t;

		struct node_t : atma::ref_counted
		{
			atma::string name;
			path_type_t type;

			node_t* parent;
			nodes_t children;
		};

		struct mount_t
		{
			path_t host_path;
			path_t mount_path;
			node_ptr mount_node;
		};


	private:
		auto merge_paths(node_t*, node_t*) -> node_ptr;

	private:
		typedef std::vector<mount_t> mounts_t;

		mounts_t mounts_;
	};

	auto filesystem_t::add_mount(path_t const& mount_path, path_t const& host_path) -> void
	{
#ifdef _DEBUG
		atma::for_each(mounts_, [&](mount_t const& x) {  ATMA_ASSERT(x.host_path != host_path);  });
#endif

		// do a thing?
		// host_path   = "relative/path/";
		// host_path2  = "/absolute/path";
		// host_path3  = "/absolute/different/path";

		mounts_.push_back({
			host_path,
			mount_path,
			// something??
		});
	}

	template <typename FN>
	auto for_each_line(atma::filesystem::file_t& file, size_t maxsize, FN&& fn) -> void
	{
		auto buf = atma::typed_unique_memory_t<char>(maxsize);

		size_t i = 0;
		while (i != maxsize) {
			auto pos = i++;
			file.read(buf.begin() + pos, 1);
			if (buf[pos] == '\n')
				break;
		}

		fn(buf.begin(), i);
	}


}

namespace math = atma::math;


#include <atma/math/aabc.hpp>
#include <atma/unique_memory.hpp>
#include <array>


struct giraffe_t
{
	atma::typed_unique_memory_t<giraffe_t> things;
};





template <typename T>
struct default_octree_subdivider_t
{
	auto subdivide(uint depth, atma::math::vector4f const& bounds, T const& x) -> void
	{
		
	}
};

struct octree_allocate_tag
{
	explicit octree_allocate_tag(uint levels) : levels(levels) {}
	uint levels;
};

struct octree_t
{
	octree_t() {}

	octree_t(octree_allocate_tag t);

	auto insert(math::triangle_t const&) -> bool;

	auto insert_point(math::vector4f const&) -> bool;
	struct node_t;

private:
	node_t* root_;

private:

private:
	
};

struct octree_t::node_t
{
	node_t()
	{}

	node_t(math::aabc_t const& aabc)
		: aabc(aabc)
	{}

	node_t(octree_allocate_tag tag, math::aabc_t const& aabc)
		: aabc(aabc)
	{
		imem_allocate(tag);
	}

	atma::math::aabc_t aabc;
	atma::math::triangle_t data;

	auto inbounds(math::vector4f const& point) -> bool;
	auto insert(math::triangle_t const&) -> bool;

private:
	auto child_ref(uint idx) -> node_t& { return reinterpret_cast<node_t*>(buf_.begin())[idx]; }

	auto imem_allocate(octree_allocate_tag tag) -> void
	{
		if (tag.levels == 0)
			return;

		ATMA_ASSERT(buf_.empty());
		auto newbuf = atma::typed_unique_memory_t<node_t>{8};
		std::swap(buf_, newbuf);

		auto subtag = octree_allocate_tag{tag.levels - 1};
		for (auto i = 0u; i != 8u; ++i)
			new (&child_ref(i)) node_t {subtag, aabc.octant_of(i)};
	}

#if 0
	auto oct_split() -> void
	{
		imem_allocate(octree_allocate_tag{1});

		for (auto const& x : data_)
			insert(x);
	}
#endif

	auto is_leaf() const -> bool { return buf_.empty(); }

	template <typename FN>
	auto imem_for_each(FN&& fn) -> void
	{
		for (auto i = reinterpret_cast<node_t*>(buf_.begin()); i != reinterpret_cast<node_t*>(buf_.end()); ++i)
			fn(*i);
	}

private:
	atma::typed_unique_memory_t<node_t> buf_;

	std::vector<math::triangle_t> data_;
};




octree_t::octree_t(octree_allocate_tag t)
	: root_( new node_t{t, math::aabc_t{}} )
{};

auto octree_t::insert(math::triangle_t const& tri) -> bool
{
	return root_->insert(tri);
}

auto octree_t::node_t::insert(math::triangle_t const& tri) -> bool
{
	if (!atma::math::intersect_aabc_triangle(aabc, tri))
	{
		return false;
	}

	data_.push_back(tri);

	if (!buf_.empty())
		imem_for_each([&tri](node_t& x) {
			x.insert(tri);
		});

	return true;
}

auto octree_t::node_t::inbounds(math::vector4f const& point) -> bool
{
	return aabc.inside(point);
}


#include <dust/runtime.hpp>
#include <dust/context.hpp>

auto go() -> void
{
	auto oct = octree_t{octree_allocate_tag{4}};

	auto tri = math::triangle_t{
		math::point4f(0.1f, 0.1f, 0.1f),
		math::point4f(0.4f, 0.2f, 0.15f),
		math::point4f(0.23f, 0.56f, 0.44f)};

	auto tri2 = math::triangle_t {
		math::point4f(0.6f, 0.6f, 0.6f),
		math::point4f(0.6f, 0.2f, 0.1f),
		math::point4f(0.1f, 0.4f, 0.6f)};

	auto r = oct.insert(tri);
	auto r2 = oct.insert(tri2);
}


struct obj_model_t
{
	obj_model_t(atma::filesystem::file_t&);

private:
	using verts_t = std::vector<math::vector4f>;
	using faces_t = std::vector<math::vector4i>;

	verts_t verts_;
	faces_t faces_;
};

obj_model_t::obj_model_t(atma::filesystem::file_t& file)
{
	//namespace rgx = atma::regex;

	shelf::for_each_line(file, 128, [](char const* str, size_t size) {
		float x, y, z;
		//rgx::parse(str, size) << rgx::str("v: ") << capture<float>(x) << 
	});
}

auto objfile_reader() -> void
{
	
}

auto voxelize() -> void
{
	
}


int main()
{
	

	go();
}
