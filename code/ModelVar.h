/*=====================================================================================

    Filename:     ModelVar.h

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

class ModelVar
{
public:
	std::string name;
	size_t idx;
	Value upperBound;
	Value lowerBound;
	std::vector<size_t> conIdxSet;
	std::vector<size_t> posInCon;
	size_t termNum;
	VarType type;

	ModelVar(
			const std::string &_name,
			size_t _idx,
			bool _integrality);
	~ModelVar();
	bool InBound(
			Value _value) const;
	void SetType(
			VarType _varType);
	void SetUpperBound(
			Value _upperBound);
	void SetLowerBound(
			Value _lowerBound);
	bool IsFixed();
	bool IsBinary();
};

class ModelVarUtil
{
public:
	std::unordered_map<std::string, size_t> name2idx;
	std::vector<ModelVar> varSet;
	std::vector<size_t> varIdx2ObjIdx;
	bool isBin;
	size_t varNum;
	size_t integerNum;
	size_t binaryNum;
	size_t fixedNum;
	size_t realNum;
	Value objBias;

	ModelVarUtil();
	~ModelVarUtil();
	size_t MakeVar(
			const std::string &_name,
			const bool _integrality);
	const ModelVar &GetVar(
			const size_t _idx) const;
	ModelVar &GetVar(
			const size_t _idx);
	ModelVar &GetVar(
			const std::string &_name);
};