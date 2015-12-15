#ifndef SRC_PORTS_STREAM_SOURCES_STREAM_QUERY_FUNCTION_HPP_
#define SRC_PORTS_STREAM_SOURCES_STREAM_QUERY_FUNCTION_HPP_

// std
#include <functional>
#include <memory>

namespace fc
{

/**
 * State source port that queries a node for the value
 * See test_stream_query_node.cpp for sample use inside a node
 *
 * Caller is responsible that the node passed to the constructor is not destroyed before the port
 */
template<class data_t>
class state_source_call_function
{
public:
	/**
	 * \brief constructs state_source_function with function to call.
	 *
	 * \param f function which is called, when data is pulled from this source
	 * \pre f needs to be non empty function.
	 */
	explicit state_source_call_function(std::function<data_t()> f)
		: call(f)
	{
		assert(call);
	}

	data_t operator()() { return call(); }

private:
	std::function<data_t()> call;
};

/**
 * Simple StreamSource implementation that holds a state_source_with_setter.
 * Can be connected to any number of SinkConnections.
 * Is a SourceConnection.
 */
template<class data_t>
class state_source_with_setter
{
public:
	explicit state_source_with_setter(data_t d)
		: d(std::make_shared<data_t>(d))
	{}

	/// pull data
	data_t operator()() { return *d; }

	/// set current value
	void set(data_t d_) { *d = d_; }

private:
	std::shared_ptr<data_t> d;
};

/**
 * Universal type proxy
 */
template<class T>
struct Type {};

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

#define OUT_PORT_TMPL_HELPER(NAME, FUNCTION) \
	auto NAME() \
	{ return make_state_source_tmpl( [this](auto f) -> auto { return this->FUNCTION(f); } ); } \
	template<class state_t> \
	state_t FUNCTION( Type<state_t> )

#define OUT_PORT_TMPL(NAME) OUT_PORT_TMPL_HELPER( NAME, NAME##MEM_FUN )

// traits
template<class T> struct is_passive_source<state_source_with_setter<T>> : std::true_type {};
template<class T> struct is_passive_source<state_source_call_function<T>> : std::true_type {};
template<class T> struct is_passive_source<state_source_tmpl<T>> : std::true_type {};

} // namespace fc

#endif /* SRC_PORTS_STREAM_SOURCES_STREAM_QUERY_FUNCTION_HPP_ */
