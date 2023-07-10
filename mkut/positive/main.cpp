#include <iostream>
#include <set>
#include "../../library/geo.h"
#include "../../library/problem.h"
#include "../../library/solution.h"
#include "../../library/simulated_annealing.h"
#include "../../library/scoring.h"
using namespace std;
using namespace manarimo;

class random {
    public:
    // [0, x)
    inline static unsigned get(unsigned x) {
        return ((unsigned long long)xorshift() * x) >> 32;
    }
    
    // [x, y]
    inline static unsigned get(unsigned x, unsigned y) {
        return get(y - x + 1) + x;
    }
    
    // [0, x] (x = 2^c - 1)
    inline static unsigned get_fast(unsigned x) {
        return xorshift() & x;
    }
    
    // [0.0, 1.0]
    inline static double probability() {
        return xorshift() * INV_MAX;
    }
    
    inline static double get_double(double x, double y) {
        return probability() * (y - x) + x;
    }
    
    inline static bool toss() {
        return xorshift() & 1;
    }
    
    private:
    constexpr static double INV_MAX = 1.0 / 0xFFFFFFFF;
    
    inline static unsigned xorshift() {
        static unsigned x = 123456789, y = 362436039, z = 521288629, w = 88675123;
        unsigned t = x ^ (x << 11);
        x = y, y = z, z = w;
        return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
    }
};

class solver_t {
    problem_t prob;

    public:
    vector<P> positions;
    int N; // number of musicians
    number Left;
    number Right;
    number Bottom;
    number Top;
    vector<vector<double>> score_multi; // attendee -> musician_pos
    vector<double> score_sensitivity;
    vector<double> acc_score_sensitivity;

    vector<int> placements;
    vector<double> max_tastes;
    double current_score;

    solver_t(const problem_t& prob) : prob(prob) {
        init(prob);
    }

    void init(const problem_t& prob) {
        N = prob.musicians.size();
        Left = prob.stage_bottom_left.first + 10;
        Right = prob.stage_bottom_left.first + prob.stage_width - 10;
        Bottom = prob.stage_bottom_left.second + 10;
        Top = prob.stage_bottom_left.second + prob.stage_height - 10;

        for (auto attendee : prob.attendees) {
            double mx = 0;
            for (double taste : attendee.tastes) {
                mx = max(mx, taste);
            }
            max_tastes.push_back(mx);
        }

        vector<pair<double,P>> scored_position;
        for (number x = Left; x <= Right; x++) {
            P p1(x, Bottom);
            scored_position.push_back(make_pair(calc_sensitivity(p1), p1));
            P p2(x, Top);
            scored_position.push_back(make_pair(calc_sensitivity(p2), p2));
        }
        for (number y = Bottom + 1; y <= Top - 1; y++) {
            P p1(Left, y);
            scored_position.push_back(make_pair(calc_sensitivity(p1), p1));
            P p2(Right, y);
            scored_position.push_back(make_pair(calc_sensitivity(p2), p2));
        }
        sort(scored_position.begin(), scored_position.end(), [](auto const& lhs, auto const& rhs) {
            return lhs.first > rhs.first;
        });

        for (int i = 0; i < scored_position.size() && positions.size() < N; i++) {
            bool ng = false;
            for (auto position : positions) {
                if (geo::d(scored_position[i].second, position) < 100) {
                    ng = true;
                }
            }
            if (!ng) {
                positions.push_back(scored_position[i].second);
            }
        }

        for (int i = positions.size(); i < prob.musicians.size(); i++) {
            while (true) {
                geo::P p(random::get(Left, Right), random::get(Bottom, Top));
                bool ng = false;
                for (int j = 0; j < i; j++) {
                    if (geo::d(positions[j], p) < 100) {
                        ng = true;
                        break;
                    }
                }
                if (!ng) {
                    positions.push_back(p);
                    break;
                }
            }
        }
        
        score_multi = vector<vector<double>>(prob.attendees.size(), vector<double>(N, 0));
        for (pair<int,int> unblocked : get_unblocked_pairs(prob, positions)) {
            int musician_id = unblocked.first;
            int attendee_id = unblocked.second;
            auto attendee = prob.attendees[attendee_id];
            score_multi[attendee_id][musician_id] = 1000000 / d(attendee.get_pos(), positions[musician_id]);
        }
        double acc_sensitivity = 0;
        for (int i = 0; i < N; i++) {
            double sensitivity = 0;
            for (int j = 0; j < prob.attendees.size(); j++) {
                sensitivity += score_multi[j][i];
                acc_sensitivity += score_multi[j][i];
            }
            score_sensitivity.push_back(sensitivity);
            acc_score_sensitivity.push_back(acc_sensitivity);
        }
        for (int i = 0; i < N; i++) {
            acc_score_sensitivity[i] /= acc_sensitivity;
        }

        for (int i = 0; i < N; i++) {
            placements.push_back(i);
        }
        current_score = score();
    }

