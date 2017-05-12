#ifndef SRC_CORE_EXCEPTIONS_HPP_
#define SRC_CORE_EXCEPTIONS_HPP_

#include <exception>

namespace fc
{

/**
 * \brief exception which signals errors in the structure of the dataflow graph.
 *
 * These errors are not meant to be caught during execution of the graph.
 * It might be possible to catch them during construction of the graph.
 */
class bad_structure : public std::runtime_error
{
public:
	using runtime_error::runtime_error;
};

/// Exception which signals that a port is called but not connected.
class not_connected : public bad_structure
{
public:
	using bad_structure::bad_structure;
};



}  // namespace fc

#endif /* SRC_CORE_EXCEPTIONS_HPP_ */
