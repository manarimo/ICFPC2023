#include <iostream>
#include <set>
#include "../../library/geo.h"
#include "../../library/xorshift.h"
#include "../../library/problem.h"
#include "../../library/simulated_annealing.h"
#include "../../library/scoring.h"
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

vector<P> bad_positions(const problem_t& problem, int n) {
    P center(problem.stage_bottom_left.first + problem.stage_width / 2, problem.stage_bottom_left.second + problem.stage_height / 2);
    vector<P> ret;
    ret.push_back(center);
    double sz = 10.00001;
    for (int r = 1; ret.size() < n ; r ++) {
        for (int t = 0; t < 6; t++) {
            double theta = M_PI / 3 * t;
            double sx = center.first + sz * r * cos(theta);
            double sy = center.second + sz * r * sin(theta);
            double dx = sz * cos(theta + M_PI * 2/ 3);
            double dy = sz * sin(theta + M_PI * 2/ 3);
            for (int i = 0; i < r; i++) {
                ret.push_back(P(sx + dx * i, sy + dy * i));
            }
        }
    }
    return ret;
}

vector<P> good_positions(const problem_t& problem) {
    vector<pair<P,double>> ret;
    int w = problem.stage_width / 10 - 1;
    int h = problem.stage_height / 10 - 1;
    double dx = (problem.stage_width - 20) / (w - 1);
    double dy = (problem.stage_height - 20) / (h - 1);
    double left = problem.stage_bottom_left.first + 10;
    double right = problem.stage_bottom_left.first + problem.stage_width - 10;
    double bottom = problem.stage_bottom_left.second + 10;
    double top = problem.stage_bottom_left.second + problem.stage_height - 10;
    cerr << problem.stage_width << endl;
    cerr << dx << "," << w << endl;
    cerr << right << "," << (left + dx * (w - 1)) << endl;
    for (int i = 0; i < w - 1; i++) {
        ret.push_back(make_pair(P(left + dx * i, bottom), 0));
        ret.push_back(make_pair(P(left + dx * i, top), 0));
    }
    for (int i = 1; i < h - 1; i++) {
        ret.push_back(make_pair(P(left, bottom + dy * i), 0));
        ret.push_back(make_pair(P(right, bottom + dy * i), 0));
    }
    for (int i = 0; i < ret.size(); i++) {
        double potential = 0;
        for (auto attendee : problem.attendees) {
            double max_tastes = 0;
            for (double taste : attendee.tastes) {
                max_tastes = max(max_tastes, taste);
            }
            potential += max_tastes / geo::d(ret[i].first, attendee.get_pos());
        }
        ret[i].second = potential;
    }
    sort(ret.begin(), ret.end(), [](auto const& lhs, auto const& rhs) {
            return lhs.second > rhs.second;
    });
    vector<P> ret2;
    for (auto p : ret) {
        ret2.push_back(p.first);
    }
    return ret2;
}

int main() {
    problem_t prob;
    load_problem(cin, prob);
    
    int good_idx = 0;
    int bad_idx = 0;
    int num_bad = 0;
    for (auto musician : prob.musicians) {
        if (musician != 3) num_bad++;
    }
    
    vector<P> bads = bad_positions(prob, num_bad);
    vector<P> goods = good_positions(prob);

    vector<P> positions;
    for (auto musician : prob.musicians) {
        if (musician != 3) {
            positions.push_back(bads[bad_idx]);
            bad_idx++;
        } else {
            positions.push_back(goods[good_idx]);
            good_idx++;
        }
    }

    output(positions);
    
    fprintf(stderr, "best_score : %lld\n", manarimo::score(prob, positions));
    
    return 0;
}