#include <type_traits>

template<class... dummy>
constexpr bool void_check(dummy...) {return 0;}

template<
		class source_t,
		class sink_t
		>
struct connection
{
	source_t source;
	sink_t sink;

	template<class param, class S = source_t, class T = sink_t>
	auto operator()(const param& p)
		-> typename std::enable_if<void_check(source, sink) == 0,
		decltype(sink(source(p)))>::type
	{
		return sink(source(p));
	}
};

template
	<	class source_t,
		class sink_t
	>
auto connect(const source_t& source, const sink_t& sink)
{
	return connection<source_t, sink_t>{source, sink};
}


auto increment = [](int i) -> int {return i+1;};
auto give_one = [](void) ->int {return 1;};

auto one_plus_one = connect(give_one, increment);
