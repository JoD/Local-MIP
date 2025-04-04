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

int LocalMIP::LocalSearch(
    Value _optimalObj,
    std::chrono::_V2::system_clock::time_point _clkStart)
{
  Allocate();
  InitSolution();
  InitState();
  auto &localObj = localConUtil.conSet[0];
  curStep = 0;
  while (true)
  {
    if (DEBUG)
      printf("\nc UNSAT Size: %-10ld; ", localConUtil.unsatConIdxs.size());
    if (localConUtil.unsatConIdxs.empty())
    {
      if (!isFoundFeasible || localObj.LHS < localObj.RHS)
      {
        UpdateBestSolution();
        LogObj(_clkStart);
        isFoundFeasible = true;
      }
      bool res = LiftMoveWithoutBreak();
      if (GetObjValue() <= _optimalObj)
        return 1;
      ++curStep;
      if (Timeout(_clkStart))
        break;
      if (res)
        continue;
    }
    if (Timeout(_clkStart))
      break;
    if (!UnsatTightMove())
    {
      if (mt() % 10000 > smoothProbability)
        UpdateWeight();
      else
        SmoothWeight();
      RandomTightMove();
    }
    ++curStep;
  }
  return 0;
}

bool LocalMIP::Timeout(
    std::chrono::_V2::system_clock::time_point &_clkStart)
{
  auto clk_now = std::chrono::high_resolution_clock::now();
  auto solve_time =
      std::chrono::duration_cast<std::chrono::seconds>(clk_now - _clkStart).count();
  if (solve_time >= CUTOFF)
    return true;
  return false;
}

void LocalMIP::LogObj(
    std::chrono::_V2::system_clock::time_point &_clkStart)
{
  auto clk = TimeNow();
  printf(
      "n %-20f %lf\n",
      (GetObjValue()),
      ElapsedTime(clk, _clkStart));
}

void LocalMIP::InitSolution()
{
  for (size_t varIdx = 0; varIdx < modelVarUtil->varNum; varIdx++)
  {
    auto &localVar = localVarUtil.GetVar(varIdx);
    const auto &modelVar = modelVarUtil->GetVar(varIdx);
    if (modelVar.lowerBound > 0)
      localVar.nowValue = modelVar.lowerBound;
    else if (modelVar.upperBound < 0)
      localVar.nowValue = modelVar.upperBound;
    else
      localVar.nowValue = 0;
    assert(modelVar.InBound(localVar.nowValue));
  }
}

void LocalMIP::PrintResult()
{
  if (!isFoundFeasible)
    printf("o no feasible solution found.\n");
  else if (VerifySolution())
  {
    printf("o Best objective: %lf\n", GetObjValue());
    // printf("B 1 %lf\n", GetObjValue());
    if (PRINTSOL)
      PrintSol();
  }
  else
    std::cout << "solution verify failed." << std::endl;
}

void LocalMIP::InitState()
{
  for (size_t conIdx = 1; conIdx < modelConUtil->conNum; ++conIdx)
  {
    auto &localCon = localConUtil.conSet[conIdx];
    auto &modelCon = modelConUtil->conSet[conIdx];
    localCon.LHS = 0;
    for (size_t termIdx = 0; termIdx < modelCon.termNum; ++termIdx)
      localCon.LHS +=
          modelCon.coeffSet[termIdx] *
          localVarUtil.GetVar(modelCon.varIdxSet[termIdx]).nowValue;
    if (localCon.UNSAT())
      localConUtil.insertUnsat(conIdx);
  }
  auto &localObj = localConUtil.conSet[0];
  auto &modelObj = modelConUtil->conSet[0];
  localObj.RHS = Infinity;
  localObj.LHS = 0;
  for (size_t termIdx = 0; termIdx < modelObj.termNum; ++termIdx)
    localObj.LHS +=
        modelObj.coeffSet[termIdx] *
        localVarUtil.GetVar(modelObj.varIdxSet[termIdx]).nowValue;
}

void LocalMIP::UpdateBestSolution()
{
  lastImproveStep = curStep;
  for (auto &localVar : localVarUtil.varSet)
    localVar.bestValue = localVar.nowValue;
  auto &localObj = localConUtil.conSet[0];
  // auto &modelObj = modelConUtil->conSet[0]; // unused?
  bestOBJ = localObj.LHS;
  localObj.RHS = bestOBJ - OptimalTol;
}

