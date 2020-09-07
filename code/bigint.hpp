#ifndef BIGINT_HPP
#define BIGINT_HPP

#include <bits/stdc++.h>

#include <boost/multiprecision/cpp_int.hpp>

using namespace std;

// *****

static_assert(0xffffffff == UINT_MAX);
static_assert(sizeof(uint) == 4 && sizeof(long) == 8, "Unexpected integer sizes");

#define isnum(c) ('0' <= c && c <= '9')

struct bigint {
    vector<uint> nums;
    bool sign = 0; // 0=positive, 1=negative

    bigint() = default;
    bigint(int n) : nums(n != 0, abs(n)), sign(n < 0) {}
    bigint(uint n, bool s = 0) : nums(n > 0, n), sign(s) {}
    bigint(const string& s, uint b = 10);

    auto& operator[](uint x) { return nums[x]; }
    const auto& operator[](uint x) const { return nums[x]; }
    bool bit(uint x) const { return nums[x / 32] & (1 << (x % 32)); }
    int len() const { return nums.size(); }
    bool zero() const { return nums.empty(); }
    void clear() { nums.clear(), sign = 0; }
    void flip() { sign = !sign; }
    void trim() {
        while (!zero() && nums.back() == 0)
            nums.pop_back();
        sign = sign && !zero();
    }
};

string bigdigits(const bigint& u) {
    stringstream ss;
    ss << (u.sign ? "-[" : "[");
    int n = u.len();
    for (int i = 0; i + 1 < n; i++)
        ss << setw(11) << u[i] << ",";
    if (n)
        ss << setw(11) << u[n - 1];
    ss << "]";
    return ss.str();
}

string lsbits(const bigint& u) {
    if (u.zero())
        return "0";
    string s(32 * u.len() + 1, '0');
    s[0] = u.sign ? '-' : '+';
    for (int i = 0; i < 32 * u.len(); i++)
        s[i + 1] = '0' + u.bit(i);
    while (!s.empty() && s.back() == '0')
        s.pop_back();
    return s;
}
string msbits(const bigint& u) {
    if (u.zero())
        return "0";
    string s(32 * u.len() + 1, '0');
    s[0] = u.sign ? '-' : '+';
    for (int i = 0; i < 32 * u.len(); i++)
        s[32 * u.len() - i] = '0' + u.bit(i);
    s.erase(begin(s) + 1, find(begin(s) + 1, end(s), '1'));
    return s;
}

ostream& operator<<(ostream& out, const bigint& u) { return out << lsbits(u); }

bool magnitude_cmp(const bigint& u, const bigint& v) {
    int L = u.nums.size(), R = v.nums.size();
    if (L != R)
        return L < R;
    else
        return lexicographical_compare(rbegin(u.nums), rend(u.nums), //
                                       rbegin(v.nums), rend(v.nums));
}

bool operator<(const bigint& u, const bigint& v) {
    if (u.sign != v.sign)
        return u.sign;
    else if (u.sign)
        return magnitude_cmp(v, u);
    else
        return magnitude_cmp(u, v);
}
bool operator>(const bigint& u, const bigint& v) { return v < u; }
bool operator<=(const bigint& u, const bigint& v) { return !(u > v); }
bool operator>=(const bigint& u, const bigint& v) { return !(u < v); }
bool operator==(const bigint& u, const bigint& v) {
    return u.sign == v.sign && u.nums == v.nums;
}
bool operator!=(const bigint& u, const bigint& v) { return !(u == v); }

bigint& operator>>=(bigint& u, uint shift) {
    int s = shift / 32, n = u.len();
    uint lo = shift % 32, hi = 32 - lo;

    if (s >= n) {
        u.clear();
    } else if (lo > 0) {
        for (int i = 0; i < n - s - 1; i++)
            u[i] = (u[i + s] >> lo) | (u[i + s + 1] << hi);
        u[n - s - 1] = u[n - 1] >> lo;
        u.nums.resize(n - s);
        u.trim();
    } else {
        u.nums.erase(begin(u.nums), begin(u.nums) + s);
    }

    return u;
}

