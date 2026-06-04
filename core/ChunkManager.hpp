#ifndef CHUNK_MANAGER_HPP
#define CHUNK_MANAGER_HPP

#include <vector>

std::vector<std::pair<long, long>> divideChunks(long fileSize, int threads);

#endif