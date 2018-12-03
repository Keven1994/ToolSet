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
#include "algorithms.hpp"
#include <iostream>
#include <memory>
#include <vector>
#include "SmartVectorTypes.hpp"

#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif

namespace kevDev {

	template<typename T, typename Setting, typename Algorithms>
	struct Result;

	template<typename T,typename Setting = Vector_Setting<>, typename Algorithms = Vector_Algorithms<T,Setting>>
	class vector {
		using size_c = typename Setting::size_c;
		static inline constexpr auto resizeFactorFunction = Algorithms::ResFctPtr;
		static inline constexpr auto searchAlgorithm = Algorithms::searchAlgo;
		static inline constexpr auto sortAlgorithm = Algorithms::sortAlgo;
		static inline constexpr bool optimized = Setting::optimized;
		static inline constexpr bool deepDelete = Setting::deepDelete && std::is_pointer<T>::value && std::is_destructible<T>::value;
		static inline constexpr bool useInitialized = Setting::useInitialized;
		static inline constexpr bool noSubscriptCheck = Setting::noSubscriptCheck;
		static_assert(!std::is_array<T>::value, "stack Arrays cannot be used in vector -> use Array instead of vector");
		static_assert(!(details::checkPointerPointer<T>::value && deepDelete), "Pointer to Pointer ist not allowed when using deep Delete");
		

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

		explicit operator std::vector<T>() {
			return std::vector<T>(begin(), end());
		}

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
			if(msize >= mcapacity) {
				resize(resizeFactorFunction(mcapacity));
			}
			mdata[msize++] = elem;
		}

		void push_back(T&& elem) {
			if (msize >= mcapacity) {
				resize(resizeFactorFunction(mcapacity));
			}
			mdata[msize++] = elem;
		}

		void sort() noexcept {
			sortAlgorithm(mdata, msize);
		}

		[[nodiscard]] Result<T,Setting,Algorithms> find(const T& elem) const noexcept{
			return Result<T, Setting, Algorithms>(searchAlgorithm(elem, mdata, msize));
		}

		[[nodiscard]] size_c index(const Result<T, Setting, Algorithms>& elem) const noexcept {
			return index((T&)elem);
		}

		[[nodiscard]] size_c index(const T& elem) const noexcept{
			if constexpr (!optimized) {
				T* tmp = &elem;
				size_c result = tmp - mdata;
				if (tmp >= mdata && (result < msize)) {
					result;
				}
				else {
					throw std::out_of_range(std::string("element was not in vector"));
				}
			}
			else {
				return static_cast<size_c>(&elem - mdata);
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

	template<typename T,typename setting = Vector_Setting<>, typename Algorithms = Vector_Algorithms<T,setting>>
	void swap(vector<T, setting, Algorithms>& lhs, vector<T, setting, Algorithms>& rhs) {
		lhs.swap(rhs);
	}

	template<typename T, typename setting = kevDev::Vector_Setting<>, typename Algorithms = Vector_Algorithms<T,setting>>
	std::ostream& operator<<(std::ostream& out,const vector<T, setting, Algorithms>& vector) {
		out << "Size: " << vector.size() << " Capacity: " << vector.capacity() << '\n';
		typename setting::size_c num = 0;
		for (auto& elem : vector) {
			out << "  Element " << num << ": " << elem << '\n';
		}
		return out;
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

	template<typename T, typename Setting, typename Algorithms>
	struct Result {

		friend class vector<T, Setting, Algorithms>;
		Result(const Result&) = delete;
		Result(Result&&) = delete;

		void operator=(Result&&) = delete;
		void operator=(const Result&) = delete;
		~Result() = default;

		operator bool() {
			return mData;
		}
		operator T&() const {
			return *mData;
		}
		operator T() = delete;
	private:
		Result() = default;
		Result(T* data) :mData(data) {}
		T* mData = nullptr;
	};
}