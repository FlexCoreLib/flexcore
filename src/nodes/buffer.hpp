#ifndef SRC_NODES_BUFFER_HPP_
#define SRC_NODES_BUFFER_HPP_

#include <ports/states/state_source_call_function.hpp>

#include <boost/range.hpp>
#include <vector>

namespace fc
{

template<class data_t>
class event_to_state
{
public:
	typedef boost::iterator_range<typename std::vector<data_t>::const_iterator> out_range_t;

	event_to_state()
		: out_port( [&]()
				{
					data_read = true;
					return boost::make_iterator_range(
							buffer_state.begin(),
							buffer_state.end());
				} )
	{}

	auto in() noexcept
	{
		return collector{this};
	}

	auto out() noexcept { return out_port; }

	auto swap_buffers() noexcept
	{
		return [this]()
		{
			if (data_read) //move data from collect buffer to output, and clear collect buffer
			{
				buffer_state.clear();
				buffer_state.swap(buffer_collect);
				data_read = false;
			}
			else //just move data from collect buffer to output buffer
			{
				buffer_state.insert(end(buffer_collect),
						begin(buffer_collect), end(buffer_collect));
				buffer_collect.clear();
			}
		};
	}

private:
	state_source_call_function<out_range_t> out_port;

	std::vector<data_t> buffer_collect;
	std::vector<data_t> buffer_state;
	bool data_read = false;

	struct collector
	{
		typedef void result_t;

		void operator() (const auto& range)
		{
			using std::begin;
			using std::end;
			owner->buffer_collect.insert(
					end(owner->buffer_collect), begin(range), end(range));
		};

		void operator()(const data_t& single_input)
		{
			owner->buffer_collect.insert(
					end(owner->buffer_collect), single_input);
		}

		event_to_state* owner;
	};
};

}  // namespace fc



#endif /* SRC_NODES_BUFFER_HPP_ */
