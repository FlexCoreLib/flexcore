#ifndef SRC_NODES_LIST_MANIPULATION_HPP_
#define SRC_NODES_LIST_MANIPULATION_HPP_

#include <core/traits.hpp>
#include <ports/ports.hpp>

// std
#include <map>

// boost
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>

namespace fc
{

/*
 * todo
 * first for events only
 */
template
	<	class range_t,			// boost::begin/end works
		class predicate_result_t // todo
	>
class list_splitter
{
public:
	list_splitter(auto p)
		: predicate(p)
		, in( [&](const range_t& range){ this->receive(range); } )
	{}

	event_in_port<range_t> in;
	event_out_port<range_t> out(predicate_result_t value) { return out_ports[value]; }

private:
	void receive(const range_t& range)
	{
		auto begin = boost::begin(range);
		auto end = boost::end(range);

		typedef typename std::iterator_traits<decltype(begin)>::value_type value_t;
		std::map<predicate_result_t, std::vector<value_t>> result_map;
		for (auto it = begin; it != end; ++it)
			result_map[predicate(*it)].push_back(*it);
		for (auto& e : result_map)
		{
			auto it = out_ports.find(e->first);
			if (it != result_map.end())
				it->second.fire(e.second);
		}
	}

	std::map<predicate_result_t, event_out_port<range_t>> out_ports;
	typedef typename std::iterator_traits<decltype(boost::begin(range_t()))>::value_type value_t;
	std::function<predicate_result_t(value_t)> predicate;
};

} // namespace fc

#endif /* SRC_NODES_LIST_MANIPULATION_HPP_ */
