#include <vector>
#include <tuple>
namespace ECS {
	class Component {
	public:
		virtual void Update(){}
		virtual void Start(){}
	protected:

	};

	template<class ComponentType>
	class ComponentReference {
	public:
		friend class DynamicComponentContainer;
		ComponentReference(ComponentType* component) : component(component) {}


		ComponentType* operator->() {
			return component;
		}

		ComponentType& operator*() {
			return *component;
		}
	protected:
		ComponentType* component;
	
	};

	

	class DynamicComponentContainer : public Component {
	public:
		void Update() override {
			for (auto& component : components) {
				component->Update();
			}
		}

		void Start() override {
			for (auto& component : components) {
				component->Start();
			}
		}

		template<typename T>
		ComponentReference<T> AddComponent() {
			T* component = new T();
			components.push_back(component);
			return ComponentReference<T>(component);
		}

		template<typename T>
		ComponentReference<T> GetFirstComponentOfType() {
			for (auto& component : components) {
				if (dynamic_cast<T*>(component)) {
					return ComponentReference<T>(dynamic_cast<T*>(component));
				}
			}
			return ComponentReference<T>(nullptr);
		}

		template<typename T>
		std::vector<ComponentReference<T>> GetAllComponentsOfType() {
			std::vector<ComponentReference<T>> componentsOfType;
			for (auto& component : components) {
				if (dynamic_cast<T*>(component)) {
					componentsOfType.push_back(ComponentReference<T>(dynamic_cast<T*>(component)));
				}
			}
			return componentsOfType;
		}

		template<class T>
		void RemoveComponent(ComponentReference<T>& component) {
			for (int i = 0; i < components.size(); i++) {
				if (component.component == components[i]) {
					components.erase(components.begin() + i);
					delete component.component;
					component.component = nullptr;
					return;
				}
			}
		}

		~DynamicComponentContainer() {
			for (auto& component : components) {
				delete component;
			}
		}	

	private:
		std::vector<Component*> components;
	};

	template<class... StaticComponents>
	class System {
	public:
		System(){}

		void Update() const {
			std::apply([](auto&... components) { (components.Update(), ...); }, components);
		}
		void Start() const {
			std::apply([](auto&... components) { (components.Start(), ...); }, components);
		}	
		template<class T>
		void AddDynamicComponent() {
			std::get<0>(components).AddComponent<T>();
		}


	private:
		std::tuple<DynamicComponentContainer, StaticComponents...> components;
	};
	
}