#ifndef LANG_FILESYSTEM_H
#define LANG_FILESYSTEM_H

#include "config.h"
#include <string>

#ifdef USE_BOOST_FILESYSTEM
#include <boost/filesystem.hpp>
#else
#include <filesystem>
#endif

namespace lang {

#ifdef USE_BOOST_FILESYSTEM
namespace fs = boost::filesystem;
#else
namespace fs = std::filesystem;
#endif

} // namespace lang

#endif // LANG_FILESYSTEM_H
