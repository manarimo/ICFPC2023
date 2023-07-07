#ifndef ICFPC2023_SCORING_H
#define ICFPC2023_SCORING_H

#include "problem.h"
#include <vector>
#include <set>
#include <algorithm>
#include <complex>

namespace manarimo {
    using namespace std;
    using namespace geo;

    using cP = complex<number>;

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

    number normalize_angle(number angle) {
        // returns angle in [-pi, +pi]
        while (angle < -M_PI) {
            angle += 2 * M_PI;
        }
        while (angle > +M_PI) {
            angle -= 2 * M_PI;
        }
        return angle;
    }

    long long score(const problem_t& problem, const vector<P>& placements) {
        const int n_musician = problem.musicians.size();
        const int n_attendee = problem.attendees.size();
        
        vector<pair<int, int>> unblocked_pairs;

        for (int i_musician = 0; i_musician < n_musician; i_musician++) {
            const cP center = {placements[i_musician].first, placements[i_musician].second};

            struct event {
                int type;  // 0: block start, 1: attendee, 2: block end
                int index;  // musician id or attendee id
            };
            vector<event> event_infos;
            vector<pair<number, int>> events;
            // add event type=1
            for (int i_attendee = 0; i_attendee < n_attendee; i_attendee++) {
                const auto& attendee = problem.attendees[i_attendee];
                const cP attendee_position = {attendee.x, attendee.y};
                const number argument = arg(attendee_position - center);
                events.emplace_back(argument, event_infos.size());
                event_infos.push_back({1, i_attendee});
            }

            // add event type=0, 2
            // assuming that musician distance is at least 10 (> 5)
            int overlapping_spans = 0;
            for (int j_musician = 0; j_musician < n_musician; j_musician++) {
                if (j_musician == i_musician) {
                    continue;
                }
                const number block_distance = 5;
                const cP location = {placements[j_musician].first, placements[j_musician].second};
                const cP vec = location - center;
                const number distance = abs(vec);
                const number vec_argument = arg(vec);
                const number offset = asin(block_distance / distance);

                const number start = normalize_angle(vec_argument - offset);
                const number end = normalize_angle(vec_argument + offset);
                if (start > end) {
                    overlapping_spans += 1;
                }
                events.emplace_back(start, event_infos.size());
                event_infos.push_back({0, j_musician});
                events.emplace_back(end, event_infos.size());
                event_infos.push_back({2, j_musician});
            }

            sort(events.begin(), events.end());
            for (const auto event: events) {
                const auto event_info = event_infos[event.second];
                switch (event_info.type) {
                    case 0:
                        overlapping_spans += 1;
                        break;
                    case 1:
                        if (overlapping_spans == 0) {
                            unblocked_pairs.emplace_back(i_musician, event_info.index);
                        }
                        break;
                    case 2:
                        overlapping_spans -= 1;
                        break;
                    default:
                        throw -1;
                } 
            }
        }

        long long score = 0;
        for (auto unblocked_pair : unblocked_pairs) {
            const int i_musician = unblocked_pair.first;
            const int i_attendee = unblocked_pair.second;
            const P attendee_location = {problem.attendees[i_attendee].x, problem.attendees[i_attendee].y};
            score += ceil(1000000 * problem.attendees[i_attendee].tastes[problem.musicians[i_musician]] / d(attendee_location, placements[i_musician]));
        }
        return score;
    }
};

#endif //ICFPC2023_SCORING_H