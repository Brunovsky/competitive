#include "../simplex.hpp"

using namespace std;

// *****

void test() {
    simplex smp(2);
    smp.set_objective({4, 3});
    smp.add_constraint({6, 2}, 120);
    smp.add_constraint({1, 4}, 100);
    smp.add_constraint({5, 5}, 150);
    auto optimum = smp.compute();
    cout << "optimum #1: " << optimum << endl;
    assert(optimum == 105L);

    smp = simplex(3);
    smp.set_objective({-1, 2, 1});
    smp.add_constraint({2, 1, -1}, 2);
    smp.add_constraint({2, -1, 5}, 6);
    smp.add_constraint({4, 1, 1}, 6);
    optimum = smp.compute();
    cout << "optimum #2: " << optimum << endl;
    assert(optimum == 10L);

    smp = simplex(4);
    smp.set_objective({10, -32, 8, 5});
    smp.add_constraint({4, 2, -5, 9}, 5);
    smp.add_constraint({1, -1, 2, -1}, 3);
    smp.add_constraint({frac(5, 2), -5, -3, -2}, 0);
    optimum = smp.compute();
    cout << "optimum #3: " << optimum << endl;
    assert(optimum == frac(381, 13));

    smp = simplex(4);
    smp.set_objective({7, -19, 31, 11});
    smp.add_constraint({4, 2, 8, -7}, 8);
    smp.add_constraint({1, -1, 2, -1}, 3);
    smp.add_constraint({3, 3, -1, 0}, 7);
    smp.add_constraint({0, -4, 2, -3}, 8);
    smp.add_constraint({-2, 7, -4, 9}, 13);
    optimum = smp.compute();
    cout << "optimum #4: " << optimum << endl;
    assert(optimum == frac(829, 7));

    smp.add_constraint({4, 3, 2, 1}, 6);
    optimum = smp.compute();
    cout << "optimum #5: " << optimum << endl;
    assert(optimum == frac(345, 4));

    cout << smp.tab;
}

int main() {
    test();
    return 0;
}
