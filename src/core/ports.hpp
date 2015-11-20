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
template<class source, class sink, class payload>
struct port_connection
{
	typedef source source_t;
	typedef sink sink_t;
	typedef payload result_t;
};

///trait to define that a type is a port. Overload this for your own ports.
template<class T>
struct is_port : std::false_type
{
};

} //namespace fc
