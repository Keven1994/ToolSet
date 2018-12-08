#pragma once
#include <random>
//merge sort from: http://www.codebind.com/cpp-tutorial/cpp-example-merge-sort-algorithm/
namespace kevDev {
	namespace vector_algorithms {
		namespace details {

			template<typename T>
			static inline T* temp = nullptr;

			template<typename T, typename size_c>
			void Merger(T arr[], size_c lo, size_c  mi, size_c hi) {
				//T *temp = new T[hi - lo + 1];//temporary merger array
				size_c i = lo, j = mi + 1;//i is for left-hand,j is for right-hand
				size_c k = 0;//k is for the temporary array
				while (i <= mi && j <= hi) {
					if (arr[i] <= arr[j])
						temp<T>[k++] = std::move(arr[i++]);
					else
						temp<T>[k++] = std::move(arr[j++]);
				}
				//rest elements of left-half
				while (i <= mi)
					temp<T>[k++] = std::move(arr[i++]);
				//rest elements of right-half
				while (j <= hi)
					temp<T>[k++] = std::move(arr[j++]);
				//copy the mergered temporary array to the original array
				for (k = 0, i = lo; i <= hi; ++i, ++k)
					arr[i] = std::move(temp<T>[k]);
			}
			template<typename T, typename size_c>
			void MergeSortHelper(T arr[], size_c lo, size_c hi) {
				size_c mid;
				if (lo < hi) {
					mid = (lo + hi) >> 1;
					MergeSortHelper<T, size_c>(arr, lo, mid);
					MergeSortHelper<T, size_c>(arr, mid + 1, hi);
					Merger<T, size_c>(arr, lo, mid, hi);
				}
			}
		}

		template<typename T, typename size_c>
		void MergeSort(T arr[], size_c arr_size) noexcept {
			//requires T <= T && T >= T
			details::temp<T> = new T[arr_size];
			details::MergeSortHelper<T,size_c>(arr, 0, arr_size - 1);
			delete[] details::temp<T>;
		}

		template<typename T, typename size_c>
		void RandomSort(T arr[], size_c arr_size) noexcept {
			std::mt19937 eng(std::random_device());
			std::uniform_int_distribution<> distr(0, arr_size-1); 
			bool b;
			size_t l = 0;

			do {
				b = true;
				for (size_c i = 0; i < arr_size-1; i++) {
					if (arr[i] > arr[i + 1]) {
						b = false;
						size_c num = distr(eng);
						T tmp = std::move(arr[num]);
						arr[num] = arr[i];
						arr[i] = tmp;
						l++;
					}

				}
			} while (!b);
			std::cout << "sorted: " << l << std::endl;
		}

		template<typename size_c>
		[[nodiscard]] inline constexpr size_c defaultResAcq(size_c size) noexcept {
			return (size + 1) * 2;
		}

		template<typename T, typename size_c>
		[[nodiscard]] inline constexpr T* linearSearch(const T& elem, T* data, size_c size) noexcept {
			//requires equality comparable type concept will follow when msvc implements it
			for (size_c i = 0; i < size; i++) {
				if (data[i] == elem) return data + i;
			}
			return nullptr;
		}

		template<typename T, typename size_c>
		[[nodiscard]] inline constexpr T* binarySearch(const T& elem, T* data, size_c size) noexcept {
			//requires equality comparable type concept will follow when msvc implements it
			auto mid = (data + size / 2);
			auto right = mid + 1;
			size_c r_size = (size / 2) - 1 + (size % 2);
			//mid
			if (*mid == elem) {
				return mid;
			}
			if (size <= 1) {
				return nullptr;
			}
			if (elem > *mid) return binarySearch(elem, right, r_size);
			else if (elem <= *mid) return binarySearch(elem, data, size / 2);
		}
	}
}