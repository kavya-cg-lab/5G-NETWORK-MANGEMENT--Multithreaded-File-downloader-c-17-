#pragma once
#include <algorithm>

// Auto-picks best thread count based on file size.
// User-passed value becomes the MAXIMUM (cap), not the exact count.
//
//  < 1 MB    →  1 thread
//  1–10 MB   →  2 threads
//  10–50 MB  →  4 threads
//  50–200 MB →  6 threads
//  200MB–1GB →  8 threads
//  > 1 GB    → 12 threads

class ThreadAutoCalculator {
public:
    static int calculate(long long fileSizeBytes, int userMaxThreads = 16) {
        int optimal;
        const long long MB = 1024LL * 1024;
        const long long GB = 1024LL * MB;

        if      (fileSizeBytes <    1 * MB) optimal = 1;
        else if (fileSizeBytes <   10 * MB) optimal = 2;
        else if (fileSizeBytes <   50 * MB) optimal = 4;
        else if (fileSizeBytes <  200 * MB) optimal = 6;
        else if (fileSizeBytes <    1 * GB) optimal = 8;
        else                                optimal = 12;

        return std::min(optimal, std::max(1, userMaxThreads));
    }
};