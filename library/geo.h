#ifndef ICFPC2021_GEO_H
#define ICFPC2021_GEO_H

#include <vector>
#include <cmath>

#define X first
#define Y second

namespace geo {
    using namespace std;
    using number = double;
    using P = pair<number, number>;

    number dot(const P& p, const P& q) {
        return p.X * q.X + p.Y * q.Y;
    }

    number cross(const P& p, const P& q) {
        return p.X * q.Y - p.Y * q.X;
    }

    number ccw (const P& p, const P& q, const P& r) {
        return (q.X - p.X) * (r.Y - p.Y) - (q.Y - p.Y) * (r.X - p.X);
    }

    bool is_on_segment(const P& p1, const P& p2, const P& p) {
        number area = (p1.X - p.X) * (p2.Y - p.Y) - (p1.Y - p.Y) * (p2.X - p.X);
        if (area == 0 && (p1.X - p.X) * (p2.X - p.X) <= 0 && (p1.Y - p.Y) * (p2.Y - p.Y) <= 0) return true;
        return false;
    }

    number d(const P& p, const P& q) {
        return (p.X - q.X) * (p.X - q.X) + (p.Y - q.Y) * (p.Y - q.Y);
    }

    double dist_point(double px1, double py1, double px2, double py2) {
        return (px2 - px1) * (px2 - px1) + (py2 - py1) * (py2 - py1);
    }

    double get_ratio(const P& l1, const P& l2, const P& p) {
        double vx = l2.X - l1.X;
        double vy = l2.Y - l1.Y;
        double wx = p.X - l1.X;
        double wy = p.Y - l1.Y;
        return (double)(vx * wx + vy * wy) / (vx * vx + vy * vy);
    }

    double dist_line(const P& l1, const P& l2, const P& p) {
        double t = get_ratio(l1, l2, p);
        if (t < 0) t = 0;
        if (t > 1) t = 1;
        return dist_point(l1.X + (l2.X - l1.X) * t, l1.Y + (l2.Y - l1.Y) * t, p.X, p.Y);
    }

    P reflection(const P& p1, const P& p2, const P& p) {
        double t = get_ratio(p1, p2, p);
        double x = p1.X + t * (p2.X - p1.X) * 2 - p.X;
        double y = p1.Y + t * (p2.Y - p1.Y) * 2 - p.Y;
        return make_pair(round(x), round(y));
    }

    bool is_in_convex(const vector<P>& convex, const P& p) {
        vector<number> ccws;
        for (int i = 0; i < (int) convex.size(); i++) {
            int j = (i + 1) % convex.size();
            ccws.push_back(ccw(convex[i], convex[j], p));
        }
        const number max = *max_element(ccws.begin(), ccws.end());
        const number min = *min_element(ccws.begin(), ccws.end());

        return max <= 0 || min >= 0;
    }
};
#endif //ICFPC2021_GEO_H