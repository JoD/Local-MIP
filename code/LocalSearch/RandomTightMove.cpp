/*=====================================================================================

    Filename:     RandomTightMove.cpp

    Description:
        Version:  1.0

    Author:       Peng Lin, penglincs@outlook.com

    Organization: Shaowei Cai Group,
                  State Key Laboratory of Computer Science,
                  Institute of Software, Chinese Academy of Sciences,
                  Beijing, China

=====================================================================================*/

#include "LocalMIP.h"

void LocalMIP::RandomTightMove()
{
  long bestScore = -100000000000;
  long bestSubscore = -std::numeric_limits<long>::max();
  size_t bestVarIdx = -1;
  Value bestDelta = 0;
  std::vector<size_t> &neighborVarIdxs = localVarUtil.tempVarIdxs;
  std::vector<Value> &neighborDeltas = localVarUtil.tempDeltas;
  neighborVarIdxs.clear();
  neighborDeltas.clear();
  if (localConUtil.unsatConIdxs.size() > 0)
  {
    size_t conIdx = localConUtil.unsatConIdxs[mt() % localConUtil.unsatConIdxs.size()];
    auto &localCon = localConUtil.conSet[conIdx];
    auto &modelCon = modelConUtil->conSet[conIdx];
    for (size_t termIdx = 0; termIdx < modelCon.termNum; ++termIdx)
    {
      size_t varIdx = modelCon.varIdxSet[termIdx];
      auto &localVar = localVarUtil.GetVar(varIdx);
      auto &modelVar = modelVarUtil->GetVar(varIdx);
      Value delta;
      if (!TightDelta(localCon, modelCon, termIdx, delta))
      {
        if (modelCon.coeffSet[termIdx] > 0)
          delta = modelVar.lowerBound - localVar.nowValue;
        else
          delta = modelVar.upperBound - localVar.nowValue;
      }
      if ((delta < 0 && curStep == localVar.lastIncStep + 1) ||
           (delta > 0 && curStep == localVar.lastDecStep + 1))
        continue;
      if (fabs(delta) < FeasibilityTol)
        continue;
      neighborVarIdxs.push_back(varIdx);
      neighborDeltas.push_back(delta);
    }
  }
  auto &localObj = localConUtil.conSet[0];
  auto &modelObj = modelConUtil->conSet[0];
  if (isFoundFeasible && localObj.UNSAT()){
    for (size_t termIdx = 0; termIdx < modelObj.termNum; ++termIdx)
    {
      size_t varIdx = modelObj.varIdxSet[termIdx];
      auto &localVar = localVarUtil.GetVar(varIdx);
      auto &modelVar = modelVarUtil->GetVar(varIdx);
      Value delta;
      if (!TightDelta(localObj, modelObj, termIdx, delta)){
        if (modelObj.coeffSet[termIdx] > 0){
          delta = modelVar.lowerBound - localVar.nowValue;
        }else{
          delta = modelVar.upperBound - localVar.nowValue;
        }
      }
      if ((delta < 0 && curStep < localVar.allowDecStep) ||
          (delta > 0 && curStep < localVar.allowIncStep))
        continue;
      if (fabs(delta) < FeasibilityTol)
        continue;
      neighborVarIdxs.push_back(varIdx);
      neighborDeltas.push_back(delta);
    }
  }
  size_t scoreSize = neighborVarIdxs.size();
  if (neighborVarIdxs.size() > bmsRandom)
  {
    scoreSize = bmsRandom;
    for (size_t bmsIdx = 0; bmsIdx < bmsRandom; ++bmsIdx)
    {
      size_t randomIdx = (mt() % (neighborVarIdxs.size() - bmsIdx)) + bmsIdx;
      size_t varIdx = neighborVarIdxs[randomIdx];
      Value delta = neighborDeltas[randomIdx];
      neighborVarIdxs[randomIdx] = neighborVarIdxs[bmsIdx];
      neighborDeltas[randomIdx] = neighborDeltas[bmsIdx];
      neighborVarIdxs[bmsIdx] = varIdx;
      neighborDeltas[bmsIdx] = delta;
    }
  }
  for (size_t idx = 0; idx < scoreSize; ++idx)
  {
    size_t varIdx = neighborVarIdxs[idx];
    Value delta = neighborDeltas[idx];
    // auto &localVar = localVarUtil.GetVar(varIdx); // unused?
    auto &modelVar = modelVarUtil->GetVar(varIdx);
    long score = TightScore(modelVar, delta);
    if (bestScore < score ||
        (bestScore == score && bestSubscore < subscore))
    {
      bestScore = score;
      bestVarIdx = varIdx;
      bestDelta = delta;
      bestSubscore = subscore;
    }
  }
  if (bestVarIdx != static_cast<size_t>(-1) && bestDelta != 0)
  {
    if (DEBUG)
      printf("Radom: %-10ld; ", bestScore);
    ++randomStep;
    ApplyMove(bestVarIdx, bestDelta);
    return;
  }
}