
#include "paras.h"
#include "../Solver.h"

int main(int argc, char **argv)
{

    INIT_ARGS
    Solver *solver = new Solver();
    solver->Run();
    delete solver;

    return 0;
}