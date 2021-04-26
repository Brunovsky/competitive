#ifndef POINT3D_HPP
#define POINT3D_HPP

#include <bits/stdc++.h>

using namespace std;

// *****

/**
 * Class to represent points, lines, axis, circles etc in 3d using doubles.
 * Primary source: kth (https://github.com/kth-competitive-programming/kactl)
 *                 https://cp-algorithms.com/geometry/basic-geometry.html
 */
struct Point3d {
    double x, y, z;
    constexpr Point3d() : x(0), y(0), z(0) {}
    constexpr Point3d(double x, double y, double z) : x(x), y(y), z(z) {}
    constexpr Point3d(const array<double, 3>& a) : x(a[0]), y(a[1]), z(a[2]) {}

    using P = Point3d;
    static inline const double inf = numeric_limits<double>::infinity();
    static inline double deps = 1e-10; // default epsilon for comparisons etc (positive)

    static auto origin() { return P(0, 0, 0); }
    static auto pinf() { return P(inf, inf, inf); }

    friend bool same(const P& a, const P& b, double eps = deps) {
        return dist(a, b) <= eps;
    }
    bool operator==(const P& b) const { return same(*this, b); }
    bool operator!=(const P& b) const { return same(*this, b); }
    explicit operator bool() const noexcept { return *this != origin(); }

    double& operator[](int i) { return assert(0 <= i && i < 3), *(&x + i); }
    double operator[](int i) const { return assert(0 <= i && i < 3), *(&x + i); }
    P operator-() const { return P(-x, -y, -z); }
    P operator+(const P& b) const { return P(x + b.x, y + b.y, z + b.z); }
    P operator-(const P& b) const { return P(x - b.x, y - b.y, z - b.z); }
    friend P operator*(double k, const P& a) { return P(k * a.x, k * a.y, k * a.z); }
    P operator*(double k) const { return P(k * x, k * y, k * z); }
    P operator/(double k) const { return P(x / k, y / k, z / k); }
    friend P& operator+=(P& a, const P& b) { return a = a + b; }
    friend P& operator-=(P& a, const P& b) { return a = a - b; }
    friend P& operator*=(P& a, double k) { return a = a * k; }
    friend P& operator/=(P& a, double k) { return a = a / k; }

