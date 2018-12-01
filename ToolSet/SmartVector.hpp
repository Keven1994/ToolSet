#pragma once

#include <type_traits>
#include <cstddef>
#include <exception>
#include "MemoryOps.h"
#include <iostream>
#include <memory>

#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif

namespace kevDev {

	inline constexpr size_t defaultResAcq(size_t size) {
		return (size + 1) * 2;
	}

	template<typename T>
	inline constexpr T* defaultSearchAlgo(const T& elem, T* data, size_t size) {
		//requires equality comparable type
		for (size_t i = 0; i < size; i++) {
			if (data[i] == elem) return data+i;
		}
		return nullptr;
	}

	namespace vector_settings{
		struct useInitialized {};
		struct noSubscriptCheck {};
		struct deepDelete {};
		struct optimized {};
	}

	template<typename T, typename... Ts>
	constexpr bool contains()
	{
		return std::disjunction<std::is_same<T, Ts>...>::value;
	}

	template<typename... Args>
	struct Vector_Setting {
		static inline constexpr bool useInitialized = contains<vector_settings::useInitialized, Args...>();
		static inline constexpr bool deepDelete = contains<vector_settings::deepDelete, Args...>();
		static inline constexpr bool optimized = contains<vector_settings::optimized, Args...>();
		static inline constexpr bool noSubscriptCheck = contains<vector_settings::noSubscriptCheck, Args...>();
	};

	template<typename T,typename setting = Vector_Setting<>, size_t(*resAcqFctPtr)(size_t size) = defaultResAcq>
	class vector {

		static inline constexpr bool optimized = setting::optimized;
		static inline constexpr bool deepDelete = setting::deepDelete && std::is_pointer<T>::value && std::is_destructible<T>::value;
		static inline constexpr bool useInitialized = setting::useInitialized;
		static inline constexpr bool noSubscriptCheck = setting::noSubscriptCheck;

		static inline constexpr bool isArray = std::is_array<T>::value;
		static_assert(!isArray, "Arrays cannot be used in vector -> use Array instead of vector");

		T* mdata = nullptr;
		size_t msize = 0, mcapacity = 0;
		using iterator = T * ;
		using const_iterator = const T*;

		vector(size_t startCapacity, size_t size) : mcapacity(startCapacity), msize(size) {
			allocate(startCapacity);
		}

	public:

		vector() = default;

		vector(size_t startCapacity) : vector(startCapacity,0) {}

		vector(const std::initializer_list<T>& list) :vector(list.size(),list.size()) {
			mcpy(list.begin(), mdata, msize);
		}

		template<typename = std::enable_if_t<(!(deepDelete && std::is_pointer<T>::value))>>
		vector(const vector& other) : vector(other.mcapacity,other.msize) {
			mcpy(other.mdata, mdata, msize);
		}

		vector(vector&& other) {
			swap(other);
		}

		template<typename = std::enable_if_t<(!(deepDelete && std::is_pointer<T>::value))>>
		void operator=(const vector& other) {
			vector tmp{ other };
			swap(other);
		}

		void operator=(vector&& other) {
			vector tmp{ other };
			swap(other);
		}

		void operator=(const std::initializer_list<T>& other) {
			vector tmp{ other };
			swap(other);
		}

		void operator+(const vector& other) = delete;
		void operator-(const vector& other) = delete;
		void operator*(const vector& other) = delete;
		void operator/(const vector& other) = delete;
		void operator~() = delete;
		void operator-() = delete;

		void reserve(size_t size) noexcept {
			if (!mdata) {
				allocate(size);
			}
			else resize(size);
		}

		[[nodiscard]] size_t size() const noexcept {
			return msize;
		}

		[[nodiscard]] T* data() const noexcept {
			return mdata;
		}

		[[nodiscard]] size_t capacity() const noexcept{
			return mcapacity;
		}

		void resize(size_t size) noexcept {
			T* old = mdata;
			allocate(size);
			mcpy(old, mdata, msize);
			delData(old);
		}

		void pop_back() noexcept {
			delElement(msize--);
		}

