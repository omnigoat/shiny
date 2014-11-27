#include <atma/string.hpp>
#include <atma/intrusive_ptr.hpp>
#include <atma/math/vector4f.hpp>
#include <atma/algorithm.hpp>
#include <atma/enable_if.hpp>

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
}

namespace math = atma::math;


#include <array>

template <typename T>
struct default_octree_subdivider_t
{
	auto subdivide(uint depth, atma::math::vector4f const& bounds, T const& x) -> void
	{
		
	}
};

struct octree_t
{
	octree_t() {}

	//auto add_triangle(T const&, math::vector4f const&, math::vector4f const&, math::vector4f const&) -> void;

	auto insert_point(math::vector4f const&) -> bool;
	struct node_t;
	//node_t root_;

private:

private:

private:
	
};

struct triangle_t
{
	math::vector4f v0, v1, v2;
};

struct box_t
{
	math::vector4f origin;
	math::vector4f half_extents;

	static auto from_minmax(math::vector4f const& min, math::vector4f const& max) -> box_t
	{
		return box_t{
			(min + max) / 2.f,
			(max - min) / 2.f};
	}
};

auto intersect_aabb_box(math::vector4f const& aabb, box_t const& box) -> bool
{
	return !(
		aabb.x + aabb.w < box.origin.x - box.half_extents.x ||
		aabb.x - aabb.w > box.origin.x + box.half_extents.x ||
		aabb.y + aabb.w < box.origin.y - box.half_extents.y ||
		aabb.y - aabb.w > box.origin.y + box.half_extents.y ||
		aabb.z + aabb.w < box.origin.z - box.half_extents.z ||
		aabb.z - aabb.w > box.origin.z + box.half_extents.z)
		;
}


auto project(math::vector4f const& axis, math::vector4f const* points, size_t point_count, float& min, float& max) -> void
{
	min = FLT_MAX;
	max = -FLT_MAX;

	for (auto p = points; p != points + point_count; ++p)
	{
		float v = math::dot_product(axis, *p);
		if (v < min) min = v;
		if (v > max) max = v;
	}
}


auto intersect_aabb_triangle(math::vector4f const& aabb, triangle_t const& tri) -> bool
{
	float box_min = 0.f, box_max = 0.f;
	float tri_min = 0.f, tri_max = 0.f;
	
	math::vector4f const tri_edges[3] ={
		tri.v1 - tri.v0,
		tri.v2 - tri.v0,
		tri.v2 - tri.v1,
	};


	math::vector4f const box_verts[] = {
			{aabb.x - aabb.w, aabb.y - aabb.w, aabb.z - aabb.w, 1.f},
			{aabb.x + aabb.w, aabb.y - aabb.w, aabb.z - aabb.w, 1.f},
			{aabb.x - aabb.w, aabb.y + aabb.w, aabb.z - aabb.w, 1.f},
			{aabb.x + aabb.w, aabb.y + aabb.w, aabb.z - aabb.w, 1.f},
			{aabb.x - aabb.w, aabb.y - aabb.w, aabb.z + aabb.w, 1.f},
			{aabb.x + aabb.w, aabb.y - aabb.w, aabb.z + aabb.w, 1.f},
			{aabb.x - aabb.w, aabb.y + aabb.w, aabb.z + aabb.w, 1.f},
			{aabb.x + aabb.w, aabb.y + aabb.w, aabb.z + aabb.w, 1.f},
	};

	math::vector4f const box_normals[3] = {
		math::vector4f{1.f, 0.f, 0.f, 1.f},
		math::vector4f{0.f, 1.f, 0.f, 1.f},
		math::vector4f{0.f, 0.f, 1.f, 1.f},
	};

#if 0
	auto tribox_min = math::point4f(
		std::min(std::min(tri.v0.x, tri.v1.x), tri.v2.x),
		std::min(std::min(tri.v0.y, tri.v1.y), tri.v2.y),
		std::min(std::min(tri.v0.z, tri.v1.z), tri.v2.z));

	auto tribox_max = math::point4f(
		std::max(std::max(tri.v0.x, tri.v1.x), tri.v2.x),
		std::max(std::max(tri.v0.y, tri.v1.y), tri.v2.y),
		std::max(std::max(tri.v0.z, tri.v1.z), tri.v2.z));

	// if minimum bounding box didn't overlap, definitely not overlapping
	if (!intersect_aabb_box(aabb, box_t::from_minmax(tribox_min, tribox_max)))
		return false;
#endif

	// test box normals
	{
		for (int i = 0; i != 3; ++i)
		{
			project(box_normals[i], &tri.v0, 3, tri_min, tri_max);
			if (tri_max < aabb.components[i] - aabb.w || tri_min > aabb.components[i] + aabb.w)
				return false;
		}
	}

	// test the triangle normal
	{
		auto tri_normal = math::cross_product(tri.v1 - tri.v0, tri.v2 - tri.v0);
		auto tri_offset = math::dot_product(tri_normal, tri.v0);

		

		project(tri_normal, box_verts, 8, box_min, box_max);

		if (box_max < tri_offset || box_min > tri_offset)
			return false;
	}


	// test nine-edges
	for (int i = 0; i != 3; ++i)
	{
		for (int j = 0; j != 3; ++j)
		{
			auto axis = math::cross_product(tri_edges[i], box_normals[j]);
			project(axis, box_verts, 8, box_min, box_max);
			project(axis, &tri.v0, 3, tri_min, tri_max);
			if (box_max <= tri_min || box_min >= tri_max)
				return false;
		}
	}


	return true;
}





