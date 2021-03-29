#ifndef DEBUG_PRINT_HPP
#define DEBUG_PRINT_HPP

#include <bits/stdc++.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

using namespace std;
using fmt::print, fmt::format;

// *****

#define DV(a) ("["s + #a + "=" + to_string(a) + "]")

struct nesting_writer {
    FILE* f;
    int width, depth;

    explicit nesting_writer(FILE* f = stdout, int width = 4, int depth = 0)
        : f(f), width(width), depth(depth) {}

    template <typename... Ts>
    void print(string_view format_str, Ts&&... args) const {
        fmt::print(f, "{:2}| {:{}}", depth, "", depth * width);
        fmt::print(f, format_str, forward<Ts>(args)...);
        fmt::print("\n");
    }
    template <typename T>
    auto& operator<<(T&& arg) const {
        return print("{}", forward<T>(arg)), *this;
    }
};

template <typename... Args>
void print_indented(int indent, string_view format_str, Args&&... args) {
    print("{:{}}", "", indent);
    print(format_str, std::forward<Args>(args)...);
}

namespace std {

template <typename U, typename V>
string to_string(const pair<U, V>& uv) {
    return '(' + to_string(uv.first) + ',' + to_string(uv.second) + ')';
}
template <typename U, typename V>
ostream& operator<<(ostream& out, const pair<U, V>& v) {
    return out << to_string(v);
}

template <typename T>
string to_string(const array<T, 2>& uv) {
    return '(' + to_string(uv[0]) + ',' + to_string(uv[1]) + ')';
}
template <typename T, size_t N>
ostream& operator<<(ostream& out, const array<T, N>& v) {
    return out << to_string(v);
}

template <typename T, size_t N>
string to_string(const array<T, N>& v) {
    string s;
    for (const auto& el : v)
        s += to_string(el) + " ";
    return s.pop_back(), s;
}

template <typename T>
string to_string(const vector<T>& v) {
    string s;
    for (const auto& el : v)
        s += to_string(el) + " ";
    return s.pop_back(), s;
}
template <typename T>
ostream& operator<<(ostream& out, const vector<T>& v) {
    return out << to_string(v);
}

template <typename T>
string to_string(const list<T>& v) {
    string s;
    for (const auto& el : v)
        s += to_string(el) + " ";
    return s.pop_back(), s;
}
template <typename T>
ostream& operator<<(ostream& out, const list<T>& v) {
    return out << to_string(v);
}

template <typename T>
string to_string(const set<T>& v) {
    string s;
    for (const auto& el : v)
        s += to_string(el) + " ";
    return s.pop_back(), s;
}
template <typename T>
ostream& operator<<(ostream& out, const set<T>& v) {
    return out << to_string(v);
}

template <typename T>
string to_string(const unordered_set<T>& v) {
    string s;
    for (const auto& el : v)
        s += to_string(el) + " ";
    return s.pop_back(), s;
}
template <typename T>
ostream& operator<<(ostream& out, const unordered_set<T>& v) {
    return out << to_string(v);
}

template <typename K, typename V>
string to_string(const map<K, V>& v) {
    string s;
    for (const auto& el : v)
        s += to_string(el) + " ";
    return s.pop_back(), s;
}
template <typename K, typename V>
ostream& operator<<(ostream& out, const map<K, V>& v) {
    return out << to_string(v);
}

template <typename K, typename V>
string to_string(const unordered_map<K, V>& v) {
    string s;
    for (const auto& el : v)
        s += to_string(el) + " ";
    return s.pop_back(), s;
}
template <typename K, typename V>
ostream& operator<<(ostream& out, const unordered_map<K, V>& v) {
    return out << to_string(v);
}

} // namespace std

#endif // DEBUG_PRINT_HPP
