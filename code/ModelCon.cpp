/*=====================================================================================

    Filename:     ModelCon.cpp

    Description:
        Version:  1.0

    Author:       Peng Lin, penglincs@outlook.com

    Organization: Shaowei Cai Group,
                  State Key Laboratory of Computer Science,
                  Institute of Software, Chinese Academy of Sciences,
                  Beijing, China

=====================================================================================*/

#include "ModelCon.h"

ModelCon::ModelCon(size_t _idx) : idx(_idx), RHS(0), inferSAT(false), termNum(0) {}

size_t ModelConUtil::MakeCon() {
  size_t conIdx = conSet.size();
  conSet.emplace_back(conIdx);
  conNum = conSet.size();  // maintain on append for incremental use
  return conIdx;
}

const ModelCon& ModelConUtil::getCon(size_t _idx) const { return conSet[_idx]; }

ModelCon& ModelConUtil::getCon(size_t _idx) { return conSet[_idx]; }
