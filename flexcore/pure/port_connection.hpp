#ifndef SRC_CORE_PORTS_H_
#define SRC_CORE_PORTS_H_

namespace fc
{
/**
 *  \brief connection type, when to ports are connected, has no runtime information or access to the ports.
 *
 *  This is the base for all connections between explicit ports of nodes.
 */
template<class source, class sink, class payload>
struct port_connection
{
	typedef source source_t;
	typedef sink sink_t;
	typedef payload result_t;
};

} //namespace fc

#endif // SRC_CORE_PORTS_H_
