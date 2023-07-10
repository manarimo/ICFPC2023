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

manarimo::problem_t problem;
double stage_left, stage_right, stage_bottom, stage_top;

const int MAX_MUSICIAN = 1500;
const int MAX_ATTENDEE = 5000;
const double RADIUS = 10;
const double RADIUS2 = RADIUS * RADIUS;
const double BLOCK_RADIUS = 5;
vector<pair<double, int>> attendee_angles[MAX_MUSICIAN];
vector<int> blocked_attendees[MAX_MUSICIAN][MAX_MUSICIAN];
int blocked_count[MAX_MUSICIAN][MAX_ATTENDEE];


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
    
    for (int i = 0; i < current_placements.size(); i++) blocked_attendees[i].clear();
    for (int i = 0; i < problem.attendees.size(); i++) blocked_count[i] = 0;
    for (int i = 0; i < current_placements.size(); i++) {
        if (i == musician) continue;
        double angle = get_angle(p, current_placements[i]);
        double offset = asin(BLOCK_RADIUS / sqrt(geo::d(p, current_placements[i])));
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

void calc_blocked(const vector<geo::P>& placements) {
    for (int i = 0; i < placements.size(); i++) calc_blocked_one(i, placements[i], attendee_angles[i], blocked_attendees[i], blocked_count[i], placements);
}

double calc_one_score(const geo::P& p1, const geo::P& p2, double taste) {
    return ceil(1000000 * taste / geo::d(p1, p2));
}

double score_all(const vector<geo::P>& placements) {
    calc_blocked(placements);
    double sum = 0;
    for (int i = 0; i < placements.size(); i++) {
        for (int j = 0; j < problem.attendees.size(); j++) {
            if (blocked_count[i][j] == 0) sum += calc_one_score(placements[i], problem.attendees[j].pos, problem.attendees[j].tastes[problem.musicians[i]]);
        }
    }
    return sum;
}

// g++ -std=c++2a -O3 kawatea_random.cpp
// ./a.out 1.x.json < ../problems/1.json > 1.json
int main(int argc, char *argv[]) {
    manarimo::load_problem(cin, problem);
    stage_left = problem.stage_bottom_left.first + 10;
    stage_right = problem.stage_bottom_left.first + problem.stage_width - 10;
    stage_bottom = problem.stage_bottom_left.second + 10;
    stage_top = problem.stage_bottom_left.second + problem.stage_height - 10;
    
    vector<double> max_tastes;
    for (int i = 0; i < problem.attendees.size(); i++) {
        double mx = 0;
        for (double t : problem.attendees[i].tastes) {
            mx = max(mx, t);
        }
        max_tastes.push_back(mx);
    }

    set<geo::P> occupied;
    vector<geo::P> placements;

    for (int i = 0; i < problem.musicians.size(); i++) {
        cerr << i << endl;
        double mx_score = -1e9;
        geo::P mx_p;
        for (double x = stage_left; x <= stage_right; x++) {
            for (double y = stage_bottom; y <= stage_top; y++) {
                geo::P p(x, y);
                double sc = 0;
                if (occupied.count(p)) continue;
                calc_blocked_one(i, p, attendee_angles[i], blocked_attendees[i], blocked_count[i], placements);
                for (int j = 0; j < problem.attendees.size(); j++) if (blocked_count[i][j] == 0) sc += max_tastes[j] / geo::d(p, problem.attendees[j].pos);
                if (sc > mx_score) {
                    mx_score = sc;
                    mx_p = p;
                }
            }
        }
        placements.push_back(mx_p);
        for (double x = mx_p.first - RADIUS; x <= mx_p.first + RADIUS; x++) {
            for (double y = mx_p.second - RADIUS; y <= mx_p.second + RADIUS; y++) {
                geo::P p(x, y);
                if (geo::d(p, mx_p) < RADIUS2) {
                    occupied.insert(p);
                }
            }
        }
    }
    
    vector<double> volumes = vector<double>();
    for (int i = 0; i < problem.musicians.size(); i++) volumes.push_back(10);
    manarimo::print_solution(cout, manarimo::solution_t(placements, volumes));
    
    return 0;
}
