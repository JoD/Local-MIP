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
#include "utils/paras.h"

class LocalVar
{
public:
  Value nowValue;
  Value bestValue;
  size_t allowIncStep;
  size_t allowDecStep;
  size_t lastIncStep;
  size_t lastDecStep;

  LocalVar();
  ~LocalVar();
};

class LocalVarUtil
{
public:
  std::vector<LocalVar> varSet;
  std::vector<Value> lowerDeltaInLiftMove;
  std::vector<Value> upperDeltaInLifiMove;
  std::vector<Value> tempDeltas;
  std::vector<size_t> tempVarIdxs;
  std::vector<bool> scoreTable;
  std::vector<size_t> binaryIdx;
  std::unordered_set<size_t> affectedVar;

  LocalVarUtil();
  ~LocalVarUtil();
  void Allocate(
      size_t _varNum,
      size_t _varNumInObj);
  LocalVar &GetVar(
      size_t _idx);
};