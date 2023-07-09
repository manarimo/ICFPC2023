#include <problem.h>
#include <solution.h>
#include <geo.h>
#include <scoring.h>
#include <iostream>
#include <fstream>

using namespace std;

// c++ -std=c++20 -O3 -I../../library main.cpp
int main(int argc, char *argv[]) {
    if (argc < 3) {
        cout << argv[0] << " problem solution [--skip-validate]" << endl;
        return 0; 
    }
    manarimo::problem_t prob;
    manarimo::load_problem(argv[1], prob);
    manarimo::solution_t sol;
    manarimo::load_solution(argv[2], sol);

    if (argc < 4 || string(argv[3]) != "--skip-validate") {
        if (!manarimo::validate(prob, sol.as_p())) {
            cerr << "invalid solution" << endl;
            cout << 0 << endl;
            return 0;
        }
    }
    cout << manarimo::score(prob, sol) << endl;
    return 0;
}