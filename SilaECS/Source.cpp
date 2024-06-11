#include <iostream>

#include "SilaECS.hpp"


class PrintHello : public ECS::component {
public:
	void Run() {
		std::cout << "Hello ";
	}
};

class PrintWorld : public ECS::component {
public:
	void Run() {
		std::cout << "World!";
	}
};

class NewLine : public ECS::component {
public:
	void Run() {
		std::cout << "\n";
	}
};

struct WorldRun {
	template<class T>
	static void Invoke(T& element) {
		element.Run();
	}
};

int main() {

	ECS::Prefab<PrintHello, PrintWorld, NewLine> prefab;
	ECS::Prefab<PrintHello, PrintWorld, NewLine> prefab2;

	prefab.AddComponent(PrintHello());
	prefab.AddComponent(PrintWorld());
	prefab.AddComponent(NewLine());


	prefab2.AddComponent(PrintHello());
	prefab2.AddComponent(PrintWorld());
	prefab2.AddComponent(NewLine());

	ECS::World<PrintHello, PrintWorld, NewLine> world;
	world.InstantiatePrefab(prefab);

	ECS::World<PrintHello, PrintWorld, NewLine>::EntityType entity = world.InstantiatePrefab(prefab2);


	world.ApplyToAll<WorldRun>();
	world.ApplyToAllOfType<WorldRun, PrintHello>();
	world.ApplyToAllOfType<WorldRun, NewLine>();
	
	world.ApplyToAllOfEntity<WorldRun>(entity);
	world.ApplyToAllOfTypeOfEntity<WorldRun, PrintWorld>(entity);
}