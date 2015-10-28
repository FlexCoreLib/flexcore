#ifndef SRC_NODES_GENERIC_HPP_
#define SRC_NODES_GENERIC_HPP_

#include <core/traits.hpp>
#include <ports/states/state_sink.hpp>

#include <utility>

namespace fc
{

template<class bin_op>
struct transform_node
{
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

template<class bin_op>
auto transform(bin_op op)
{
	return transform_node<bin_op>(op);
}

struct n_ary_switch
{

};

}  // namespace fc

#endif /* SRC_NODES_GENERIC_HPP_ */
