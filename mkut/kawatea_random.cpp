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
    inline bool accept(double current_score, double next_score, int type);
    inline void reject(int type);
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
const double MAIN_TIME_LIMIT = 30;
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
vector<pair<double, int>> tmp_attendee_angles[MAX_MUSICIAN];
vector<int> tmp_blocked_attendees[MAX_MUSICIAN][MAX_MUSICIAN];
int tmp_blocked_count[MAX_MUSICIAN][MAX_ATTENDEE];
vector<geo::P> best_placements;

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

void output(const vector<geo::P>& placements) {
    manarimo::print_solution(std::cout, manarimo::solution_t(placements));
}

void store_to_file(const vector<geo::P>& placements, const string file_name) {
    std::ofstream ofs(file_name);

    manarimo::print_solution(ofs, manarimo::solution_t(placements));
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

void calc_blocked_one(int musician, const geo::P& p, vector<pair<double, int>>& attendee_angles, vector<int>* blocked_attendees, int* blocked_count, const vector<geo::P>& current_placements) {
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
        double angle = get_angle(p, current_placements[i]);
        double offset = asin(BLOCK_RADIUS / dist(p, current_placements[i]));
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
    for (int i = 0; i < problem.musicians.size(); i++) calc_blocked_one(i, placements[i], attendee_angles[i], blocked_attendees[i], blocked_count[i], placements);
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
    calc_blocked();
}

bool validate(const vector<geo::P>& ps) {
    for (auto p : ps) {
        if (p.first < stage_left || p.first > stage_right || p.second < stage_bottom || p.second > stage_top) {
            cerr << "OUT OF STAGE" << endl;
            return false;
        }
    }
    for (int i = 0; i < ps.size(); i++) {
        for (int j = i + 1; j < ps.size(); j++) {
            if (dist2(ps[i], ps[j]) < RADIUS2) {
                cerr << "TOO NEAR" << endl;
                return false;
            }
        }
    }
    return true;
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

enum Move {
    MOVE,
    SWAP,
    THREE_SWAP,
    BEST_SWAP,
};

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

set<int> move(vector<geo::P>& placements, int idx, double dx, double dy) {
    set<int> moved;
    map<int, geo::P> to_move;
    to_move[idx] = geo::P(dx, dy);

    for (int i = 0; i < 1000 && to_move.size() > 0; i++) {
        if (i == 999) {
            cerr << "Chaos move!" << endl;
            return set<int>();
        }
        map<int, geo::P> next_to_move;
        for (auto iter = to_move.begin(); iter != to_move.end(); ++iter) {
            int m = iter->first;
            placements[m].first += iter->second.first;
            placements[m].second += iter->second.second;
            if (placements[m].first < stage_left) { return set<int>(); }
            if (placements[m].first > stage_right) { return set<int>(); }
            if (placements[m].second < stage_bottom) { return set<int>(); }
            if (placements[m].second > stage_top) { return set<int>(); }
            moved.insert(m);
        }
        for (auto iter = to_move.begin(); iter != to_move.end(); ++iter) {
            int m = iter->first;
            for (int i = 0; i < placements.size(); i++) {
                if (i == m) continue;
                if (dist2(placements[i], placements[m]) < RADIUS2) {
                    double d = dist(placements[i], placements[m]);
                    double dx2, dy2;
                    if (d < 1e-15) {
                        dx2 = 5;
                        dy2 = 5;
                    } else {
                        dx2 = placements[i].first - placements[m].first;
                        dy2 = placements[i].second - placements[m].second;
                        dx2 *= RADIUS / d;
                        dy2 *= RADIUS / d;
                    }
                    next_to_move[i] = geo::P(next_to_move[i].first + dx2, next_to_move[i].second + dy2);
                }
            }
        }
        to_move = next_to_move;
    }
    return moved;
}

// g++ -std=c++2a -O3 kawatea_random.cpp
// ./a.out 1.x.json < ../problems/1.json > 1.json
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

    vector<double> max_diff = {1, 10, 50, 100};
    for (int times = 0; times < 1000; ++times) {
        max_diff_width = max_diff_height = max_diff[times % max_diff.size()];
        fprintf(stderr, "max diff width: %lf, max diff height: %lf \n", max_diff_width, max_diff_height);
        double current_score = best_score;
        
        int unchanged = 0;
        vector<vector<pair<int, int>>> new_blocked(MAX_MUSICIAN);
        simulated_annealing sa(MAIN_TIME_LIMIT);
        while (!sa.end()) {
            unchanged++;
            if (unchanged == 10000) {
                current_score = best_score;
                load_best_state();
                unchanged = 0;
            }
            
            if (random::get(100) < 80) {
                int _m = random::get(problem.musicians.size());
                geo::P& _current_p = placements[_m];
                double r = random::get_double(0.2, max_diff_width);
                double theta = random::get_double(0, 2 * M_PI);
                double dx = clamp(r * cos(theta), stage_left - _current_p.X, stage_right - _current_p.X);
                double dy = clamp(r * sin(theta), stage_bottom - _current_p.Y, stage_top - _current_p.Y);
                vector<geo::P> next_placements = placements;
                set<int> moved = move(next_placements, _m, dx, dy);
                if (moved.size() == 0) continue;
                //if (moved.size() != 1) continue;
                double next_score = current_score;
                
                // 稼いでいたスコアが消える
                for (int i = 0; i < problem.attendees.size(); i++) {
                    for (int m : moved) {
                        if (blocked_count[m][i] == 0) next_score -= calc_one_score(placements[m], problem.attendees[i].pos, problem.attendees[i].tastes[problem.musicians[m]]);
                    }
                }
                
                // ブロックが消えることによるスコア
                for (int i = 0; i < problem.musicians.size(); i++) {
                    if (moved.count(i)) continue;
                    for (int m : moved) {
                        for (int j : blocked_attendees[i][m]) {
                            blocked_count[i][j]--;
                            if (blocked_count[i][j] == 0) next_score += calc_one_score(placements[i], problem.attendees[j].pos, problem.attendees[j].tastes[problem.musicians[i]]);
                        }
                    }
                }
                
                // 移動した演奏家がブロックされる聴衆を計算 & スコア計算
                for (int m : moved) {
                    calc_blocked_one(m, next_placements[m], tmp_attendee_angles[m], tmp_blocked_attendees[m], tmp_blocked_count[m], next_placements);
                    for (int i = 0; i < problem.attendees.size(); i++) {
                        if (tmp_blocked_count[m][i] == 0) next_score += calc_one_score(next_placements[m], problem.attendees[i].pos, problem.attendees[i].tastes[problem.musicians[m]]);
                    }
                }
                
                // 移動先でブロックする聴衆を計算
                set<pair<int,int>> new_blocked2;
                for (int m : moved) {
                    new_blocked[m].clear();
                    for (int i = 0; i < problem.musicians.size(); i++) {
                        if (moved.count(i)) continue;
                        double angle = get_angle(placements[i], next_placements[m]);
                        double offset = asin(BLOCK_RADIUS / dist(placements[i], next_placements[m]));
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
                            new_blocked[m].emplace_back(i, attendee);
                            if (blocked_count[i][attendee] == 0 && new_blocked2.count(make_pair(i, attendee)) == 0) next_score -= calc_one_score(placements[i], problem.attendees[attendee].pos, problem.attendees[attendee].tastes[problem.musicians[i]]);
                            new_blocked2.insert(make_pair(i, attendee));
                        }
                    }
                }
                if (sa.accept(current_score, next_score, MOVE)) {
                    current_score = next_score;
                    for (int m : moved) {
                        placements[m] = next_placements[m];
                        swap(attendee_angles[m], tmp_attendee_angles[m]);
                        for (int i = 0; i < problem.musicians.size(); i++) swap(blocked_attendees[m][i], tmp_blocked_attendees[m][i]);
                        for (int i = 0; i < problem.attendees.size(); i++) blocked_count[m][i] = tmp_blocked_count[m][i];
                        for (int i = 0; i < problem.musicians.size(); i++) {
                            if (moved.count(i)) continue;
                            blocked_attendees[i][m].clear();
                        }
                        for (const pair<int, int>& p : new_blocked[m]) {
                            blocked_attendees[p.first][m].push_back(p.second);
                            blocked_count[p.first][p.second]++;
                        }
                    }
                    if (current_score > best_score) {
                        best_score = current_score;
                        save_best_state();
                        unchanged = 0;
                    }
                } else {
                    for (int m : moved) {
                        for (int i = 0; i < problem.musicians.size(); i++) {
                            if (moved.count(i)) continue;
                            for (int j : blocked_attendees[i][m]) blocked_count[i][j]++;
                        }
                    }
                }
            } else if (random::get(100) < 95) {
                if (random::get(100) < 80) {
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
                    for (int i = 0; i < problem.attendees.size(); i++) {
                        if (blocked_count[m1][i] == 0 || blocked_count[m2][i] == 0 || blocked_count[m3][i] == 0) {
                            geo::P p = problem.attendees[i].pos;
                            if (blocked_count[m1][i] == 0) {
                                next_score -= calc_one_score(placements[m1], p, problem.attendees[i].tastes[problem.musicians[m1]]);
                                next_score += calc_one_score(placements[m1], p, problem.attendees[i].tastes[problem.musicians[m3]]);
                            }
                            if (blocked_count[m2][i] == 0) {
                                next_score -= calc_one_score(placements[m2], p, problem.attendees[i].tastes[problem.musicians[m2]]);
                                next_score += calc_one_score(placements[m2], p, problem.attendees[i].tastes[problem.musicians[m1]]);
                            }
                            if (blocked_count[m3][i] == 0) {
                                next_score -= calc_one_score(placements[m3], p, problem.attendees[i].tastes[problem.musicians[m3]]);
                                next_score += calc_one_score(placements[m3], p, problem.attendees[i].tastes[problem.musicians[m2]]);
                            }
                        }
                    }
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
                int cand = m1;

                for (int m2 = 0; m2 < problem.musicians.size(); ++m2) {
                    if (m1 == m2) continue;
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
                    if (next_score > next_score_cand) {
                        next_score_cand = next_score;
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
        placements = best_placements;
        if (!validate(placements)) {
            fprintf(stderr, "INVALID!!!!!!\n");
        }
        fprintf(stderr, "true score : %lf\n", score_all());
        fprintf(stderr, "storing to %s\n", (file_name + "-" + to_string(times) + ".json").c_str());

        store_to_file(best_placements, file_name + "-" + to_string(times) + ".json");
        sa.print();
    }

    output(best_placements);
    
    return 0;
}
