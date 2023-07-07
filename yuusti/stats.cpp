#include <cstdio>
#include <cmath>
#include <iostream>
#include <vector>
#include "../library/problem.h"
#include "../library/simulated_annealing.h"
#include "../library/geo.h"
#include "../library/scoring.h"

using namespace std;

#define X first
#define Y second

void stats(manarimo::problem_t& prob) {
    int taste = prob.attendees[0].tastes.size();
    vector<long long> mint(taste), maxt(taste), sumt(taste);
    for (int i = 0; i < taste; ++i) {
        auto& a = prob.attendees;
        mint[i] = maxt[i] = sumt[i] = a[0].tastes[i];
        for (int j = 1; j < a.size(); ++j) {
            maxt[i] = max(maxt[i], (long long) a[j].tastes[i]);
            mint[i] = min(mint[i], (long long) a[j].tastes[i]);
            sumt[i] += (long long) a[j].tastes[i];
        }
    }
    
    vector<int> cnt(taste);
    for (auto m: prob.musicians) {
        cnt[m]++;
    }

    for (int i = 0; i < taste; ++i) {
        printf("taste %d: %d musicians. max=%lld, min=%lld, sum=%lld\n",
        i, cnt[i], maxt[i], mint[i], sumt[i]);
    }
}

int main() {
    manarimo::problem_t prob;
    manarimo::load_problem(cin, prob);
    sa::simulated_annealing sa;

    stats(prob);

    return 0;
}
