/*=====================================================================================

    Filename:     LocalVar.h

    Description:
        Version:  1.0

    Author:       Peng Lin, penglincs@outlook.com

    Organization: Shaowei Cai Group,
                  State Key Laboratory of Computer Science,
                  Institute of Software, Chinese Academy of Sciences,
                  Beijing, China

=====================================================================================*/

#pragma once
#include "utils/header.h"

struct LocalVar {
  Value nowValue = 0;
  Value bestValue = 0;
  size_t allowIncStep = 0;
  size_t allowDecStep = 0;
  size_t lastIncStep = 0;
  size_t lastDecStep = 0;
};

struct LocalVarUtil {
  std::vector<LocalVar> varSet;
  std::vector<Value> lowerDeltaInLiftMove;
  std::vector<Value> upperDeltaInLifiMove;
  std::vector<Value> tempDeltas;
  std::vector<size_t> tempVarIdxs;
  std::vector<uint8_t> scoreTable;
  std::vector<size_t> binaryIdx;
  std::vector<int64_t> binaryIdxPos;  // position of each var within binaryIdx, or -1 if absent
  unordered_set<size_t> affectedVar;

  LocalVar& GetVar(size_t _idx);
};