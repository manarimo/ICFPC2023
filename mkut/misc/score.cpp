#include <iostream>
#include <fstream>
#include "../../library/problem.h"
#include "../../library/solution.h"
#include "../../library/scoring.h"
using namespace std;
using namespace manarimo;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        return 0;
    }
    string ifname = "../../problems/";
    ifname += argv[1];
    ifname += ".json";
    ifstream ifs(ifname);
    problem_t problem;
    load_problem(ifs, problem);
    solution_t solution;
    load_solution(cin, solution);
    long long sc = score(problem, solution.as_p());
    cout << "Score: " << sc << endl;
}