#ifndef SRC_NODES_GENERIC_HPP_
#define SRC_NODES_GENERIC_HPP_

#include <core/traits.hpp>
#include <ports/states/state_sink.hpp>

#include <utility>

namespace fc
{

/**
 * \brief generic unary node which applys transform with parameter to all inputs
 *
 * \tparam bin_op binary operator, argument is input of node, second is parameter
 *
 * \pre bin_op needs to be callable with two argments
 */
template<class bin_op>
struct transform_node
{
	static_assert(utils::function_traits<bin_op>::arity == 2,
			"operator in transform node needs to take two parameters");
	typedef typename result_of<bin_op>::type result_type;
	typedef typename argtype_of<bin_op,1>::type param_type;
	typedef typename argtype_of<bin_op,0>::type data_t;

	explicit transform_node(bin_op op) : op(op) {}

	state_sink<param_type> param;

	decltype(auto) operator()(data_t&& in)
	{
		return op(in, param());
	}

private:
	bin_op op;
};

/// creates transform_node with op as operation.
template<class bin_op>
auto transform(bin_op op)
{
	return transform_node<bin_op>(op);
}

//todo requires way to define state or event ports through template parameters
//template<class data_t>
//class n_ary_switch
//{
//
//	state_sink<size_t> index;
//	typedef void port_type;
//
//	std::vector<port_type> in_ports;
//
//	data_t operator()()
//	{
//		return in_ports.at(index())();
//	}
//
//};

}  // namespace fc

#endif /* SRC_NODES_GENERIC_HPP_ */
