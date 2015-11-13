#ifndef SRC_RANGE_ALGORITHM_HPP_
#define SRC_RANGE_ALGORITHM_HPP_

namespace fc {

template<class range, class algorithm>
struct range_view
{
	template<class param, class result>
	decltype(auto) operator()(const param&& p)
	{
		return algorithm(std::forward(p));
	}
};

}  // namespace fc

#endif /* SRC_RANGE_ALGORITHM_HPP_ */
