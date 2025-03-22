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

class LocalCon
{
public:
  size_t weight;
  size_t posInUnsatConIdxs;
  Value RHS;
  Value LHS;

  LocalCon();
  ~LocalCon();
  bool SAT();
  bool UNSAT();
};

class LocalConUtil
{
public:
  std::vector<LocalCon> conSet;
  std::vector<size_t> unsatConIdxs;
  std::vector<size_t> tempUnsatConIdxs;
  std::vector<size_t> tempSatConIdxs;
  std::unordered_set<size_t> sampleSet;

  LocalConUtil();
  ~LocalConUtil();
  void Allocate(
      const size_t _conNum);
  LocalCon &GetCon(
      const size_t _idx);
  void insertUnsat(
      const size_t _conIdx);
  void RemoveUnsat(
      const size_t _conIdx);
};
