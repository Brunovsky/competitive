#include "test_utils.hpp"
#include "../numeric/primes.hpp"
#include "../numeric/sieves.hpp"
#include "../numeric/modnum.hpp"
#include "../random.hpp"

inline namespace speed_testing_sieves {

void speed_test_sieves() {
    for (int N : {31'600, 100'000, 316'000, 1'000'000, 3'160'000, 10'000'000, 31'600'000,
                  100'000'000}) {
        print(" speed test sieves N={}\n", N);

        START(classic);
        classic_sieve(N);
        TIME(classic);
        PRINT_TIME_MS(classic);

        START(least_prime);
        least_prime_sieve(N);
        TIME(least_prime);
        PRINT_TIME_MS(least_prime);

        START(num_prime_divisors);
        num_prime_divisors_sieve(N);
        TIME(num_prime_divisors);
        PRINT_TIME_MS(num_prime_divisors);

        START(num_divisors);
        num_divisors_sieve(N);
        TIME(num_divisors);
        PRINT_TIME_MS(num_divisors);

        START(sum_divisors);
        sum_divisors_sieve(N);
        TIME(sum_divisors);
        PRINT_TIME_MS(sum_divisors);

        START(phi);
        phi_sieve(N);
        TIME(phi);
        PRINT_TIME_MS(phi);

        START(modinv);
        modinv_sieve(N, 1'000'000'007);
        TIME(modinv);
        PRINT_TIME_MS(modinv);

        START(logfac);
        logfac_sieve(N);
        TIME(logfac);
        PRINT_TIME_MS(logfac);

        START(modnum_1000000007);
        pascal_sieve<modnum<1'000'000'007>>(N);
        TIME(modnum_1000000007);
        PRINT_TIME_MS(modnum_1000000007);

        START(modnum_998244353);
        pascal_sieve<modnum<998'244'353>>(N);
        TIME(modnum_998244353);
        PRINT_TIME_MS(modnum_998244353);
    }
}

} // namespace speed_testing_sieves

inline namespace unit_testing_sieves {

void unit_test_classic_sieve() {
    auto primes = classic_sieve(100'000);
    assert(primes.size() == 9592u);

    assert(count_primes(10, 20, primes) == 4);
    assert(count_primes(100, 200, primes) == 21);
    assert(count_primes(1, 9, primes) == 4);
    // wolfram: 1e6th prime is 15485863, 2e6th prime is 32452843
    assert(count_primes(15485863, 32452843, primes) == 1'000'001);
    // wolfram: 1.00e7th prime is 179424673, 1.05e7th prime is 188943803
    assert(count_primes(179424674, 188943803, primes) == 500'000);
}

void unit_test_sieves() {
    constexpr int N = 100, M = 21;

    auto primes = classic_sieve(N);
    auto least = least_prime_sieve(N);
    auto tau_primes = num_prime_divisors_sieve(N);
    auto tau = num_divisors_sieve(N);
    auto sigma = sum_divisors_sieve(N);
    auto phi = phi_sieve(N);
    auto modinv1e9 = modinv_sieve(N, 23);

    int ans[][M] = {
        {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73},
        {0, 0, 2, 3, 2, 5, 2, 7, 2, 3, 2, 11, 2, 13, 2, 3, 2, 17, 2, 19, 2},
        {0, 0, 1, 1, 1, 1, 2, 1, 1, 1, 2, 1, 2, 1, 2, 2, 1, 1, 2, 1, 2},
        {0, 1, 2, 2, 3, 2, 4, 2, 4, 3, 4, 2, 6, 2, 4, 4, 5, 2, 6, 2, 6},
        {0, 1, 3, 4, 7, 6, 12, 8, 15, 13, 18, 12, 28, 14, 24, 24, 31, 18, 39, 20, 42},
        {0, 1, 1, 2, 2, 4, 2, 6, 4, 6, 4, 10, 4, 12, 6, 8, 8, 16, 6, 18, 8},
        {0, 1, 12, 8, 6, 14, 4, 10, 3, 18, 7, 21, 2, 16, 5, 20, 13, 19, 9, 17, 15},
    };

    for (int n = 0; n < M; n++) {
        assert(primes[n] == ans[0][n]);
        assert(least[n] == ans[1][n]);
        assert(tau_primes[n] == ans[2][n]);
        assert(tau[n] == ans[3][n]);
        assert(sigma[n] == ans[4][n]);
        assert(phi[n] == ans[5][n]);
        assert(modinv1e9[n] == ans[6][n]);
    }
}

void unit_test_num_divisors_sieve() {
    constexpr int N = 1'000'000;

    auto lp = least_prime_sieve(N);
    auto divs = num_divisors_sieve(N);

    for (int n = 2; n <= N; n++) {
        int m = n, actual = 1;
        while (m > 1) {
            int i = 0, f = lp[m];
            do {
                m /= f, i++;
            } while (lp[m] == f);
            actual *= i + 1;
        }
        assert(actual == divs[n]);
    }
}

} // namespace unit_testing_sieves

inline namespace stress_testing_primes {

void stress_test_jacobi() {
    for (long n = 1; n < 300; n += 2) {
        for (long m = 1; m < 300; m += 2) {
            if (gcd(n, m) == 1) {
                int reciprocity = ((n % 4) == 3 && (m % 4) == 3) ? -1 : 1;
                assert(jacobi(n, m) * jacobi(m, n) == reciprocity);
            }
        }
    }
}

void stress_test_miller_rabin() {
    static const char* what[2] = {"composite", "prime"};
    constexpr long N = 4'000'000;

    auto primes = classic_sieve(N);

    vector<bool> small_prime(N + 1, false);
    for (int p : primes) {
        small_prime[p] = true;
    }
    for (long n = 1; n <= N; n++) {
        if (small_prime[n] != miller_rabin(n)) {
            print("miller_rabin failed for n={}\n", n);
            print("expected: {}\n", what[small_prime[n]]);
            print("     got: {}\n", what[miller_rabin(n)]);
        }
        assert(small_prime[n] == miller_rabin(n));
    }
    print("small miller_rabin OK\n");

    for (int v : {5, 20, 300, 1000}) {
        long L = N * (N - v), R = N * (N - v + 5);
        auto large_primes = get_primes(L, R, primes);
        vector<bool> large_prime(R - L + 1, false);
        for (long n : large_primes) {
            large_prime[n - L] = true;
        }
        for (long n = L; n <= N; n++) {
            if (large_primes[n - L] != miller_rabin(n)) {
                print("miller_rabin failed for n={}\n", n);
                print("expected: {}\n", what[small_prime[n - L]]);
                print("     got: {}\n", what[miller_rabin(n)]);
            }
            assert(large_prime[n - L] == miller_rabin(n));
        }
        print("large miller_rabin {}..{} OK\n", L, R);
    }
}

} // namespace stress_testing_primes

int main() {
    RUN_SHORT(unit_test_sieves());
    RUN_SHORT(unit_test_num_divisors_sieve());
    RUN_BLOCK(stress_test_jacobi());
    RUN_BLOCK(stress_test_miller_rabin());
    RUN_BLOCK(speed_test_sieves());
    return 0;
}
