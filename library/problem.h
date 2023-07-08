#ifndef ICFPC2023_PROBLEM_H
#define ICFPC2023_PROBLEM_H

#include "json.hpp"
#include "geo.h"
#include <vector>
#include <string>
#include <fstream>

namespace manarimo {
    using namespace std;
    using namespace geo;
    using json = nlohmann::json;
    using number = double;

    struct P2 {
        number first;
        number second;
    };

    struct atendee_t {
        number x;
        number y;
        P pos;
        P get_pos() { return pos; }
        vector<number> tastes;
    };

    struct problem_t {
        // 問題定義に書いてある情報
        number room_width;
        number room_height;
        number stage_width;
        number stage_height;
        P2 stage_bottom_left;

        vector<int> musicians;
        vector<atendee_t> attendees;
    };

    void from_json(const json& j, P2& p) {
        j.at(0).get_to(p.first);
        j.at(1).get_to(p.second);
    }

    void from_json(const json& j, atendee_t& a) {
        j.at("x").get_to(a.x);
        j.at("y").get_to(a.y);
        j.at("tastes").get_to(a.tastes);
        a.pos = P(a.x, a.y);
    }

    void from_json(const json& j, problem_t& p) {
        j.at("room_width").get_to(p.room_width);
        j.at("room_height").get_to(p.room_height);
        j.at("stage_width").get_to(p.stage_width);
        j.at("stage_height").get_to(p.stage_height);
        j.at("stage_bottom_left").get_to(p.stage_bottom_left);
        j.at("musicians").get_to(p.musicians);
        j.at("attendees").get_to(p.attendees);
    }

    // problem_t が大きすぎて json::get<> で読むとスタックオーバーフローするので、出力先は引数で取る
    void load_problem(istream &f, problem_t &out) {
        json j;
        f >> j;
        from_json(j, out);

        // out.init();
    }

    void load_problem(const string &filename, problem_t &out) {
        ifstream f(filename);
        return load_problem(f, out);
    }
};

#endif //ICFPC2023_PROBLEM_H