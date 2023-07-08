#include <problem.h>
#include <solution.h>
#include <geo.h>
#include <scoring.h>
#include <iostream>
#include <fstream>
#include <chrono>

using namespace std;

// c++ -std=c++20 -I../../library test.cpp -O3
int main() {
    manarimo::problem_t prob;
    manarimo::load_problem("../../problems/90.json", prob);

    manarimo::solution_t sol;
    manarimo::load_solution("../../solutions/kawatea_block_pillar/90.json", sol);
    const auto& placements = sol.as_p();

    cout << manarimo::validate(prob, placements) << endl;
    auto start = chrono::system_clock::now();
    const auto actual = manarimo::score(prob, placements);
    auto end = chrono::system_clock::now();
    auto dur = end - start;
    auto msec = chrono::duration_cast<std::chrono::milliseconds>(dur).count();
    cout << actual << endl;
    const long long expected = 59243905;
    cout << "expected: " << expected << endl;
    cout << "diff: " << expected - actual << endl;
    cout << "elapsed: " << msec << " msec" << endl;
    return 0;
}