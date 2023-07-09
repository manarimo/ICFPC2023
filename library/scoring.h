#ifndef ICFPC2023_SCORING_H
#define ICFPC2023_SCORING_H

#include "problem.h"
#include <vector>
#include <set>
#include <algorithm>
#include <complex>
#include <cmath>
#include <iostream>
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

            if (placements[i].first < problem.stage_bottom_left.first + min_distance) {
                return false;
            }
            if (placements[i].second < problem.stage_bottom_left.second + min_distance) {
                return false;
            }
            if (placements[i].first > problem.stage_bottom_left.first + problem.stage_width - min_distance) {
                return false;
            }
            if (placements[i].second > problem.stage_bottom_left.second + problem.stage_height - min_distance) {
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

    bool is_pillar_effective(const atendee_t& attendee, const P& stage_bottom_left, const number stage_width, const number stage_height, const pillar_t& pillar) {
        // maybe a good idea to cache result
        vector<P> stage_corners;
        stage_corners.push_back(stage_bottom_left);
        stage_corners.push_back({stage_bottom_left.first, stage_bottom_left.second + stage_height});
        stage_corners.push_back({stage_bottom_left.first + stage_width, stage_bottom_left.second});
        stage_corners.push_back({stage_bottom_left.first + stage_width, stage_bottom_left.second + stage_height});

        for (const auto& corner : stage_corners) {
            if (dist_line(attendee.pos, corner, pillar.center) < pillar.radius * pillar.radius) {
                return true;
            }
        }

        if (is_in_convex({attendee.pos, stage_corners[0], stage_corners[3]}, pillar.center)) {
            return true;
        }
        if (is_in_convex({attendee.pos, stage_corners[1], stage_corners[2]}, pillar.center)) {
            return true;
        }
        return false;
    }

    vector<int> get_unblocked_musician_of_attendee(const problem_t& problem, const vector<P>& placements, const int attendee_id) {
        // only considers musician-pillar-attendee blocking.
        const int n_musician = problem.musicians.size();
        const int n_pillar = problem.pillars.size();

        vector<int> unblocked_musicians;
        const cP center = {problem.attendees[attendee_id].x, problem.attendees[attendee_id].y};

        struct event {
            int type;  // 0: block end, 1: musician, 2: block start
            int index;  // musician id or pillar id
        };
        vector<event> event_infos;
        vector<pair<number, int>> events;
        // add event type=1
        for (int i_musician = 0; i_musician < n_musician; i_musician++) {
            const cP musician_position = {placements[i_musician].first, placements[i_musician].second};
            const number argument = arg(musician_position - center);
            events.emplace_back(argument, event_infos.size());
            event_infos.push_back({1, i_musician});
        }

        // add event type=0, 2
        int overlapping_spans = 0;
        for (int i_pillar = 0; i_pillar < n_pillar; i_pillar++) {
            if (!is_pillar_effective(
                problem.attendees[attendee_id], 
                {problem.stage_bottom_left.first, problem.stage_bottom_left.second}, 
                problem.stage_width, 
                problem.stage_height, 
                problem.pillars[i_pillar]
            )) {
                continue;
            }

            const number pillar_radius = problem.pillars[i_pillar].radius;
            const cP location = {problem.pillars[i_pillar].center.first, problem.pillars[i_pillar].center.second};
            const cP vec = location - center;
            const number distance = abs(vec);
            if (distance < pillar_radius) {
                return {};
            }
            const number vec_argument = arg(vec);
            const number offset = asin(pillar_radius / distance);

            const number start = normalize_angle(vec_argument - offset);
            const number end = normalize_angle(vec_argument + offset);
            if (start > end) {
                overlapping_spans += 1;
            }
            events.emplace_back(start, event_infos.size());
            event_infos.push_back({2, i_pillar});
            events.emplace_back(end, event_infos.size());
            event_infos.push_back({0, i_pillar});
        }

        sort(events.begin(), events.end());
        for (const auto& event: events) {
            const auto event_info = event_infos[event.second];
            switch (event_info.type) {
                case 0:
                    overlapping_spans -= 1;
                    break;
                case 1:
                    if (overlapping_spans == 0) {
                        unblocked_musicians.push_back(event_info.index);
                    }
                    break;
                case 2:
                    overlapping_spans += 1;
                    break;
                default:
                    throw -1;
            } 
        }
        return unblocked_musicians;
    }

    vector<int> get_unblocked_attendees_of_musician(const problem_t& problem, const vector<P>& placements, const int musician_id) {
        // only considers musician-musician-attendee blocking.
        const int n_musician = problem.musicians.size();
        const int n_attendee = problem.attendees.size();

        vector<int> unblocked_attendees;
        const cP center = {placements[musician_id].first, placements[musician_id].second};

        struct event {
            int type;  // 0: block end, 1: attendee, 2: block start
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
            if (j_musician == musician_id) {
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
            event_infos.push_back({2, j_musician});
            events.emplace_back(end, event_infos.size());
            event_infos.push_back({0, j_musician});
        }

        sort(events.begin(), events.end());
        for (const auto& event: events) {
            const auto event_info = event_infos[event.second];
            switch (event_info.type) {
                case 0:
                    overlapping_spans -= 1;
                    break;
                case 1:
                    if (overlapping_spans <= 0) {
                        unblocked_attendees.push_back(event_info.index);
                    }
                    break;
                case 2:
                    overlapping_spans += 1;
                    break;
                default:
                    throw -1;
            } 
        }
        return unblocked_attendees;
    }
    
    vector<pair<int, int>> get_unblocked_pairs(const problem_t& problem, const vector<P>& placements) {
        const int n_musician = problem.musicians.size();
        const int n_attendee = problem.attendees.size();
        
        vector<pair<int, int>> musician_unblocked_pairs;
        for (int i_musician = 0; i_musician < n_musician; i_musician++) {
            for (auto i_attendee: get_unblocked_attendees_of_musician(
                problem, placements, i_musician
            )) {
                musician_unblocked_pairs.emplace_back(i_musician, i_attendee);
            }
        }

        if (problem.pillars.empty()) {
            return musician_unblocked_pairs;
        }

        vector<pair<int, int>> pillar_unblocked_pairs;
        for (int i_attendee = 0; i_attendee < n_attendee; i_attendee++) {
            for (auto i_musician: get_unblocked_musician_of_attendee(
                problem, placements, i_attendee
            )) {
                pillar_unblocked_pairs.emplace_back(i_musician, i_attendee);
            }
        }
        
        sort(musician_unblocked_pairs.begin(), musician_unblocked_pairs.end());
        sort(pillar_unblocked_pairs.begin(), pillar_unblocked_pairs.end());

        vector<pair<int, int>> unblocked_pairs;
        set_intersection(
            musician_unblocked_pairs.begin(), musician_unblocked_pairs.end(),
            pillar_unblocked_pairs.begin(), pillar_unblocked_pairs.end(),
            std::inserter(unblocked_pairs, unblocked_pairs.end())
        );

        return unblocked_pairs;
    }

    long long score(const problem_t& problem, const solution_t& solution) {
        const auto& placements = solution.as_p();
        map<int, vector<int>> instrument_groups;
        for (int musician_id = 0; musician_id < (int) problem.musicians.size(); musician_id++) {
            instrument_groups[problem.musicians[musician_id]].push_back(musician_id);
        }
        vector<number> closeness(problem.musicians.size(), 1);
        if (problem.playing_together) {
            for (const auto& group : instrument_groups) {
                const auto& musician_ids = group.second;
                for (int j = 0; j < (int) musician_ids.size(); j++) {
                    for (int i = 0; i < j; i++) {
                        const number distance = sqrt(d(placements[musician_ids[i]], placements[musician_ids[j]]));
                        closeness[musician_ids[i]] += 1. / distance;
                        closeness[musician_ids[j]] += 1. / distance;
                    }
                }
            }
        }

        vector<pair<int, int>> unblocked_pairs = get_unblocked_pairs(problem, placements);

        long long score = 0;
        for (auto unblocked_pair : unblocked_pairs) {
            const int i_musician = unblocked_pair.first;
            const int i_attendee = unblocked_pair.second;
            const P attendee_location = {problem.attendees[i_attendee].x, problem.attendees[i_attendee].y};
            score += ceil(solution.volumes[i_musician] * closeness[i_musician] * ceil(1000000 * problem.attendees[i_attendee].tastes[problem.musicians[i_musician]] / d(attendee_location, placements[i_musician])));
        }
        return score;
    }
};

#endif //ICFPC2023_SCORING_H