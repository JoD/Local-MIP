/*=====================================================================================

    Filename:     ModelVar.cpp

    Description:
        Version:  1.0

    Author:       Peng Lin, penglincs@outlook.com

    Organization: Shaowei Cai Group,
                  State Key Laboratory of Computer Science,
                  Institute of Software, Chinese Academy of Sciences,
                  Beijing, China

=====================================================================================*/

#include "ModelVar.h"
#include "LocalVar.h"

ModelVar::ModelVar(const xct::IntVar* _iv, size_t _idx)
    : iv(_iv),
      idx(_idx),
      upperBound(static_cast<double>(iv->upperBound)),
      lowerBound(static_cast<double>(iv->lowerBound)),
      termNum(0),
      type(upperBound == 1 && lowerBound == 0 ? type = VarType::Binary : VarType::Integer) {
  // TODO: what with unscaled (binary) variables, e.g., x in [3,4]
}

bool ModelVar::InBound(Value value) const {
  return lowerBound - FeasibilityTol < value && value < upperBound + FeasibilityTol;
}

void ModelVar::SetType(VarType _varType) { type = _varType; }

void ModelVar::SetLowerBound(Value _lowerBound) { lowerBound = ceil(_lowerBound); }

void ModelVar::SetUpperBound(Value _upperBound) { upperBound = floor(_upperBound); }

bool ModelVar::IsFixed() const { return fabs(lowerBound - upperBound) < FeasibilityTol; }

bool ModelVar::IsBinary() const {
  return type == VarType::Binary || (type == VarType::Integer && fabs(lowerBound - 0.0) < FeasibilityTol &&
                                     fabs(upperBound - 1.0) < FeasibilityTol);
}

size_t ModelVarUtil::MakeVar(const xct::IntVar* _iv, LocalVarUtil& lvu) {
  auto iter = iv2idx.find(_iv);
  if (iter != iv2idx.end()) return iter->second;
  size_t varIdx = varSet.size();
  varSet.emplace_back(_iv, varIdx);
  iv2idx[_iv] = varIdx;
  varNum = varSet.size();  // maintain on append for incremental use

  lvu.varSet.emplace_back();
  lvu.scoreTable.push_back(false);
  lvu.binaryIdxPos.push_back(-1);  // not in binaryIdx until SetVarType / setVarBounds says so
  // NOTE: binaryIdx is filled at the end of ReaderMPS::SetVarType, after type promotion to Fixed.

  return varIdx;
}

const ModelVar& ModelVarUtil::GetVar(size_t _idx) const {
  assert(_idx < varSet.size());
  return varSet[_idx];
}

ModelVar& ModelVarUtil::GetVar(size_t _idx) { return varSet[_idx]; }

ModelVar& ModelVarUtil::GetVar(const xct::IntVar* _iv) { return varSet[iv2idx[_iv]]; }