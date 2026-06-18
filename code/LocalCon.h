/*=====================================================================================

    Filename:     LocalCon.h

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

struct LocalCon {
  size_t weight = 1;
  size_t posInUnsatConIdxs = 0;
  Value RHS = 0;
  Value LHS = 0;

  bool SAT() const;
  bool UNSAT() const;
};

struct LocalConUtil {
  std::vector<LocalCon> conSet;
  std::vector<size_t> unsatConIdxs;
  std::vector<size_t> tempUnsatConIdxs;
  std::vector<size_t> tempSatConIdxs;

  LocalCon& getCon(size_t _idx);
  void insertUnsat(size_t _conIdx);
  void removeUnsat(size_t _conIdx);
};
