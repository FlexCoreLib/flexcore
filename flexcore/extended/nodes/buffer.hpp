#ifndef SRC_NODES_BUFFER_HPP_
#define SRC_NODES_BUFFER_HPP_

#include <flexcore/core/traits.hpp>

#include <boost/range.hpp>
#include <boost/circular_buffer.hpp>
#include <vector>

namespace fc
{

/**
	\defgroup nodes
	\brief Generic Nodes already implemented and supplied by flexcore.

	This group contains all nodes
	which are generic enough to be included in flexcore itself.
*/

namespace detail
{
template<class data_t, template<class...> class container_t, class base_t>
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
template<class data_t, class buffer_policy, class base_t>
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
 * \ingroup nodes
 */
template<class data_t, class base_t>
class list_collector<data_t, swap_on_tick, base_t>
		: public detail::base_event_to_state<data_t, std::vector, base_t>
{
public:
	typedef boost::iterator_range<typename std::vector<data_t>::const_iterator> out_range_t;
	template<class... args_t>
	explicit list_collector(args_t&&... args)
		: detail::base_event_to_state<data_t, std::vector, base_t>{
				[this]()
				{
					data_read = true;
					return boost::make_iterator_range(
							this->buffer_state.begin(),
							this->buffer_state.end());
				},
				std::forward<args_t>(args)...}
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
 * Events are stored in vector which grows until pull is called.
 */
template<class data_t, class base_t>
class list_collector<data_t, swap_on_pull, base_t>
		: public detail::base_event_to_state<data_t, std::vector, base_t>
{
public:
	/// Type of range provided as output is an immutable range of data_t.
	typedef boost::iterator_range<typename std::vector<data_t>::const_iterator>
			out_range_t;

	template<class... args_t>
	explicit list_collector(args_t&&... args)
		: detail::base_event_to_state<data_t, std::vector, base_t>{[&]()
				{
					return this->get_state();
				},
				std::forward<args_t>(args)...}
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
	// result_t is defined to allow result_of trait with overloaded operator().
	typedef void result_t;

	template <class range_t>
	void operator()(const range_t& range)
	{
		using std::begin;
		using std::end;
		//check if the node owning the buffer has been deleted. which is a bug.
		assert(buffer);
		buffer->insert(end(*buffer), begin(range), end(range));
	}

	void operator()(const data_t& single_input)
	{
		//check if the node owning the buffer has been deleted. which is a bug.
		assert(buffer);
		buffer->insert(end(*buffer), single_input);
	}

	container_t<data_t>* buffer; ///< non-owning access to the buffer of node.
};

/**
 * \brief  Base class for nodes which take events and provide a range as state.
 *
 * \tparam data_t type of data accepted and provided by the node.
 * param container_t container used to store the incoming data.
 *
 * Extend this class to easily implement your own nodes.
 * \ingroup nodes
 */
template<class data_t, template<class...> class container_t, class base_t>
class base_event_to_state : public base_t
{
public:
	typedef boost::iterator_range<typename container_t<data_t>::const_iterator>
			out_range_t;

	/// Input Port accepting both ranges and single events of type data_t
	auto in() noexcept
	{
		using collector = detail::collector<data_t, container_t>;
		return typename base_t::template mixin<collector>{this, collector{buffer_collect.get()}};
	}

	/// Output Port providing a range of data_t
	auto& out() noexcept { return out_port; }

protected:
	/**
	 * \brief constructor used by classes extending this
	 * \param action is the operation executed on incoming data
	 *  to store it in outputs.
	 *  Action can be a simple write to the output buffer or a more complex action.
	 */
	template<class action_t, class... args_t>
	explicit base_event_to_state(
			action_t&& action, args_t&&... args) :
			base_t(std::forward<args_t>(args)...),
		buffer_collect(std::make_unique<std::vector<data_t>>()),
		out_port(this, std::forward<action_t>(action))
	{
	}

	std::unique_ptr<container_t<data_t>> buffer_collect;
	container_t<data_t> buffer_state;

	typename base_t::template state_source<out_range_t> out_port;
};
} //namespace detail

/**
 * \brief buffer which receives events and stores the last event received as state.
 *
 * \tparam data_t is type of token received as event and then stored.
 * \ingroup nodes
 */
template<class data_t, class base_t>
class hold_last : public base_t
{
public:
	static constexpr auto default_name = "hold_last";

	static_assert(!std::is_void<data_t>(),
			"data stored in hold_last cannot be void");

	/// Constructs hold_last with initial state.
	template<class... args_t>
	explicit hold_last(const data_t& initial_value, args_t&&... args)
		: base_t(std::forward<args_t>(args)...)
		, storage(initial_value)
		, in_port{this, [this](data_t in){ storage = in; }}
		, out_port{this,[this](){ return storage;} }
	{
	}

	/// Event in Port expecting data_t.
	auto& in() { return in_port; }
	/// State out port supplying data_t.
	auto& out() { return out_port; }
private:
	data_t storage;
	typename base_t::template event_sink<data_t> in_port;
	typename base_t::template state_source<data_t> out_port;
};

/**
 * \brief Simple circular buffer.
 *
 * hold_n accepts events of data_t and ranges of data_t as inputs
 * and stores them in a circular buffer.
 *
 * \tparam data_t type of data stored in buffer
 * \invariant capacity of buffer is > 0.
 * \ingroup nodes
 */
template<class data_t, class base_t>
class hold_n : public base_t
{
public:
	static constexpr auto default_name = "hold_n";
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
	template<class... args_t>
	explicit hold_n(size_t capacity, args_t&&... args)
		: base_t(std::forward<args_t>(args)...)
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
		using collector = detail::collector<data_t, boost::circular_buffer>;

		return typename base_t::template mixin<collector>{this, collector{storage.get()}};
	}
	/// State out port supplying range of data_t.
	auto& out() noexcept { return out_port; }
private:
	std::unique_ptr<buffer_t> storage;
	typename base_t::template state_source<out_range_t> out_port;
};

}  // namespace fc

#endif /* SRC_NODES_BUFFER_HPP_ */
