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
    simulated_annealing(double time_limit);
    inline bool end();
    inline bool accept(double current_score, double next_score);
    void print() const;
    
    private:
    constexpr static bool MAXIMIZE = true;
    constexpr static int LOG_SIZE = 0xFFFF;
    constexpr static int UPDATE_INTERVAL = 0xFF;
    constexpr static double START_TEMP = 10000;
    constexpr static double END_TEMP = 1e-9;
    double time_limit;
    double temp_ratio;
    double log_probability[LOG_SIZE + 1];
    long long iteration = 0;
    long long accepted = 0;
    long long rejected = 0;
    double time = 0;
    double temp = START_TEMP;
    timer sa_timer;
};

simulated_annealing::simulated_annealing(double time_limit) : time_limit(time_limit) {
    temp_ratio = (END_TEMP - START_TEMP) / time_limit;
    sa_timer.start();
    for (int i = 0; i <= LOG_SIZE; i++) log_probability[i] = log(random::probability());
}

inline bool simulated_annealing::end() {
    iteration++;
    if ((iteration & UPDATE_INTERVAL) == 0) {
        time = sa_timer.get_time();
        temp = START_TEMP + temp_ratio * time;
        return time >= time_limit;
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

const double INIT_TIME_LIMIT = 10;
const double MAIN_TIME_LIMIT = 250;
const int MAX_MUSICIAN = 1000;
const int MAX_ATTENDEE = 700;
const double RADIUS = 10;
const double RADIUS2 = RADIUS * RADIUS;
const double BLOCK_RADIUS = 5;
const double VOLUME = 10;
manarimo::problem_t problem;
double stage_left;
double stage_right;
double stage_bottom;
double stage_top;
double max_diff_width;
double max_diff_height;
vector<int> instrument[MAX_MUSICIAN];
vector<geo::P> placements;
vector<pair<double, int>> attendee_angles[MAX_MUSICIAN];
vector<int> blocked_attendees[MAX_MUSICIAN][MAX_MUSICIAN];
int blocked_count[MAX_MUSICIAN][MAX_ATTENDEE];
double q[MAX_MUSICIAN];
double impact_sum[MAX_MUSICIAN];
vector<pair<double, int>> tmp_attendee_angles;
vector<int> tmp_blocked_attendees[MAX_MUSICIAN];
int tmp_blocked_count[MAX_ATTENDEE];
double tmp_q[MAX_MUSICIAN];
double tmp_impact_sum[MAX_MUSICIAN];
vector<geo::P> best_placements;
vector<double> volumes;

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
    
    for (int i = 0; i < problem.musicians.size(); i++) instrument[problem.musicians[i]].push_back(i);
}

void output(const vector<geo::P>& placements, const vector<double>& volumes) {
    manarimo::print_solution(std::cout, manarimo::solution_t(placements, volumes));
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
        int index = lower_bound(attendee_angles.begin(), attendee_angles.end(), make_pair(start, 100000000)) - attendee_angles.begin();
        for (; index < attendee_angles.size(); index++) {
            if (attendee_angles[index].first >= end) break;
            blocked_attendees[i].push_back(attendee_angles[index].second);
            blocked_count[attendee_angles[index].second]++;
        }
    }
    for (int i = 0; i < problem.pillars.size(); i++) {
        double angle = get_angle(p, problem.pillars[i].center);
        double offset = asin(problem.pillars[i].radius / dist(p, problem.pillars[i].center));
        double start = angle - offset;
        double end = angle + offset;
        if (start < -M_PI) {
            start += M_PI * 2;
            end += M_PI * 2;
        }
        int index = lower_bound(attendee_angles.begin(), attendee_angles.end(), make_pair(start, 100000000)) - attendee_angles.begin();
        for (; index < attendee_angles.size(); index++) {
            if (attendee_angles[index].first >= end) break;
            if (geo::get_ratio(p, problem.attendees[attendee_angles[index].second].pos, problem.pillars[i].center) < 1) blocked_count[attendee_angles[index].second]++;
        }
    }
}

void calc_blocked() {
    for (int i = 0; i < problem.musicians.size(); i++) calc_blocked_one(i, placements[i], attendee_angles[i], blocked_attendees[i], blocked_count[i]);
}

double calc_one_score(const geo::P& p1, const geo::P& p2, double taste) {
    return ceil(1000000 * taste / dist2(p1, p2));
}

