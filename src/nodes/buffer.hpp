#ifndef SRC_NODES_BUFFER_HPP_
#define SRC_NODES_BUFFER_HPP_

#include <core/traits.hpp>

#include <ports/states/state_sources.hpp>
#include <ports/events/event_sinks.hpp>

#include <boost/range.hpp>
#include <vector>

namespace fc
{

template<class data_t>
class base_event_to_state
{
public:
	typedef boost::iterator_range<typename std::vector<data_t>::const_iterator> out_range_t;

	auto in() noexcept { return collector{buffer_collect}; }

	auto out() noexcept { return out_port; }

protected:
	explicit base_event_to_state(const state_source_call_function<out_range_t>& out_port) :
		buffer_collect(std::make_shared<std::vector<data_t>>()),
		out_port(out_port)
	{
	}

	std::shared_ptr<std::vector<data_t>> buffer_collect;
	std::vector<data_t> buffer_state;

	state_source_call_function<out_range_t> out_port;

	struct collector
	{
		typedef void result_t;

		void operator() (const auto& range)
		{
			using std::begin;
			using std::end;
			//check if the node owning the buffer has been deleted. which is a bug.
			assert(!buffer.expired());
			buffer.lock()->insert(
					end(*buffer.lock()), begin(range), end(range));
		};

		void operator()(const data_t& single_input)
		{
			//check if the node owning the buffer has been deleted. which is a bug.
			assert(!buffer.expired());
			buffer.lock()->insert(
					end(*buffer.lock()), single_input);
		}

		std::weak_ptr<std::vector<data_t>> buffer;
	};
};

template<class data_t>
class event_to_state : public base_event_to_state<data_t>
{
public:
	typedef boost::iterator_range<typename std::vector<data_t>::const_iterator> out_range_t;
	event_to_state()
		: base_event_to_state<data_t>( state_source_call_function<out_range_t>([&]()
				{
					data_read = true;
					return boost::make_iterator_range(
							this->buffer_state.begin(),
							this->buffer_state.end());
				} ))
	{}

	auto swap_buffers() noexcept
	{
		return [this]()
		{
			if (data_read) //move data from collect buffer to output, and clear collect buffer
			{
				this->buffer_state.clear();
				this->buffer_state.swap(*this->buffer_collect);
				data_read = false;
			}
			else //just move data from collect buffer to output buffer
			{
				this->buffer_state.insert(end(*this->buffer_collect),
						begin(*this->buffer_collect), end(*this->buffer_collect));
				this->buffer_collect->clear();
			}
		};
	}

private:
	bool data_read = false;
};

/**
 * Collects list contents and store them into a buffer.
 * Sends the buffer as state when pulled.
 */
template<class data_t>
class list_collector : public base_event_to_state<data_t>
{
public:
	typedef boost::iterator_range<typename std::vector<data_t>::const_iterator> out_range_t;

	list_collector()
		: base_event_to_state<data_t>( state_source_call_function<out_range_t>([&]()
				{
					return this->get_state();
				} ))
	{}

private:
	out_range_t get_state()
	{
		this->buffer_state.clear();
		this->buffer_state.swap(*this->buffer_collect);
		return boost::make_iterator_range(
				this->buffer_state.begin(),
				this->buffer_state.end());
	}
};

/**
 * \brief buffer which receives events and stores the last event received as state.
 *
 * \tparam data_t is type of token received as event and then stored.
 */
template<class data_t>
class hold_last
{
public:
	static_assert(!std::is_void<data_t>(),
			"data stored in hold_last cannot be void");

	/// Constructs hold_last with initial state.
	explicit hold_last(const data_t& initial_value = data_t())
		: storage(initial_value)
	{
	}

	/// Event in Port expecting data_t.
	auto in() noexcept { return [this](data_t in){ storage = in; }; }
	/// State out port supplying data_t.
	auto out() const noexcept { return [this](){ return storage;}; }
private:
	data_t storage;
};

}  // namespace fc

#endif /* SRC_NODES_BUFFER_HPP_ */
