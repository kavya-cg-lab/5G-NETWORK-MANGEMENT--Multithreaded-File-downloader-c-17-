#pragma once
#include <chrono>
#include <mutex>
#include <string>
#include <vector>

// ─────────────────────────────────────────
// Per-thread runtime stats
// ─────────────────────────────────────────
struct ThreadStat {
    long long downloaded = 0;     // bytes written to disk so far
    long long total      = 0;     // expected bytes for this chunk
    double    speedKBs   = 0.0;   // EWMA-smoothed speed KB/s
    double    etaSec     = 0.0;   // estimated seconds remaining
    bool      active     = false; // thread has started
    bool      done       = false; // thread has finished
    double    elapsedSec = 0.0;   // per-thread wall time
};

// ─────────────────────────────────────────
// Thread-safe download progress tracker
// Two mutexes, never held simultaneously:
//   dataMutex_  — protects stats_ vectors
//   printMutex_ — serialises terminal output
// ─────────────────────────────────────────
class ProgressTracker {
public:

    // Call once before any thread starts
    static void init(long long totalSize, int numThreads);

    // Called by curl write callback — lock-free on the hot path
    static void updateThread(int threadId, long long bytes);

    // Reset byte counter for a thread (e.g. before a retry)
    static void resetThread(int threadId);

    // Mark a thread as started (call at beginning of downloadChunk)
    static void markActive(int threadId);

    // Mark a thread done — stores confirmed byte count
    static void markDone(int threadId, long long actualBytes);

    // Print a log line then redraw the live block below it
    static void logLine(const std::string& text);

    // Print the final summary box
    static void finish(long long mergedFileSize = -1);

private:
    // Draw the live progress block — must NOT hold either mutex
    static void renderProgress();

    static long long                                          totalBytes_;
    static int                                               totalThreads_;
    static std::vector<ThreadStat>                           stats_;
    static std::vector<std::chrono::steady_clock::time_point> prevTime_;
    static std::vector<long long>                            prevBytes_;
    static std::vector<std::chrono::steady_clock::time_point> threadStart_;
    static std::chrono::steady_clock::time_point             wallStart_;
    static bool                                              blockReady_;
    static std::mutex                                        dataMutex_;
    static std::mutex                                        printMutex_;
};