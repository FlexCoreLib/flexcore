#ifndef SRC_CORE_TYPE_PROXY_HPP_
#define SRC_CORE_TYPE_PROXY_HPP_

namespace fc
{

/**
 * Universal type proxy
 */
template<class T>
struct Type
{
	typedef T type;
};

} // namespace fc

#endif /* SRC_CORE_TYPE_PROXY_HPP_ */
