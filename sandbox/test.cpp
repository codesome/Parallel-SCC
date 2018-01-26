#include <iostream>
#include <thread>
#include "AtomicQueue.h"
using namespace std;

int main(int argc, char const *argv[]) {

	AtomicEnDqQueue<int> tasks;
	thread t1([&tasks](){
		for (int i = 0; i < 1e6; ++i) {
			tasks.enqueue(i);
		}
	});
	// thread t2([&tasks](){
	// 	for (int i = 0; i < 1e6; ++i) {
	// 		tasks.enqueue(i);
	// 	}
	// });

	t1.join();
	//t2.join();
	std::cout<<"JOY TANKS\n";
	return 0;
}