bigint& operator<<=(bigint& u, uint shift) {
    int s = shift / 32, n = u.len();
    uint hi = shift % 32, lo = 32 - hi;

    if (hi > 0) {
        u.nums.resize(n + s + 1, 0);
        for (int i = n + s; i > s; i--)
            u[i] = (u[i - s - 1] >> lo) | (u[i - s] << hi);
        u[s] = u[0] << hi;
        for (int i = s - 1; i >= 0; i--)
            u[i] = 0;
        u.trim();
    } else {
        u.nums.insert(begin(u.nums), s, 0);
    }

    return u;
}

bigint operator>>(bigint u, uint shift) { return u >>= shift; }
bigint operator<<(bigint u, uint shift) { return u <<= shift; }

void add_int(bigint& u, uint v) {
    for (int i = 0; v && i < u.len(); i++) {
        bool carry = u[i] > UINT_MAX - v;
        u[i] += v;
        v = carry;
    }
    if (v > 0)
        u.nums.push_back(v);
}

void sub_int(bigint& u, uint v) {
    if (v == 0)
        return;
    if (u.zero()) {
        u.nums = {v}, u.sign = 1;
        return;
    }
    if (u.len() == 1 && u[0] < v) {
        u.nums = {v - u[0]}, u.sign = !u.sign;
        return;
    }
    for (int i = 0; v && i < u.len(); i++) {
        long sum = long(u[i]) - v;
        u[i] = sum + UINT_MAX + 1;
        v = sum < 0;
    }
    u.trim();
}

void mul_int(bigint& u, uint v) {
    if (v == 0) {
        u.clear();
        return;
    }
    if (v == 1) {
        return;
    }
    ulong sum = 0;
    for (int i = 0; i < u.len(); i++) {
        sum += ulong(u[i]) * v;
        u[i] = sum & UINT_MAX;
        sum >>= 32;
    }
    if (sum > 0)
        u.nums.push_back(sum);
}

uint div_int(bigint& u, uint v) {
    constexpr ulong b = 1UL + UINT_MAX;
    assert(v > 0);
    if (v == 1 || u.zero())
        return 0;
    uint r = 0;
    for (int i = u.len() - 1; i >= 0; i--) {
        ulong p = r * b + u[i];
        u[i] = p / v, r = p % v;
    }
    u.trim();
    return r;
}

/**
 * Big integer operations
 */

void add_vec(bigint& u, const bigint& v) {
    int n = u.len(), m = v.len(), hi = max(n, m), lo = min(n, m);
    u.nums.resize(hi, 0);
    ulong k = 0;
    for (int i = 0; i < lo; i++) {
        k += u[i], k += v[i];
        u[i] = k, k = k > UINT_MAX;
    }
    for (int i = lo; i < m; i++) {
        k += v[i];
        u[i] = k, k = k > UINT_MAX;
    }
    for (int i = m; k && i < hi; i++) {
        u[i]++;
        k = u[i] == 0;
    }
    if (k)
        u.nums.push_back(k);
}

void sub_vec(bigint& u, const bigint& v) {
    int n = u.len(), m = v.len();
    assert(n >= m);
    long k = 0;
    for (int i = 0; i < m; i++) {
        long sum = u[i] - k - v[i];
        u[i] = sum + UINT_MAX + 1;
        k = sum < 0;
    }
    for (int i = m; k && i < n; i++) {
        k = u[i] == 0;
        u[i]--;
    }
    assert(k == 0);
    u.trim();
}

void rev_sub_vec(bigint& u, const bigint& v) {
    int n = u.len(), m = v.len();
    assert(n <= m);
    u.nums.resize(m, 0);
    long k = 0;
    for (int i = 0; i < n; i++) {
        long sum = v[i] - k - u[i];
        u[i] = sum + UINT_MAX + 1;
        k = sum < 0;
    }
    for (int i = n; i < m; i++) {
        u[i] = v[i];
    }
    for (int i = n; k && i < m; i++) {
        k = v[i] == 0;
        u[i]--;
    }
    assert(k == 0);
    u.trim();
}

