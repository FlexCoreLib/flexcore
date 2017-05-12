#include <benchmark/benchmark.h>

#include <flexcore/range/actions.hpp>
#include <flexcore/core/connection.hpp>

#include <random>
#include <algorithm>

namespace fc
{
namespace bench
{

// Benchmark of flexcore range functions

using fc::operator>>;

struct map_loop {
	decltype(auto) operator()(std::vector<float> in, float x, float y) {
		for (size_t i = 0; i != in.size(); ++i) {
			in[i] = y + x * in[i];
		}
		return in;
	}
};

struct fc_map_inline {
	decltype(auto) operator()(std::vector<float> in, float x, float y) {
		return (fc::actions::map([x](auto in) {return x * in;})
				>> fc::actions::map([y](auto in) {return y + in;}))(std::move(in));
	}
};

struct fc_map {
	decltype(auto) operator()(std::vector<float> in, float x, float y) {
		return fc::actions::map([x,y](auto in) {return y + x * in;})(std::move(in));
	}
};

constexpr auto filter_factor = 0.25;
constexpr auto filter_value = filter_factor * 10000;

struct filter_loop {
	decltype(auto) operator()(std::vector<float> in, float x, float y) {
		for (size_t i = 0; i != in.size(); ++i) {
			if(in[i] > filter_value)
				in[i] = y + x * in[i];
		}
		return in;
	}
};

struct fc_filter_map {
	decltype(auto) operator()(std::vector<float> in, float x, float y) {
		return (fc::actions::filter([](auto in){ return in > filter_value;})
				>> fc::actions::map([x](auto in) {return x * in;})
				>> fc::actions::map([y](auto in) {return y + in;}))(std::move(in));
	}
};

constexpr auto benchmark_size = 2 << 15;

/// Copying a std::vector serves as a simple baseline
static void VectorCopy(benchmark::State& state) {
	std::vector<int> vec(state.range(0));
	while (state.KeepRunning())
	{
		std::vector<int> copy(vec);
		benchmark::DoNotOptimize(copy);
	}
}

template<class T> void vector_f(benchmark::State& state) {
	T f;

	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_real_distribution<> d(0, 10000);
	std::vector<float> a(state.range(0));
	std::vector<float> b(state.range(0));

	std::generate(b.begin(), b.end(), [&]() {return d(gen);});


	float x = d(gen);
	float y = d(gen);
	while (state.KeepRunning()) {
		auto tmp = b;
		benchmark::DoNotOptimize(tmp);

		a = f(tmp,x,y);

		benchmark::DoNotOptimize(a);
	}
}

BENCHMARK(VectorCopy)
		->RangeMultiplier(2)->Range(64, benchmark_size);
BENCHMARK_TEMPLATE(vector_f, map_loop)
		->RangeMultiplier(2)->Range(64, benchmark_size);
BENCHMARK_TEMPLATE(vector_f, fc_map)
		->RangeMultiplier(2)->Range(64, benchmark_size);
BENCHMARK_TEMPLATE(vector_f, fc_map_inline)
		->RangeMultiplier(2)->Range(64, benchmark_size);

BENCHMARK_TEMPLATE(vector_f, filter_loop)
		->RangeMultiplier(2)->Range(64, benchmark_size);
BENCHMARK_TEMPLATE(vector_f, fc_filter_map)
		->RangeMultiplier(2)->Range(64, benchmark_size);

}
}

BENCHMARK_MAIN()
