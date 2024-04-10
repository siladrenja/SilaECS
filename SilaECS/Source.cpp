#include <iostream>

#include "SilaECS.hpp"


class PrintHello : public ECS::Component {
public:
	void Start() override {
		std::cout << "Hello" << std::endl;
	}
};

class PrintWorld : public ECS::Component {
public:
	void Start() override {
		std::cout << "World" << std::endl;
	}
};

class Dynamic : public ECS::Component {
public:
	void Start()override {
		std::cout << "WUBBA LUBBA DUB DUB \n";
	}
};

int main() {
	ECS::System<PrintHello, PrintWorld> container;
	container.AddDynamicComponent<Dynamic>();
	container.Start();
	container.Update();
}