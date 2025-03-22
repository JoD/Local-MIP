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
#include "utils/paras.h"

class ModelCon
{
public:
  std::string name;
  size_t idx;
  bool isEqual;
  bool isLarge;
  std::vector<Value> coeffSet;
  std::vector<size_t> varIdxSet;
  std::vector<size_t> posInVar;
  Value RHS;
  bool inferSAT;
  size_t termNum;

  ModelCon(
      const std::string &_name,
      const size_t _idx);
  ~ModelCon();
};

class ModelConUtil
{
public:
  std::unordered_map<std::string, size_t> name2idx;
  std::vector<ModelCon> conSet;
  std::string objName;
  size_t conNum;
  int MIN = 1;

  ModelConUtil();
  ~ModelConUtil();
  size_t MakeCon(
      const std::string &_name);
  size_t GetConIdx(
      const std::string &_name);
  const ModelCon &GetCon(
      const size_t _idx) const;
  ModelCon &GetCon(
      const size_t _idx);
  ModelCon &GetCon(
      const std::string &_name);
};