#if 0



bool IsIntersecting(IAABox box, ITriangle triangle)
{
	double triangleMin, triangleMax;
	double boxMin, boxMax;

	// Test the box normals (x-, y- and z-axes)
	var boxNormals = new IVector[] {
		new Vector(1,0,0),
			new Vector(0,1,0),
			new Vector(0,0,1)
	};
	for (int i = 0; i < 3; i++)
	{
		IVector n = boxNormals[i];
		Project(triangle.Vertices, boxNormals[i], out triangleMin, out triangleMax);
		if (triangleMax < box.Start.Coords[i] || triangleMin > box.End.Coords[i])
			return false; // No intersection possible.
	}

	// Test the triangle normal
	double triangleOffset = triangle.Normal.Dot(triangle.A);
	Project(box.Vertices, triangle.Normal, out boxMin, out boxMax);
	if (boxMax < triangleOffset || boxMin > triangleOffset)
		return false; // No intersection possible.

	// Test the nine edge cross-products
	IVector[] triangleEdges = new IVector[] {
		triangle.A.Minus(triangle.B),
			triangle.B.Minus(triangle.C),
			triangle.C.Minus(triangle.A)
	};
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
		{
		// The box normals are the same as it's edge tangents
		IVector axis = triangleEdges[i].Cross(boxNormals[j]);
		Project(box.Vertices, axis, out boxMin, out boxMax);
		Project(triangle.Vertices, axis, out triangleMin, out triangleMax);
		if (boxMax <= triangleMin || boxMin >= triangleMax)
			return false; // No intersection possible
		}

	// No separating axis found.
	return true;
}

void Project(IEnumerable<IVector> points, IVector axis,
	out double min, out double max)
{
	double min = double.PositiveInfinity;
	double max = double.NegativeInfinity;
	foreach (var p in points)
	{
		double val = axis.Dot(p);
		if (val < min) min = val;
		if (val > max) max = val;
	}
}

interface IVector
{
	double X { get; }
	double Y { get; }
	double Z { get; }
	double[] Coords { get; }
	double Dot(IVector other);
	IVector Minus(IVector other);
	IVector Cross(IVector other);
}

interface IShape
{
	IEnumerable<IVector> Vertices { get; }
}

interface IAABox : IShape
{
	IVector Start { get; }
	IVector End { get; }
}

interface ITriangle : IShape {
	IVector Normal { get; }
	IVector A { get; }
	IVector B { get; }
	IVector C { get; }
}


#endif

















#if 0

struct octree_t::node_t
{
	node_t()
		: bounds(0.f, 0.f, 0.f, 0.5f)
		, children_()
		, datas_()
	{
	}

	node_t(math::vector4f const& bounds)
		: bounds(bounds)
		, children_()
		, datas_()
	{
	}

	atma::math::vector4f bounds;
	triangle_t data;

	auto inbounds(math::vector4f const& point) -> bool;
	auto insert(math::vector4f const& point, T const& data) -> bool;

private:
	auto imem_allocate() -> void
	{
		children_ = (node_t*)new char[8 * sizeof(node_t)];
	}

	auto imem_deallocate() -> void;


	auto oct_split() -> void
	{
		children_ = (node_t*)new char[8 * sizeof(node_t)];

		for (auto i = 0u; i != 8u; ++i)
			new (children_ + i) node_t( oct_subbound(bounds, i) );

		for (auto i = 0u; i != 8u; ++i)
			insert(dloc_[i], data_[i]);
	}

	auto oct_subbound(math::vector4f const&, uint) -> math::vector4f;

private:
	node_t* children_;

	std::array<T, 8> data_;
	std::array<math::vector4f, 8> dloc_;

	uint datas_;
};


auto octree_t::node_t::insert(triangle_t const& tri) -> bool
{
	
	if (!inbounds(point))
	{
		return false;
	}

	// we are a full leaf node
	if (!children_ && datas_ == 8)
	{
		oct_split();
	}

	if (children_)
	{
		return std::any_of(children_, children_ + 8, [&point, &data](node_t& node) {
			return node.insert(point, data);
		});
	}

	data_[datas_] = data;
	dloc_[datas_] = point;
	++datas_;

	return true;
}

auto octree_t::node_t::inbounds(math::vector4f const& point) -> bool
{
	return
		point.x < bounds.x + bounds.w && point.x > bounds.x - bounds.w &&
		point.y < bounds.y + bounds.w && point.y > bounds.y - bounds.w &&
		point.z < bounds.z + bounds.w && point.z > bounds.z - bounds.w
		;
}

auto octree_t::node_t::oct_subbound(math::vector4f const& bounds, uint idx) -> math::vector4f
{
	return math::vector4f(
		(0.5f - ((idx & 1)     ) * 1.f) * bounds.w + bounds.x,
		(0.5f - ((idx & 2) >> 1) * 1.f) * bounds.w + bounds.y,
		(0.5f - ((idx & 4) >> 2) * 1.f) * bounds.w + bounds.z,
		bounds.w * 0.5f);
}

#endif




int main()
{
#if 0
	auto oct = octree_t<int>();

	for (float i = 0.f; i < 8.f; ++i)
		oct.root_.insert(math::point4f(0.2f + i * 0.001f, 0.3f  * 0.001f, 0.4f * 0.001f), (int)i);
#endif
}
