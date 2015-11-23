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

/**
 * \brief node for splitting lists according to a predicate
 *
 * out() generates a separate output port for every predicate
 * result value.
 *
 * \tparam range_t: any type for which boost::begin/end works (see also boost range)
 * \tparam predicate_result_t: result type of predicate
 */
template
	<	class range_t,
		class predicate_result_t
	>
class list_splitter
{
public:
	typedef typename std::iterator_traits<decltype(boost::begin(range_t()))>::value_type value_t;
	typedef std::vector<value_t> out_range_t;

	list_splitter(auto p)
		: in( [&](const range_t& range){ this->receive(range); } )
		, entries()
		, predicate(p)
	{}

	event_in_port<range_t> in;
	event_out_port<out_range_t> out(predicate_result_t value) { return entries[value].port; }
	/**
	 * number of dropped elements (due to unconnected output ports)
	 * (Can be used for verification)
	 */
	state_source_with_setter<size_t> out_num_dropped;

private:
	void receive(const range_t& range)
	{
		auto begin = boost::begin(range);
		auto end = boost::end(range);

		for (auto it = begin; it != end; ++it)
		{
			auto p = predicate(*it);
			auto entry_it = entries.find(p);
			if (entry_it != entries.end())
				entry_it->second.data.push_back(*it);
			else
				++out_num_dropped.access();
		}
		for (auto& e : entries)
		{
			auto entry_it = entries.find(e.first);
			if (entry_it != entries.end())
				entry_it->second.port.fire(e.second.data);
			else
				assert(false);
			e.second.data.clear();
		}
	}
	struct entry_t
	{
		event_out_port<out_range_t> port;
		std::vector<value_t> data;
	};

	std::map<predicate_result_t, entry_t> entries;
	std::function<predicate_result_t(value_t)> predicate;
};

} // namespace fc

#endif /* SRC_NODES_LIST_MANIPULATION_HPP_ */
