#ifndef ICFPC2023_SCORING_H
#define ICFPC2023_SCORING_H

#include "problem.h"
#include <vector>
#include <set>

namespace manarimo {
    using namespace std;
    using namespace geo;

    bool validate(const problem_t& problem, const vector<P>& placements) {
        const number min_distance = 10;

        const int n_musician = problem.musicians.size();
        for (int i = 0; i < n_musician; i++) {
            for (int j = i + 1; j < n_musician; j++) {
                if (d(placements[i], placements[j]) < min_distance * min_distance) {
                    return false;
                }
            }

            if (placements[i].first < problem.stage_bottom_left.first) {
                return false;
            }
            if (placements[i].second < problem.stage_bottom_left.second) {
                return false;
            }
            if (placements[i].first > problem.stage_bottom_left.first + problem.stage_width) {
                return false;
            }
            if (placements[i].second > problem.stage_bottom_left.second + problem.stage_height) {
                return false;
            }
        }

        return true;
    }

    number score(const problem_t& problem, const vector<P>& placements) {
        if (!validate(problem, placements)) {
            return 0;
        }

        const int n_musician = problem.musicians.size();
        const int n_attendee = problem.attendees.size();
        const number block_distance = 5;
        
        set<pair<int, int>> blocked_pairs;

        for (int i_musician = 0; i_musician < n_musician; i_musician++) {
            
        }

        number score = 0;
        for (int i_attendee = 0; i_attendee < n_attendee; i_attendee++) {
            const P attendee_location = {problem.attendees[i_attendee].x, problem.attendees[i_attendee].y};
            for (int i_musician = 0; i_musician < n_musician; i_musician++) {
                if (blocked_pairs.find({i_musician, i_attendee}) != blocked_pairs.end()) {
                    continue;
                }
                score += 1000000 * problem.attendees[i_attendee].tastes[problem.musicians[i_musician]] / d(attendee_location, placements[i_musician]);
            }
        }
        return score;
    }
};

#endif //ICFPC2023_SCORING_H