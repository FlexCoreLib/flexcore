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
 * It stores arbitrary objects and takes ownership of them.
 * This makes it not necessary anymore to store nodes as members
 * if they are only used in a single place.
 */
class generic_container
{
public :
	generic_container() = default;
	generic_container(const generic_container&) = delete;

	/// adds a new element and takes ownership of it.
	template <class T, class... Args>
	T& add (Args&&... args)
	{
		auto tmp_ptr = std::make_shared<T>(std::forward<Args>(args)...);
		store.push_back(tmp_ptr);
		return *tmp_ptr;
	}

private:
	// shared_ptr instead of unique_ptr since unique_ptr can't store void.
	// even though ownership is never shared.
	std::vector<std::shared_ptr<void>> store;
};

}

#endif /* SRC_UTIL_GENERIC_CONTAINER_HPP_ */
