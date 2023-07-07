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

    struct atendee_t {
        number x;
        number y;
        P get_pos() { return P(x, y); }
        vector<number> tastes;
    };

    struct problem_t {
        // 問題定義に書いてある情報
        number room_width;
        number room_height;
        number stage_width;
        number stage_height;
        P stage_bottom_left;

        vector<int> musicians;
        vector<atendee_t> attendees;

        // 前計算。直接読んでもいいけど、外から書き換えると壊れるので注意
        /*
        P outer;
        bool inside[MAX_C][MAX_C];
        bool inside_double[MAX_C * 2][MAX_C * 2];
        double dist[MAX_C][MAX_C];
        int degree[MAX_P];
        number min_x = 1e18, min_y = 1e18, max_x = 0, max_y = 0;

        void init();
        bool is_point_inside(const P& point);
        bool is_edge_inside(const P& p1, const P& p2);
        number calc_dislike(const vector<P>& positions);
        
        void output(const vector<P>& vertices);
    private:
        // 前計算の初期化用にinit()内で使うための関数群
        bool is_point_inside(const vector<P>& hole, const P& point);
        double dist_hole_point(const vector<P>& hole, const P& point);
        */
    };

    void from_json(const json& j, P& p) {
        j.at(1).get_to(p.first);
        j.at(0).get_to(p.second);
    }

    void from_json(const json& j, atendee_t& a) {
        j.at("x").get_to(a.x);
        j.at("y").get_to(a.y);
        j.at("tastes").get_to(a.tastes);
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

    void load_problem(const string &filename, problem_t &out) {
        ifstream f(filename);
        return load_problem(filename, out);
    }

    // problem_t が大きすぎて json::get<> で読むとスタックオーバーフローするので、出力先は引数で取る
    void load_problem(istream &f, problem_t &out) {
        json j;
        f >> j;
        from_json(j, out);

        // out.init();
    }
};

#endif //ICFPC2023_PROBLEM_H