		void assign(T* arr) noexcept {
			delData(mdata);
			mdata = arr;
		}

		void assign(vector& arr) noexcept {
			delData(mdata);
			mdata = arr.mdata;
			arr.mdata = nullptr;
		}

		void swap(vector& other) noexcept {
			std::swap(this->mdata, other.mdata);
			std::swap(this->mcapacity, other.mcapacity);
			std::swap(this->msize, other.msize);
		}

		template<typename = std::enable_if_t<std::is_scalar<T>::value>>
		void push_back(T elem) noexcept {
			if (msize < mcapacity) {
				mdata[msize++] = elem;
			} else {
				auto newCap = resAcqFctPtr(mcapacity);
				resize(newCap);
				mdata[msize++] = elem;
			}
		}

		void push_back(const T& elem) noexcept {
			if (msize < mcapacity) {
				mdata[msize++] = elem;
			}
			else {
				auto newCap = resAcqFctPtr(mcapacity);
				resize(newCap);
				mdata[msize++] = elem;
			}
		}

		[[nodiscard]] iterator begin() noexcept {
			return mdata;
		}

		[[nodiscard]] const iterator begin() const noexcept {
			return mdata;
		}

		[[nodiscard]] iterator end() noexcept {
			return mdata + msize;
		}

		[[nodiscard]] const iterator end() const noexcept {
			return mdata + msize;
		}

		[[nodiscard]] T& operator[](size_t index) {
			if constexpr (!noSubscriptCheck) {
				if (!(index < mcapacity)) {
					if constexpr (!optimized) {
						throw std::out_of_range(std::string("subscription was out of bounds by: ") + (std::to_string(index - mcapacity + 1)));
					}
					else {
						return mdata[0];
					}
				}
				else {
					return mdata[index];
				}
			}
			else {
				return mdata[index];
			}
		}

		[[nodiscard]] T* operator[](const T& elem) {
			return defaultSearchAlgo<T>(elem, mdata, msize);
		}

		void clear() noexcept {
			msize = 0;
		}

		[[nodiscard]] bool empty() const noexcept {
			return size;
		}

		~vector() {
			delData(mdata);
		}
		private:

		void delData(T* data) {
			if (data != nullptr) {
				if constexpr (deepDelete) {
					for (size_t i = 0; i < mcapacity; i++) {
						if (data[i] != nullptr)
							delete data[i];
					}
					delete[] data;
				}
				else if constexpr (std::is_destructible<T>::value) {
					delete[] data;
				}
			}
		}

		inline void delElement(size_t index) {
			if constexpr (std::is_pointer<T>::value) {
				if constexpr (deepDelete) {
					delete mdata[index];
					mdata[index] = nullptr;
				}
				else {
					mdata[index] = nullptr;
				}
			}
			else if constexpr (std::is_scalar<T>::value) {
				mdata[index] = 0;
			}
			else {
				mdata[index] = T();
			}
		}

		void allocate(size_t capacity) noexcept {
			mcapacity = capacity;
			if constexpr ((std::is_pointer<T>::value && deepDelete) || useInitialized) {
				mdata = DBG_NEW T[mcapacity]();
			}
			else {
				mdata = DBG_NEW T[mcapacity];
			}
		}
	};

	template<typename T,typename setting = kevDev::Vector_Setting<>, size_t(*resAcqFctPtr)(size_t size) = defaultResAcq, typename = std::true_type>
	void swap(vector<T, setting, resAcqFctPtr>& lhs, vector<T, setting, resAcqFctPtr>& rhs) {
		lhs.swap(rhs);
	}

	template<typename T, typename setting = kevDev::Vector_Setting<>, size_t(*resAcqFctPtr)(size_t size) = defaultResAcq, typename = std::true_type>
	std::ostream& operator<<(std::ostream& out,const vector<T, setting, resAcqFctPtr>& vector) {
		out << "Size: " << vector.size() << " Capacity: " << vector.capacity() << '\n';
		size_t num = 0;
		for (auto& elem : vector) {
			out << "  Element " << num << ": " << elem << '\n';
		}
		return out;
	}

}