double score_all_approximate() {
    calc_blocked();
    double sum = 0;
    for (int i = 0; i < problem.musicians.size(); i++) {
        q[i] = 1;
        for (int j : instrument[problem.musicians[i]]) {
            if (i == j) continue;
            q[i] += 1 / dist(placements[i], placements[j]);
        }
        impact_sum[i] = 0;
        for (int j = 0; j < problem.attendees.size(); j++) {
            if (blocked_count[i][j] == 0) impact_sum[i] += calc_one_score(placements[i], problem.attendees[j].pos, problem.attendees[j].tastes[problem.musicians[i]]);
        }
        sum += ceil(VOLUME * q[i] * max(impact_sum[i], 0.0));
    }
    return sum;
}

double score_all_exact() {
    calc_blocked();
    volumes.clear();
    double sum = 0;
    for (int i = 0; i < problem.musicians.size(); i++) {
        double q = 1;
        for (int j : instrument[problem.musicians[i]]) {
            if (i == j) continue;
            q += 1 / dist(placements[i], placements[j]);
        }
        double tmp = 0;
        for (int j = 0; j < problem.attendees.size(); j++) {
            if (blocked_count[i][j] == 0) tmp += ceil(VOLUME * q * calc_one_score(placements[i], problem.attendees[j].pos, problem.attendees[j].tastes[problem.musicians[i]]));
        }
        if (tmp >= 0) {
            sum += tmp;
            volumes.push_back(VOLUME);
        } else {
            volumes.push_back(0);
        }
    }
    return sum;
}

double score_one_no_block(const geo::P& p, int musician) {
    double sum = 0;
    for (const manarimo::atendee_t& a : problem.attendees) {
        sum += calc_one_score(p, a.pos, a.tastes[musician]);
    }
    return sum;
}

double score_all_no_block() {
    double sum = 0;
    for (int i = 0; i < problem.musicians.size(); i++) sum += score_one_no_block(placements[i], problem.musicians[i]);
    return sum;
}

void save_best_state() {
    best_placements = placements;
}

void load_best_state() {
    placements = best_placements;
    score_all_approximate();
}

void sa_no_block() {
    double best_score = -1e18;
    for (int i = 0; i < 10; i++) {
        random_init();
        double score = score_all_no_block();
        if (score > best_score) {
            best_score = score;
            best_placements = placements;
        }
    }
    placements = best_placements;
    double current_score = best_score;
    
    int unchanged = 0;
    simulated_annealing sa(INIT_TIME_LIMIT);
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
            double dx = clamp(random::get_double(-max_diff_width, max_diff_width), stage_left - current_p.X, stage_right - current_p.X);
            double dy = clamp(random::get_double(-max_diff_height, max_diff_height), stage_bottom - current_p.Y, stage_top - current_p.Y);
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
            next_score -= score_one_no_block(current_p, problem.musicians[m]);
            next_score += score_one_no_block(next_p, problem.musicians[m]);
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
            next_score -= score_one_no_block(placements[m1], problem.musicians[m1]);
            next_score -= score_one_no_block(placements[m2], problem.musicians[m2]);
            next_score += score_one_no_block(placements[m1], problem.musicians[m2]);
            next_score += score_one_no_block(placements[m2], problem.musicians[m1]);
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
}

