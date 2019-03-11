#ifndef LANG_FILESYSTEM_H
#define LANG_FILESYSTEM_H

#include "config.h"
#include <string>

#ifdef USE_BOOST_FILESYSTEM
#include <boost/filesystem.hpp>
#elif USE_STD17_FILESYSTEM
#include <experimental/filesystem>
#endif

namespace lang {

#ifdef USE_BOOST_FILESYSTEM
namespace fs = boost::filesystem;
#elif USE_STD17_FILESYSTEM
namespace fs = std::experimental::filesystem;
#endif

} // namespace lang

#endif // LANG_FILESYSTEM_H
