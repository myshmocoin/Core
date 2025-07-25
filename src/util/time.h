// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2020 The Bitcoin Core developers
// Copyright (c) 2025 The ShmoCoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_UTIL_TIME_H
#define BITCOIN_UTIL_TIME_H

#include <stdint.h>
#include <string>
#include <chrono>

using namespace std::chrono_literals;

void UninterruptibleSleep(const std::chrono::microseconds& n);

/**
 * Helper to count the seconds of a duration/time_point.
 *
 * All durations/time_points should be using std::chrono and calling this should generally
 * be avoided in code. Though, it is still preferred to an inline t.count() to
 * protect against a reliance on the exact type of t.
 *
 * This helper is used to convert durations/time_points before passing them over an
 * interface that doesn't support std::chrono (e.g. RPC, debug log, or the GUI)
 */
template <typename Dur1, typename Dur2>
inline auto Ticks(Dur2 d)
{
    return std::chrono::duration_cast<Dur1>(d).count();
}
template <typename Duration, typename Timepoint>
inline auto TicksSinceEpoch(Timepoint t)
{
    return Ticks<Duration>(t.time_since_epoch());
}
inline int64_t count_seconds(std::chrono::seconds t) { return t.count(); }
inline int64_t count_milliseconds(std::chrono::milliseconds t) { return t.count(); }
inline int64_t count_microseconds(std::chrono::microseconds t) { return t.count(); }

using HoursDouble = std::chrono::duration<double, std::chrono::hours::period>;
using SecondsDouble = std::chrono::duration<double, std::chrono::seconds::period>;
using MillisecondsDouble = std::chrono::duration<double, std::chrono::milliseconds::period>;

/**
 * DEPRECATED
 * Use either GetSystemTimeInSeconds (not mockable) or GetTime<T> (mockable)
 */
int64_t GetTime();

/** Returns the system time (not mockable) */
int64_t GetTimeMillis();
/** Returns the system time (not mockable) */
int64_t GetTimeMicros();
/** Returns the system time (not mockable) */
int64_t GetSystemTimeInSeconds(); // Like GetTime(), but not mockable

/** For testing. Set e.g. with the setmocktime rpc, or -mocktime argument */
void SetMockTime(int64_t nMockTimeIn);
/** For testing */
int64_t GetMockTime();

/** Return system time (or mocked time, if set) */
template <typename T>
T GetTime();

/**
 * ISO 8601 formatting is preferred. Use the FormatISO8601{DateTime,Date}
 * helper functions if possible.
 */
std::string FormatISO8601DateTime(int64_t nTime);
std::string FormatISO8601Date(int64_t nTime);
int64_t ParseISO8601DateTime(const std::string& str);

#endif // BITCOIN_UTIL_TIME_H
