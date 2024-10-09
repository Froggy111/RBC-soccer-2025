#include <future> 

struct LoopPromises {
	std::future<void>* input;
	std::future<void>* strategy;
	std::future<void>* output;
};