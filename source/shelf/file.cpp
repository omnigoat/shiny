#include <shelf/file.hpp>


using namespace shelf;
using shelf::file_t;

auto file_t::file_t(atma::string const& filename, file_access_t access)
	: filename_(filename), access_(access)
{
	//char const* fa[] = {"r", "w", "wr"}
	//handle_ = fopen(filename.c_str(), )
}
