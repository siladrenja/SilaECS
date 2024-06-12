#include <vector>
#include <array>
#include <tuple>
#include <type_traits>
#include <functional>
#include <optional>

namespace ECS {
	template <class... Components>
	class World;
	namespace {
		template<class T>
		class Matrix {
		public:
			Matrix(const size_t rows, const size_t cols) : rows(rows), cols(cols) {
				data = new T[rows * cols];
			}

			~Matrix() {
				delete[] data;
			}

			void PushRow() {
				rows++;
				T* newData = new T[rows * cols];
				for (size_t i = 0; i < rows - 1; i++) {
					for (size_t j = 0; j < cols; j++) {
						newData[i * cols + j] = data[i * cols + j];
					}
				}

				for (size_t j = 0; j < cols; j++) {
					newData[(rows - 1) * cols + j] = { 0 };
				}

				delete[] data;
				data = newData;
			}

			T& operator()(size_t row, size_t col) {
				return data[row * cols + col];
			}

			size_t GetRows() {
				return rows;
			}

			size_t GetCols() {
				return cols;
			}

		private:
			size_t rows;
			size_t cols;
			T* data;
		};


		
	}

	template <typename T, typename... Ts>
	struct Index;
	
	template <typename T>
	struct Index<T> : std::integral_constant<std::size_t, 0> {};;

	template <typename T, typename... Ts>
	struct Index<T, T, Ts...> : std::integral_constant<std::size_t, 0> {};

	template <typename T, typename U, typename... Ts>
	struct Index<T, U, Ts...> : std::integral_constant<std::size_t, 1 + Index<T, Ts...>::value> {};

	template <typename T, typename... Ts>
	constexpr std::size_t Index_v = Index<T, Ts...>::value;
	
	struct Range {
		size_t start;
		size_t size;
	};

	class component {

	};

	template<class... Components>
	class Prefab {
	public:
		template<class T>
		void AddComponent(T component) {
			std::get<std::vector<T>>(this->components).push_back(component);
		}

		template<class...WorldComponents>
		friend class World;
	protected:
		std::tuple<std::vector<Components>...> components;
	};

	
	template <class... Components>
	class Entity {
	public:
		Entity(World<Components...>& world, size_t EntityID) : world(world), EntityID(EntityID) {
		}


		template<class T>
		void AddComponent(T component) {
		}

		
		friend class World<Components...>;
	protected:
		World<Components...>& world;
		size_t EntityID;
	private:
	};

	template <class... Components>
	class World {
	public:

		using EntityType = Entity<Components...>;

		template<class... PrefabComponents>
		EntityType InstantiatePrefab(const Prefab<PrefabComponents...>& prefab) {
			size_t entityID = entityRanges.GetRows();
			entityRanges.PushRow();
			EntityType entity(*this, entityID);

			std::apply([&entity, this](auto Component, auto... remainingComponents) {
				this->AddComponents_Vector(entity, Component, remainingComponents...);
				},  prefab.components);

			return entity;
		}

		template<class Functor>
		constexpr void ApplyToAll() {
			std::apply([this](auto ...x) {this->ApplyVector<Functor>(x...); }, components);
		}

		template<class Functor, class Type>
		constexpr void ApplyToAllOfType() {
			ApplyVector<Functor>(std::get<std::vector<Type>>(components));
		}

		template<class Functor>
		constexpr void ApplyToAllOfEntity(EntityType& entity) {
			std::apply([this, &entity](auto ...x) {this->ApplyEntityVector<Functor>(entity, x...); }, components);
		}

		template<class Functor, class Type>
		constexpr void ApplyToAllOfTypeOfEntity(EntityType& entity) {
			ApplyEntityVector<Functor>(entity, std::get<std::vector<Type>>(components));
		}

		template<class T>
		void AddComponent(EntityType& entity, T component) {
			Range& range = entityRanges(entity.EntityID, Index<T, Components...>::value);
			auto temp = std::get<std::vector<T>>(this->components);
			temp.insert(temp.begin() + range.start + range.size, component);
			
			range.size++;

			for (size_t i = entity.EntityID + 1; i < entityRanges.GetRows(); i++) {
				Range& r = entityRanges(i, Index<T, Components...>::value);
				r.start++;
			}
		}

		template<class T, class... Args>
		constexpr void AddComponents(EntityType& entity, T val, Args... args) {
			AddComponents(entity, val);

			AddComponents(entity, args...);
		}


		template<class T>
		constexpr std::optional<T> GetComponent(const EntityType& entity){
			Range& range = entityRanges(entity.EntityID, Index<T, Components...>::value);
			if (range.size == 0)
				return std::nullopt;
			return std::get<std::vector<T>>(this->components)[range.start];
		}

	protected:
		std::tuple<std::vector<Components>...> components;
		Matrix<Range> entityRanges = Matrix<Range>(0, sizeof...(Components));


		template<class T>
		constexpr void AddComponents(EntityType& entity, T val) {
			AddComponent(entity, val);
		}

		template<class T>
		constexpr void AddComponents_Vector(const EntityType& entity, std::vector<T> component) {
			size_t entityID = entity.EntityID;
			std::vector<T>& c = std::get<std::vector<T>>(this->components);
			Range& range = entityRanges(entityID, Index<T, Components...>::value);
			c.insert(c.begin() + range.start + range.size, component.begin(), component.end());

			range.size += component.size();

			for (size_t i = entityID + 1; i < entityRanges.GetRows(); i++) {
				Range& r = entityRanges(i, Index<T, Components...>::value);
				r.start += component.size();
			}


		}

		template<class T, class... RemainingComponents>
		constexpr void AddComponents_Vector(const EntityType& entity, std::vector<T> component, RemainingComponents... remainingComponents) {
			AddComponents_Vector(entity, component);

			AddComponents_Vector(entity, remainingComponents...);

		}

		template<class Functor, class T>
		constexpr void ApplyVector(std::vector<T>& vec) {
			for (T& element : vec) {
				Functor::Invoke(element);
			}
		}

		template<class Functor, class T, class... Args>
		constexpr void ApplyVector(std::vector<T>& vec, Args... rest) {
			ApplyVector<Functor>(vec);

			ApplyVector<Functor>(rest...);
		}
		
		template<class Functor, class T>
		void ApplyEntityVector(EntityType& entity, std::vector<T>& vec) {
			Range& range = entityRanges(entity.EntityID, Index<T, Components...>::value);
			for (size_t i = range.start; i < range.start + range.size; ++i) {
				Functor::Invoke(vec[i]);
			}
		}

		template<class Functor, class T, class... Args>
		void ApplyEntityVector(EntityType& entity, std::vector<T>& vec, Args... rest) {
			ApplyEntityVector<Functor>(entity, vec);

			ApplyEntityVector<Functor>(entity, rest...);
		}
		
	};

	

	namespace {
		
	}
}