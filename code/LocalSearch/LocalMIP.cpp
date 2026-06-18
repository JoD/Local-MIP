/*=====================================================================================

    Filename:     LocalMIP.cpp

    Description:
        Version:  1.0

    Author:       Peng Lin, penglincs@outlook.com

    Organization: Shaowei Cai Group,
                  State Key Laboratory of Computer Science,
                  Institute of Software, Chinese Academy of Sciences,
                  Beijing, China

=====================================================================================*/

#include "LocalMIP.h"

int32_t LocalMIP::LocalSearch(const std::function<bool()>& stop_condition,
                              const std::function<void(double, const std::vector<LocalVar>&)>& update_sol,
                              const std::function<void()>& apply_changes) {
  InitSolution();
  InitState();
  auto& localObj = localConUtil.conSet[0];
  curStep = 0;
  while (true) {
    apply_changes();  // drain externally-fixed units etc.; invariants (LHS, unsat list) hold here
    if (localConUtil.unsatConIdxs.empty()) {
      if (!isFoundFeasible || localObj.LHS < localObj.RHS) {
        UpdateBestSolution();
        update_sol(GetObjValue(), localVarUtil.varSet);
        isFoundFeasible = true;
      }
      bool res = LiftMoveWithoutBreak();
      ++curStep;
      if (stop_condition()) break;
      if (res) continue;
    }
    if (stop_condition()) break;
    if (!UnsatTightMove()) {
      if (mt() % 32768 > smoothProbability)
        UpdateWeight();
      else
        SmoothWeight();
      RandomTightMove();
    }
    ++curStep;
  }
  return 0;
}

void LocalMIP::InitSolution() {
  for (size_t varIdx = 0; varIdx < modelVarUtil.varNum; varIdx++) {
    auto& localVar = localVarUtil.GetVar(varIdx);
    const auto& modelVar = modelVarUtil.GetVar(varIdx);
    if (modelVar.lowerBound > 0)
      localVar.nowValue = modelVar.lowerBound;
    else if (modelVar.upperBound < 0)
      localVar.nowValue = modelVar.upperBound;
    else
      localVar.nowValue = 0;
    assert(modelVar.InBound(localVar.nowValue));
  }
}

void LocalMIP::PrintResult() const {
  if (!isFoundFeasible)
    printf("c   LOCAL-MIP no feasible solution found.\n");
  else {
    printf("c   LOCAL-MIP Best objective: %lf\n", GetObjValue());
  }
}

// Compute conIdx's LHS from the current assignment. For real constraints (conIdx != 0) also insert
// into the unsat list if violated. Reused by InitState and by live constraint addition.
void LocalMIP::InitConstraint(size_t _conIdx) {
  auto& localCon = localConUtil.conSet[_conIdx];
  auto& modelCon = modelConUtil.conSet[_conIdx];
  localCon.LHS = 0;
  for (size_t termIdx = 0; termIdx < modelCon.termNum; ++termIdx)
    localCon.LHS += modelCon.coeffSet[termIdx] * localVarUtil.GetVar(modelCon.varIdxSet[termIdx]).nowValue;
  if (_conIdx != 0 && localCon.UNSAT()) localConUtil.insertUnsat(_conIdx);
}

void LocalMIP::InitState() {
  isKeepFeas = false;
  isFoundFeasible = false;
  bestOBJ = Infinity;

  for (size_t conIdx = 1; conIdx < modelConUtil.conNum; ++conIdx) InitConstraint(conIdx);
  auto& localObj = localConUtil.conSet[0];
  localObj.RHS = Infinity;
  InitConstraint(0);  // computes localObj.LHS (conIdx 0 => no unsat bookkeeping)
  isInitialized = true;
}

void LocalMIP::UpdateBestSolution() {
  for (auto& localVar : localVarUtil.varSet) localVar.bestValue = localVar.nowValue;
  auto& localObj = localConUtil.conSet[0];
  bestOBJ = localObj.LHS;
  localObj.RHS = bestOBJ - OptimalTol;
}

void LocalMIP::ApplyMove(size_t _varIdx, Value _delta) {
  auto& localVar = localVarUtil.GetVar(_varIdx);
  auto& modelVar = modelVarUtil.GetVar(_varIdx);
  localVar.nowValue += _delta;
  for (size_t termIdx = 0; termIdx < modelVar.termNum; ++termIdx) {
    size_t conIdx = modelVar.conIdxSet[termIdx];
    auto& localCon = localConUtil.conSet[conIdx];
    auto& modelCon = modelConUtil.conSet[conIdx];
    Value newLHS = 0;
    for (size_t termIdx = 0; termIdx < modelCon.termNum; ++termIdx)
      newLHS += modelCon.coeffSet[termIdx] * localVarUtil.GetVar(modelCon.varIdxSet[termIdx]).nowValue;
    if (conIdx == 0)
      localCon.LHS = newLHS;
    else {
      bool isPreSat = localCon.SAT();
      bool isNowSat = newLHS < localCon.RHS + FeasibilityTol;
      if (isPreSat && !isNowSat)
        localConUtil.insertUnsat(conIdx);
      else if (!isPreSat && isNowSat)
        localConUtil.removeUnsat(conIdx);
      localCon.LHS = newLHS;
    }
  }
  if (_delta > 0) {
    localVar.lastIncStep = curStep;
    localVar.allowDecStep = curStep + tabuBase + mt() % tabuVariation;
  } else {
    localVar.lastDecStep = curStep;
    localVar.allowIncStep = curStep + tabuBase + mt() % tabuVariation;
  }
}

