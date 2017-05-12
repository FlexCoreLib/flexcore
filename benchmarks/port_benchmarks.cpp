#include <benchmark/benchmark.h>

#include <flexcore/core/connection.hpp>
#include <flexcore/core/connectables.hpp>

#include "benchmarkfunctions.h"

#include <random>

namespace fc
{
/// Performance benchmarks of flexcore
namespace bench
{


// benchmnark of flexcore basic functions, nodes, ports etc.
// tested against std function, function calls, function pointers etc.

using fc::operator>>;


void lambda(benchmark::State& state) {
	std::random_device rd;
	std::mt19937 gen(rd());


	float x = gen();

	while (state.KeepRunning()) {
		benchmark::DoNotOptimize(x);

		const float a = fc::identity{}(x);

		assert(a == x);
		benchmark::DoNotOptimize(a);
	}
}

void pure_port(benchmark::State& state) {
	std::random_device rd;
	std::mt19937 gen(rd());


	float x = gen();
	float a = 0.0;

	fc::pure::event_sink<float> sink{[&a](float in){ a = in; }};

	while (state.KeepRunning()) {
		benchmark::DoNotOptimize(x);

		sink(x);

		assert(a == x);
		benchmark::DoNotOptimize(a);
	}
}

void virtual_function(benchmark::State& state) {
	std::random_device rd;
	std::mt19937 gen(rd());


	float x = gen();

	//try to hide the dynmic class
	//by constructing it in a different compilation unit.
	std::unique_ptr<base_class> obj
			= make_inherited();

	while (state.KeepRunning()) {
		benchmark::DoNotOptimize(x);

		const float a = obj->foo(x);

		assert(a == x);
		benchmark::DoNotOptimize(a);
	}
}

void extended_node(benchmark::State& state) {
	std::random_device rd;
	std::mt19937 gen(rd());


	float x = gen();

	identity_node node{};
	//make sure we create the connection outside of the loop
	//we don't want to measure the overhead of creating the connection.
	[&x](){ return x;} >> node.internal.in();

	while (state.KeepRunning()) {
		benchmark::DoNotOptimize(x);

		const float a = node.internal.out()();

		assert(a == x);
		benchmark::DoNotOptimize(a);
	}
}



BENCHMARK(lambda);
BENCHMARK(virtual_function);
BENCHMARK(pure_port);
BENCHMARK(extended_node);

}
}
