#ifndef SRC_PORTS_PORTS_HPP_
#define SRC_PORTS_PORTS_HPP_

#include "event_ports.hpp"
#include "stream_ports.hpp"

namespace fc
{

template<class source_t, class sink_t>
struct stream_proxy
{
	stream_proxy(source_t source, sink_t sink_) :
			stored_source(source),
			sink(sink_)
	{

	}

	typename result_of<source_t>::type operator()()
	{
		//returns stored_source() if it exists with void parameters, throws otherwise.
		//ToDo
	}

	source_t stored_source;
	//access to connect method of sink port
	sink_t sink;

	template<class new_source_t,
			class = typename std::enable_if<is_source_port<new_source_t>::value>::type
			>
	auto connect(new_source_t new_source);
//	{
//		auto tmp = stream_proxy(connect(new_source, stored_source), sink);  //todo
//		sink.connect(tmp);
//		return tmp;
//	}

};

template<class source_t, class sink_t, class new_source_t,
		 class
		>
auto stream_proxy<source_t, sink_t>::connect(new_source_t new_source)
{
	auto tmp = stream_proxy(connect(new_source, stored_source), sink);  //todo
	sink.connect(tmp);
	return tmp;
}



template<class source_t, class sink_t,
	class = typename std::enable_if<is_connectable<source_t>::value>::type,
	class = typename std::enable_if<is_connectable<sink_t>::value>::type>
auto connect(source_t source, sink_t sink)
{
	return detail::connect_impl<sink_t, source_t>()(source, sink);
}

} // namespace fc

#endif /* SRC_PORTS_PORTS_HPP_ */
