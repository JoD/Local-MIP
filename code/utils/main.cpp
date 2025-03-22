
#include <ReaderMPS.h>
#include <LocalSearch/LocalMIP.h>

#include "paras.h"

int main(int argc, char **argv)
{

    INIT_ARGS

    Value optimalObj=NegativeInfinity;
    ReaderMPS *readerMPS;
    ModelConUtil *modelConUtil;
    ModelVarUtil *modelVarUtil;
    LocalMIP *localMIP;
    std::chrono::_V2::system_clock::time_point clkStart =
        std::chrono::high_resolution_clock::now();

    modelConUtil = new ModelConUtil();
    modelVarUtil = new ModelVarUtil();
    readerMPS = new ReaderMPS(modelConUtil, modelVarUtil);
    localMIP = new LocalMIP(modelConUtil, modelVarUtil, OPT(DEBUG), OPT(PrintSol), OPT(cutoff));

    readerMPS->Read(OPT(instance).c_str());
    [[maybe_unused]] int Result = localMIP->LocalSearch(optimalObj, clkStart); // unused?
    localMIP->PrintResult();

    delete modelConUtil;
    delete modelVarUtil;
    delete readerMPS;
    delete localMIP;

    return 0;
}