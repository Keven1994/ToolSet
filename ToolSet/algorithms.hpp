#pragma once
namespace kevDev {
	namespace vector_algorithms {
		namespace details {
			template<typename T, typename size_c>
			void Merger(T arr[], size_c lo, size_c  mi, size_c hi) {
				T *temp = new T[hi - lo + 1];//temporary merger array
				size_c i = lo, j = mi + 1;//i is for left-hand,j is for right-hand
				size_c k = 0;//k is for the temporary array
				while (i <= mi && j <= hi) {
					if (arr[i] <= arr[j])
						temp[k++] = arr[i++];
					else
						temp[k++] = arr[j++];
				}
				//rest elements of left-half
				while (i <= mi)
					temp[k++] = arr[i++];
				//rest elements of right-half
				while (j <= hi)
					temp[k++] = arr[j++];
				//copy the mergered temporary array to the original array
				for (k = 0, i = lo; i <= hi; ++i, ++k)
					arr[i] = temp[k];

				delete[]temp;
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
			details::MergeSortHelper<T,size_c>(arr, 0, arr_size - 1);
		}

		template<typename size_c>
		[[nodiscard]] inline constexpr size_c defaultResAcq(size_c size) noexcept {
			return (size + 1) * 2;
		}

		template<typename T, typename size_c>
		[[nodiscard]] inline constexpr T* defaultSearchAlgo(const T& elem, T* data, size_c size) noexcept {
			//requires equality comparable type concept will follow when msvc implements it
			for (size_c i = 0; i < size; i++) {
				if (data[i] == elem) return data + i;
			}
			return nullptr;
		}
	}
}