    double calc_sensitivity(const P& p) {
        double ret = 0;
        for (int i = 0; i < prob.attendees.size(); i++) {
            auto attendee = prob.attendees[i];
            ret += 1 / geo::d(attendee.pos, attendee.get_pos()) * max_tastes[i];
        }
        return ret;
    }

    double score() {
        double ans = 0;
        for (int attendee_id = 0; attendee_id < prob.attendees.size(); attendee_id++) {
            auto attendee = prob.attendees[attendee_id];
            for (int i = 0; i < N; i++) {
                int musician_id = placements[i];
                int sound_id = prob.musicians[musician_id];
                ans += attendee.tastes[sound_id] * score_multi[attendee_id][i];
            }
        }
        return ans;
    }

    double score_diff(int pos_a, int pos_b) {
        double ans = 0;
        for (int attendee_id = 0; attendee_id < prob.attendees.size(); attendee_id++) {
            auto attendee = prob.attendees[attendee_id];
            {
                int musician_id_a = placements[pos_a];
                int musician_id_b = placements[pos_b];
                int sound_id_a = prob.musicians[musician_id_a];
                int sound_id_b = prob.musicians[musician_id_b];
                ans += (attendee.tastes[sound_id_b] - attendee.tastes[sound_id_a]) * score_multi[attendee_id][pos_a];
            }
            {
                int musician_id_a = placements[pos_a];
                int musician_id_b = placements[pos_b];
                int sound_id_a = prob.musicians[musician_id_a];
                int sound_id_b = prob.musicians[musician_id_b];
                ans += (attendee.tastes[sound_id_a] - attendee.tastes[sound_id_b]) * score_multi[attendee_id][pos_b];
            }
        }
        return ans;
    }

    void swap(int pos_a, int pos_b) {
        current_score += score_diff(pos_a, pos_b);
        int tmp = placements[pos_b];
        placements[pos_b] = placements[pos_a];
        placements[pos_a] = tmp;
    }

    solution_t get_solution() {
        vector<P> ret(N);
        for (int i = 0; i < N; i++) {
            ret[placements[i]] = positions[i];
        }
        return solution_t(ret);
    }
};

int main() {
    problem_t prob;
    load_problem(cin, prob);
    
    solver_t solver(prob);
    double best_score = -1e18;
    solution_t best_solution;
    sa::simulated_annealing sa;
    while (!sa.end()) {
        double rnd = random::get_double(0, 1);
        int x = lower_bound(solver.acc_score_sensitivity.begin(), solver.acc_score_sensitivity.end(), rnd) - solver.acc_score_sensitivity.begin(); //xor32() % solver.M;
        // int x = xor32() % solver.M;
        int y = random::get(solver.N - 1);
        if (y >= x) y++;
        double current_score = solver.current_score;
        double next_score = current_score + solver.score_diff(x, y);
        if (sa.accept(current_score, next_score)) {
            // use
            solver.swap(x, y);
            if (solver.current_score > best_score) {
                best_score = solver.current_score;
                best_solution = solver.get_solution();
            }
        } else {
            // not use
        }
    }
    //sa.print();

    print_solution(cout, best_solution);
    
    // fprintf(stderr, "best_score : %lf\n", best_score);
    fprintf(stderr, "global_best_score : %lld\n", score(prob, best_solution.as_p()));
    
    return 0;
}