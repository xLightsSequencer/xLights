//
//  Parallel.hpp
//  xLights
//
//

#ifndef Parallel_hpp
#define Parallel_hpp

#include <functional>

void parallel_for(int start, int stop, std::function<void(int)>&& f, int minStep = 1);


#endif /* Parallel_hpp */
