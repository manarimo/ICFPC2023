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

    struct P3 {
        number x;
        number y;
    };

    struct solution_t {
        vector<P3> placements;
        vector<number> volumes;

        vector<P> as_p() const {
            vector<P> ret;
            for (P3 p : placements) {
                ret.push_back(P(p.x, p.y));
            }
            return ret;
        }
        solution_t() {}
        solution_t(const vector<P>& placements): solution_t(placements, vector<number>(placements.size(), 1.)) {}
        solution_t(const vector<P>& placements, const vector<number>& volumes): volumes(volumes) {
            for (auto p : placements) {
                P3 p3;
                p3.x = p.first;
                p3.y = p.second;
                this->placements.push_back(p3);
            }
        }
    };

    void to_json(json& j, const P3& p) {
        j = json { {"x", p.x}, {"y", p.y} };
    }

    void from_json(const json& j, P3& p) {
        j.at("x").get_to(p.x);
        j.at("y").get_to(p.y);
    }

    void from_json(const json& j, solution_t& s) {
        j.at("placements").get_to(s.placements);
        if (j.count("volumes")) {
            j.at("volumes").get_to(s.volumes);
        } else {
            s.volumes = vector<number>(s.placements.size(), 1.);
        }
    }

    void to_json(json& j, const solution_t& s) {
        j["placements"] = s.placements;
        j["volumes"] = s.volumes;
    }

    // problem_t が大きすぎて json::get<> で読むとスタックオーバーフローするので、出力先は引数で取る
    void load_solution(istream &f, solution_t &out) {
        json j;
        f >> j;
        from_json(j, out);

        // out.init();
    }

    void load_solution(const string &filename, solution_t &out) {
        ifstream f(filename);
        return load_solution(f, out);
    }

    void print_solution(ostream &f, const solution_t &val) {
        json j(val);
        f << j;
    }
};

#endif //ICFPC2023_SOLUTION_H