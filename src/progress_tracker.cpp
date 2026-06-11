#include "../include/progress_tracker.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

using Clock = std::chrono::steady_clock;
using Dur   = std::chrono::duration<double>;

// ── static definitions ──────────────────────────────────────────────────────
long long                       ProgressTracker::totalBytes_   = 0;
int                             ProgressTracker::totalThreads_ = 0;
std::vector<ThreadStat>         ProgressTracker::stats_;
std::vector<Clock::time_point>  ProgressTracker::prevTime_;
std::vector<long long>          ProgressTracker::prevBytes_;
std::vector<Clock::time_point>  ProgressTracker::threadStart_;
Clock::time_point               ProgressTracker::wallStart_;
bool                            ProgressTracker::blockReady_   = false;
std::mutex                      ProgressTracker::dataMutex_;
std::mutex                      ProgressTracker::printMutex_;

// ─────────────────────────────────────────
// init
// ─────────────────────────────────────────
void ProgressTracker::init(long long totalSize, int numThreads) {
    totalBytes_   = totalSize;
    totalThreads_ = numThreads;
    wallStart_    = Clock::now();
    blockReady_   = false;

    stats_.assign(numThreads, ThreadStat{});
    prevTime_.assign(numThreads, Clock::now());
    prevBytes_.assign(numThreads, 0LL);
    threadStart_.assign(numThreads, Clock::now());

    long long base = (numThreads > 0) ? totalSize / numThreads : 0;
    long long rem  = (numThreads > 0) ? totalSize % numThreads : 0;
    for (int i = 0; i < numThreads; ++i)
        stats_[i].total = base + (i == numThreads - 1 ? rem : 0);
}

// ─────────────────────────────────────────
// markActive — thread has started work
// ─────────────────────────────────────────
void ProgressTracker::markActive(int threadId) {
    if (threadId < 0 || threadId >= totalThreads_) return;
    std::lock_guard<std::mutex> lk(dataMutex_);
    stats_[threadId].active = true;
    threadStart_[threadId]  = Clock::now();
    prevTime_[threadId]     = Clock::now();
    prevBytes_[threadId]    = 0;
}

// ─────────────────────────────────────────
// resetThread — zero byte counter before retry
// ─────────────────────────────────────────
void ProgressTracker::resetThread(int threadId) {
    if (threadId < 0 || threadId >= totalThreads_) return;
    std::lock_guard<std::mutex> lk(dataMutex_);
    stats_[threadId].downloaded = 0;
    stats_[threadId].speedKBs   = 0.0;
    stats_[threadId].etaSec     = 0.0;
    prevBytes_[threadId]        = 0;
    prevTime_[threadId]         = Clock::now();
}

// ─────────────────────────────────────────
// updateThread — hot path from curl callback
// Only acquires dataMutex_, never printMutex_
// ─────────────────────────────────────────
void ProgressTracker::updateThread(int threadId, long long bytes) {
    if (threadId < 0 || threadId >= totalThreads_) return;

    std::lock_guard<std::mutex> lk(dataMutex_);
    ThreadStat& s = stats_[threadId];
    s.downloaded += bytes;

    auto   now     = Clock::now();
    double elapsed = Dur(now - prevTime_[threadId]).count();

    if (elapsed >= 0.2) {
        long long delta = s.downloaded - prevBytes_[threadId];
        double    spd   = (delta / 1024.0) / elapsed;
        s.speedKBs = (s.speedKBs < 1e-9)
                     ? spd : 0.7 * spd + 0.3 * s.speedKBs;

        long long rem = s.total - s.downloaded;
        s.etaSec = (s.speedKBs > 0.01)
                   ? (rem / 1024.0) / s.speedKBs : 0.0;

        prevBytes_[threadId] = s.downloaded;
        prevTime_[threadId]  = now;
        s.elapsedSec = Dur(now - threadStart_[threadId]).count();
    }
}