void LocalMIP::ApplyMove(
    size_t _varIdx,
    Value _delta)
{
  auto &localVar = localVarUtil.GetVar(_varIdx);
  auto &modelVar = modelVarUtil->GetVar(_varIdx);
  localVar.nowValue += _delta;
  if (DEBUG)
    printf("varType: %d; varIdx: %-10ld; delta: %-10lf; ",
           static_cast<int>(modelVar.type), _varIdx, _delta);
  for (size_t termIdx = 0; termIdx < modelVar.termNum; ++termIdx)
  {
    size_t conIdx = modelVar.conIdxSet[termIdx];
    // size_t posInCon = modelVar.posInCon[termIdx]; // unused?
    auto &localCon = localConUtil.conSet[conIdx];
    auto &modelCon = modelConUtil->conSet[conIdx];
    Value newLHS = 0;
    for (size_t termIdx = 0; termIdx < modelCon.termNum; ++termIdx)
      newLHS +=
          modelCon.coeffSet[termIdx] *
          localVarUtil.GetVar(modelCon.varIdxSet[termIdx]).nowValue;
    if (conIdx == 0)
      localCon.LHS = newLHS;
    else
    {
      bool isPreSat = localCon.SAT();
      bool isNowSat = newLHS < localCon.RHS + FeasibilityTol;
      if (isPreSat && !isNowSat)
        localConUtil.insertUnsat(conIdx);
      else if (!isPreSat && isNowSat)
        localConUtil.RemoveUnsat(conIdx);
      localCon.LHS = newLHS;
    }
  }
  if (_delta > 0)
  {
    localVar.lastIncStep = curStep;
    localVar.allowDecStep =
        curStep + tabuBase + mt() % tabuVariation;
  }
  else
  {
    localVar.lastDecStep = curStep;
    localVar.allowIncStep =
        curStep + tabuBase + mt() % tabuVariation;
  }
}

void LocalMIP::Restart()
{
  lastImproveStep = curStep;
  ++restartTimes;
  for (auto unsatIdx : localConUtil.unsatConIdxs)
    localConUtil.RemoveUnsat(unsatIdx);
  for (size_t varIdx = 0; varIdx < modelVarUtil->varNum; varIdx++)
  {
    auto &localVar = localVarUtil.GetVar(varIdx);
    auto &modelVar = modelVarUtil->GetVar(varIdx);
    if (modelVar.type == VarType::Binary)
      localVar.nowValue = mt() % 2;
    else if (modelVar.type == VarType::Integer &&
             modelVar.lowerBound > -1e15 &&
             modelVar.upperBound < 1e15)
    {
      long long lowerBound = (long long)modelVar.lowerBound;
      long long upperBound = (long long)modelVar.upperBound;
      localVar.nowValue = modelVar.lowerBound + (mt() % (upperBound + 1 - lowerBound));
      // printf("c %lf; %lf; %lld; %lf; %lld\n",
      //        localVar.nowValue, modelVar.lowerBound, lowerBound, modelVar.upperBound, upperBound);
    }
    else
    {
      if (modelVar.lowerBound > 0)
        localVar.nowValue = modelVar.lowerBound;
      else if (modelVar.upperBound < 0)
        localVar.nowValue = modelVar.upperBound;
      else
        localVar.nowValue = 0;
    }
    assert(modelVar.InBound(localVar.nowValue));
    if (isFoundFeasible && mt() % 100 > 50)
      localVar.nowValue = localVar.bestValue;
    localVar.lastDecStep = curStep;
    localVar.allowIncStep = 0;
    localVar.lastIncStep = curStep;
    localVar.allowDecStep = 0;
  }
  for (size_t conIdx = 1; conIdx < modelConUtil->conNum; ++conIdx)
  {
    auto &localCon = localConUtil.conSet[conIdx];
    auto &modelCon = modelConUtil->conSet[conIdx];
    localCon.LHS = 0;
    for (size_t termIdx = 0; termIdx < modelCon.termNum; ++termIdx)
      localCon.LHS +=
          modelCon.coeffSet[termIdx] *
          localVarUtil.GetVar(modelCon.varIdxSet[termIdx]).nowValue;
    if (localCon.UNSAT())
      localConUtil.insertUnsat(conIdx);
    localCon.weight = 1;
  }
  auto &localObj = localConUtil.conSet[0];
  auto &modelObj = modelConUtil->conSet[0];
  localObj.LHS = 0;
  localObj.weight = 1;
  for (size_t termIdx = 0; termIdx < modelObj.termNum; ++termIdx)
    localObj.LHS +=
        modelObj.coeffSet[termIdx] *
        localVarUtil.GetVar(modelObj.varIdxSet[termIdx]).nowValue;
}

