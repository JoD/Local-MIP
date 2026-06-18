/*=====================================================================================

    Filename:     LocalCon.cpp

    Description:
        Version:  1.0

    Author:       Peng Lin, penglincs@outlook.com

    Organization: Shaowei Cai Group,
                  State Key Laboratory of Computer Science,
                  Institute of Software, Chinese Academy of Sciences,
                  Beijing, China

=====================================================================================*/

#include "LocalCon.h"

bool LocalCon::SAT() const { return LHS < RHS + FeasibilityTol; }

bool LocalCon::UNSAT() const { return LHS >= RHS + FeasibilityTol; }

LocalCon& LocalConUtil::getCon(size_t _idx) { return conSet[_idx]; }

void LocalConUtil::insertUnsat(size_t _conIdx) {
  conSet[_conIdx].posInUnsatConIdxs = unsatConIdxs.size();
  unsatConIdxs.push_back(_conIdx);
}

void LocalConUtil::removeUnsat(size_t _conIdx) {
  assert(unsatConIdxs.size() > 0);
  size_t pos = conSet[_conIdx].posInUnsatConIdxs;
  if (pos != unsatConIdxs.size() - 1) {
    unsatConIdxs[pos] = unsatConIdxs.back();
    conSet[unsatConIdxs[pos]].posInUnsatConIdxs = pos;
  }
  unsatConIdxs.pop_back();
}
