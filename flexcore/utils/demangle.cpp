#include "demangle.hpp"
#ifdef __GNUG__
#include <cstdlib>
#include <memory>
#include <cxxabi.h>

// from http://stackoverflow.com/questions/281818/unmangling-the-result-of-stdtype-infoname
std::string demangle(const char* name)
{
	int status = -1;
	std::unique_ptr<char, void(*)(void*)> res
	{
		abi::__cxa_demangle(name, nullptr, nullptr, &status),
		std::free
	};
	return status == 0 ? res.get() : name;
}

#else
std::string demangle(const char* name)
{
	return name;
}
#endif
