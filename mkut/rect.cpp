#include <iostream>
#include <set>
#include "../library/geo.h"
#include "../library/xorshift.h"
#include "../library/problem.h"
#include "../library/simulated_annealing.h"
#include "../library/scoring.h"
using namespace std;
using namespace manarimo;

class solver_t {
    problem_t prob;

    public:
    vector<P> positions;
    int N; // number of musicians
    int M; // number of active musicians
    int W; // capacity of a row;
    int H; // capacity of a column;
    number Left;
    number Right;
    number Bottom;
    number Top;
    vector<int> leftPositions, rightPositions, bottomPositions, topPositions;
    vector<vector<double>> score_multi; // attendee -> musician_pos

    vector<int> placements;
    double current_score;

    solver_t(problem_t& prob) : prob(prob) {
        init();
    }

    void init() {
        N = prob.musicians.size();
        W = prob.stage_width / 10;
        H = prob.stage_width / 10;
        Left = prob.stage_bottom_left.first + 5;
        Bottom = prob.stage_bottom_left.second + 5;
        Right = Left + (W-1) * 10;
        Top = Bottom + (H-1) * 10;
        // bottom
        for (int i = 0; i < W && positions.size() < N; i++) {
            bottomPositions.push_back(positions.size());
            positions.push_back(P(Left + i * 10, Bottom));
        }
        // top
        for (int i = 0; i < W && positions.size() < N; i++) {
            topPositions.push_back(positions.size());
            positions.push_back(P(Left + i * 10, Top));
        }
        // left
        leftPositions.push_back(bottomPositions[0]);
        for (int i = 1; i < H-1 && positions.size() < N; i++) {
            leftPositions.push_back(positions.size());
            positions.push_back(P(Left, Bottom + i * 10));
        }
        leftPositions.push_back(topPositions[0]);
        // right
        rightPositions.push_back(bottomPositions[bottomPositions.size() - 1]);
        for (int i = 1; i < H-1 && positions.size() < N; i++) {
            rightPositions.push_back(positions.size());
            positions.push_back(P(Right, Bottom + i * 10));
        }
        rightPositions.push_back(topPositions[topPositions.size() - 1]);
        M = positions.size();
        // rest
        for (int i = 1; i < W-1 && positions.size() < N; i++) {
            for (int j = 1; j < H-1 && positions.size() < N; j++) {
                positions.push_back(P(Left + i * 10, Bottom + j * 10));
            }
        }

        score_multi = vector<vector<double>>(prob.attendees.size(), vector<double>(N, 0));
        for (pair<int,int> unblocked : get_unblocked_pairs(prob, positions)) {
            int musician_id = unblocked.first;
            int attendee_id = unblocked.second;
            auto attendee = prob.attendees[attendee_id];
            score_multi[attendee_id][musician_id] = 1000000 / d(attendee.get_pos(), positions[musician_id]);
        }

        for (int i = 0; i < N; i++) {
            placements.push_back(i);
        }
        current_score = score();
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
            if (pos_a < M) {
                int musician_id_a = placements[pos_a];
                int musician_id_b = placements[pos_b];
                int sound_id_a = prob.musicians[musician_id_a];
                int sound_id_b = prob.musicians[musician_id_b];
                ans += (attendee.tastes[sound_id_b] - attendee.tastes[sound_id_a]) * score_multi[attendee_id][pos_a];
            }
            if (pos_b < M) {
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

    vector<P> get_placements() {
        vector<P> ret(N);
        for (int i = 0; i < N; i++) {
            ret[placements[i]] = positions[i];
        }
        return ret;
    }
};


void output(const vector<geo::P>& placements) {
    printf("{\n");
    printf("  \"placements\": [\n");
    for (int i = 0; i < placements.size(); i++) {
        printf("    {\"x\": %lf, \"y\": %lf}", placements[i].X, placements[i].Y);
        if (i + 1 < placements.size()) printf(",");
        printf("\n");
    }
    printf("  ]\n");
    printf("}\n");
}

int main() {
    problem_t prob;
    load_problem(cin, prob);
    solver_t solver(prob);
    
    double best_score = -1e18;
    vector<geo::P> best_placements;
    sa::simulated_annealing sa;
    while (!sa.end()) {
        int x = xor32() % solver.M;
        int y = xor32() % (solver.N - 1);
        if (y >= x) y++;
        double current_score = solver.current_score;
        double next_score = current_score + solver.score_diff(x, y);
        if (sa.accept(current_score, next_score)) {
            // use
            solver.swap(x, y);
            if (solver.current_score > best_score) {
                best_score = solver.current_score;
                best_placements = solver.get_placements();
            }
        } else {
            // not use
        }
    }

    output(best_placements);
    
    fprintf(stderr, "best_score : %lf\n", best_score);
    fprintf(stderr, "best_score : %lld\n", score(prob, best_placements));
    
    return 0;
}