/*=====================================================================================

    Filename:     LocalMIP.h

    Description:
        Version:  1.0

    Author:       Peng Lin, penglincs@outlook.com

    Organization: Shaowei Cai Group,
                  State Key Laboratory of Computer Science,
                  Institute of Software, Chinese Academy of Sciences,
                  Beijing, China

=====================================================================================*/

#pragma once
#include <functional>
#include "LocalCon.h"
#include "LocalVar.h"
#include "ModelCon.h"
#include "ModelVar.h"

class LocalMIP {
 public:
  ModelConUtil modelConUtil;
  ModelVarUtil modelVarUtil;
  LocalVarUtil localVarUtil;
  LocalConUtil localConUtil;

 private:
  size_t curStep;
  std::mt19937 mt;
  const size_t smoothProbability = 12;
  const size_t tabuBase = 3;
  const size_t tabuVariation = 10;
  bool isFoundFeasible;
  bool isKeepFeas;
  const size_t sampleUnsat = 12;
  const size_t bmsUnsatInfeas = 2000;
  const size_t bmsUnsatFeas = 3000;
  const size_t sampleSat = 20;
  const size_t bmsSat = 190;
  const size_t bmsFlip = 20;
  const size_t bmsRandom = 150;
  Value bestOBJ;
  int64_t subscore;
  bool isInitialized = false;

  // Saved variable state for retractable (assumption) bound fixings; restored LIFO.
  struct BoundSave {
    size_t varIdx;
    Value lowerBound;
    Value upperBound;
    VarType type;
  };
  std::vector<BoundSave> boundSaveStack;

  void InitState();
  void InitConstraint(size_t _conIdx);  // compute one constraint's LHS (+unsat bookkeeping for conIdx!=0)
  void addBinaryIdx(size_t _varIdx);
  void removeBinaryIdx(size_t _varIdx);
  void UpdateBestSolution();
  bool UnsatTightMove();
  bool FlipMove(std::vector<uint8_t>& _scoreTable, std::vector<size_t>& _scoreIdx);
  void RandomTightMove();
  bool LiftMoveWithoutBreak();
  bool SatTightMove(std::vector<uint8_t>& _scoreTable, std::vector<size_t>& _scoreIdx);
  void UpdateWeight();
  void SmoothWeight();
  void ApplyMove(size_t _varIdx, Value _delta);
  int64_t TightScore(const ModelVar& _var, Value _delta);
  bool TightDelta(LocalCon& _con, const ModelCon& _modelCon, size_t _i, Value& _res);
  void InitSolution();

 public:
  LocalMIP();

  int32_t LocalSearch(
      const std::function<bool()>& stop_condition,
      const std::function<void(double, const std::vector<LocalVar>&)>& update_sol,
      const std::function<void()>& apply_changes = [] {});
  void PrintResult() const;
  Value GetObjValue() const;

  // ---- Incremental API (see LocalSolver) ----
  bool initialized() const { return isInitialized; }

  // Set new [lb,ub] for a variable. Recomputes its type (Fixed/Binary/Integer), patches binaryIdx,
  // and projects its current value into the new range via ApplyMove (recomputing affected LHS).
  // retractable=true pushes the previous bounds/type onto the save-stack for later popVarBoundsTo().
  void setVarBounds(size_t _varIdx, Value _newLb, Value _newUb, bool _retractable);
  size_t boundSaveSize() const { return boundSaveStack.size(); }
  void popVarBoundsTo(size_t _target);  // undo retractable fixings down to the given stack size

  // Bring a freshly appended constraint into the live search state (compute LHS, mark unsat if so).
  void addConstraintLive(size_t _conIdx);

  // After the objective (conIdx 0) terms have been rewritten, reset the optimization state so the
  // search re-evaluates the incumbent under the new objective. _objTermNum is its new term count.
  void reinitObjective(size_t _objTermNum);
};