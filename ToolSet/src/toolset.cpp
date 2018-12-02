#define _CRTDBG_MAP_ALLOC  
//#define DEBUG
#include "../SmartVector.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <chrono>

size_t fptr(size_t size);

size_t fptr(size_t size) {
	return (size + 1) * 3;
}

class A {

public:
	~A() {
		std::cout << "DTOR" << std::endl;
	}
	int x = 42;
	bool operator==(const A& other) const {
		if (other.x == this->x) return true;
		return false;
	}
};
std::ostream& operator<<(std::ostream& out, A c) ;
std::ostream& operator<<(std::ostream& out, A c) {
	return out << c.x << std::endl;
}
bool operator==(const A& rhs, A& lhs);
bool operator==(const A& rhs, A& lhs) {
	return rhs.operator==(lhs);
}



void measure(void (*fptr) (void)) ;

void measure(void (*fptr) (void)) {
	std::cout << "start time measurement" << std::endl;

	auto start_time = std::chrono::high_resolution_clock::now();

	fptr();

	auto end_time = std::chrono::high_resolution_clock::now();
	auto time = std::chrono::duration_cast<std::chrono::milliseconds>((end_time - start_time)).count();

	std::cout << "time elapsed: " << time << std::endl;
}
using mtest = typename kevDev::CalculateMaxCount<20000>::type;
using namespace kevDev;
using setting = typename kevDev::Vector_Setting<kevDev::vector_settings::optimized,vector_settings::noSubscriptCheck, mtest>;


static_assert(std::is_same_v<mtest,kevDev::vector_settings::maxCount::_16BIT>,"fail");

int main() {
		constexpr auto measuresize = 20000;

		measure([]() {
		vector<int, setting> vec(20000);
		for (int i = 0; i < measuresize; i++) {
			vec.push_back(std::rand());
			for (int j = 0; j < measuresize; j++) {
				vec[j++] += std::rand();
			}
		}});


	measure([](){
		std::vector<int> vec{};
		vec.reserve(measuresize);
		for (int i = 0; i < measuresize; i++) {
			vec.push_back(std::rand());
			for (int j = 0; j < measuresize; j++) {
				vec[j++] += std::rand();
			}
		}
	});

	measure([]() {
		int* arr = new int[measuresize];
		for (int i = 0; i < measuresize; i++) {
			arr[i] = std::rand();
			for (int j = 0; j < measuresize; j++) {
				arr[j++] += std::rand();
			}
		}
	});

	#ifdef _WIN32
	_CrtDumpMemoryLeaks();
	#endif
    std::cout << "Hello World!\n"; 
}