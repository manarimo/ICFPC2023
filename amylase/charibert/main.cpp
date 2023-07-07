#include <cstdio>
#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>
#include "../../library/problem.h"
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif
#include "../../library/scoring.h"
#include "../../library/solution.h"

using namespace std;

class timer {
    public:
    void start() {
        origin = chrono::system_clock::now();
    }
    
    inline double get_time() {
        auto current = chrono::system_clock::now();
        return chrono::duration_cast<std::chrono::milliseconds>(current - origin).count() / 1000.;
    }
    
    chrono::time_point<chrono::system_clock> origin;
};

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

class simulated_annealing {
    public:
    simulated_annealing();
    inline bool end();
    inline bool accept(double current_score, double next_score);
    void print() const;
    
    private:
    constexpr static bool MAXIMIZE = true;
    constexpr static int LOG_SIZE = 0xFFFF;
    constexpr static int UPDATE_INTERVAL = 0xFF;
    constexpr static double TIME_LIMIT = 30;
    constexpr static double START_TEMP = 100000;
    constexpr static double END_TEMP = 1e-9;
    constexpr static double TEMP_RATIO = (END_TEMP - START_TEMP) / TIME_LIMIT;
    double log_probability[LOG_SIZE + 1];
    long long iteration = 0;
    long long accepted = 0;
    long long rejected = 0;
    double time = 0;
    double temp = START_TEMP;
    timer sa_timer;
};

simulated_annealing::simulated_annealing() {
    sa_timer.start();
    for (int i = 0; i <= LOG_SIZE; i++) log_probability[i] = log(random::probability());
}

inline bool simulated_annealing::end() {
    iteration++;
    if ((iteration & UPDATE_INTERVAL) == 0) {
        time = sa_timer.get_time();
        temp = START_TEMP + TEMP_RATIO * time;
        return time >= TIME_LIMIT;
    } else {
        return false;
    }
}

inline bool simulated_annealing::accept(double current_score, double next_score) {
    double diff = (MAXIMIZE ? next_score - current_score : current_score - next_score);
    if (diff >= 0 || diff > log_probability[random::get_fast(LOG_SIZE)] * temp) {
        accepted++;
        return true;
    } else {
        rejected++;
        return false;
    }
}

void simulated_annealing::print() const {
    fprintf(stderr, "iteration: %lld\n", iteration);
    fprintf(stderr, "accepted: %lld\n", accepted);
    fprintf(stderr, "rejected: %lld\n", rejected);
}

const double RADIUS = 10;
const double RADIUS2 = RADIUS * RADIUS;
manarimo::problem_t problem;
double stage_left;
double stage_right;
double stage_bottom;
double stage_top;
double max_diff_width;
double max_diff_height;
vector<geo::P> placements;

void input() {
    manarimo::load_problem(std::cin, problem);
    
    stage_left = problem.stage_bottom_left.Y;
    stage_right = stage_left + problem.stage_width;
    stage_left += RADIUS;
    stage_right -= RADIUS;
    max_diff_width = (stage_right - stage_left) / 10;
    
    stage_bottom = problem.stage_bottom_left.X;
    stage_top = stage_bottom + problem.stage_height;
    stage_bottom += RADIUS;
    stage_top -= RADIUS;
    max_diff_height = (stage_top - stage_bottom) / 10;
}

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

double dist2(const geo::P& p1, const geo::P& p2) {
    return (p1.X - p2.X) * (p1.X - p2.X) + (p1.Y - p2.Y) * (p1.Y - p2.Y);
}

double dist(const geo::P& p1, const geo::P& p2) {
    return sqrt(dist2(p1, p2));
}

void random_init() {
    placements.clear();
    for (int i = 0; i < problem.musicians.size(); i++) {
        while (true) {
            geo::P p(random::get(stage_left, stage_right), random::get(stage_bottom, stage_top));
            bool ng = false;
            for (int j = 0; j < i; j++) {
                if (dist2(placements[j], p) < RADIUS2) {
                    ng = true;
                    break;
                }
            }
            if (!ng) {
                placements.push_back(p);
                break;
            }
        }
    }
}

double score_one(const geo::P& p, int musician) {
    double sum = 0;
    for (const manarimo::atendee_t& a : problem.attendees) {
        sum += ceil(1000000 * a.tastes[musician] / dist2(p, geo::P(a.x, a.y)));
    }
    return sum;
}

double score_all() {
    double sum = 0;
    for (int i = 0; i < problem.musicians.size(); i++) sum += score_one(placements[i], problem.musicians[i]);
    return sum;
}

int main(int argc, char *argv[]) {
    input();
    
    double best_score = -1e18;
    vector<geo::P> best_placements;
    if (argc < 2) {    
        for (int i = 0; i < 100; i++) {
            random_init();
            double score = score_all();
            if (score > best_score) {
                best_score = score;
                best_placements = placements;
            }
        }
    } else {
        manarimo::solution_t intermediate_solution;
        manarimo::load_solution(string(argv[1]), intermediate_solution);
        for (auto p : intermediate_solution.placements) {
            best_placements.push_back(make_pair(p.x, p.y));
        }
        long long loaded_score = manarimo::score(problem, best_placements);
        fprintf(stderr, "score at load : %lld\n", loaded_score);
        best_score = loaded_score;
    }
    double current_score = best_score;
    placements = best_placements;

    
    int unchanged = 0;
    simulated_annealing sa;
    while (!sa.end()) {
        unchanged++;
        if (unchanged == 10000) {
            current_score = best_score;
            placements = best_placements;
            unchanged = 0;
        }
        
        if (random::get(100) < 80) {
            int m = random::get(problem.musicians.size());
            geo::P& current_p = placements[m];
            double dx = random::get_double(max(-max_diff_width, stage_left - current_p.X), min(max_diff_width, stage_right - current_p.X));
            double dy = random::get_double(max(-max_diff_height, stage_bottom - current_p.Y), min(max_diff_height, stage_top - current_p.Y));
            geo::P next_p = current_p;
            next_p.X += dx;
            next_p.Y += dy;
            bool ng = false;
            for (int i = 0; i < placements.size(); i++) {
                if (i == m) continue;
                if (dist2(placements[i], next_p) < RADIUS2) {
                    ng = true;
                    break;
                }
            }
            if (ng) continue;
            double next_score = current_score;
            next_score -= score_one(current_p, problem.musicians[m]);
            next_score += score_one(next_p, problem.musicians[m]);
            if (sa.accept(current_score, next_score)) {
                current_score = next_score;
                placements[m] = next_p;
                if (current_score > best_score) {
                    best_score = current_score;
                    best_placements = placements;
                    unchanged = 0;
                }
            }
        } else {
            int m1 = random::get(problem.musicians.size());
            int m2 = random::get(problem.musicians.size() - 1);
            if (m2 >= m1) m2++;
            double next_score = current_score;
            next_score -= score_one(placements[m1], problem.musicians[m1]);
            next_score -= score_one(placements[m2], problem.musicians[m2]);
            next_score += score_one(placements[m1], problem.musicians[m2]);
            next_score += score_one(placements[m2], problem.musicians[m1]);
            if (sa.accept(current_score, next_score)) {
                current_score = next_score;
                swap(placements[m1], placements[m2]);
                if (current_score > best_score) {
                    best_score = current_score;
                    best_placements = placements;
                    unchanged = 0;
                }
            }
        }
    }
    
    output(best_placements);
    
    fprintf(stderr, "best_score : %lld\n", manarimo::score(problem, best_placements));
    
    return 0;
}