    P& normalize() { return *this /= norm(); }
    P normalized() const { return *this / norm(); }
    double norm() const { return dist(*this); }
    double norm2() const { return dist2(*this); }
    friend auto dot(const P& a, const P& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
    friend auto dot2(const P& a, const P& b) { return dot(a, b) * dot(a, b); }
    friend double dist(const P& a) { return sqrt(dist2(a)); }
    friend double dist(const P& a, const P& b) { return sqrt(dist2(a, b)); }
    friend double dist2(const P& a) { return dot(a, a); }
    friend double dist2(const P& a, const P& b) { return dist2(a - b); }
    double xcross(const P& a, const P& b) const { return xcrossed(a - *this, b - *this); }
    double ycross(const P& a, const P& b) const { return ycrossed(a - *this, b - *this); }
    double zcross(const P& a, const P& b) const { return zcrossed(a - *this, b - *this); }
    P cross(const P& a, const P& b) const { return crossed(a - *this, b - *this); }
    friend double xcrossed(const P& a, const P& b) { return a.y * b.z - a.z * b.y; }
    friend double ycrossed(const P& a, const P& b) { return a.z * b.x - a.x * b.z; }
    friend double zcrossed(const P& a, const P& b) { return a.x * b.y - a.y * b.x; }
    friend P crossed(const P& a, const P& b) {
        return P(xcrossed(a, b), ycrossed(a, b), zcrossed(a, b));
    }

    // Round to nearest lattice point
    array<long, 3> closest_lattice_point() const {
        return {long(round(x)), long(round(y)), long(round(z))};
    }

    // -- Lines

    // Are points a, b, c collinear in any order? (degenerate=yes)
    friend bool collinear(const P& a, const P& b, const P& c, double eps = deps) {
        return a.cross(b, c).norm() <= eps;
    }
    // Are point a, b, c collinear in this order? (degenerate=yes)
    friend bool onsegment(const P& a, const P& b, const P& c, double eps = deps) {
        return collinear(a, b, c, eps) && dot(a - b, c - b) <= 0; // <=0, not eps
    }
    // Are vectors u and v parallel? (either way)
    friend bool parallel(const P& u, const P& v, double eps = deps) {
        double n = u.norm() * v.norm();
        return abs(dot(u, v) - n) / n <= eps;
    }

    // k<0 => before a, k=0 => a, k=1 => b, k>1 => after b, 0<k<1 => in segment [ab]
    friend P interpolate(const P& a, const P& b, double k) { return (1 - k) * a + k * b; }
    // Distance of a to line uv
    friend double linedist(const P& a, const P& u, const P& v) {
        return a.cross(u, v).norm() / dist(u, v);
    }
    // Squared distance of a to line uv
    friend double linedist2(const P& a, const P& u, const P& v) {
        return a.cross(u, v).norm2() / dist2(u, v);
    }
    friend double cos(const P& u, const P& v) {
        return clamp(dot(u, v) / sqrt(u.norm2() * v.norm2()), -1.0, 1.0);
    }
    friend double sin(const P& u, const P& v) {
        return clamp(crossed(u, v).norm() / sqrt(u.norm2() * v.norm2()), -1.0, 1.0);
    }
    friend double cos2(const P& u, const P& v) {
        return clamp(dot2(u, v) / (u.norm2() * v.norm2()), 0.0, 1.0);
    }
    friend double sin2(const P& u, const P& v) {
        return 1.0 - cos2(u, v); // well...
    }

    // -- Planes

    // Are points a, b, c, d coplanar in any order?
    friend bool coplanar(const P& a, const P& b, const P& c, const P& d,
                         double eps = deps) {
        return parallel(a.cross(c, d), b.cross(c, d), eps);
    }
    // Is point P above (1), in (0) or below (-1) the plane by C with normal N?
    friend int planeside(const P& p, const P& c, const P& n, double eps = deps) {
        double s = dot(n, p - c);
        return (s >= eps) - (s <= -eps);
    }

    // -- Area

    // Area of triangle abc
    friend double area(const P& a, const P& b, const P& c) {
        return a.cross(b, c).norm() / 2;
    }

    bool boxed(const P& min, const P& max) const {
        return min.x <= x && x <= max.x && //
               min.y <= y && y <= max.y && //
               min.z <= z && z <= max.z;
    }

    friend string to_string(const P& a) {
        return '(' + to_string(a.x) + ',' + to_string(a.y) + ',' + to_string(a.z) + ')';
    }
    friend ostream& operator<<(ostream& out, const P& a) { return out << to_string(a); }
};

struct Plane {
    using P = Point3d;
    P n;      // normal; null if plane is degenerate (e.g. 3 collinear points)
    double d; // distance to origin; plane equation: dot(n,x) + d = 0

    Plane() = default;
    Plane(const P& n, double d) : n(n), d(d) {}
    Plane(const P& a, const P& b, const P& c) : n(a.cross(b, c)), d(-dot(n, a)) {}

    Plane& normalize() { return d /= n.norm(), n /= n.norm(), *this; }
    bool is_degenerate(double eps = Point3d::deps) const { return n.norm() <= eps; }

    // True if same plane and same orientation
    friend bool same_oriented(const Plane& a, const Plane& b,
                              double eps = Point3d::deps) {
        double da = a.d / a.n.norm(), db = b.d / b.n.norm();
        return abs(da - db) <= eps && same(a.n.normalized(), b.n.normalized(), eps);
    }
    // True if same plane and same orientation
    bool operator==(const Plane& b) const { return same_oriented(*this, b); }
    // True if different plane or different orientation
    bool operator!=(const Plane& b) const { return !(*this == b); }

    // Same plane as this one but opposite orientation
    Plane operator-() const { return Plane(-n, -d); }

    // Is point P above (1), in (0) or below (-1) this plane?
    int planeside(const P& p, double eps = Point3d::deps) const {
        double s = (dot(p, n) + d) / p.norm();
        return (s >= eps) - (s <= -eps);
    }

    double planedist(const P& p) const { return abs(dot(p, n) + d) / n.norm(); }
    double signed_planedist(const P& p) const { return (dot(p, n) + d) / n.norm(); }
};

#endif // POINT3D_HPP