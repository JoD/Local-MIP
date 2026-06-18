/*=====================================================================================

    Filename:     LocalVar.cpp

    Description:
        Version:  1.0

    Author:       Peng Lin, penglincs@outlook.com

    Organization: Shaowei Cai Group,
                  State Key Laboratory of Computer Science,
                  Institute of Software, Chinese Academy of Sciences,
                  Beijing, China

=====================================================================================*/

#include "LocalVar.h"

LocalVar& LocalVarUtil::GetVar(size_t _idx) {
  assert(_idx < varSet.size());
  return varSet[_idx];
}