Value LocalMIP::GetObjValue() const { return bestOBJ + modelVarUtil.objBias; }

LocalMIP::LocalMIP() { mt.seed(2832); }

// ---- Incremental API ----------------------------------------------------------------------------

void LocalMIP::addBinaryIdx(size_t _varIdx) {
  if (localVarUtil.binaryIdxPos[_varIdx] >= 0) return;  // already present
  localVarUtil.binaryIdxPos[_varIdx] = static_cast<int64_t>(localVarUtil.binaryIdx.size());
  localVarUtil.binaryIdx.push_back(_varIdx);
}

void LocalMIP::removeBinaryIdx(size_t _varIdx) {
  int64_t pos = localVarUtil.binaryIdxPos[_varIdx];
  if (pos < 0) return;  // not present
  auto& binaryIdx = localVarUtil.binaryIdx;
  size_t last = binaryIdx.size() - 1;
  if (static_cast<size_t>(pos) != last) {
    binaryIdx[pos] = binaryIdx[last];
    localVarUtil.binaryIdxPos[binaryIdx[pos]] = pos;
  }
  binaryIdx.pop_back();
  localVarUtil.binaryIdxPos[_varIdx] = -1;
}

void LocalMIP::setVarBounds(size_t _varIdx, Value _newLb, Value _newUb, bool _retractable) {
  auto& modelVar = modelVarUtil.GetVar(_varIdx);
  if (_retractable) boundSaveStack.push_back({_varIdx, modelVar.lowerBound, modelVar.upperBound, modelVar.type});
  modelVar.lowerBound = _newLb;
  modelVar.upperBound = _newUb;
  if (!isInitialized) return;  // SetVarType / InitSolution / InitState will handle the rest at run start

  VarType newType;
  if (fabs(_newLb - _newUb) < FeasibilityTol)
    newType = VarType::Fixed;
  else if (fabs(_newLb) < FeasibilityTol && fabs(_newUb - 1.0) < FeasibilityTol)
    newType = VarType::Binary;
  else
    newType = VarType::Integer;
  if (newType == VarType::Binary)
    addBinaryIdx(_varIdx);  // FlipMove asserts type==Binary for everything in binaryIdx
  else
    removeBinaryIdx(_varIdx);
  modelVar.type = newType;

  // Project the current value into the new range. ApplyMove recomputes the LHS of all incident
  // constraints (incl. the objective) and keeps the unsat list consistent.
  auto& localVar = localVarUtil.GetVar(_varIdx);
  Value target = localVar.nowValue;
  if (target < _newLb) target = _newLb;
  if (target > _newUb) target = _newUb;
  if (target != localVar.nowValue) ApplyMove(_varIdx, target - localVar.nowValue);
}

void LocalMIP::popVarBoundsTo(size_t _target) {
  while (boundSaveStack.size() > _target) {
    BoundSave s = boundSaveStack.back();
    boundSaveStack.pop_back();
    auto& modelVar = modelVarUtil.GetVar(s.varIdx);
    modelVar.lowerBound = s.lowerBound;
    modelVar.upperBound = s.upperBound;
    if (s.type == VarType::Binary)
      addBinaryIdx(s.varIdx);
    else
      removeBinaryIdx(s.varIdx);
    modelVar.type = s.type;
    // nowValue stays as-is: restored bounds are no tighter than the ones we set, and the value was
    // projected into the tighter range, so it remains within the restored range. No LHS recompute.
  }
}

void LocalMIP::addConstraintLive(size_t _conIdx) {
  // termNum and conNum are maintained on append (MakeCon/PushCoeffVarIdx); only the live search
  // state needs folding in.
  InitConstraint(_conIdx);
}

void LocalMIP::reinitObjective(size_t _objTermNum) {
  auto& v2o = modelVarUtil.varIdx2ObjIdx;
  std::fill(v2o.begin(), v2o.end(), static_cast<size_t>(-1));
  v2o.resize(modelVarUtil.varNum, static_cast<size_t>(-1));
  const auto& modelObj = modelConUtil.conSet[0];
  for (size_t idx = 0; idx < modelObj.termNum; ++idx) v2o[modelObj.varIdxSet[idx]] = idx;

  localVarUtil.lowerDeltaInLiftMove.resize(_objTermNum);
  localVarUtil.upperDeltaInLifiMove.resize(_objTermNum);

  // Constraints and the assignment are unchanged, so the unsat list stays valid; only the notion of
  // "best" is reset so the search re-evaluates the incumbent under the new objective.
  isKeepFeas = false;
  isFoundFeasible = false;
  bestOBJ = Infinity;
  auto& localObj = localConUtil.conSet[0];
  localObj.RHS = Infinity;
  localObj.weight = 1;
  InitConstraint(0);
}