#pragma once
#include <chrono>

namespace dprobe {
using time_point = std::chrono::time_point<std::chrono::high_resolution_clock>;
using std::chrono::milliseconds;
}
