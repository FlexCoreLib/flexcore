#include <core/traits.hpp>

/**
 * This file contains all information the core package has and needs about ports
 * Ports are types which are used by nodes to serve as access points to connections.
 *
 * Thus this is the access point for the interaction between the ports package
 * and core package.
 */

namespace fc
{
/**
 *  \brief connection type, when to ports are connected, has no runtime information or access to the ports.
 *
 *  This is the base for all connections between explicit ports of nodes.
 */
template<class source, class sink>
struct port_connection
{
	typedef source source_t;
	typedef sink sink_t;
	typedef typename result_of<source_t>::type payload_t;
};

} //namespace fc
