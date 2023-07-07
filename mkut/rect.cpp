#include <cstdio>
#include <cmath>
#include <iostream>
#include "../library/problem.h"
#include "../library/simulated_annealing.h"
using namespace std;

int main() {
    manarimo::problem_t prob;
    manarimo::load_problem(cin, prob);
    sa::simulated_annealing sa;
    while (!sa.end()) {
        double current_score = 100;
        double next_score = 100;
        if (sa.accept(current_score, next_score)) {
            // use
        } else {
            // not use
        }
    }
    
    return 0;
}