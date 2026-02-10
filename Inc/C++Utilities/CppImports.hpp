#pragma once

#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>
#include <set>
#include <stdint.h>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <chrono>
#include <algorithm>
#include <functional>
#include <array>
#include <bitset>
#include <ctype.h>
#if defined(SIM_ON)
# ifdef __APPLE__
#  include <stdlib.h>  // macOS
# else
#  include <malloc.h>  // Linux/Unix
# endif
#else
# include <malloc.h>
#endif
#include <math.h>
#include <deque>
#include <queue>
#include <stack>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <random>
#include <typeinfo>
#include <forward_list>
#include <ostream>
#include <cstring>
#include <span>
#include <ranges>
#include <cstdarg>
#include <stdarg.h>
