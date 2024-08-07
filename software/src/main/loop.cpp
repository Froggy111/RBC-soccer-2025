#include <future> 

struct LoopPromises {
	std::promise<void>* input;
	std::promise<void>* strategy;
	std::promise<void>* output;
};