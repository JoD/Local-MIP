/*=====================================================================================

    Filename:     header.h

    Description:
        Version:  1.0

    Author:       Peng Lin, penglincs@outlook.com

    Organization: Shaowei Cai Group,
                  State Key Laboratory of Computer Science,
                  Institute of Software, Chinese Academy of Sciences,
                  Beijing, China

=====================================================================================*/

#pragma once

#include <stdlib.h>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include "../../../auxiliary.hpp"
#include "../../../interface//IntConstraint.hpp"

using Value = double;
const Value Infinity = 1e20;
const Value NegativeInfinity = -Infinity;
const Value DefaultIntegerUpperBound = 1.0;
const Value DefaultRealUpperBound = Infinity;
const Value DefaultLowerBound = 0.0;
const Value InfiniteUpperBound = Infinity;
const Value InfiniteLowerBound = NegativeInfinity;
const Value FeasibilityTol = 1e-6;
const Value OptimalTol = 1e-4;
enum class VarType { Binary, Integer, Fixed };
