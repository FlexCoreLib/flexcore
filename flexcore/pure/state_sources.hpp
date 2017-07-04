#ifndef SRC_PORTS_STREAM_SOURCES_STREAM_QUERY_FUNCTION_HPP_
#define SRC_PORTS_STREAM_SOURCES_STREAM_QUERY_FUNCTION_HPP_

#include <flexcore/core/connection.hpp>
#include <flexcore/core/traits.hpp>

#include <cassert>
#include <functional>
#include <memory>
#include <utility>

namespace fc
{
namespace pure
{

/**
 * \brief State source port that queries a node for the value
 *
 * state_source fulfills passive_source.
 * \tparam data_t type of token provided by this port.
 * \ingroup ports
 */
template<class data_t>
class state_source
{
public:
	/**
	 * \brief constructs state_source with function to call.
	 *
	 * \tparam provide_action callable type with signature data_t(void)
	 * \param f function which is called, when data is pulled from this source
	 * \pre f needs to be non empty function.
	 */
	template<class provide_action>
	explicit state_source(provide_action&& f)
		: call(std::forward<provide_action>(f))
	{
		static_assert(std::is_constructible<std::function<data_t()>, provide_action>(),
				"action given to state_source needs to have signature data_t()."
				" Where data_t is type of token provided by state_source.");
		assert(call);
	}

	state_source(const state_source&) = delete;
	state_source(state_source&& o)
	{
		assert(o.connection_breakers.empty() &&
				"It is illegal to move a state_source which is connected");
		// Only move the handler so that if the assert doesn't fire (e.g. when
		// NDEBUG is defined) the moved-from-object will still disconnect
		// itself.
		swap(call, o.call);
	}

	state_source& operator=(state_source&& o)
	{
		assert(o.connection_breakers.empty() &&
				"It is illegal to move a state_source which is connected");
		swap(call, o.call);
		return *this;
	}

	/// Destructor disconnects existing connection and then deletes object.
	~state_source()
	{
		auto self = std::hash<decltype(this)>{}(this);
		for (auto& breaker_ptr : connection_breakers)
		{
			auto breaker = breaker_ptr.lock();
			if (breaker)
				(*breaker)(self);
		}
	}

	/// Provides token
	data_t operator()() { return call(); }

	/// Registers callback to disconnect port
	void register_callback(std::shared_ptr<std::function<void(size_t)>>& visit_fun)
	{
		assert(visit_fun);
		assert(*visit_fun);
		connection_breakers.emplace_back(visit_fun);
	}

	using result_t = data_t;
	using token_t = data_t;

private:
	std::function<data_t()> call;
	std::vector<std::weak_ptr<std::function<void(size_t)>>> connection_breakers;
};

} // namespace pure
} // namespace fc

#endif /* SRC_PORTS_STREAM_SOURCES_STREAM_QUERY_FUNCTION_HPP_ */