void dyn_sub_vec(bigint& u, const bigint& v) {
    int n = u.len(), m = v.len();
    if (n > m) {
        return sub_vec(u, v);
    } else if (n < m) {
        u.flip();
        return rev_sub_vec(u, v);
    } else {
        int i = n - 1;
        while (i >= 0 && u[i] == v[i])
            u.nums.pop_back(), i--;
        if (i < 0) {
            u.sign = 0;
            return;
        }
        n = i + 1;
        long k = 0;
        if (u[i] > v[i]) {
            for (i = 0; i < n; i++) {
                long sum = u[i] - k - v[i];
                u[i] = sum + UINT_MAX + 1;
                k = sum < 0;
            }
        } else {
            for (i = 0; i < n; i++) {
                long sum = v[i] - k - u[i];
                u[i] = sum + UINT_MAX + 1;
                k = sum < 0;
            }
            u.flip();
        }
        u.trim();
        assert(k == 0);
    }
}

bigint mul_vec(const bigint& u, const bigint& v) {
    if (u.zero() || v.zero())
        return 0;
    int n = u.len(), m = v.len();
    bigint c;
    c.nums.resize(n + m, 0);
    c.sign = u.sign ^ v.sign;
    for (int j = 0; j < m; j++) {
        uint k = 0;
        for (int i = 0; i < n; i++) {
            ulong t = ulong(u[i]) * v[j] + c[i + j] + k;
            c[i + j] = t & UINT_MAX;
            k = t >> 32;
        }
        c[n + j] = k;
    }
    c.trim();
    return c;
}

bigint div_vec(bigint& u, bigint v) {
    constexpr ulong b = 1L + UINT_MAX;

    // return the remainder and set u to the quotient, but throughout the algorithm
    // u is the remainder and d is the quotient.
    int n = v.len(), m = u.len() - n;
    uint c = __builtin_clz(v[n - 1]);
    u <<= c, v <<= c;
    if (u.len() == n + m)
        u.nums.push_back(0);
    assert(u.len() == n + m + 1 && v.len() == n && v[n - 1] >= b / 2);

    bigint d;
    d.nums.resize(m + 1, 0);

    for (int j = m; j >= 0; j--) {
        ulong q = (u[n + j] * b + u[n - 1 + j]) / v[n - 1];
        ulong r = (u[n + j] * b + u[n - 1 + j]) % v[n - 1];

        while (r < b && q * v[n - 2] > r * b + u[n - 2 + j])
            q--, r += v[n - 1];

        long k = 0, t;
        for (int i = 0; i < n; i++) {
            ulong p = q * v[i];
            t = u[i + j] - (p & UINT_MAX) - k;
            u[i + j] = t;
            k = (p >> 32) - (t >> 32);
        }
        u[j + n] = t = u[j + n] - k;

        d[j] = q;
        if (t < 0) {
            assert(q > 0 && t == -1);
            d[j]--;
            k = 0;
            for (int i = 0; i < n; i++) {
                t = u[i + j] + k + v[i];
                u[i + j] = t;
                k = t > UINT_MAX;
            }
            u[j + n] = u[j + n] + k;
        }
        assert(u[j + n] == 0);
    }

    u.trim(), u >>= c, d.trim();
    swap(u, d);
    return d;
}

bigint div_mod(bigint& u, const bigint& v) {
    int su = u.sign, sv = v.sign;
    bigint r;
    if (magnitude_cmp(u, v)) {
        r = move(u);
        u.clear();
        return r;
    } else if (v.len() == 1) {
        r = bigint(div_int(u, v[0]));
    } else {
        r = div_vec(u, v);
    }
    r.sign = su && !r.zero();
    u.sign = su ^ sv;
    return r;
}

bigint& operator+=(bigint& u, const bigint& v) {
    u.sign == v.sign ? add_vec(u, v) : dyn_sub_vec(u, v);
    return u;
}
bigint& operator-=(bigint& u, const bigint& v) {
    u.sign != v.sign ? add_vec(u, v) : dyn_sub_vec(u, v);
    return u;
}
bigint& operator*=(bigint& u, const bigint& v) {
    u = mul_vec(u, v);
    return u;
}
bigint& operator/=(bigint& u, const bigint& v) {
    div_mod(u, v);
    return u;
}
bigint& operator%=(bigint& u, const bigint& v) {
    u = div_mod(u, v);
    return u;
}

