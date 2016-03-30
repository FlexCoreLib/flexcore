#ifndef SRC_NODES_LIST_MANIPULATION_HPP_
#define SRC_NODES_LIST_MANIPULATION_HPP_

#include <core/traits.hpp>
#include <nodes/base_node.hpp>
#include <map>

// boost
#include <boost/range.hpp>

#include <ports/ports.hpp>

namespace fc
{

/**
 * \brief node for splitting lists according to a predicate
 *
 * out() generates a separate output port for every predicate
 * result value.
 *
 * \tparam range_t: any type for which non member functions begin() and end() exist.
 * \tparam predicate_result_t: result type of predicate
 */
template
	<	class range_t,
		class predicate_result_t,
		class base_t = tree_base_node
	>
class list_splitter : public base_t
{
public:
	typedef typename std::iterator_traits<decltype(std::begin(range_t()))>::value_type value_t;
	typedef boost::iterator_range<typename std::vector<value_t>::iterator> out_range_t;

	template <class predicate_t, class... args_t>
	explicit list_splitter(predicate_t pred, args_t&&... args)
		: base_t(std::forward<args_t>(args)...)
		, in(this, [&](const range_t& range){ this->receive(range); } )
		, out_num_dropped(this, [this](){ return dropped_counter;})
		, entries()
		, predicate(pred)
	{}

	typename base_t::template event_sink<range_t> in;
	auto& out(predicate_result_t value)
	{
		auto it = entries.find(value);
		if (it == entries.end())
			it = entries.insert(std::make_pair(value, entry_t(this))).first;
		return it->second.port;
	}
	/**
	 * number of dropped elements (due to unconnected output ports)
	 * (Can be used for verification)
	 */
	typename base_t::template state_source<size_t> out_num_dropped;

private:
	size_t dropped_counter = 0;

	void receive(const range_t& range)
	{
		auto begin = std::begin(range);
		auto end = std::end(range);

		// sort elements by predicate
		for (auto it = begin; it != end; ++it)
		{
			auto p = predicate(*it);
			auto entry_it = entries.find(p);
			if (entry_it != entries.end())
				entry_it->second.data.push_back(*it);
			else
				++dropped_counter;
		}
		// send sorted elements
		for (auto& e : entries)
		{
			e.second.port.fire(boost::make_iterator_range( e.second.data.begin(),
														   e.second.data.end() ));
			e.second.data.clear();
		}
	}
	struct entry_t
	{
		entry_t(list_splitter* p) : port(p), data() {}
		typename base_t::template event_source<out_range_t> port;
		std::vector<value_t> data;
	};

	std::map<predicate_result_t, entry_t> entries;
	std::function<predicate_result_t(value_t)> predicate;
};

} // namespace fc

#endif /* SRC_NODES_LIST_MANIPULATION_HPP_ */
