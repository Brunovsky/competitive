#include "../general_matching.hpp"

// *****

constexpr int A = 10, B = 11;

void debug_header(string name) {
    dflash();
    print("{}{}{}\n", string(70, '~'), name, string(70, '~'));
    dflash();
}

/**
 *
 * [0]-----[1]~ ~ ~[2]-----[3]~ ~ ~[4]-----[5]~ ~ ~[6]-----[7]
 *
 * The augmenting path to be found is 0->1->2->3->4->5->6->7.
 */
void test_straight_line_1() {
    debug_header("test straight line 1");
    micali_vazirani vg(8);
    vg.add(0, 1); // 0
    vg.add(1, 2); // 1 ~
    vg.add(2, 3); // 2
    vg.add(3, 4); // 3 ~
    vg.add(4, 5); // 4
    vg.add(5, 6); // 5 ~
    vg.add(6, 7); // 6
    vg.mate[1] = vg.mate[2] = 1;
    vg.mate[3] = vg.mate[4] = 3;
    vg.mate[5] = vg.mate[6] = 5;
    int augmentation = vg.max_matching();
    cout << "AUG: " << augmentation << endl;
    assert(augmentation == 4);
}

/**
 *
 * [0]-----[1]~ ~ ~[2]-----[3]
 *  |                       |
 *  |                       |
 * [4]~ ~ ~[5]-----[6]~ ~ ~[7]
 *
 * The augmenting path to be found is 0->1->2->3.
 */
void test_straight_line_2() {
    debug_header("test straight line 2");
    micali_vazirani vg(8);
    vg.add(0, 1); // 0
    vg.add(1, 2); // 1 ~
    vg.add(2, 3); // 2
    vg.add(0, 4); // 3
    vg.add(4, 5); // 4 ~
    vg.add(5, 6); // 5
    vg.add(6, 7); // 6 ~
    vg.add(3, 7); // 7
    vg.mate[1] = vg.mate[2] = 1;
    vg.mate[4] = vg.mate[5] = 4;
    vg.mate[6] = vg.mate[7] = 6;
    int augmentation = vg.max_matching();
    cout << "AUG: " << augmentation << endl;
    assert(augmentation == 4);
}

/**
 *                 [3]             [7]
 *                 / ~             ~
 *                /   ~           ~
 *               /     ~         ~
 *     [1]~ ~ ~[2]     [4]     [6]
 *       \     /         \     /
 *        \   /           \   /
 *         \ /             \ /
 *         [0]             [5]
 *
 * The augmenting path to be found is 0->1->2->3->4->5.
 */
void test_immediate_blossom_1() {
    debug_header("immediate blossom 1");
    micali_vazirani vg(8);
    vg.add(0, 1); // 0
    vg.add(0, 2); // 1
    vg.add(1, 2); // 2 ~
    vg.add(2, 3); // 3
    vg.add(3, 4); // 4 ~
    vg.add(4, 5); // 5
    vg.add(5, 6); // 6
    vg.add(6, 7); // 7 ~
    vg.mate[1] = vg.mate[2] = 2;
    vg.mate[3] = vg.mate[4] = 4;
    vg.mate[6] = vg.mate[7] = 7;
    int augmentation = vg.max_matching();
    cout << "AUG: " << augmentation << endl;
    assert(augmentation == 4);
}

/**
 *
 *     [1]~ ~ ~[2]     [3]~ ~ ~[4]
 *       \       \     /       / \
 *        \_______\   /_______/   \
 *                \\ //            \
 *                 [0]             [5]
 *
 * The augmenting path to be found is 0->3->4->5.
 */
void test_immediate_blossom_2() {
    debug_header("immediate blossom 2");
    micali_vazirani vg(6);
    vg.add(0, 1); // 0
    vg.add(0, 2); // 1
    vg.add(0, 3); // 2
    vg.add(0, 4); // 3
    vg.add(1, 2); // 4 ~
    vg.add(3, 4); // 5 ~
    vg.add(4, 5); // 6
    vg.mate[1] = vg.mate[2] = 4;
    vg.mate[3] = vg.mate[4] = 5;
    int augmentation = vg.max_matching();
    cout << "AUG: " << augmentation << endl;
    assert(augmentation == 3);
}

/**
 *
 *     [1]~ ~ ~[2]
 *       \     / \
 *        \   /   \
 *         \ /     \
 *         [0]     [3]
 *
 * The augmenting path to be found is 0->1->2->3.
 */
void test_immediate_blossom_3() {
    debug_header("immediate blossom 3");
    micali_vazirani vg(4);
    vg.add(0, 1); // 0
    vg.add(0, 2); // 1
    vg.add(1, 2); // 2 ~
    vg.add(2, 3); // 3
    vg.mate[1] = vg.mate[2] = 2;
    int augmentation = vg.max_matching();
    cout << "AUG: " << augmentation << endl;
    assert(augmentation == 2);
}

/**
 *
 *           [5]~ ~ ~[6]-----[7]~ ~ ~[8]
 *           / \     /         \     /
 *          /   \   /           \   /
 *         /     \ /             \ /
 *       [B]     [3]-------------[4]
 *        ~        ~             ~
 *        ~         ~           ~
 *        ~          ~         ~
 *       [A]         [1]     [2]
 *        |            \     /
 *        |             \   /
 *        |              \ /
 *       [9]             [0]
 *
 * The augmenting path to be found is 0->1->3->6->5->9->A->B.
 */
void test_embedded_blossom_1() {
    debug_header("embedded blossom 1");
    micali_vazirani vg(12);
    vg.add(0, 1); // 0
    vg.add(0, 2); // 1
    vg.add(1, 3); // 2 ~
    vg.add(2, 4); // 3 ~
    vg.add(3, 5); // 4
    vg.add(3, 6); // 5
    vg.add(4, 7); // 6
    vg.add(4, 8); // 7
    vg.add(3, 4); // 8
    vg.add(5, 6); // 9 ~
    vg.add(6, 7); // 10
    vg.add(7, 8); // 11 ~
    vg.add(9, A); // 12
    vg.add(A, B); // 13 ~
    vg.add(B, 5); // 14
    vg.mate[1] = vg.mate[3] = 2;
    vg.mate[2] = vg.mate[4] = 3;
    vg.mate[5] = vg.mate[6] = 9;
    vg.mate[7] = vg.mate[8] = 11;
    vg.mate[A] = vg.mate[B] = 13;
    int augmentations = vg.max_matching();
    assert(augmentations == 6);
}

int main() {
    setbuf(stdout, nullptr);
    setbuf(stderr, nullptr);
    test_straight_line_1();
    test_straight_line_2();
    test_immediate_blossom_1();
    test_immediate_blossom_2();
    test_immediate_blossom_3();
    test_embedded_blossom_1();
    return 0;
}
