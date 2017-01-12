#ifndef SRC_GRAPH_GRAPH_HPP_
#define SRC_GRAPH_GRAPH_HPP_

#include <boost/functional/hash.hpp>
#include <boost/uuid/uuid.hpp>
#include <flexcore/scheduler/parallelregion.hpp>

#include <map>
#include <set>
#include <unordered_set>

namespace fc
{
/**
 * \brief Contains all classes and functions to access
 * and read the abstract connection graph of a flexcore application.
 */
namespace graph
{

/**
 * \brief Contains the information carried by a node of the dataflow graph
 */
class graph_node_properties
{
public:
	typedef boost::uuids::uuid unique_id;

	explicit graph_node_properties(
			const std::string& name, parallel_region* region, unique_id id, bool is_pure = false);

	explicit graph_node_properties(
			const std::string& name, parallel_region* region, bool is_pure = false);

	explicit graph_node_properties(const std::string& name, bool is_pure = false)
		: graph_node_properties(name, nullptr, is_pure)
	{
	}

	bool operator==(const graph_node_properties& o) const { return id_ == o.id_; }

	const std::string& name() const { return human_readable_name_; }
	std::string& name() { return human_readable_name_; }
	unique_id get_id() const { return id_; }
	parallel_region* region() const { return region_; }
	bool is_pure() const { return is_pure_; }
private:
	std::string human_readable_name_;
	unique_id id_;
	parallel_region* region_;
	bool is_pure_;
};

/**
 * \brief Contains the information carried by a port of the dataflow graph
 */
class graph_port_properties
{
public:
	typedef boost::uuids::uuid unique_id;
	enum class port_type
	{
		UNDEFINED,
		EVENT,
		STATE
	};

	explicit graph_port_properties(std::string description, unique_id owning_node, port_type type);

	template <class T>
	static constexpr port_type to_port_type()
	{
		if (is_event_port<T>{})
			return port_type::EVENT;
		else if (is_state_port<T>{})
			return port_type::STATE;
		else
			return port_type::UNDEFINED;
	}

	bool operator<(const graph_port_properties&) const;
	bool operator==(const graph_port_properties& o) const { return id_ == o.id_; }

	const std::string& description() const { return description_; }
	unique_id owning_node() const { return owning_node_; }
	unique_id id() const { return id_; }
	port_type type() const { return type_; };

private:
	std::string description_;
	unique_id owning_node_;
	unique_id id_;
	port_type type_;
};

struct graph_properties
{
	typedef boost::uuids::uuid unique_id;
	graph_properties(graph_node_properties node, graph_port_properties port)
		: node_properties(std::move(node)), port_properties(std::move(port))
	{
	}
	graph_node_properties node_properties;
	graph_port_properties port_properties;
	bool operator<(const graph_properties& o) const
	{
		assert(!(port_properties == o.port_properties) || node_properties == o.node_properties);
		return port_properties < o.port_properties;
	}
	bool operator==(const graph_properties& o) const
	{
		bool is_equal = port_properties == o.port_properties;
		assert(!is_equal || node_properties == o.node_properties);
		return is_equal;
	}
};

struct graph_edge
{
	graph_edge(graph_properties source, graph_properties sink) : source(source), sink(sink) {}
	graph_properties source;
	graph_properties sink;
	bool operator==(const graph_edge& o) const { return source == o.source && sink == o.sink; }
};

/**
 * \brief The abstract connection graph of a flexcore application.
 *
 * Contains all nodes which where declared with the additional information
 * and edges between these nodes.
 *
 * \invariant Number of vertices/nodes in dataflow_graph == vertex_map.size().
 */
class connection_graph
{
public:
	connection_graph();
	connection_graph(const connection_graph&) = delete;

	/// Adds a new Connection without ports to the graph.
	void add_connection(const graph_properties& source_node, const graph_properties& sink_node);

	void add_port(const graph_properties& port_info);

	const std::set<graph_properties>& ports() const;
	const std::unordered_set<graph_edge>& edges() const;

	/// Prints current state of the abstract graph in graphviz format to stream.
	void print(std::ostream& stream);

	/// deleted the current graph
	void clear_graph();

	~connection_graph();

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

} // namespace graph
} // namespace fc

namespace std
{
template <>
struct hash<fc::graph::graph_edge>
{
	size_t operator()(const fc::graph::graph_edge& e) const
	{
		size_t seed = 0;
		boost::hash_combine(seed, e.source.port_properties.id());
		boost::hash_combine(seed, e.sink.port_properties.id());
		return seed;
	}
};
}

#endif /* SRC_GRAPH_GRAPH_HPP_ */
