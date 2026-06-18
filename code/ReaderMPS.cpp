/*=====================================================================================

    Filename:     ReaderMPS.cpp

    Description:
        Version:  1.0

    Author:       Peng Lin, penglincs@outlook.com

    Organization: Shaowei Cai Group,
                  State Key Laboratory of Computer Science,
                  Institute of Software, Chinese Academy of Sciences,
                  Beijing, China

=====================================================================================*/

#include "ReaderMPS.h"
#include "LocalCon.h"
#include "LocalSearch/LocalMIP.h"
#include "LocalVar.h"

ReaderMPS::ReaderMPS(LocalMIP& localmip)
    : modelConUtil(localmip.modelConUtil),
      modelVarUtil(localmip.modelVarUtil),
      localConUtil(localmip.localConUtil),
      localVarUtil(localmip.localVarUtil) {}

void ReaderMPS::PushCoeffVarIdx(size_t _conIdx, Value _coeff, const xct::IntVar* _iv) {
  auto& con = modelConUtil.conSet[_conIdx];
  size_t _varIdx = modelVarUtil.MakeVar(_iv, localVarUtil);
  auto& var = modelVarUtil.GetVar(_varIdx);

  var.conIdxSet.push_back(_conIdx);
  var.posInCon.push_back(con.varIdxSet.size());
  con.coeffSet.push_back(_coeff);
  con.varIdxSet.push_back(_varIdx);
  con.posInVar.push_back(var.conIdxSet.size() - 1);
  // maintain term counts on append so live additions don't need a separate snapshot pass
  con.termNum = con.varIdxSet.size();
  var.termNum = var.conIdxSet.size();
}

bool ReaderMPS::TightenBound() {
  for (size_t conIdx = 1; conIdx < modelConUtil.conNum; ++conIdx) {
    auto& modelCon = modelConUtil.conSet[conIdx];
    if (modelCon.varIdxSet.size() == 1) TightenBoundVar(modelCon);
    if (modelCon.varIdxSet.size() == 0) {
      assert(modelCon.coeffSet.size() == 0 && modelCon.posInVar.size() == 0);
      if (modelCon.RHS + 1e-6 >= 0) {
        modelCon.inferSAT = true;
        deleteConNum++;
      } else {
        printf("c   LOCAL-MIP con.rhs %lf\n", modelCon.RHS);
        return false;
      }
    }
  }
  return true;
}

void ReaderMPS::TightenBoundVar(ModelCon& modelCon) {
  Value coeff = modelCon.coeffSet[0];
  auto& modelvar = modelVarUtil.GetVar(modelCon.varIdxSet[0]);
  Value newBound = (modelCon.RHS + FeasibilityTol) / coeff;
  if (coeff > 0 && newBound < modelvar.upperBound)  // x <= bound
    modelvar.SetUpperBound(newBound);
  else if (coeff < 0 && modelvar.lowerBound < newBound)  // x >= bound
    modelvar.SetLowerBound(newBound);
}

bool ReaderMPS::TightBoundGlobally() {
  for (auto& modelVar : modelVarUtil.varSet)
    if (modelVar.IsFixed()) {
      modelVar.SetType(VarType::Fixed);
      fixedIdxs.push_back(modelVar.idx);
    }
  while (fixedIdxs.size() > 0) {
    size_t removeVarIdx = fixedIdxs.back();
    fixedIdxs.pop_back();
    deleteVarNum++;
    ModelVar& removeVar = modelVarUtil.GetVar(removeVarIdx);
    Value removeVarValue = removeVar.lowerBound;
    for (size_t termIdx = 0; termIdx < removeVar.conIdxSet.size(); termIdx++) {
      size_t conIdx = removeVar.conIdxSet[termIdx];
      size_t posInCon = removeVar.posInCon[termIdx];
      ModelCon& modelCon = modelConUtil.getCon(conIdx);
      Value coeff = modelCon.coeffSet[posInCon];
      size_t movedVarIdx = modelCon.varIdxSet.back();
      Value movedCoeff = modelCon.coeffSet.back();
      size_t movedPosInVar = modelCon.posInVar.back();
      modelCon.varIdxSet[posInCon] = movedVarIdx;
      modelCon.coeffSet[posInCon] = movedCoeff;
      modelCon.posInVar[posInCon] = movedPosInVar;
      ModelVar& movedVar = modelVarUtil.GetVar(movedVarIdx);
      assert(movedVar.conIdxSet[movedPosInVar] == conIdx);
      movedVar.posInCon[movedPosInVar] = posInCon;
      modelCon.varIdxSet.pop_back();
      modelCon.coeffSet.pop_back();
      modelCon.posInVar.pop_back();
      if (conIdx == 0)
        modelVarUtil.objBias += coeff * removeVarValue;
      else {
        modelCon.RHS -= coeff * removeVarValue;
        if (modelCon.varIdxSet.size() == 1) {
          TightenBoundVar(modelCon);
          ModelVar& relatedVar = modelVarUtil.GetVar(modelCon.varIdxSet[0]);
          if (relatedVar.type != VarType::Fixed && relatedVar.IsFixed()) {
            relatedVar.SetType(VarType::Fixed);
            fixedIdxs.push_back(relatedVar.idx);
            inferVarNum++;
          }
        } else if (modelCon.varIdxSet.size() == 0) {
          assert(modelCon.coeffSet.size() == 0 && modelCon.posInVar.size() == 0);
          if (modelCon.RHS + 1e-2 >= 0) {
            modelCon.inferSAT = true;
            deleteConNum++;
          } else {
            printf("c   LOCAL-MIP con.rhs %lf\n", modelCon.RHS);
            return false;
          }
        }
      }
    }
  }
  return true;
}

