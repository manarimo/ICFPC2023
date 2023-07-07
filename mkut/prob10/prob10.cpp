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
    
    vector<int> good_musicians, bad_musicians;
    for (int i = 0; i < prob.musicians.size(); i++) {
        if (prob.musicians[i] == 3) {
            good_musicians.push_back(i);
        } else {
            bad_musicians.push_back(i);
        }
    }
    
    double right = prob.stage_bottom_left.first + 100;
    double bottom = prob.stage_bottom_left.second + 10;
    vector<P> positions(prob.musicians.size());

    cerr << good_musicians.size() << "," << bad_musicians.size() << endl;

    for (int i = 0; i < 10; i++) {
        cerr << good_musicians[i] << endl;
        positions[good_musicians[i]] = P(right, bottom + i * 10);
    }
    for (int i = 1; i < 10; i++) {
        cerr << good_musicians[i+9] << endl;
        positions[good_musicians[i+9]] = P(right - i * 10, bottom);
    }
    for (int i = 19; i < good_musicians.size(); i++) {
        cerr << good_musicians[i] << " " << 9 << " " << (i - 18) << endl;
        positions[good_musicians[i]] = P(right - 90, bottom + (i - 18) * 10);
    }

    int idx = 0;
    for (int i = 1; i < 10 && idx < bad_musicians.size(); i++) {
        for (int j = 1; j < 10 && idx < bad_musicians.size(); j++) {
            if (i == 9 && j <= good_musicians.size() - 19) {
                continue;
            }
            cerr << bad_musicians[idx] << " " << i << " " << j << endl;
            positions[bad_musicians[idx]] = P(right - i * 10, bottom + j * 10);
            idx++;
        }
    }

    output(positions);
    
    fprintf(stderr, "best_score : %lld\n", manarimo::score(prob, positions));
    
    return 0;
}