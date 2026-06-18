/*=====================================================================================

    Filename:     ModelCon.h

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

struct ModelCon {
  size_t idx;
  std::vector<Value> coeffSet;
  std::vector<size_t> varIdxSet;
  std::vector<size_t> posInVar;
  Value RHS;
  bool inferSAT;
  size_t termNum;

  ModelCon(size_t _idx);
};

struct ModelConUtil {
  std::vector<ModelCon> conSet;
  std::string objName;
  size_t conNum = -1;

  size_t MakeCon();
  const ModelCon& getCon(size_t _idx) const;
  ModelCon& getCon(size_t _idx);
};