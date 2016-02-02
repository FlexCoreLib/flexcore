#ifndef SRC_NODES_DATA_SOURCES_HPP_
#define SRC_NODES_DATA_SOURCES_HPP_

namespace fc
{

///Provides a constant value which never changes as state.
template<class data_t>
struct constant_state
{
	explicit constant_state(data_t d) : stored(d) {}

	data_t operator()() const
	{
		return stored;
	}

private:
	data_t stored;
};

/**
 * \brief Creates a node which provides a constant value as state.
 *
 * \tparam data_t type of data stored, needs to be copyable.
 * \param value Constant value provided.
 */
template<class data_t>
decltype(auto) constant(data_t value)
{
	return graph::named(constant_state<data_t>{value},
			"Constant = " + std::to_string(value));
}

/**
 * \brief Provides Random Numbers as state.
 *
 * The provided state changes every it is polled!
 * This means that client code might need to cache it it a random number
 * is expected to stay the same for a cycle.
 */
template<class data_t, class rng_engine, template<class>class distribution>
struct randon_number
{
	data_t operator()()
	{
		return distr(engine);
	}

private:
	distribution<data_t> distr;
	rng_engine engine;
};

}  // namespace fc

#endif /* SRC_NODES_DATA_SOURCES_HPP_ */
