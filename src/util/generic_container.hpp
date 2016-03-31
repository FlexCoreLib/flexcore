/*
 * generic_container.hpp
 *
 *  Created on: Mar 31, 2016
 *      Author: jschwan
 */

#ifndef SRC_UTIL_GENERIC_CONTAINER_HPP_
#define SRC_UTIL_GENERIC_CONTAINER_HPP_

#include <vector>
#include <memory>

namespace fc
{

/**
 * \brief stores arbitrary objects with their type erased.
 *
 * generic_container is used as an implementation of the pimple idiom.
 * It stores arbitrary nodes and takes ownership of them.
 * This makes it not necessary anymore to store nodes as members
 * if they are only used in a single place.
 */
class generic_container
{
public :
	generic_container() = default;

	/// adds a new element and takes ownership of it.
	template <class T, class... Args>
	T& add (Args&&... args)
	{
		auto tmp_ptr = std::make_shared<T>(args...);
		store.push_back(tmp_ptr);
		return *tmp_ptr;
	}

private:
	std::vector<std::shared_ptr<void>> store;
};

}

#endif /* SRC_UTIL_GENERIC_CONTAINER_HPP_ */