// ─────────────────────────────────────────
// markDone
// ─────────────────────────────────────────
void ProgressTracker::markDone(int threadId, long long actualBytes) {
    if (threadId < 0 || threadId >= totalThreads_) return;
    std::lock_guard<std::mutex> lk(dataMutex_);
    ThreadStat& s = stats_[threadId];
    if (actualBytes > 0) s.downloaded = actualBytes;
    s.done       = true;
    s.active     = false;
    s.etaSec     = 0.0;
    s.speedKBs   = 0.0;
    s.elapsedSec = Dur(Clock::now() - threadStart_[threadId]).count();
}

// ─────────────────────────────────────────
// renderProgress
//
// Block is always exactly N+4 lines:
//   1 header line
//   1 bar line
//   1 overall stats line
//   N thread lines
//   1 footer line
//
// blockReady_ tracks whether a previous block
// exists to overwrite.
// ─────────────────────────────────────────
void ProgressTracker::renderProgress() {

    // ── 1. Snapshot under dataMutex_ ──────────────────────────────
    long long               totalDone = 0;
    int                     doneCnt   = 0;
    double                  wallSec   = 0;
    std::vector<ThreadStat> snap;
    {
        std::lock_guard<std::mutex> lk(dataMutex_);
        for (const auto& s : stats_) {
            totalDone += s.downloaded;
            if (s.done) ++doneCnt;
        }
        wallSec = Dur(Clock::now() - wallStart_).count();
        snap    = stats_;
    }

    // ── 2. Build display values ────────────────────────────────────
    double pct      = (totalBytes_ > 0)
                      ? 100.0 * totalDone / totalBytes_ : 0.0;
    double totalMB  = totalBytes_ / (1024.0 * 1024.0);
    double doneMB   = totalDone   / (1024.0 * 1024.0);
    double speedMBs = (wallSec > 0.1) ? doneMB / wallSec : 0.0;
    double etaTotal = (speedMBs > 0.001)
                      ? (totalMB - doneMB) / speedMBs : 0.0;

    // Progress bar — 40 chars wide
    int bar_fill = static_cast<int>(pct / 2.5);   // 100% → 40 chars
    std::string bar(40, ' ');
    for (int i = 0; i < bar_fill && i < 40; ++i)
        bar[i] = (i == bar_fill - 1 && pct < 100.0) ? '>' : '=';

    // Milestone: each completed thread = one step
    int milestonePct = (totalThreads_ > 0)
                       ? (doneCnt * 100) / totalThreads_ : 0;

    int blockLines = 4 + totalThreads_;

    // ── 3. Write under printMutex_ ────────────────────────────────
    std::lock_guard<std::mutex> lk(printMutex_);

    if (blockReady_) {
        std::cout << "\033[" << blockLines << "A";
    }

    // Header line
    std::cout << "\r\033[K"
              << "┌─ Progress ── " << doneCnt << "/" << totalThreads_
              << " threads done ── milestone: " << milestonePct << "%"
              << " ─────────────────────┐\n";

    // Bar + overall %
    std::cout << "\r\033[K│ ["
              << bar << "] "
              << std::fixed << std::setprecision(1) << std::setw(5) << pct
              << "%  │\n";

    // Overall stats
    std::ostringstream overall;
    overall << std::fixed << std::setprecision(2)
            << doneMB << "/" << totalMB << " MB"
            << "  " << std::setprecision(2) << speedMBs << " MB/s"
            << "  Elapsed: " << std::setprecision(1) << wallSec << "s"
            << "  ETA: " << std::setprecision(0) << etaTotal << "s";
    std::cout << "\r\033[K│ " << std::left << std::setw(54)
              << overall.str() << "│\n";

    // Per-thread lines
    for (int i = 0; i < totalThreads_; ++i) {
        const ThreadStat& s   = snap[i];
        double tPct    = (s.total > 0)
                         ? 100.0 * s.downloaded / s.total : 0.0;
        double tDoneMB = s.downloaded / (1024.0 * 1024.0);
        double tTotMB  = s.total      / (1024.0 * 1024.0);

        std::ostringstream tline;
        tline << "│ T" << i << " │ "
              << std::fixed << std::setprecision(1)
              << std::setw(5) << tPct << "% │ "
              << std::setprecision(2) << std::setw(6) << tDoneMB
              << "/" << std::setw(6) << tTotMB << " MB │ ";

        if (s.done) {
            tline << std::setprecision(1) << std::setw(5) << s.elapsedSec
                  << "s │ Done \u2705   ";
        } else if (s.active) {
            double spd = s.speedKBs / 1024.0;
            tline << std::setprecision(2) << std::setw(5) << spd
                  << " MB/s ETA:"
                  << std::setprecision(0) << std::setw(4) << s.etaSec
                  << "s │ \u23bd\u23bd\u23bd\u23bd   ";
        } else {
            tline << "  ---   │ Waiting ";
        }
        tline << "│";

        std::cout << "\r\033[K" << tline.str() << "\n";
    }

    // Footer
    std::cout << "\r\033[K"
              << "└────────────────────────────────────────────────"
                 "──────────┘\n"
              << std::flush;

    blockReady_ = true;
}

