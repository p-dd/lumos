#ifndef SIMPLE_OBJECTS_H
#define SIMPLE_OBJECTS_H

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp> 

namespace tester {
	namespace fs = boost::filesystem;
	typedef size_t id;
	typedef std::string text;
	typedef std::vector<char> binary;
	typedef boost::shared_ptr<binary> binary_ptr;
};

#endif