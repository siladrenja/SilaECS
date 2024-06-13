#include <iostream>
#include <string>
#include <chrono>
#include "SilaECS.hpp"

constexpr size_t iterations = 1E9;

class Count : public ECS::component {
public:
	void Run() {
		++count;
	}

	static size_t count;
};

size_t Count::count = 0;

struct Functor {
	template <class T>
	static void Invoke(T& print) {
		print.Run();
	}
};

int main() {

	auto t1 = std::chrono::steady_clock::now();
	ECS::Prefab<Count> prefab;
	for (size_t i = 0; i < iterations; i++) {
		prefab.AddComponent(Count());
	}
	auto t2 = std::chrono::steady_clock::now();
	ECS::World<Count> world;
	world.InstantiatePrefab(prefab);

	auto t3 = std::chrono::steady_clock::now();
	world.ApplyToAll<Functor>();
	auto t4 = std::chrono::steady_clock::now();
	std::cout << Count::count << std::endl;
	std::cout << "Prefab Creation: " << ((double)(t2 - t1).count() / 1E9) << "s\n" << 
		"Entity creation: " << ((double)(t3 - t2).count() / 1E9) << "s\n" << 
		"Iteration: " << (t4-t3).count() << "ns\n" << 
		"Total time: " << (t4 - t1).count() << "ns\n" << 
		"Time Per iteration:" << ((double)(t4 - t3).count() / iterations) << "ns\n";
}