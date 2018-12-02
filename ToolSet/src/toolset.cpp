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

using namespace kevDev;
using setting = typename kevDev::Vector_Setting<kevDev::vector_settings::optimized,kevDev::vector_settings::noSubscriptCheck, kevDev::vector_settings::maxCount::_32BIT>;


int main() {

	constexpr auto measuresize = 20000;
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


	measure([]() {
		vector<int, setting> vec(measuresize);
		for (int i = 0; i < measuresize; i++) {
			vec.push_back(std::rand());
			for (int j = 0; j < measuresize; j++) {
				vec[j++] += std::rand();
			}
		}});

	#ifdef _WIN32
	_CrtDumpMemoryLeaks();
	#endif
    std::cout << "Hello World!\n"; 
}

// Programm ausführen: STRG+F5 oder "Debuggen" > Menü "Ohne Debuggen starten"
// Programm debuggen: F5 oder "Debuggen" > Menü "Debuggen starten"

// Tipps für den Einstieg: 
//   1. Verwenden Sie das Projektmappen-Explorer-Fenster zum Hinzufügen/Verwalten von Dateien.
//   2. Verwenden Sie das Team Explorer-Fenster zum Herstellen einer Verbindung mit der Quellcodeverwaltung.
//   3. Verwenden Sie das Ausgabefenster, um die Buildausgabe und andere Nachrichten anzuzeigen.
//   4. Verwenden Sie das Fenster "Fehlerliste", um Fehler anzuzeigen.
//   5. Wechseln Sie zu "Projekt" > "Neues Element hinzufügen", um neue Codedateien zu erstellen, bzw. zu "Projekt" > "Vorhandenes Element hinzufügen", um dem Projekt vorhandene Codedateien hinzuzufügen.
//   6. Um dieses Projekt später erneut zu öffnen, wechseln Sie zu "Datei" > "Öffnen" > "Projekt", und wählen Sie die SLN-Datei aus.