int main() {
    input();
    
    sa_no_block();
    placements = best_placements;
    double best_score = score_all_approximate();
    double current_score = best_score;
    
    int unchanged = 0;
    vector<pair<int, int>> new_blocked;
    simulated_annealing sa(MAIN_TIME_LIMIT);
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
            double dx = clamp(random::get_double(-max_diff_width, max_diff_width), stage_left - current_p.X, stage_right - current_p.X);
            double dy = clamp(random::get_double(-max_diff_height, max_diff_height), stage_bottom - current_p.Y, stage_top - current_p.Y);
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
            int in = problem.musicians[m];
            for (int i = 0; i < problem.musicians.size(); i++) {
                tmp_q[i] = q[i];
                tmp_impact_sum[i] = impact_sum[i];
            }
            tmp_q[m] = 1;
            for (int musician : instrument[in]) {
                if (musician == m) continue;
                double new_dist = 1 / dist(next_p, placements[musician]);
                tmp_q[musician] = q[musician] - 1 / dist(current_p, placements[musician]) + new_dist;
                tmp_q[m] += new_dist;
            }
            for (int i = 0; i < problem.musicians.size(); i++) {
                if (i == m) continue;
                for (int j : blocked_attendees[i][m]) {
                    blocked_count[i][j]--;
                    if (blocked_count[i][j] == 0) tmp_impact_sum[i] += calc_one_score(placements[i], problem.attendees[j].pos, problem.attendees[j].tastes[problem.musicians[i]]);
                }
            }
            calc_blocked_one(m, next_p, tmp_attendee_angles, tmp_blocked_attendees, tmp_blocked_count);
            tmp_impact_sum[m] = 0;
            for (int i = 0; i < problem.attendees.size(); i++) {
                if (tmp_blocked_count[i] == 0) tmp_impact_sum[m] += calc_one_score(next_p, problem.attendees[i].pos, problem.attendees[i].tastes[in]);
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
                int index = lower_bound(attendee_angles[i].begin(), attendee_angles[i].end(), make_pair(start, 100000000)) - attendee_angles[i].begin();
                for (; index < attendee_angles[i].size(); index++) {
                    if (attendee_angles[i][index].first >= end) break;
                    int attendee = attendee_angles[i][index].second;
                    new_blocked.emplace_back(i, attendee);
                    if (blocked_count[i][attendee] == 0) tmp_impact_sum[i] -= calc_one_score(placements[i], problem.attendees[attendee].pos, problem.attendees[attendee].tastes[problem.musicians[i]]);
                }
            }
            double next_score = 0;
            for (int i = 0; i < problem.musicians.size(); i++) {
                next_score += ceil(VOLUME * tmp_q[i] * max(tmp_impact_sum[i], 0.0));
            }
            if (sa.accept(current_score, next_score)) {
                current_score = next_score;
                placements[m] = next_p;
                swap(attendee_angles[m], tmp_attendee_angles);
                for (int i = 0; i < problem.musicians.size(); i++) blocked_attendees[m][i].swap(tmp_blocked_attendees[i]);
                for (int i = 0; i < problem.attendees.size(); i++) blocked_count[m][i] = tmp_blocked_count[i];
                for (int i = 0; i < problem.musicians.size(); i++) {
                    if (i == m) continue;
                    blocked_attendees[i][m].clear();
                }
                for (const pair<int, int>& p : new_blocked) {
                    blocked_attendees[p.first][m].push_back(p.second);
                    blocked_count[p.first][p.second]++;
                }
                for (int i = 0; i < problem.musicians.size(); i++) {
                    q[i] = tmp_q[i];
                    impact_sum[i] = tmp_impact_sum[i];
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
            int in1 = problem.musicians[m1];
            int in2 = problem.musicians[m2];
            if (in1 == in2) continue;
            double next_score = current_score;
            tmp_q[m1] = tmp_q[m2] = 1;
            for (int musician : instrument[in1]) {
                if (musician == m1) continue;
                double new_dist = 1 / dist(placements[m2], placements[musician]);
                tmp_q[musician] = q[musician] - 1 / dist(placements[m1], placements[musician]) + new_dist;
                tmp_q[m1] += new_dist;
                next_score -= ceil(VOLUME * q[musician] * max(impact_sum[musician], 0.0));
                next_score += ceil(VOLUME * tmp_q[musician] * max(impact_sum[musician], 0.0));
            }
            for (int musician : instrument[in2]) {
                if (musician == m2) continue;
                double new_dist = 1 / dist(placements[m1], placements[musician]);
                tmp_q[musician] = q[musician] - 1 / dist(placements[m2], placements[musician]) + new_dist;
                tmp_q[m2] += new_dist;
                next_score -= ceil(VOLUME * q[musician] * max(impact_sum[musician], 0.0));
                next_score += ceil(VOLUME * tmp_q[musician] * max(impact_sum[musician], 0.0));
            }
            double is1 = 0, is2 = 0;
            next_score -= ceil(VOLUME * q[m1] * max(impact_sum[m1], 0.0));
            next_score -= ceil(VOLUME * q[m2] * max(impact_sum[m2], 0.0));
            for (int i = 0; i < problem.attendees.size(); i++) {
                if (blocked_count[m1][i] == 0) is2 += calc_one_score(placements[m1], problem.attendees[i].pos, problem.attendees[i].tastes[in2]);
                if (blocked_count[m2][i] == 0) is1 += calc_one_score(placements[m2], problem.attendees[i].pos, problem.attendees[i].tastes[in1]);
            }
            next_score += ceil(VOLUME * tmp_q[m1] * max(is1, 0.0));
            next_score += ceil(VOLUME * tmp_q[m2] * max(is2, 0.0));
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
                for (int musician : instrument[in1]) q[musician] = tmp_q[musician];
                for (int musician : instrument[in2]) q[musician] = tmp_q[musician];
                impact_sum[m1] = is1;
                impact_sum[m2] = is2;
                if (current_score > best_score) {
                    best_score = current_score;
                    save_best_state();
                    unchanged = 0;
                }
            }
        }
    }
    
    placements = best_placements;
    best_score = score_all_exact();
    
    output(best_placements, volumes);
    
    fprintf(stderr, "best_score : %lf\n", best_score);
    
    return 0;
}
