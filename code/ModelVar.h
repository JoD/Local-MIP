/*=====================================================================================

    Filename:     ModelVar.h

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

struct LocalVarUtil;

struct ModelVar {
  const xct::IntVar* iv;
  size_t idx;
  Value upperBound;
  Value lowerBound;
  std::vector<size_t> conIdxSet;
  std::vector<size_t> posInCon;
  size_t termNum;
  VarType type;

  ModelVar(const xct::IntVar* _iv, size_t _idx);

  bool InBound(Value _value) const;
  void SetType(VarType _varType);
  void SetUpperBound(Value _upperBound);
  void SetLowerBound(Value _lowerBound);
  bool IsFixed() const;
  bool IsBinary() const;
};

struct ModelVarUtil {
  unordered_map<const xct::IntVar*, size_t> iv2idx;
  std::vector<ModelVar> varSet;
  std::vector<size_t> varIdx2ObjIdx;
  size_t varNum = -1;
  Value objBias = 0;

  size_t MakeVar(const xct::IntVar* _iv, LocalVarUtil& lvu);
  const ModelVar& GetVar(size_t _idx) const;
  ModelVar& GetVar(size_t _idx);
  ModelVar& GetVar(const xct::IntVar* _iv);
};