bool ReaderMPS::SetVarType() {
  // Sync localCon.RHS with modelCon.RHS after TightBoundGlobally may have shifted it
  // (by removeVarValue * coeff for each eliminated fixed variable). Skip conIdx 0:
  // the objective's localCon.RHS is managed by InitState / UpdateBestSolution.
  // For inferSAT constraints, force localCon.RHS = Infinity so SAT/UNSAT checks
  // can never flip them into unsatConIdxs (the inferSAT criterion uses a looser
  // tolerance than LocalCon::UNSAT, so they would otherwise leak in).
  for (size_t conIdx = 1; conIdx < modelConUtil.conSet.size(); ++conIdx) {
    auto& modelCon = modelConUtil.conSet[conIdx];
    localConUtil.conSet[conIdx].RHS = modelCon.inferSAT ? Infinity : modelCon.RHS;
  }
  localVarUtil.binaryIdx.clear();
  localVarUtil.binaryIdxPos.assign(modelVarUtil.varNum, -1);
  for (size_t varIdx = 0; varIdx < modelVarUtil.varNum; varIdx++) {
    auto& modelVar = modelVarUtil.GetVar(varIdx);
    modelVar.termNum = modelVar.conIdxSet.size();
    if (modelVar.lowerBound >= modelVar.upperBound + FeasibilityTol) {
      printf("c Inconsistent LS variable bounds: %s LB %lf UB %lf\n", modelVar.iv->name.c_str(), modelVar.lowerBound,
             modelVar.upperBound);
      return false;
    }
    if (modelVar.IsFixed()) {
      modelVar.SetType(VarType::Fixed);
    } else if (modelVar.IsBinary()) {
      modelVar.SetType(VarType::Binary);
      localVarUtil.binaryIdxPos[varIdx] = static_cast<int64_t>(localVarUtil.binaryIdx.size());
      localVarUtil.binaryIdx.push_back(varIdx);
    } else {
      assert(modelVar.type == VarType::Integer);
    }
  }
  for (size_t conIdx = 0; conIdx < modelConUtil.conNum; conIdx++) {
    auto& modelCon = modelConUtil.getCon(conIdx);
    modelCon.termNum = modelCon.varIdxSet.size();
    if (modelCon.inferSAT) assert(modelCon.termNum == 0);
  }
  return true;
}

// Remove every term of constraint _conIdx, swap-popping each occurrence out of the corresponding
// variable's adjacency lists and fixing the back-reference of whichever (other) constraint's term
// got moved into the freed slot. A variable refers to a constraint at most once (normalized input),
// so the moved entry never belongs to _conIdx itself -- hence _conIdx's own arrays (still being
// iterated) are never touched. The constraint's own arrays are cleared at the end.
void ReaderMPS::ClearConstraintTerms(size_t _conIdx) {
  auto& con = modelConUtil.getCon(_conIdx);
  for (size_t t = 0; t < con.varIdxSet.size(); ++t) {
    size_t varIdx = con.varIdxSet[t];
    size_t posInVar = con.posInVar[t];  // index of this con within the var's adjacency
    auto& var = modelVarUtil.GetVar(varIdx);
    size_t last = var.conIdxSet.size() - 1;
    if (posInVar != last) {
      size_t movedConIdx = var.conIdxSet[last];
      size_t movedPosInCon = var.posInCon[last];  // term index of var inside movedConIdx
      var.conIdxSet[posInVar] = movedConIdx;
      var.posInCon[posInVar] = movedPosInCon;
      // movedConIdx's term at movedPosInCon now finds var at adjacency index posInVar
      modelConUtil.getCon(movedConIdx).posInVar[movedPosInCon] = posInVar;
    }
    var.conIdxSet.pop_back();
    var.posInCon.pop_back();
    var.termNum = var.conIdxSet.size();
  }
  con.coeffSet.clear();
  con.varIdxSet.clear();
  con.posInVar.clear();
  con.termNum = 0;
}

void ReaderMPS::SetVarIdx2ObjIdx() {
  // Fully reset: on a live objective change, stale entries from the previous objective must be cleared.
  std::fill(modelVarUtil.varIdx2ObjIdx.begin(), modelVarUtil.varIdx2ObjIdx.end(), static_cast<size_t>(-1));
  modelVarUtil.varIdx2ObjIdx.resize(modelVarUtil.varNum, -1);
  const auto& modelObj = modelConUtil.conSet[0];
  for (size_t idx = 0; idx < modelObj.termNum; ++idx) modelVarUtil.varIdx2ObjIdx[modelObj.varIdxSet[idx]] = idx;
}