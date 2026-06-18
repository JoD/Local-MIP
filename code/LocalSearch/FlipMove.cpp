/*=====================================================================================

    Filename:     FlipMove.cpp

    Description:
        Version:  1.0

    Author:       Peng Lin, penglincs@outlook.com

    Organization: Shaowei Cai Group,
                  State Key Laboratory of Computer Science,
                  Institute of Software, Chinese Academy of Sciences,
                  Beijing, China

=====================================================================================*/

#include "LocalMIP.h"

bool LocalMIP::FlipMove(std::vector<uint8_t>& _scoreTable, std::vector<size_t>& _scoreIdx) {
  if (localVarUtil.binaryIdx.size() == 0) return false;
  int64_t bestScore = 0;
  int64_t bestSubscore = -std::numeric_limits<int64_t>::max();
  size_t bestVarIdx = -1;
  Value bestDelta = 0;
  for (size_t idx = 0; idx < bmsFlip; ++idx) {
    size_t varIdx = localVarUtil.binaryIdx[mt() % localVarUtil.binaryIdx.size()];
    if (_scoreTable[varIdx])
      continue;
    else {
      _scoreTable[varIdx] = true;
      _scoreIdx.push_back(varIdx);
    }
    auto& localVar = localVarUtil.GetVar(varIdx);
    auto& modelVar = modelVarUtil.GetVar(varIdx);
    assert(modelVar.type == VarType::Binary);
    Value delta = 0;
    if (localVar.nowValue > 0.5)
      delta = -1;
    else
      delta = 1;
    if ((delta < 0 && curStep < localVar.allowDecStep) || (delta > 0 && curStep < localVar.allowIncStep)) continue;
    int64_t score = TightScore(modelVar, delta);
    if (bestScore < score || (bestScore == score && bestSubscore < subscore)) {
      bestScore = score;
      bestVarIdx = varIdx;
      bestDelta = delta;
      bestSubscore = subscore;
    }
  }
  if (bestScore > 0) {
    ApplyMove(bestVarIdx, bestDelta);
    return true;
  }
  return false;
}