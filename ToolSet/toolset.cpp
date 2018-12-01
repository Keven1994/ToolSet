#define _CRTDBG_MAP_ALLOC  

#include "SmartVector.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <chrono>

size_t fptr(size_t size) {
	return (size + 1) * 3;
}

class A {


public:
	~A() {
		//std::cout << "DTOR" << std::endl;
	}
	int x = 42;
	bool operator==(const A& other) const {
		if (other.x == this->x) return true;
		return false;
	}
};
std::ostream& operator<<(std::ostream& out, A c) {
	return out << c.x << std::endl;
}

bool operator==(const A& rhs, A& lhs) {
	return rhs.operator==(lhs);
}





void measure(void (*fptr) (void)) {
	std::cout << "start time measurement" << std::endl;

	auto start_time = std::chrono::high_resolution_clock::now();

	fptr();

	auto end_time = std::chrono::high_resolution_clock::now();
	auto time = std::chrono::duration_cast<std::chrono::milliseconds>((end_time - start_time)).count();

	std::cout << "time elapsed: " << time << std::endl;
}

using namespace kevDev;
using setting = typename kevDev::Vector_Setting<kevDev::vector_settings::optimized, kevDev::vector_settings::deepDelete>;


int main() {
	/*
	measure([](){
		std::vector<int> vec{};
		vec.reserve(10000);
		for (int i = 0; i < 10000; i++) {
			vec.push_back(std::rand());
			int j = 0;
			for (auto& elem : vec) {
				vec[j++] = std::rand();
			}
		}
	});

	measure([]() {
		int* arr = new int[10000];
		for (int i = 0; i < 10000; i++) {
			arr[i] = std::rand();
			for (int j = 0; j < 10000; j++) {
				arr[j++] = std::rand();
			}
		}
	});


	measure([]() {
		vector<int, setting> vec(10000);
		for (int i = 0; i < 10000; i++) {
			vec.push_back(std::rand());
			int j = 0;
			for (auto& elem : vec) {
				vec[j++] = std::rand();
			}
		}});*/

	{

		
		//int* x1 = new int(6);
		//int* x2 = new int(6);
		//int* x3 = new int(6);
		//int* x4 = new int(6);
		vector<A, setting> fv(0);
		{
			A A1{}, A2{}, A3{};

			fv.push_back(A1);
			fv.push_back(A2);
			std::cout << fv[A2] << " found";
			fv.push_back(A3);
			vector<A, setting> fv1(0);
			fv1.push_back(A1);
			fv1.push_back(A2);
			fv1.push_back(A3);
		}
		

		//fv.reserve(25);
		//fv[14] = x4;
		//std::cout << fv << std::endl;

		try {
			std::cout << fv << std::endl;
			//auto test = fv[fv.capacity() + 1];
		}
		catch (std::exception& e) {
			std::cerr << __FILE__ << " line: " << __LINE__ << std::endl << "	->" << e.what() << std::endl;
		}
		
	}
	_CrtDumpMemoryLeaks();
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
