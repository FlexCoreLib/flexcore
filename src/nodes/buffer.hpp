#ifndef SRC_NODES_BUFFER_HPP_
#define SRC_NODES_BUFFER_HPP_

#include <core/traits.hpp>

#include <ports/ports.hpp>

#include <boost/range.hpp>
#include <boost/circular_buffer.hpp>
#include <vector>

namespace fc
{

namespace detail
{
template<class data_t, template<class...> class container_t>
class base_event_to_state;
}
/**
 * \brief Buffer that takes events and provides a range as state
 *
 * Accepts both single events and ranges of events as inputs.
 *
 * policy classes control when the buffer is cleared
 * as well as how and when the state is available.
 */
template<class data_t, class buffer_policy>
class list_collector;

/**
 * \brief Buffer Policy to have buffers swapped on a tick and stored between ticks.
 *
 * The state is constant between swap ticks.
 * Events are available after the next swap tick.
 */
struct swap_on_tick {};
/**
 * \brief Buffer Policy to have buffers swapped on pull.
 *
 * This means pulling the collector twice gives different results!
 * New events are available as soon as they are received
 */
struct swap_on_pull {};

/**
 * \brief Collects list contents and store them into a buffer.
 *
 * Sends the buffer as state when pulled.
 * inputs are made available on tick received at port swap_buffers.
 */
template<class data_t>
class list_collector<data_t, swap_on_tick>
		: public detail::base_event_to_state<data_t, std::vector>
{
public:
	typedef boost::iterator_range<typename std::vector<data_t>::const_iterator> out_range_t;
	list_collector()
		: detail::base_event_to_state<data_t, std::vector>{
						[this]()
						{
							data_read = true;
							return boost::make_iterator_range(
									this->buffer_state.begin(),
									this->buffer_state.end());
						}}
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
 * \brief Collects list contents and store them into a buffer.
 *
 * Sends the buffer as state when pulled.
 */
template<class data_t>
class list_collector<data_t, swap_on_pull>
		: public detail::base_event_to_state<data_t, std::vector>
{
public:
	typedef boost::iterator_range<typename std::vector<data_t>::const_iterator>
			out_range_t;

	list_collector()
		: detail::base_event_to_state<data_t, std::vector>{[&]()
				{
					return this->get_state();
				}}
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

namespace detail
{
/// a port which receives both single events and ranges.
template<class data_t, template<class...> class container_t>
struct collector
{
	typedef void result_t;

	void operator() (const auto& range)
	{
		using std::begin;
		using std::end;
		//check if the node owning the buffer has been deleted. which is a bug.
		assert(buffer);
		buffer->insert(end(*buffer), begin(range), end(range));
	};

	void operator()(const data_t& single_input)
	{
		//check if the node owning the buffer has been deleted. which is a bug.
		assert(buffer);
		buffer->insert(end(*buffer), single_input);
	}

	container_t<data_t>* buffer;
};

/// Base class for nodes which take events and provide a range as state.
template<class data_t, template<class...> class container_t>
class base_event_to_state : public tree_base_node
{
public:
	typedef boost::iterator_range<typename container_t<data_t>::const_iterator> out_range_t;

	auto in() noexcept
	{
		return detail::collector<data_t, container_t>{buffer_collect.get()};
	}

	auto out() noexcept { return out_port; }

protected:
	template<class action_t>
	explicit base_event_to_state(
			action_t&& action) :
		tree_base_node("event to state"),
		buffer_collect(std::make_unique<std::vector<data_t>>()),
		out_port(this, action)
	{
	}

	std::unique_ptr<container_t<data_t>> buffer_collect;
	container_t<data_t> buffer_state;

	state_source_call_function<out_range_t> out_port;
};
} //namespace detail

/**
 * \brief buffer which receives events and stores the last event received as state.
 *
 * \tparam data_t is type of token received as event and then stored.
 */
template<class data_t>
class hold_last : public tree_base_node
{
public:
	static_assert(!std::is_void<data_t>(),
			"data stored in hold_last cannot be void");

	/// Constructs hold_last with initial state.
	explicit hold_last(const data_t& initial_value = data_t())
		: tree_base_node("hold_last")
		, storage(initial_value)
	{
	}

	/// Event in Port expecting data_t.
	auto in() noexcept { return [this](data_t in){ storage = in; }; }
	/// State out port supplying data_t.
	auto out() const noexcept { return [this](){ return storage;}; }
private:
	data_t storage;
};

/**
 * \brief Simple circular buffer.
 *
 * hold_n accepts events of data_t and ranges of data_t as inputs
 * and stores them in a circular buffer.
 *
 * \tparam data_t type of data stored in buffer
 * \invariant capacity of buffer is > 0.
 */
template<class data_t>
class hold_n : tree_base_node
{
public:
	typedef boost::circular_buffer<data_t> buffer_t;
	typedef boost::iterator_range<
			typename buffer_t::const_iterator> out_range_t;

	static_assert(!std::is_void<data_t>(),
			"data stored in hold_last cannot be void");

	/**
	 * \brief constructs circular_buffer with capacity parameter.
	 * \param capacity sets the max nr of elements in the circular buffer.
	 * \pre capacity > 0
	 */
	explicit hold_n(size_t capacity)
		: tree_base_node("hold")
		, storage(std::make_unique<buffer_t>(capacity))
		, out_port(this,
				[this]()
				{
					return boost::make_iterator_range(
							storage->begin(),
							storage->end());
				} )
		{
			assert(capacity > 0); //precondition
			assert(storage->capacity() > 0); //invariant
		}

	/// Event in Port expecting data_t or range of data_t.
	auto in() noexcept
	{
		return detail::collector<data_t, boost::circular_buffer>{storage.get()};
	}
	/// State out port supplying range of data_t.
	auto& out() const noexcept { return out_port; }
private:
	std::unique_ptr<buffer_t> storage;
	state_source_call_function<out_range_t> out_port;
};

}  // namespace fc

#endif /* SRC_NODES_BUFFER_HPP_ */