// ─────────────────────────────────────────
// logLine
// Print a message, then re-render the live
// block below it. Mutexes acquired separately.
// ─────────────────────────────────────────
void ProgressTracker::logLine(const std::string& text) {
    {
        std::lock_guard<std::mutex> lk(printMutex_);
        std::cout << "\r\033[K" << text << "\n" << std::flush;
    }
    renderProgress();
}

// ─────────────────────────────────────────
// finish — final summary box
// ─────────────────────────────────────────
void ProgressTracker::finish(long long mergedFileSize) {

    long long               totalDone = 0;
    double                  wallSec   = 0;
    std::vector<ThreadStat> snap;
    {
        std::lock_guard<std::mutex> lk(dataMutex_);
        for (const auto& s : stats_) totalDone += s.downloaded;
        wallSec = Dur(Clock::now() - wallStart_).count();
        snap    = stats_;
    }

    double doneMB   = totalDone   / (1024.0 * 1024.0);
    double totalMB  = totalBytes_ / (1024.0 * 1024.0);
    double speedMBs = (wallSec > 0.1) ? doneMB / wallSec : 0.0;

    std::lock_guard<std::mutex> lk(printMutex_);

    std::cout
        << "\n╔══════════════════════════════════════════════════════╗\n"
        << "║                 DOWNLOAD COMPLETE                   ║\n"
        << "╠══════════════════════════════════════════════════════╣\n"
        << std::fixed
        << "║  Downloaded  : " << std::setw(7) << std::setprecision(2)
        << doneMB << " MB  /  " << std::setw(7) << totalMB << " MB"
        << "          ║\n"
        << "║  Avg speed   : " << std::setw(7) << std::setprecision(2)
        << speedMBs << " MB/s"
        << "                            ║\n"
        << "║  Total time  : " << std::setw(7) << std::setprecision(1)
        << wallSec << " s"
        << "                              ║\n";

    if (mergedFileSize >= 0) {
        double mergedMB = mergedFileSize / (1024.0 * 1024.0);
        bool   ok       = (mergedFileSize == totalBytes_);
        std::cout
            << "║  Merged file : " << std::setw(7) << std::setprecision(2)
            << mergedMB << " MB  "
            << (ok ? "\u2713 Size matches original         "
                   : "\u2717 Size MISMATCH — check chunks  ")
            << "║\n";
    }

    std::cout << "╠══════════════════════════════════════════════════════╣\n";

    for (int i = 0; i < (int)snap.size(); ++i) {
        const ThreadStat& s  = snap[i];
        double td  = s.downloaded / (1024.0 * 1024.0);
        double tt  = s.total      / (1024.0 * 1024.0);
        bool   ok  = s.done && (s.downloaded >= (s.total * 95 / 100));
        std::cout
            << "║  Thread " << i << "     : "
            << std::setprecision(2) << std::setw(7) << td
            << " / " << std::setw(7) << tt << " MB"
            << "  " << std::setprecision(1) << std::setw(5) << s.elapsedSec << "s"
            << "  " << (ok ? "Done \u2705  " : "Incomplete \u2717")
            << "          ║\n";
    }

    std::cout
        << "╚══════════════════════════════════════════════════════╝\n"
        << std::flush;
}