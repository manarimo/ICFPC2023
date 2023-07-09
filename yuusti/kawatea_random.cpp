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

enum Move {
    MOVE,
    SWAP,
    THREE_SWAP,
    BEST_SWAP,
    MOVE_EDGE,
    MOVE_GROUP,
};

// map<Move, string> move_name = {
//     {MOVE, "MOVE"},
//     {SWAP, "SWAP"},
//     {THREE_SWAP, "THREE_SWAP"},
//     {BEST_SWAP, "BEST_SWAP"},
//     {MOVE_EDGE, "MOVE_EDGE"},
//     {MOVE_GROUP, "MOVE_GROUP"}
// };

class simulated_annealing {
    public:
    simulated_annealing(double time_limit);
    inline bool end();
    inline bool accept(double current_score, double next_score, int type);
    inline void reject(int type);
    void print() const;
    
    private:
    constexpr static bool MAXIMIZE = true;
    constexpr static int LOG_SIZE = 0xFFFF;
    constexpr static int UPDATE_INTERVAL = 0xFF;
    constexpr static double START_TEMP = 1e4;
    constexpr static double END_TEMP = 1e-9;
    double time_limit;
    double temp_ratio;
    double log_probability[LOG_SIZE + 1];
    long long iteration = 0;
    long long accepted = 0;
    long long rejected = 0;
    map<int, long long> accepted_map;
    map<int, long long> rejected_map;
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

inline bool simulated_annealing::accept(double current_score, double next_score, int type) {
    double diff = (MAXIMIZE ? next_score - current_score : current_score - next_score);
    if (diff >= 0 || diff > log_probability[random::get_fast(LOG_SIZE)] * temp) {
        accepted++;
        accepted_map[type]++;

        return true;
    } else {
        rejected++;
        rejected_map[type]++;
        return false;
    }
}

inline void simulated_annealing::reject(int type) {
    rejected++;
    rejected_map[type]++;
}

void simulated_annealing::print() const {
    fprintf(stderr, "iteration: %lld\n", iteration);
    fprintf(stderr, "accepted: %lld\n", accepted);
    for (auto& e: accepted_map) {
        fprintf(stderr, "\taccepted-%d: %lld\n", e.first, e.second);        
    }
    fprintf(stderr, "rejected: %lld\n", rejected);
    for (auto& e: rejected_map) {
        fprintf(stderr, "\trejected-%d: %lld\n", e.first, e.second);        
    }
}

const double INIT_TIME_LIMIT = 20;
const double MAIN_TIME_LIMIT = 60;
const int MAX_MUSICIAN = 1500;
const int MAX_ATTENDEE = 5000;
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
vector<geo::P> placements;
vector<pair<double, int>> attendee_angles[MAX_MUSICIAN];
vector<int> blocked_attendees[MAX_MUSICIAN][MAX_MUSICIAN];
int blocked_count[MAX_MUSICIAN][MAX_ATTENDEE];
double impact_sum[MAX_MUSICIAN];
vector<pair<double, int>> tmp_attendee_angles;
vector<int> tmp_blocked_attendees[MAX_MUSICIAN];
int tmp_blocked_count[MAX_ATTENDEE];
double tmp_impact_sum[MAX_MUSICIAN];
vector<geo::P> best_placements;
vector<double> volumes;

void input() {
    manarimo::load_problem(std::cin, problem);
    
    stage_left = problem.stage_bottom_left.X;
    stage_right = stage_left + problem.stage_width;
    stage_left += RADIUS;
    stage_right -= RADIUS;
    
    stage_bottom = problem.stage_bottom_left.Y;
    stage_top = stage_bottom + problem.stage_height;
    stage_bottom += RADIUS;
    stage_top -= RADIUS;
}

void output(const vector<geo::P>& placements, const vector<double>& volumes) {
    manarimo::print_solution(std::cout, manarimo::solution_t(placements, volumes));
}


void store_to_file(const vector<geo::P>& placements, const vector<double>& volumes, const string file_name) {
    std::ofstream ofs(file_name);

    manarimo::print_solution(ofs, manarimo::solution_t(placements, volumes));
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

bool is_outside(double x, double y) {
    return x < stage_left + RADIUS || x > stage_right - RADIUS || y < stage_bottom + RADIUS || y > stage_top - RADIUS;
}

vector<geo::P> packed_place(double offset_x, double offset_y) {
    vector<geo::P> pos;
    int n = problem.musicians.size();
    int cnt = 0;
    while (cnt * cnt < n) ++cnt;

    for (int i = 0; i < cnt; ++i) {
        for (int j = 0; j < cnt; ++j) {
            double x = stage_left + offset_x + RADIUS * (i + 1);
            double y = stage_bottom + offset_y + RADIUS * (j + 1);

            if (is_outside(x, y)) {
                pos.clear();
                return pos;
            }

            pos.emplace_back(x, y);
            if (pos.size() == (size_t) n) return pos;
        }
    }

    assert(false);
}

void random_init2() {
    for (int i = 0; i < 100; ++i) {
        double ox = random::get_double(0, stage_right - stage_left);
        double oy = random::get_double(0, stage_top - stage_bottom);
        placements = packed_place(ox, oy);
        if (placements.empty()) continue;
        return;
    }
    // fail

    assert(false);
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
}

void calc_blocked() {
    for (int i = 0; i < problem.musicians.size(); i++) calc_blocked_one(i, placements[i], attendee_angles[i], blocked_attendees[i], blocked_count[i]);
}

double calc_one_score(const geo::P& p1, const geo::P& p2, double taste) {
    return ceil(1000000 * taste / dist2(p1, p2));
}

double score_all() {
    calc_blocked();
    volumes.clear();
    double sum = 0;
    for (int i = 0; i < problem.musicians.size(); i++) {
        impact_sum[i] = 0;
        for (int j = 0; j < problem.attendees.size(); j++) {
            if (blocked_count[i][j] == 0) impact_sum[i] += calc_one_score(placements[i], problem.attendees[j].pos, problem.attendees[j].tastes[problem.musicians[i]]);
        }
        if (impact_sum[i] >= 0) {
            sum += VOLUME * impact_sum[i];
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
    score_all();
}

void sa_no_block() {
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
            if (sa.accept(current_score, next_score, 0)) {
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
            if (sa.accept(current_score, next_score, 1)) {
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

string get_prob_name(string s) {
    int si = 0, ei = s.size();
    for (int i = s.size() - 1; i >= 0; --i) {
        if (s[i] == '.') ei = i;
        if (s[i] == '/') {
            si = i + 1;
            break;
        }
    }

    assert(si != s.size());

    return s.substr(si, ei - si);
}

geo::P edge_point() {
    int h = problem.stage_height, w = problem.stage_width;
    double r = random::get_double(RADIUS, h + w - 3 * RADIUS);
    int x, y;
    if (r < h - RADIUS) {
        y = problem.stage_bottom_left.second + r;
        if (random::get_fast(1) == 0) {
            // x = 0
            x = problem.stage_bottom_left.first + RADIUS;
        } else {
            // x = w;
            x = problem.stage_bottom_left.first + w - RADIUS;
        }
    } else {
        x = problem.stage_bottom_left.first + r - (h - RADIUS) + RADIUS;
        if (random::get_fast(1) == 0) {
            // y == 0;
            y = problem.stage_bottom_left.second + RADIUS;
        } else {
            // y == h;
            y = problem.stage_bottom_left.second + h - RADIUS;
        }
    }

    return {x, y};
}

// g++ -std=c++2a -O3 kawatea_random.cpp
// ./a.out ../solutions/sync-bests/1.json < ../problems/1.json > 1.json
int main(int argc, char *argv[]) {
    input();

    max_diff_width = 10.0;
    max_diff_height = 10.0;

    string file_name = "unknown";

    double best_score;
    if (argc < 2) {
        sa_no_block();
        placements = best_placements;
        best_score = score_all();
        fprintf(stderr, "no file loaded, initial score: %lf\n", best_score);
    } else {
        manarimo::solution_t intermediate_solution;
        manarimo::load_solution(string(argv[1]), intermediate_solution);
        file_name = get_prob_name(string(argv[1]));
        for (auto p : intermediate_solution.placements) {
            best_placements.push_back(make_pair(p.x, p.y));
        }
        placements = best_placements;
        best_score = score_all();
        fprintf(stderr, "score at load : %lf\n", best_score);
    }

    double init_best = best_score;

    double default_max_diff_width = problem.stage_width / 10;
    double default_max_diff_height = problem.stage_height / 10;
    double k = 1.0;
    double reduction = 0.999;
    for (int times = 0; times < 10000000; ++times) {
        fprintf(stderr, "default max diff width: %lf, default max diff height: %lf , reduction: %lf\n", default_max_diff_width, default_max_diff_height, reduction);
        double current_score = best_score;
        
        int unchanged = 0;
        vector<pair<int, int>> new_blocked;
        simulated_annealing sa(MAIN_TIME_LIMIT);
        while (!sa.end()) {
            max_diff_width = max(0.5, default_max_diff_width);
            max_diff_height = max(0.5, default_max_diff_height);
            k *= reduction;

            unchanged++;
            if (unchanged == 10000) {
                current_score = best_score;
                load_best_state();
                unchanged = 0;
            }

            if (random::get(100) < 80) {
                if (random::get(100) < 95) {
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
                    for (int i = 0; i < problem.musicians.size(); i++) tmp_impact_sum[i] = impact_sum[i];
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
                         if (tmp_blocked_count[i] == 0) tmp_impact_sum[m] += calc_one_score(next_p, problem.attendees[i].pos, problem.attendees[i].tastes[problem.musicians[m]]);
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
                        next_score += ceil(VOLUME * max(tmp_impact_sum[i], 0.0));
                    }
                    if (sa.accept(current_score, next_score, MOVE)) {
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
                    for (int i = 0; i < problem.musicians.size(); i++) impact_sum[i] = tmp_impact_sum[i];
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
                    int m = random::get(problem.musicians.size());
                    geo::P& current_p = placements[m];

                    geo::P next_p = edge_point();

                    bool ng = false;
                    for (int i = 0; i < placements.size(); i++) {
                        if (i == m) continue;
                        if (dist2(placements[i], next_p) < RADIUS2) {
                            ng = true;
                            break;
                        }
                    }
                    if (ng) continue;
                    for (int i = 0; i < problem.musicians.size(); i++) tmp_impact_sum[i] = impact_sum[i];
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
                         if (tmp_blocked_count[i] == 0) tmp_impact_sum[m] += calc_one_score(next_p, problem.attendees[i].pos, problem.attendees[i].tastes[problem.musicians[m]]);
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
                        next_score += ceil(VOLUME * max(tmp_impact_sum[i], 0.0));
                    }
                    if (sa.accept(current_score, next_score, MOVE_EDGE)) {
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
                        for (int i = 0; i < problem.musicians.size(); i++) impact_sum[i] = tmp_impact_sum[i];
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
                }
            } else if (random::get(100) < 95) {
                if (random::get(100) < 80) {
                    int m1 = random::get(problem.musicians.size());
                    int m2 = random::get(problem.musicians.size() - 1);
                    if (m2 >= m1) m2++;
                    if (problem.musicians[m1] == problem.musicians[m2]) continue;
                    double next_score = current_score;
                    double is1 = 0, is2 = 0;
                    next_score -= ceil(VOLUME * max(impact_sum[m1], 0.0));
                    next_score -= ceil(VOLUME * max(impact_sum[m2], 0.0));
                    for (int i = 0; i < problem.attendees.size(); i++) {
                        if (blocked_count[m1][i] == 0) is2 += calc_one_score(placements[m1], problem.attendees[i].pos, problem.attendees[i].tastes[problem.musicians[m2]]);
                        if (blocked_count[m2][i] == 0) is1 += calc_one_score(placements[m2], problem.attendees[i].pos, problem.attendees[i].tastes[problem.musicians[m1]]);
                    }
                    next_score += ceil(VOLUME * max(is1, 0.0));
                    next_score += ceil(VOLUME * max(is2, 0.0));
                    if (sa.accept(current_score, next_score, SWAP)) {
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
                        impact_sum[m1] = is1;
                        impact_sum[m2] = is2;
                        if (current_score > best_score) {
                            best_score = current_score;
                            save_best_state();
                            unchanged = 0;
                        }
                    }
                } else {
                    int m1 = random::get(problem.musicians.size());
                    int m2, m3;
                    do {
                        m2 =  random::get(problem.musicians.size());
                    } while (m2 == m1);
                    do {
                        m3 =  random::get(problem.musicians.size());
                    } while (m3 == m1 || m3 == m2);

                    double next_score = current_score;
                    double is1 = 0, is2 = 0, is3 = 0;
                    next_score -= ceil(VOLUME * max(impact_sum[m1], 0.0));
                    next_score -= ceil(VOLUME * max(impact_sum[m2], 0.0));
                    next_score -= ceil(VOLUME * max(impact_sum[m3], 0.0));
                    for (int i = 0; i < problem.attendees.size(); i++) {
                        if (blocked_count[m1][i] == 0) is3 += calc_one_score(placements[m1], problem.attendees[i].pos, problem.attendees[i].tastes[problem.musicians[m3]]);
                        if (blocked_count[m2][i] == 0) is1 += calc_one_score(placements[m2], problem.attendees[i].pos, problem.attendees[i].tastes[problem.musicians[m1]]);
                        if (blocked_count[m3][i] == 0) is2 += calc_one_score(placements[m3], problem.attendees[i].pos, problem.attendees[i].tastes[problem.musicians[m2]]);
                    }

                    next_score += ceil(VOLUME * max(is1, 0.0));
                    next_score += ceil(VOLUME * max(is2, 0.0));
                    next_score += ceil(VOLUME * max(is3, 0.0));

                    if (sa.accept(current_score, next_score, THREE_SWAP)) {
                        current_score = next_score;
                        swap(placements[m1], placements[m2]);
                        swap(placements[m2], placements[m3]);
                        attendee_angles[m1].swap(attendee_angles[m2]);
                        attendee_angles[m2].swap(attendee_angles[m3]);
                        for (int i = 0; i < problem.musicians.size(); i++) {
                            if (i == m1 || i == m2 || i == m3) continue;
                            blocked_attendees[i][m1].swap(blocked_attendees[i][m2]);
                            blocked_attendees[i][m2].swap(blocked_attendees[i][m3]);
                            blocked_attendees[m1][i].swap(blocked_attendees[m2][i]);
                            blocked_attendees[m2][i].swap(blocked_attendees[m3][i]);
                        }
                        // blocked_attendees[m1][m2].swap(blocked_attendees[m2][m1]);
                        blocked_attendees[m1][m2].swap(blocked_attendees[m2][m3]);
                        blocked_attendees[m2][m3].swap(blocked_attendees[m3][m1]);

                        blocked_attendees[m2][m1].swap(blocked_attendees[m3][m2]);
                        blocked_attendees[m3][m2].swap(blocked_attendees[m1][m3]);
                        for (int i = 0; i < problem.attendees.size(); i++) {
                            swap(blocked_count[m1][i], blocked_count[m2][i]);
                            swap(blocked_count[m2][i], blocked_count[m3][i]);
                        }
                        impact_sum[m1] = is1;
                        impact_sum[m2] = is2;
                        impact_sum[m3] = is3;
                        if (current_score > best_score) {
                            best_score = current_score;
                            save_best_state();
                            unchanged = 0;
                        }
                    }
                }
            } else {
                int m1 = random::get(problem.musicians.size());

                double next_score_cand = current_score;
                double best_is1, best_is2;
                int cand = m1;

                for (int m2 = 0; m2 < problem.musicians.size(); ++m2) {
                    if (m1 == m2) continue;
                    double next_score = current_score;
                    double is1 = 0, is2 = 0;
                    next_score -= ceil(VOLUME * max(impact_sum[m1], 0.0));
                    next_score -= ceil(VOLUME * max(impact_sum[m2], 0.0));
                    for (int i = 0; i < problem.attendees.size(); i++) {
                        if (blocked_count[m1][i] == 0) is2 += calc_one_score(placements[m1], problem.attendees[i].pos, problem.attendees[i].tastes[problem.musicians[m2]]);
                        if (blocked_count[m2][i] == 0) is1 += calc_one_score(placements[m2], problem.attendees[i].pos, problem.attendees[i].tastes[problem.musicians[m1]]);
                    }
                    next_score += ceil(VOLUME * max(is1, 0.0));
                    next_score += ceil(VOLUME * max(is2, 0.0));
                    
                    if (next_score > next_score_cand) {
                        next_score_cand = next_score;
                        best_is1 = is1;
                        best_is2 = is2;
                        cand = m2;
                    }
                }

                int m2 = cand;
                if (m1 == m2) {
                    sa.reject(BEST_SWAP);
                    continue;
                } 

                if (sa.accept(current_score, next_score_cand, BEST_SWAP)) {
                    current_score = next_score_cand;
                    swap(placements[m1], placements[m2]);
                    attendee_angles[m1].swap(attendee_angles[m2]);
                    for (int i = 0; i < problem.musicians.size(); i++) {
                        if (i == m1 || i == m2) continue;
                        blocked_attendees[i][m1].swap(blocked_attendees[i][m2]);
                        blocked_attendees[m1][i].swap(blocked_attendees[m2][i]);
                    }
                    blocked_attendees[m1][m2].swap(blocked_attendees[m2][m1]);
                    for (int i = 0; i < problem.attendees.size(); i++) swap(blocked_count[m1][i], blocked_count[m2][i]);
                    impact_sum[m1] = best_is1;
                    impact_sum[m2] = best_is2;
                    if (current_score > best_score) {
                        best_score = current_score;
                        save_best_state();
                        unchanged = 0;
                    }
                }
            }
        }
        fprintf(stderr, "best_score : %lf\n", best_score);
        fprintf(stderr, "best_score increase so far : %lf\n", best_score - init_best);
        fprintf(stderr, "true score : %lf\n", score_all());
        fprintf(stderr, "storing to %s\n", (file_name + "-" + to_string(times) + ".json").c_str());

        store_to_file(best_placements, volumes, file_name + "-" + to_string(times) + ".json");
        sa.print();
    }

    output(best_placements, volumes);
    
    return 0;
}