bool LocalMIP::VerifySolution()
{
  for (size_t var_idx = 0; var_idx < modelVarUtil->varNum; var_idx++)
  {
    auto &var = localVarUtil.GetVar(var_idx);
    auto &modelVar = modelVarUtil->GetVar(var_idx);
    if (!modelVar.InBound(var.bestValue))
      return false;
  }

  for (size_t conIdx = 1; conIdx < modelConUtil->conNum; ++conIdx)
  {
    // auto &con = localConUtil.conSet[conIdx]; // unused?
    auto &modelCon = modelConUtil->conSet[conIdx];
    Value lhs = 0;
    for (size_t termIdx = 0; termIdx < modelCon.termNum; ++termIdx)
      lhs +=
          modelCon.coeffSet[termIdx] *
          localVarUtil.GetVar(modelCon.varIdxSet[termIdx]).bestValue;
    if (lhs > modelCon.RHS + FeasibilityTol)
    {
      printf("c lhs: %lf; rhs: %lf\n", lhs, modelCon.RHS);
      return false;
    }
  }
  // Obj
  // auto &localObj = localConUtil.conSet[0]; // unused?
  auto &modelObj = modelConUtil->conSet[0];
  Value objValue = 0;
  for (size_t termIdx = 0; termIdx < modelObj.termNum; ++termIdx)
    objValue +=
        modelObj.coeffSet[termIdx] *
        localVarUtil.GetVar(modelObj.varIdxSet[termIdx]).bestValue;
  // printf("c %lf; %lf\n", objValue, bestOBJ);
  return fabs(objValue - bestOBJ) < 1e-3;
}

void LocalMIP::PrintSol()
{
  printf("c best-found solution:\n");
  printf("%-50s        %s\n", "Variable name", "Variable value");
  for (size_t varIdx = 0; varIdx < modelVarUtil->varNum; varIdx++)
  {
    const auto &var = localVarUtil.GetVar(varIdx);
    const auto &modelVar = modelVarUtil->GetVar(varIdx);
    if (var.bestValue)
      printf("%-50s        %lf\n", modelVar.name.c_str(), var.bestValue);
  }
}

void LocalMIP::Allocate()
{
  liftStep = 0;
  breakStep = 0;
  tightStepUnsat = 0;
  tightStepSat = 0;
  flipStep = 0;
  randomStep = 0;
  restartTimes = 0;
  smoothProbability = 3;
  tabuBase = 3;
  tabuVariation = 10;
  isBin = modelVarUtil->isBin;
  isKeepFeas = false;
  isFoundFeasible = false;
  weightUpperBound = 10000000;
  objWeightUpperBound = 10000000;
  lastImproveStep = 0;
  sampleUnsat = 12;
  bmsUnsatInfeas = 2000;
  bmsUnsatFeas = 3000;
  sampleSat = 20;
  bmsSat = 190;
  bmsFlip = 20;
  bmsRandom = 150;
  bestOBJ = Infinity;
  localVarUtil.Allocate(
      modelVarUtil->varNum,
      modelConUtil->conSet[0].varIdxSet.size());
  localConUtil.Allocate(modelConUtil->conNum);
  for (size_t conIdx = 1; conIdx < modelConUtil->conNum; conIdx++)
    localConUtil.conSet[conIdx].RHS = modelConUtil->conSet[conIdx].RHS;
  for (size_t varIdx = 0; varIdx < modelVarUtil->varNum; varIdx++)
  {
    auto &modelVar = modelVarUtil->GetVar(varIdx);
    if (modelVar.type == VarType::Binary)
      localVarUtil.binaryIdx.push_back(varIdx);
  }
  mt.seed(2832);
}

Value LocalMIP::GetObjValue()
{
  return modelConUtil->MIN * (bestOBJ + modelVarUtil->objBias);
}

LocalMIP::LocalMIP(
    const ModelConUtil *_modelConUtil,
    const ModelVarUtil *_modelVarUtil,
    bool debug, bool printsol, double cutoff)
    : modelConUtil(_modelConUtil),
      modelVarUtil(_modelVarUtil),
      DEBUG(debug),
      PRINTSOL(printsol),
      CUTOFF(cutoff)
{
}

LocalMIP::~LocalMIP()
{
}