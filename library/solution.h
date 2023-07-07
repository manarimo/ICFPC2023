#ifndef ICFPC2023_SOLUTION_H
#define ICFPC2023_SOLUTION_H

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

    struct solution_t {
        vector<P> placements;
        solution_t() {}
        solution_t(const vector<P>& placements) : placements(placements) {}
    };

    void from_json(const json& j, P& p) {
        j.at("x").get_to(p.first);
        j.at("y").get_to(p.second);
    }

    void to_json(json& j, const P& p) {
        j = json { {"x", p.first}, {"y", p.second} };
    }

    void from_json(const json& j, solution_t& s) {
        j.at("placements").get_to(s.placements);
    }

    void to_json(json& j, const solution_t& s) {
        j["placements"] = s.placements;
    }

    void load_solution(const string &filename, solution_t &out) {
        ifstream f(filename);
        return load_solution(filename, out);
    }

    // problem_t が大きすぎて json::get<> で読むとスタックオーバーフローするので、出力先は引数で取る
    void load_solution(istream &f, solution_t &out) {
        json j;
        f >> j;
        from_json(j, out);

        // out.init();
    }

    void print_solution(ostream &f, const solution_t &val) {
        json j(val);
        f << j;
    }
};

#endif //ICFPC2023_SOLUTION_H