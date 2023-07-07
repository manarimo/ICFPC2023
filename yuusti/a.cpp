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

const int D = 1000;
double drand(int x, int y) {
    return sa::random::get(x * D, y * D) / (double) D;
}

const int RAD = 10;

vector<geo::P> random_place(manarimo::problem_t& prob) {
    int stage_bottom = prob.stage_bottom_left.second;
    int stage_top = stage_bottom + prob.stage_height; 

    int stage_left = prob.stage_bottom_left.first;
    int stage_right = stage_left + prob.stage_width;

    vector<geo::P> pos;
    int cnt = 0;
    while (pos.size() < prob.musicians.size()) {
        double x = drand(stage_left + RAD, stage_right - RAD);
        double y = drand(stage_bottom + RAD, stage_top - RAD);

        bool ok = true;
        for (auto p: pos) {
            if (geo::dist_point(p.X, p.Y, x, y) < RAD * RAD) {
                ok = false;
                break;
            }
        }

        if (ok) {
            pos.emplace_back(x, y);
        }

        assert(cnt < 10000);
    }

    return pos;
}

vector<geo::P> packed_place(manarimo::problem_t& prob) {
    int stage_bottom = prob.stage_bottom_left.second;

    int stage_left = prob.stage_bottom_left.first;

    vector<geo::P> pos;
    int n = prob.musicians.size();
    int cnt = 0;
    // TODO: stage size miru
    while (cnt * cnt < n) ++cnt;

    for (int i = 0; i < cnt; ++i) {
        for (int j = 0; j < cnt; ++j) {
            pos.emplace_back(stage_left + RAD * (i + 1), stage_bottom + RAD * (j + 1));
            if (pos.size() == (size_t) n) return pos;
        }
    }

    assert(false);
}


long long scoring(const manarimo::problem_t &prob, const vector<geo::P> &musicians) {
    return manarimo::score(prob, musicians);
}

void print_point(const geo::P &p) {
    printf("{\"x\": %lf, \"y\": %lf}", p.X, p.Y);
}

void print_json(const vector<geo::P>& vp) {
    printf("{\"placements\":[");
    for (size_t i = 0; i < vp.size(); ++i) {
        if (i) printf(",");
        print_point(vp[i]);
    }

    printf("]}\n");
}

int main() {
    manarimo::problem_t prob;
    manarimo::load_problem(cin, prob);
    sa::simulated_annealing sa;

    auto p = packed_place(prob);

    double current_score = scoring(prob, p);
    while (!sa.end()) {
        int a = sa::random::get(p.size());
        int b = sa::random::get(p.size());
        if (a != b || sa::random::get(100) < 70) {
            // swap
            swap(p[a], p[b]);
            double next_score = scoring(prob, p);
            if (sa.accept(current_score, next_score)) {
                // use
                current_score = next_score;
            } else {
                // not use
                swap(p[a], p[b]);
            }
        } else if (true) {
            // all move

            double stage_left = prob.stage_bottom_left.first;
            double stage_right = stage_left + prob.stage_width;

            double stage_bottom = prob.stage_bottom_left.second;
            double stage_top = stage_bottom + prob.stage_height; 

            int cnt = 0;
            while (true) {
                double dx = drand(0, prob.stage_width / 5) - prob.stage_width / 10;
                double dy = drand(0, prob.stage_height / 5) - prob.stage_height / 10;

                double minx = 1e9, miny = 1e9, maxx = -1e9, maxy = -1e9;
                for (auto& ps: p) {
                    minx = min(minx, ps.X + dx);
                    miny = min(miny, ps.Y + dy);
                    maxx = max(maxx, ps.X + dx);
                    maxy = max(maxy, ps.Y + dy);
                }
                if (stage_left + RAD < minx && maxx < stage_right - RAD 
                && stage_bottom + RAD < miny && maxy < stage_top - RAD) {
                    for (auto& ps: p) {
                        ps.X += dx;
                        ps.Y += dy;
                    }
                    double next_score = scoring(prob, p);
                    if (sa.accept(current_score, next_score)) {
                        // use
                        current_score = next_score;
                    } else {
                        // not use
                        for (auto& ps: p) {
                            ps.X -= dx;
                            ps.Y -= dy;
                        }
                    }

                    break;
                }
                if (++cnt > 1000) break;
            }
        } else {
            // move
            auto tmp = p[a];

            double stage_left = prob.stage_bottom_left.first;
            double stage_right = stage_left + prob.stage_width;

            double stage_bottom = prob.stage_bottom_left.second;
            double stage_top = stage_bottom + prob.stage_height; 

            int cnt = 0;
            while (true) {
                p[a].X = drand(stage_left + RAD, stage_right - RAD);
                p[a].Y = drand(stage_bottom + RAD, stage_top - RAD);

                bool ok = true;
                for (size_t i = 0; i < p.size(); ++i) {
                    if (i != (size_t) a && geo::dist_point(p[i].X, p[i].Y, p[a].X, p[a].Y) < RAD * RAD) {
                        ok = false;
                        break;
                    }
                }
                if (ok) break;
                if (++cnt > 1000) break;
            }

            double next_score = scoring(prob, p);
            if (cnt <= 100 && sa.accept(current_score, next_score)) {
                // use
                current_score = next_score;
            } else {
                // not use
                p[a] = tmp;
            }
        }

    }

    sa.print();

    long long score = scoring(prob, p);
    std::cerr << score << endl;

    print_json(p);

    return 0;
}
