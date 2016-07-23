#include "statistics.hpp"

std::mutex statistics::mutex_;
statistics *statistics::instance_ = NULL;