bigint& operator+=(bigint& u, uint n) {
    u.sign == 0 ? add_int(u, n) : dyn_sub_vec(u, bigint(n));
    return u;
}
bigint& operator-=(bigint& u, uint n) {
    u.sign == 1 ? add_int(u, n) : dyn_sub_vec(u, bigint(n));
    return u;
}
bigint& operator*=(bigint& u, uint n) {
    mul_int(u, n);
    return u;
}
bigint& operator/=(bigint& u, uint n) {
    div_int(u, n);
    return u;
}
bigint& operator%=(bigint& u, uint n) {
    u = bigint(div_int(u, n), u.sign);
    return u;
}

bigint& operator+=(bigint& u, int n) {
    n >= 0 ? u += uint(n) : u -= uint(abs(n));
    return u;
}
bigint& operator-=(bigint& u, int n) {
    n > 0 ? u -= uint(n) : u += uint(abs(n));
    return u;
}
bigint& operator*=(bigint& u, int n) {
    mul_int(u, abs(n)), u.sign ^= n < 0;
    return u;
}
bigint& operator/=(bigint& u, int n) {
    div_int(u, abs(n)), u.sign ^= n < 0;
    return u;
}
bigint& operator%=(bigint& u, int n) {
    u = bigint(div_int(u, abs(n)), u.sign);
    return u;
}

bigint operator+(bigint u, const bigint& v) { return u += v; }
bigint operator+(bigint u, uint n) { return u += n; }
bigint operator+(bigint u, int n) { return u += n; }
bigint operator+(uint n, bigint u) { return u += n; }
bigint operator+(int n, bigint u) { return u += n; }

bigint operator-(bigint u, const bigint& v) { return u -= v; }
bigint operator-(bigint u, uint n) { return u -= n; }
bigint operator-(bigint u, int n) { return u -= n; }

bigint operator*(const bigint& u, const bigint& v) { return mul_vec(u, v); }
bigint operator*(bigint u, uint n) { return u *= n; }
bigint operator*(bigint u, int n) { return u *= n; }
bigint operator*(uint n, bigint u) { return u *= n; }
bigint operator*(int n, bigint u) { return u *= n; }

bigint operator/(bigint u, const bigint& v) { return u /= v; }
bigint operator/(bigint u, uint n) { return u /= n; }
bigint operator/(bigint u, int n) { return u /= n; }

bigint operator%(bigint u, const bigint& v) { return u %= v; }
bigint operator%(bigint u, uint n) { return u %= n; }
bigint operator%(bigint u, int n) { return u %= n; }

bigint operator-(bigint u) { return u.flip(), u; }

bigint& operator&=(bigint& u, const bigint& v) {
    int n = min(u.len(), v.len());
    u.nums.resize(n);
    for (int i = 0; i < n; i++)
        u[i] = u[i] & v[i];
    u.trim();
    return u;
}
bigint& operator|=(bigint& u, const bigint& v) {
    int n = max(u.len(), v.len());
    u.nums.resize(n);
    for (int i = 0; i < v.len(); i++)
        u[i] = u[i] | v[i];
    return u;
}
bigint& operator^=(bigint& u, const bigint& v) {
    int n = max(u.len(), v.len());
    u.nums.resize(n);
    for (int i = 0; i < v.len(); i++)
        u[i] = u[i] ^ v[i];
    u.trim();
    return u;
}
bigint operator~(bigint u) {
    for (int i = 0; i < u.len(); i++)
        u[i] = ~u[i];
    u.trim();
    return u;
}

bigint operator&(bigint u, const bigint& v) { return u &= v; }
bigint operator|(bigint u, const bigint& v) { return u |= v; }
bigint operator^(bigint u, const bigint& v) { return u ^= v; }

bigint::bigint(const string& s, uint b) {
    assert(2 <= b && b <= 10);
    int i = 0, S = s.size();
    while (i < S && s[i] != '+' && s[i] != '-' && !isnum(s[i])) {
        i++;
    }
    if (i == S) {
        return;
    }
    if (s[i] == '-') {
        sign = true;
    }
    if (!isnum(s[i])) {
        i++;
    }
    uint n = 0, tens = 1, threshold = UINT_MAX / (b + 1);
    while (i < S && isnum(s[i])) {
        n = b * n + (s[i++] - '0');
        tens *= b;
        if (n >= threshold) {
            mul_int(*this, tens);
            add_int(*this, n);
            n = 0;
            tens = 1;
        }
    }
    mul_int(*this, tens);
    add_int(*this, n);
}

#endif // BIGINT_HPP
