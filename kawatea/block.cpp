#include <cstdio>
#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>
#include "../library/problem.h"
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif
#include "../library/scoring.h"
#include "../library/solution.h"

using namespace std;

class timer {
    public:
    void start() {
        origin = rdtsc();
    }
    
    inline double get_time() {
        return (rdtsc() - origin) * SECONDS_PER_CLOCK;
    }
    
    private:
    constexpr static double SECONDS_PER_CLOCK = 1 / 3.0e9;
    unsigned long long origin;
    
    inline static unsigned long long rdtsc() {
        unsigned long long lo, hi;
        __asm__ volatile ("rdtsc" : "=a" (lo), "=d" (hi));
        return (hi << 32) | lo;
    }
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
    constexpr static double TIME_LIMIT = 10;
    constexpr static double START_TEMP = 10000;
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

const int MAX_MUSICIAN = 1500;
const int MAX_ATTENDEE = 5000;
const double RADIUS = 10;
const double RADIUS2 = RADIUS * RADIUS;
const double BLOCK_RADIUS = 5;
manarimo::problem_t problem;
double stage_left;
double stage_right;
double stage_bottom;
double stage_top;
double max_diff_width;
double max_diff_height;
vector<geo::P> placements;
vector<pair<double, int>> attendee_angles[MAX_MUSICIAN];
vector<int> blocked_attendees[MAX_MUSICIAN][MAX_MUSICIAN];
int blocked_count[MAX_MUSICIAN][MAX_ATTENDEE];
vector<pair<double, int>> tmp_attendee_angles;
vector<int> tmp_blocked_attendees[MAX_MUSICIAN];
int tmp_blocked_count[MAX_ATTENDEE];
vector<geo::P> best_placements;

void input() {
    manarimo::load_problem(std::cin, problem);
    
    stage_left = problem.stage_bottom_left.X;
    stage_right = stage_left + problem.stage_width;
    stage_left += RADIUS;
    stage_right -= RADIUS;
    max_diff_width = (stage_right - stage_left) / 10;
    
    stage_bottom = problem.stage_bottom_left.Y;
    stage_top = stage_bottom + problem.stage_height;
    stage_bottom += RADIUS;
    stage_top -= RADIUS;
    max_diff_height = (stage_top - stage_bottom) / 10;
}

void output(const vector<geo::P>& placements) {
    manarimo::print_solution(std::cout, manarimo::solution_t(placements));
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

double get_angle(const geo::P& p1, const geo::P& p2) {
    return atan2(p2.Y - p1.Y, p2.X - p1.X);
}

void calc_blocked_one(int musician, const geo::P& p, vector<pair<double, int>>& attendee_angles, vector<int>* blocked_attendees, int* blocked_count) {
    attendee_angles.clear();
    for (int i = 0; i < problem.attendees.size(); i++) {
        double angle = get_angle(p, problem.attendees[i].pos);
        attendee_angles.emplace_back(angle, i);
        attendee_angles.emplace_back(angle + M_PI * 2, i);
    }
    sort(attendee_angles.begin(), attendee_angles.end());
    
    for (int i = 0; i < problem.musicians.size(); i++) blocked_attendees[i].clear();
    for (int i = 0; i < problem.attendees.size(); i++) blocked_count[i] = 0;
    for (int i = 0; i < problem.musicians.size(); i++) {
        if (i == musician) continue;
        double angle = get_angle(p, placements[i]);
        double offset = asin(BLOCK_RADIUS / dist(p, placements[i]));
        double start = angle - offset;
        double end = angle + offset;
        if (start < -M_PI) {
            start += M_PI * 2;
            end += M_PI * 2;
        }
        int index = lower_bound(attendee_angles.begin(), attendee_angles.end(), make_pair(start, -1)) - attendee_angles.begin();
        for (; index < attendee_angles.size(); index++) {
            if (attendee_angles[index].first > end) break;
            blocked_attendees[i].push_back(attendee_angles[index].second);
            blocked_count[attendee_angles[index].second]++;
        }
    }
}

void calc_blocked() {
    for (int i = 0; i < problem.musicians.size(); i++) calc_blocked_one(i, placements[i], attendee_angles[i], blocked_attendees[i], blocked_count[i]);
}

double calc_one_score(const geo::P& p1, const geo::P& p2, double taste) {
    return ceil(1000000 * taste / dist2(p1, p2));
}

double score_all() {
    calc_blocked();
    double sum = 0;
    for (int i = 0; i < problem.musicians.size(); i++) {
        for (int j = 0; j < problem.attendees.size(); j++) {
            if (blocked_count[i][j] == 0) sum += calc_one_score(placements[i], problem.attendees[j].pos, problem.attendees[j].tastes[problem.musicians[i]]);
        }
    }
    return sum;
}

void save_best_state() {
    best_placements = placements;
}

void load_best_state() {
    placements = best_placements;
    calc_blocked();
}

int main() {
    input();
    
    double best_score = -1e18;
    for (int i = 0; i < 10; i++) {
        random_init();
        double score = score_all();
        if (score > best_score) {
            best_score = score;
            best_placements = placements;
        }
    }
    placements = best_placements;
    double current_score = score_all();
    
    int unchanged = 0;
    vector<pair<int, int>> new_blocked;
    simulated_annealing sa;
    while (!sa.end()) {
        unchanged++;
        if (unchanged == 10000) {
            current_score = best_score;
            load_best_state();
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
            for (int i = 0; i < problem.attendees.size(); i++) {
                if (blocked_count[m][i] == 0) next_score -= calc_one_score(placements[m], problem.attendees[i].pos, problem.attendees[i].tastes[problem.musicians[m]]);
            }
            for (int i = 0; i < problem.musicians.size(); i++) {
                if (i == m) continue;
                for (int j : blocked_attendees[i][m]) {
                    blocked_count[i][j]--;
                    if (blocked_count[i][j] == 0) next_score += calc_one_score(placements[i], problem.attendees[j].pos, problem.attendees[j].tastes[problem.musicians[i]]);
                }
            }
            calc_blocked_one(m, next_p, tmp_attendee_angles, tmp_blocked_attendees, tmp_blocked_count);
            for (int i = 0; i < problem.attendees.size(); i++) {
                if (tmp_blocked_count[i] == 0) next_score += calc_one_score(next_p, problem.attendees[i].pos, problem.attendees[i].tastes[problem.musicians[m]]);
            }
            new_blocked.clear();
            for (int i = 0; i < problem.musicians.size(); i++) {
                if (i == m) continue;
                double angle = get_angle(placements[i], next_p);
                double offset = asin(BLOCK_RADIUS / dist(placements[i], next_p));
                double start = angle - offset;
                double end = angle + offset;
                if (start < -M_PI) {
                    start += M_PI * 2;
                    end += M_PI * 2;
                }
                int index = lower_bound(attendee_angles[i].begin(), attendee_angles[i].end(), make_pair(start, -1)) - attendee_angles[i].begin();
                for (; index < attendee_angles[i].size(); index++) {
                    if (attendee_angles[i][index].first > end) break;
                    int attendee = attendee_angles[i][index].second;
                    new_blocked.emplace_back(i, attendee);
                    if (blocked_count[i][attendee] == 0) next_score -= calc_one_score(placements[i], problem.attendees[attendee].pos, problem.attendees[attendee].tastes[problem.musicians[i]]);
                }
            }
            if (sa.accept(current_score, next_score)) {
                current_score = next_score;
                placements[m] = next_p;
                swap(attendee_angles[m], tmp_attendee_angles);
                for (int i = 0; i < problem.musicians.size(); i++) swap(blocked_attendees[m][i], tmp_blocked_attendees[i]);
                for (int i = 0; i < problem.attendees.size(); i++) blocked_count[m][i] = tmp_blocked_count[i];
                for (int i = 0; i < problem.musicians.size(); i++) {
                    if (i == m) continue;
                    blocked_attendees[i][m].clear();
                }
                for (const pair<int, int>& p : new_blocked) {
                    blocked_attendees[p.first][m].push_back(p.second);
                    blocked_count[p.first][p.second]++;
                }
                if (current_score > best_score) {
                    best_score = current_score;
                    save_best_state();
                    unchanged = 0;
                }
            } else {
                for (int i = 0; i < problem.musicians.size(); i++) {
                    if (i == m) continue;
                    for (int j : blocked_attendees[i][m]) blocked_count[i][j]++;
                }
            }
        } else {
            int m1 = random::get(problem.musicians.size());
            int m2 = random::get(problem.musicians.size() - 1);
            if (m2 >= m1) m2++;
            double next_score = current_score;
            for (int i = 0; i < problem.attendees.size(); i++) {
                if (blocked_count[m1][i] == 0 || blocked_count[m2][i] == 0) {
                    geo::P p = problem.attendees[i].pos;
                    if (blocked_count[m1][i] == 0) {
                        next_score -= calc_one_score(placements[m1], p, problem.attendees[i].tastes[problem.musicians[m1]]);
                        next_score += calc_one_score(placements[m1], p, problem.attendees[i].tastes[problem.musicians[m2]]);
                    }
                    if (blocked_count[m2][i] == 0) {
                        next_score -= calc_one_score(placements[m2], p, problem.attendees[i].tastes[problem.musicians[m2]]);
                        next_score += calc_one_score(placements[m2], p, problem.attendees[i].tastes[problem.musicians[m1]]);
                    }
                }
            }
            if (sa.accept(current_score, next_score)) {
                current_score = next_score;
                swap(placements[m1], placements[m2]);
                attendee_angles[m1].swap(attendee_angles[m2]);
                for (int i = 0; i < problem.musicians.size(); i++) {
                    if (i == m1 || i == m2) continue;
                    blocked_attendees[i][m1].swap(blocked_attendees[i][m2]);
                    blocked_attendees[m1][i].swap(blocked_attendees[m2][i]);
                }
                blocked_attendees[m1][m2].swap(blocked_attendees[m2][m1]);
                for (int i = 0; i < problem.attendees.size(); i++) swap(blocked_count[m1][i], blocked_count[m2][i]);
                if (current_score > best_score) {
                    best_score = current_score;
                    save_best_state();
                    unchanged = 0;
                }
            }
        }
    }
    
    output(best_placements);
    
    fprintf(stderr, "best_score : %lf\n", best_score);
    
    return 0;
}
