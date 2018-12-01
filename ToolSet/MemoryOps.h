#pragma once
#include <cstddef>
#include <iostream>

namespace kevDev {
	template<typename T>
	static void mcpy(const T* source, T* destination, size_t size) {
		for (size_t i = 0; i < size; i++) {
			destination[i] = source[i];
		}
	}
}