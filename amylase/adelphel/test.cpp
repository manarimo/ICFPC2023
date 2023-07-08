#include <problem.h>
#include <solution.h>
#include <geo.h>
#include <scoring.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <complex>

using namespace std;
using namespace geo;
using cP = complex<number>;


// c++ -std=c++20 -I../../library test.cpp -O3
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "usage: " << argv[0] << " problem.json" << endl;
        return 0;
    }
    manarimo::problem_t problem;
    manarimo::load_problem(argv[1], problem);

    for (int attendee_id = 0; attendee_id < problem.attendees.size(); attendee_id++) {
        // only considers musician-pillar-attendee blocking.
        const int n_musician = problem.musicians.size();
        const int n_pillar = problem.pillars.size();

        const cP center = {problem.attendees[attendee_id].x, problem.attendees[attendee_id].y};

        struct event {
            int type;  // 0: block end, 2: block start, 4: stage start, 6: stage end
            int index;  // musician id or pillar id
        };
        vector<event> event_infos;
        vector<pair<number, int>> events;

        // add event type=0, 2
        int overlapping_spans = 0;
        bool is_stage_visible = false;
        for (int i_pillar = 0; i_pillar < n_pillar; i_pillar++) {
            if (!manarimo::is_pillar_effective(
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

            const number start = manarimo::normalize_angle(vec_argument - offset);
            const number end = manarimo::normalize_angle(vec_argument + offset);
            if (start > end) {
                overlapping_spans += 1;
            }
            events.emplace_back(start, event_infos.size());
            event_infos.push_back({2, i_pillar});
            events.emplace_back(end, event_infos.size());
            event_infos.push_back({0, i_pillar});
        }
        // event 4, 6
        vector<number> stage_args;
        stage_args.emplace_back(arg(cP(problem.stage_bottom_left.first, problem.stage_bottom_left.second) - center));
        stage_args.emplace_back(arg(cP(problem.stage_bottom_left.first + problem.stage_width, problem.stage_bottom_left.second) - center));
        stage_args.emplace_back(arg(cP(problem.stage_bottom_left.first, problem.stage_bottom_left.second + problem.stage_height) - center));
        stage_args.emplace_back(arg(cP(problem.stage_bottom_left.first + problem.stage_width, problem.stage_bottom_left.second + problem.stage_height) - center));
        sort(stage_args.begin(), stage_args.end());
        if (stage_args.back() - stage_args.front() < M_PI) {
            // no reverses.
            events.emplace_back(stage_args.front(), event_infos.size());
            event_infos.push_back({4, 0});
            events.emplace_back(stage_args.back(), event_infos.size());
            event_infos.push_back({6, 0});
        } else {
            // reverse!
            is_stage_visible = true;
            for (int i = 0; i < 4; i++) {
                if (stage_args[i] > 0) {
                    events.emplace_back(stage_args[i], event_infos.size());
                    event_infos.push_back({4, 0});
                    events.emplace_back(stage_args[(i + 3) % 4], event_infos.size());
                    event_infos.push_back({6, 0});                    
                    break;
                }
                if (i == 3) {
                    throw -1;
                }
            }
        }

        sort(events.begin(), events.end());
        events.push_back({-1, 0}); // dummy for final state check.
        bool removable = true;
        for (const auto& event: events) {
            if (is_stage_visible && overlapping_spans <= 0) {
                removable = false;
            }

            const auto event_info = event_infos[event.second];
            switch (event_info.type) {
                case 0:
                    overlapping_spans -= 1;
                    break;
                case 2:
                    overlapping_spans += 1;
                    break;
                case 4:
                    is_stage_visible = true;
                    break;
                case 6:
                    is_stage_visible = false;
                    break;
                case -1:
                    break;
                default:
                    throw -1;
            } 
        }
        
        if (removable) {
            cout << attendee_id << endl;
        }
        
    }
    return 0;
}