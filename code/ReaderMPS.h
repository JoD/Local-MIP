/*=====================================================================================

    Filename:     ReaderMPS.h

    Description:
        Version:  1.0

    Author:       Peng Lin, penglincs@outlook.com

    Organization: Shaowei Cai Group,
                  State Key Laboratory of Computer Science,
                  Institute of Software, Chinese Academy of Sciences,
                  Beijing, China

=====================================================================================*/

#pragma once
#include "ModelCon.h"
#include "ModelVar.h"

struct LocalConUtil;
struct LocalVarUtil;
class LocalMIP;

struct ReaderMPS {
  ModelConUtil& modelConUtil;
  ModelVarUtil& modelVarUtil;
  LocalConUtil& localConUtil;
  LocalVarUtil& localVarUtil;
  std::istringstream iss;
  std::string readLine;
  bool TightenBound();
  void TightenBoundVar(ModelCon& _modelCon);
  bool TightBoundGlobally();
  bool SetVarType();
  void SetVarIdx2ObjIdx();
  void ClearConstraintTerms(size_t _conIdx);  // remove all terms of a constraint, patching var adjacency
  std::vector<size_t> fixedIdxs;
  size_t deleteConNum;
  size_t deleteVarNum;
  size_t inferVarNum;
  void PushCoeffVarIdx(size_t _conIdx, Value _coeff, const xct::IntVar* _iv);

  ReaderMPS(LocalMIP& localmip);
};