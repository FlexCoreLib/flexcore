#ifndef SRC_PORTS_STREAM_SOURCES_STREAM_QUERY_FUNCTION_HPP_
#define SRC_PORTS_STREAM_SOURCES_STREAM_QUERY_FUNCTION_HPP_

// std
#include <functional>
#include <memory>

#include <core/connection.hpp>
#include <core/traits.hpp>
#include <core/type_proxy.hpp>

namespace fc
{
namespace pure
{

/**
 * State source port that queries a node for the value
 * See test_stream_query_node.cpp for sample use inside a node
 *
 * Caller is responsible that the node passed to the constructor is not destroyed before the port
 */
template<class data_t>
class state_source
{
public:
	/**
	 * \brief constructs state_source_function with function to call.
	 *
	 * \param f function which is called, when data is pulled from this source
	 * \pre f needs to be non empty function.
	 */
	explicit state_source(std::function<data_t()> f)
		: call(f)
	{
		assert(call);
	}

	state_source(const state_source&) = delete;
	state_source(state_source&&) = default;
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

	data_t operator()() { return call(); }

	void register_callback(std::shared_ptr<std::function<void(size_t)>>& visit_fun)
	{
		assert(visit_fun);
		assert(*visit_fun);
		connection_breakers.emplace_back(visit_fun);
	}

	typedef data_t result_t;

private:
	std::function<data_t()> call;
	std::vector<std::weak_ptr<std::function<void(size_t)>>> connection_breakers;
};

/**
 * \brief Templated state source port
 *
 * Call a (generic) lambda when a state is requested. This allows to defer
 * the actual type of the token until the port is called and also allows it to be
 * called for different types.
 *
 * When calling the lambda, you need to give the requested type as pseudo-parameter
 * because there is no type inference based on the return type.
 * The OUT_PORT_TMPL macro can be used to define a getter for the port and a header
 * a node member in one go.
 *
 * see test_state_source_tmpl for an example.
 *
 * \tparam lambda_t Lambda to call for the state value
 */
template<class lambda_t>
struct state_source_tmpl
{
	explicit state_source_tmpl(lambda_t h)
		: lambda(h)
	{}

	template<class token_t>
	auto operator()(Type<token_t> f) { return lambda(f); }

	state_source_tmpl() = delete;

	lambda_t lambda;
};

/*
 * Helper needed for type inference
 */
template<class lambda_t>
auto make_state_source_tmpl(lambda_t h) { return state_source_tmpl<lambda_t>{h}; }

} // namespace pure

// traits
template<class T> struct is_passive_source<pure::state_source<T>> : std::true_type {};
template<class T> struct is_passive_source<pure::state_source_tmpl<T>> : std::true_type {};

} // namespace fc

#endif /* SRC_PORTS_STREAM_SOURCES_STREAM_QUERY_FUNCTION_HPP_ */
