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
	int* df = new int(42);
public:
	~A() {
		std::cout << "DTOR "<< *df << std::endl;
		*df = 21;
		delete df;
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
using setting = typename kevDev::Vector_Setting<kevDev::vector_settings::optimized, mtest>;
using setting2 = typename kevDev::Vector_Setting<kevDev::vector_settings::optimized,vector_settings::deepDelete, mtest>;


static_assert(std::is_same_v<mtest,kevDev::vector_settings::maxCount::_16BIT>,"fail");

int main() {
	constexpr auto measuresize = 20000;


	{
		int n1 = 0, n2 = 1;
		int* ptr1 = new int(); int* ptr2 = new int();
		vector<int,setting> v1{};
		v1.push_back(n1);v1.push_back(n2);
		vector<int*,setting> v2{};
		v2.push_back(ptr1);v2.push_back(ptr2);
	}
		{
		A n1{}, n2{};
		A* ptr1 = new A(); A* ptr2 = new A();
		vector<A,setting> v1{};
		v1.push_back(std::move(n1));v1.push_back(std::move(n2));
		vector<A,setting> v3{v1};
		vector<A*,setting> v2{};
		v2.push_back(ptr1);v2.push_back(ptr2);
	}

	measure([]() {
	vector<int, setting> vec(20000);
	for (int i = 0; i < measuresize; i++) {
		vec.push_back(std::rand());
		for (int j = 0; j < measuresize; j++) {
			vec[j++] = std::rand();
		}
	}});


	measure([](){
		std::vector<int> vec{};
		vec.reserve(measuresize);
		for (int i = 0; i < measuresize; i++) {
			vec.push_back(std::rand());
			for (int j = 0; j < measuresize; j++) {
				vec[j++] = std::rand();
			}
		}
	});

	measure([]() {
		int* arr = new int[measuresize];
		for (int i = 0; i < measuresize; i++) {
			arr[i] = std::rand();
			for (int j = 0; j < measuresize; j++) {
				arr[j++] = std::rand();
			}
		}
	});

	#ifdef _WIN32
	_CrtDumpMemoryLeaks();
	#endif
    std::cout << "Hello World!\n"; 
}