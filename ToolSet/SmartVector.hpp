/*Copyright (c) <2018> <Keven Klöckner>

Permission is hereby granted, free of charge, 
to any person obtaining a copy of this software and associated documentation files (the "Software"), 
to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

The above copyright notice and this permission notice shall be 
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
*/

#pragma once

#include <type_traits>
#include <cstddef>
#include <climits>
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

	template<typename size_c>
	inline constexpr size_c defaultResAcq(size_c size) {
		return (size + 1) * 2;
	}

	template<typename T,typename size_c>
	inline constexpr T* defaultSearchAlgo(const T& elem, T* data, size_c size) {
		//requires equality comparable type concept will follow when msvc implements concepts
		for (size_c i = 0; i < size; i++) {
			if (data[i] == elem) return data+i;
		}
		return nullptr;
	}

	template<typename T>
	struct checkPointerPointer {
		static inline constexpr bool value = false;
	};

	template<typename T>
	struct checkPointerPointer<T**> {
		static inline constexpr bool value = true;
	};

	namespace vector_settings{
		namespace maxCount{
			struct _8BIT{};
			struct _16BIT{};
			struct _32BIT{};
			struct _64BIT{};
		}
		struct useInitialized {};
		 //consider to use this only when you dont know your bounds @ compile time and you are safe to dont get out of bounds
		struct noSubscriptCheck {};

		template<bool isArray>
		struct deepDelete {
			static inline constexpr bool value = false;
		};

		template<>
		struct deepDelete<true> {
			static inline constexpr bool value = true;
		};

		struct optimized {};
	}

	template<size_t x>
	struct CalculateMaxCount {
		using type = 
		typename std::conditional<x <= UINT8_MAX, vector_settings::maxCount::_8BIT,
			typename std::conditional<x <= UINT16_MAX, vector_settings::maxCount::_16BIT,
				typename std::conditional<x <= UINT32_MAX, vector_settings::maxCount::_32BIT, vector_settings::maxCount::_64BIT
				>::type
			>::type
		>::type;
	};

	template<typename T, typename... Ts>
	constexpr bool contains()
	{
		return std::disjunction<std::is_same<T, Ts>...>::value;
	}

	template<typename... Args>
	struct Vector_Setting {
		using size_c = typename std::conditional<contains<vector_settings::maxCount::_8BIT, Args...>(), uint8_t,
		typename std::conditional<contains<vector_settings::maxCount::_16BIT, Args...>(), uint16_t,
		typename std::conditional<contains<vector_settings::maxCount::_32BIT, Args...>(), uint32_t,uint64_t>::type>::type>::type;
		static inline constexpr bool useInitialized = contains<vector_settings::useInitialized, Args...>();
		static inline constexpr bool deepDelete = contains<vector_settings::deepDelete<true>, Args...>() || 
													contains<vector_settings::deepDelete<false>, Args...>();
		static inline constexpr bool isArray = contains<vector_settings::deepDelete<true>, Args...>();
		static inline constexpr bool optimized = contains<vector_settings::optimized, Args...>();
		static inline constexpr bool noSubscriptCheck = contains<vector_settings::noSubscriptCheck, Args...>();
	};

	template<typename T, typename setting = Vector_Setting<>, typename setting::size_c(*resAcqFctPtr)(typename setting::size_c size) = defaultResAcq>
	struct Vector_Algorithms{
		static inline constexpr auto RessourceAcqFctPtr = resAcqFctPtr;
	};

	template<typename T,typename setting = Vector_Setting<>, typename Algorithms = Vector_Algorithms<T>>
	class vector {
		using size_c = typename setting::size_c;
		static inline constexpr auto RessourceAcqFctPtr = Algorithms::RessourceAcqFctPtr;
		static inline constexpr bool optimized = setting::optimized;
		static inline constexpr bool deepDelete = setting::deepDelete && std::is_pointer<T>::value && std::is_destructible<T>::value;
		static inline constexpr bool useInitialized = setting::useInitialized;
		static inline constexpr bool noSubscriptCheck = setting::noSubscriptCheck;
		static_assert(!std::is_array<T>::value, "stack Arrays cannot be used in vector -> use Array instead of vector");
		static_assert(!(checkPointerPointer<T>::value && deepDelete), "Pointer to Pointer ist not allowed when using deep Delete");
		

		T* mdata = nullptr;
		size_c mcapacity = 0,msize = 0;
		using iterator = T * ;
		using const_iterator = const T*;

		vector(size_c startCapacity, size_c size) : mcapacity(startCapacity), msize(size) {
			if(startCapacity > 0)
				allocate(startCapacity);
		}

	public:
		vector() = default;

		vector(size_c startCapacity) : vector(startCapacity,0) {}

		vector(const std::initializer_list<T>& list) :vector(static_cast<size_c>(list.size()),static_cast<size_c>(list.size())) {
			mcpy(list.begin(), mdata, msize);
		}

		vector(const vector& other) : vector(other.mcapacity,other.msize) {
			static_assert(!deepDelete,"do not copy while deepDelete is active!");
			mcpy(other.mdata, mdata, msize);
		}

		vector(vector&& other) {
			swap(other);
		}

		void operator=(const vector& other) {
			static_assert(!deepDelete,"do not copy while deepDelete is active!");
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

		void reserve(size_c size) noexcept {
#if !(defined(DEBUG) || defined(_DEBUG))				
			if constexpr(!optimized){
#endif
				if (size > 0) {
					if (!mdata) {
						allocate(size);
					}
					else resize(size);
				}
			#if !(defined(DEBUG) || defined(_DEBUG))
			} else {

				if (!mdata) {
					allocate(size);
				}
				else resize(size);
				
			}
			#endif
		}

		[[nodiscard]] size_c size() const noexcept {
			return msize;
		}

		[[nodiscard]] T* data() const noexcept {
			return mdata;
		}

		[[nodiscard]] size_c capacity() const noexcept{
			return mcapacity;
		}

		void resize(size_c size) noexcept {
			T* old = mdata;
			allocate(size);
			mcpy(old, mdata, msize);
			delData(old);
		}

		void pop_back() noexcept {
			delElement(--msize);
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

		void push_back(const T& elem) {
			if (msize < mcapacity) {
				mdata[msize++] = elem;
			}
			else {
				auto newCap = RessourceAcqFctPtr(mcapacity);
				resize(newCap);
				mdata[msize++] = elem;
			}
		}

		void push_back(T&& elem) {
			if (msize < mcapacity) {
				mdata[msize++] = elem;
			}
			else {
				auto newCap = RessourceAcqFctPtr(mcapacity);
				resize(newCap);
				mdata[msize++] = elem;
			}
		}

		[[nodiscard]] iterator begin() noexcept {
			return mdata;
		}

		[[nodiscard]] const_iterator begin() const noexcept {
			return const_cast<T*>(mdata);
		}

		[[nodiscard]] iterator end() noexcept {
			return mdata + msize;
		}

		[[nodiscard]] const_iterator end() const noexcept {
			return const_cast<T*>(mdata + msize);
		}

		[[nodiscard]] T& operator[](size_c index) {
			if constexpr (!noSubscriptCheck) {
				if (!(index < mcapacity)) {
					if constexpr (!optimized) {
						throw std::out_of_range(std::string("subscription was out of bounds by: ") + (std::to_string(index - mcapacity + 1)));
					}
					else {
						#if (defined(DEBUG) || defined(_DEBUG))
							std::cerr << "subscription was out of bounds" << std::endl;
						#endif
						
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

		void clear() noexcept {
			msize = 0;
		}

		[[nodiscard]] bool empty() const noexcept {
			return size;
		}

		~vector() noexcept {
			delData(mdata);
		}
		private:

		void delData(T* data) {
			if (data != nullptr) {
				if constexpr (deepDelete) {
					for (size_c i = 0; i < msize; i++) {
					if constexpr(setting::isArray){
						if(data[i] != nullptr)
							delete[] data[i];
					}
					else {
						if(data[i] != nullptr)
							delete data[i];
					}
					}
					delete[] data;
				}
				else if constexpr (std::is_destructible<T>::value) {
					delete[] data;
				}
			}
		}

		inline void delElement(size_c index) {
			if constexpr (std::is_pointer<T>::value) {
				if constexpr (deepDelete) {
					if constexpr(setting::isArray){
						delete[] mdata[index];
					}
					else {
						delete mdata[index];
					}
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

		void allocate(size_c capacity) noexcept {
			mcapacity = capacity;
			if constexpr ((std::is_pointer<T>::value && deepDelete) || useInitialized) {
				mdata = DBG_NEW T[mcapacity]();
			}
			else {
				mdata = DBG_NEW T[mcapacity];
			}
		}
	};

	template<typename T,typename setting = Vector_Setting<>, typename Algorithms = Vector_Algorithms<T>>
	void swap(vector<T, setting, Algorithms>& lhs, vector<T, setting, Algorithms>& rhs) {
		lhs.swap(rhs);
	}

	template<typename T, typename setting = kevDev::Vector_Setting<>, typename Algorithms = Vector_Algorithms<T>>
	std::ostream& operator<<(std::ostream& out,const vector<T, setting, Algorithms>& vector) {
		out << "Size: " << vector.size() << " Capacity: " << vector.capacity() << '\n';
		typename setting::size_c num = 0;
		for (auto& elem : vector) {
			out << "  Element " << num << ": " << elem << '\n';
		}
		return out;
	}

}