#include <cassert>
#include <functional>
#include <iostream>
#include <limits>

// *****

/**
 * AVL rotation notes
 *
 *       y                     x
 *      / \                   / \
 *    [a]  x        ->       y  [c]
 *        / \               / \
 *      [b] [c]           [a] [b]
 *
 * height(a) = h
 * height(b) = {h-1, h, h+1}
 * height(c) = {h-1, h, h+1}
 *
 * assert(balance(y) >= +1 and balance(y) >= balance(x))
 *
 * balance(y) == +1 {
 *   balance(x) == -1:
 *     heights: h, h, h-1
 *     new:    balance(y) = 0    balance(x) = -2  (!)
 *   balance(x) == 0:
 *     heights: h, h, h
 *     new:    balance(y) = 0    balance(x) = -1
 *   balance(x) == +1:
 *     heights: h, h-1, h
 *     new:    balance(y) = -1   balance(x) = -1
 * }
 *
 * balance(y) == +2 {
 *   balance(x) == -1:
 *     heights: h, h+1, h
 *     new:    balance(y) = +1   balance(x) = -2  (!)
 *   balance(x) == 0:
 *     heights: h, h+1, h+1
 *     new:    balance(y) = +1   balance(x) = -1
 *   balance(x) == +1:
 *     heights: h, h, h+1
 *     new:    balance(y) = 0    balance(x) = 0   (height reduced)
 *   balance(x) == +2:
 *     heights: h, h-1, h+1
 *     new:    balance(y) = -1   balance(x) = 0   (height reduced)
 * }
 *
 * Note: the height delta (difference in height compared to the previous tree)
 * is -1 (i.e. the height diminished) iff rotations occurred and the new root
 * is 0-balanced.
 *
 * AVL functions
 * We handle rotations and rebalancing professionally, namely:
 *   - roots which are unbalanced 'know' how to rebalance themselves
 *   - rotation functions maintain invariants themselves
 *   - rotation functions should compose, so that e.g. a left-right rotation can
 *     be literally coded as a right rotation followed by a left rotation and
 *     the result will be exactly what is expected, with invariants maintained
 */

/**
 * AVL node
 * This same class is used to represent the head node. The node is the tree's head
 * iff it does not hold data iff the parent pointer is nullptr.
 * We hide the head constructor to prevent default-constructed data from generating
 * head nodes
 */
template <typename T>
struct avl_node {
    using node_t = avl_node<T>;

    node_t* parent = nullptr;
    node_t* link[2] = {};
    union {
        int8_t _dummy;
        T data;
    };
    int8_t balance = 0;

    avl_node(T data) : data(std::move(data)) {}
    template <typename... Args>
    avl_node(Args&&... args) : data(std::forward<Args>(args)...) {}

    ~avl_node() {
        delete link[0];
        delete link[1];
        if (this != parent)
            data.~T();
    }

    static node_t* minimum(node_t* n) noexcept {
        while (n->link[0])
            n = n->link[0];
        return n;
    }
    static const node_t* minimum(const node_t* n) noexcept {
        while (n->link[0])
            n = n->link[0];
        return n;
    }
    static node_t* maximum(node_t* n) noexcept {
        while (n->link[1])
            n = n->link[1];
        return n;
    }
    static const node_t* maximum(const node_t* n) noexcept {
        while (n->link[1])
            n = n->link[1];
        return n;
    }
    static node_t* increment(node_t* n) noexcept {
        if (n->link[1])
            return minimum(n->link[1]);
        while (n == n->parent->link[1])
            n = n->parent;
        return n->parent;
    }
    static const node_t* increment(const node_t* n) noexcept {
        if (n->link[1])
            return minimum(n->link[1]);
        while (n == n->parent->link[1])
            n = n->parent;
        return n->parent;
    }
    static node_t* decrement(node_t* n) noexcept {
        if (n->link[0])
            return maximum(n->link[0]);
        while (n == n->parent->link[0])
            n = n->parent;
        return n->parent;
    }
    static const node_t* decrement(const node_t* n) noexcept {
        if (n->link[0])
            return maximum(n->link[0]);
        while (n == n->parent->link[0])
            n = n->parent;
        return n->parent;
    }

  private:
    struct avl_head_tag_t {};
    avl_node([[maybe_unused]] avl_head_tag_t tag) : parent(this) {}

  public:
    static node_t* new_empty() {
        return new node_t(avl_head_tag_t{});
    }
};

/**
 * AVL binary search tree core
 * Tree is completely open
 */
template <typename T>
struct avl_tree {
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using node_type = avl_node<T>;
    using tree_type = avl_tree<T>;

    using node_t = avl_node<T>;
    using tree_t = avl_tree<T>;

    // The real tree's root is head->link[0]. head is never nullptr.
    node_t* head;
    size_t node_count;

    avl_tree() noexcept : head(node_t::new_empty()), node_count(0) {}

    // Move constructor
    avl_tree(avl_tree&& other) noexcept
        : head(node_t::new_empty()), node_count(other.node_count) {
        adopt_node(head, other.head->link[0], 0);
        other.head->link[0] = nullptr;
        other.node_count = 0;
    }
    // Copy constructor
    avl_tree(const avl_tree& other) noexcept
        : head(node_t::new_empty()), node_count(other.node_count) {
        adopt_node(head, deep_clone_node(other.head->link[0]), 0);
    }
    // Move assignment
    avl_tree& operator=(avl_tree&& other) noexcept {
        delete head->link[0];
        adopt_node(head, other.head->link[0], 0);
        node_count = other.node_count;
        other.head->link[0] = nullptr;
        other.node_count = 0;
        return *this;
    }
    // Copy assignment
    avl_tree& operator=(const avl_tree& other) noexcept {
        delete head->link[0];
        adopt_node(head, deep_clone_node(other.head->link[0]), 0);
        node_count = other.node_count;
        return *this;
    }

    ~avl_tree() noexcept {
        delete head;
    }

    inline void swap(avl_tree& other) noexcept {
        std::swap(head, other.head);
        std::swap(node_count, other.node_count);
    }
    friend inline void swap(avl_tree& lhs, avl_tree& rhs) noexcept {
        lhs.swap(rhs);
    }
    inline void clear() noexcept {
        delete head->link[0];
        head->link[0] = nullptr;
        node_count = 0;
    }
    inline size_t size() const noexcept {
        return node_count;
    }
    inline bool empty() const noexcept {
        return node_count == 0;
    }
    constexpr size_t max_size() const noexcept {
        return std::numeric_limits<size_t>::max();
    }

    inline node_t* minimum() noexcept {
        return head->link[0] ? node_t::minimum(head->link[0]) : head;
    }
    inline const node_t* minimum() const noexcept {
        return head->link[0] ? node_t::minimum(head->link[0]) : head;
    }
    inline node_t* maximum() noexcept {
        return head->link[0] ? node_t::maximum(head->link[0]) : head;
    }
    inline const node_t* maximum() const noexcept {
        return head->link[0] ? node_t::maximum(head->link[0]) : head;
    }

  private:
    static inline void drop_node(node_t* node) {
        node->link[0] = node->link[1] = nullptr;
        delete node;
    }
    static inline void drop_subtree(node_t* node) {
        delete node;
    }
    static inline void adopt_node(node_t* parent, node_t* child, bool is_right) {
        parent->link[is_right] = child;
        if (child)
            child->parent = parent;
    }
    static inline void clear_node(node_t* node) {
        node->link[0] = node->link[1] = nullptr;
        node->parent = node;
        node->balance = 0;
    }
    static node_t* deep_clone_node(const node_t* node) {
        if (!node)
            return nullptr;
        node_t* clone = new node_t(node->data);
        clone->balance = node->balance;
        adopt_node(clone, deep_clone_node(node->link[0]), 0);
        adopt_node(clone, deep_clone_node(node->link[1]), 1);
        return clone;
    }

  public:
    /**
     *       y                     x
     *      / \                   / \
     *    [a]  x        ->       y  [c]
     *        / \               / \
     *      [b] [c]           [a] [b]
     */
    node_t* rotate_left(node_t* y) {
        node_t* x = y->link[1];
        assert(y->balance >= +1 && y->balance >= x->balance);
        bool is_right = y == y->parent->link[1];
        adopt_node(y->parent, x, is_right);
        adopt_node(y, x->link[0], 1);
        adopt_node(x, y, 0);
        int xb = x->balance;
        int y1 = y->balance == +1;
        int y2 = y->balance == +2;
        y->balance = -std::max(xb - y2, -y2);
        x->balance = std::min(xb - 1, -y1);
        return x;
    }

    /**
     *         y                  x
     *        / \                / \
     *       x  [c]     ->     [a]  y
     *      / \                    / \
     *    [a] [b]                [b] [c]
     */
    node_t* rotate_right(node_t* y) {
        node_t* x = y->link[0];
        assert(y->balance <= -1 && y->balance <= x->balance);
        bool is_right = y == y->parent->link[1];
        adopt_node(y->parent, x, is_right);
        adopt_node(y, x->link[1], 0);
        adopt_node(x, y, 1);
        int xb = x->balance;
        int y1 = y->balance == -1;
        int y2 = y->balance == -2;
        y->balance = -std::min(xb + y2, y2);
        x->balance = std::max(xb + 1, y1);
        return x;
    }

    /**
     * Recalibrate the tree rooted at y that has become unbalanced, deducing
     * the necessary rotations. Does nothing if y is already balanced.
     * Returns the new root after calibration.
     */
    node_t* rebalance(node_t* y) {
        if (y->balance == -2) {
            if (y->link[0]->balance == +1) {
                rotate_left(y->link[0]);
            }
            return rotate_right(y);
        }
        if (y->balance == +2) {
            if (y->link[1]->balance == -1) {
                rotate_right(y->link[1]);
            }
            return rotate_left(y);
        }
        return y;
    }

    /**
     *             p(-1)                     p(0)                       p(+1)
     *  h+1->h    / \   h         h->h-1    / \    h         h-1->h-2  / \   h
     *           /   \                     /   \                      /   \
     *          l     r                   l     r                    l     r
     *         / \   / \                 / \   / \                  / \   / \
     *       -.. .. .. ..              -.. .. .. ..               -.. .. .. ..
     * height lower in left       height lower in left    height lower in left
     *  new balance: 0             new balance: +1         new balance: +2 -> rebalance!
     *  delta height: h+1->h       delta height: h->h      delta height: 0 if new root
     *  overall height decreased   overall height did not  is imperfectly balanced (stop)
     *  continue on p's parent.    change so stop.         otherwise -1 so continue.
     */
    void rebalance_after_erase(node_t* y) {
        if (y == head)
            return;
        y = rebalance(y);
        while (y->parent != head && y->balance == 0) {
            bool is_right = y == y->parent->link[1];
            y->parent->balance += is_right ? -1 : 1;
            y = rebalance(y->parent);
        }
    }

    /**
     *            p(+1)                     p(0)                        p(-1)
     * h-1->h    / \   h         h->h+1    / \    h          h+1->h+2  / \   h
     *          /   \                     /   \                       /   \
     *         l     r                   l     r                     l     r
     *        / \   / \                 / \   / \                   / \   / \
     *      +.. .. .. ..              +.. .. .. ..                +.. .. .. ..
     * height lower in left    height lower in left      height lower in left
     *  new balance: 0          new balance: -1           new balance: -2 -> rebalance!
     *  delta height: h->h      delta height: h->h+1      delta height: h+1->h+1 (always)
     *  overall height did not  overall height increased  overall height did not
     *  change so stop.         continue on p's parent    change so stop.
     */
    void rebalance_after_insert(node_t* y) {
        node_t* parent = y->parent;
        while (parent != head && parent->balance == 0) {
            bool is_right = y == parent->link[1];
            parent->balance = is_right ? +1 : -1;
            y = parent;
            parent = y->parent;
        }
        if (parent != head) {
            bool is_right = y == parent->link[1];
            parent->balance += is_right ? +1 : -1;
            rebalance(parent);
        }
    }

    /**
     *   parent       parent  <-- rebalance here
     *     |            |
     *     y     ->    [x]
     *    /
     *  [x]
     *                 balance(parent) := ±1
     */
    void erase_node_pull_left(node_t* y) {
        node_t* x = y->link[0];
        node_t* parent = y->parent;
        bool y_is_right = y == parent->link[1];
        adopt_node(parent, x, y_is_right);
        if (parent != head) {
            parent->balance += y_is_right ? -1 : 1;
            rebalance_after_erase(parent);
        }
    }

    /**
     *     |            |
     *     y            x  <-- rebalance here
     *    / \    ->    / \
     *  [a]  x       [a] [b]
     *        \
     *        [b]
     *               balance(x) := balance(y) - 1
     */
    void erase_node_pull_right(node_t* y) {
        node_t* x = y->link[1];
        node_t* parent = y->parent;
        bool y_is_right = y == parent->link[1];
        adopt_node(parent, x, y_is_right);
        adopt_node(x, y->link[0], 0);
        x->balance = y->balance - 1;
        rebalance_after_erase(x);
    }

    /**
     *        |                       |
     *        y                       x
     *       / \                     / \
     *     [a]  x1                 [a]  x1
     *         / \                     / \
     *       ... [b]      ->         ... [b]
     *       / \                     / \
     *      w  [c]   rebalance -->  w  [c]
     *     / \          here       / \
     *    x  [d]                 [e]  [d]
     *     \
     *     [e]
     *                       balance(x) := balance(y)
     *                       balance(w) := balance(w) + 1
     */
    void erase_node_minimum(node_t* y) {
        node_t* x = node_t::minimum(y->link[1]->link[0]);
        node_t* w = x->parent;
        node_t* parent = y->parent;
        bool y_is_right = y == parent->link[1];
        adopt_node(parent, x, y_is_right);
        adopt_node(w, x->link[1], 0);
        adopt_node(x, y->link[0], 0);
        adopt_node(x, y->link[1], 1);
        x->balance = y->balance;
        w->balance += 1;
        rebalance_after_erase(w);
    }

    /**
     * Select the erase position based on y's right subtree
     */
    void erase_node_and_rebalance(node_t* y) {
        if (!y->link[1])
            erase_node_pull_left(y);
        else if (!y->link[1]->link[0])
            erase_node_pull_right(y);
        else
            erase_node_minimum(y);
    }

    /**
     * Insert node y into the tree as a child of parent on the given side.
     * parent must not have a child on that side and y must be a free node.
     *
     *    parent         parent                 parent        parent
     *     /       ->     /  \         or           \    ->    /  \
     *   [l]            [l]   y                     [r]       y   [r]
     */
    void insert_node(node_t* parent, node_t* y, bool is_right) {
        adopt_node(parent, y, is_right);
        rebalance_after_insert(y);
        node_count++;
    }

    /**
     * Insert node y after node, so that incrementing node afterwards gives y.
     * Usually this will insert y as the right child of node.
     * y must be a free node.
     *
     *    parent         parent                 parent        parent
     *     /       ->     /  \         or        /  \    ->    /  \
     *   [l]            [l]   y                [l]   r       [l]   r
     *                                              /             /
     *                                            ...           ...
     *                                            /             /
     *                              ++parent --> x             x
     *                                            \           / \
     *                                            [r]        y  [r]
     */
    void insert_node_after(node_t* node, node_t* y) {
        if (node->link[1]) {
            insert_node(node_t::increment(node), y, 0);
        } else {
            insert_node(node, y, 1);
        }
    }

    /**
     * Insert node y before node, so that decrementing node afterwards gives y.
     * Usually this will insert y as the left child of node.
     * y must be a free node.
     *
     *   parent        parent                  parent        parent
     *       \    ->    /  \          or        /  \    ->    /  \
     *       [r]       y   [r]                 l   [r]       l   [r]
     *                                          \             \
     *                                          ...           ...
     *                                            \             \
     *                                --parent --> x             x
     *                                            /             / \
     *                                          [l]           [l]  y
     */
    void insert_node_before(node_t* node, node_t* y) {
        if (node->link[0]) {
            insert_node(node_t::decrement(node), y, 1);
        } else {
            insert_node(node, y, 0);
        }
    }

    /**
     * Remove node y from the tree and destroy it.
     */
    void erase_node(node_t* y) {
        erase_node_and_rebalance(y);
        drop_node(y);
        node_count--;
    }

    /**
     * Remove node y from the tree but do not destroy it.
     */
    void yank_node(node_t* y) {
        erase_node_and_rebalance(y);
        clear_node(y);
        node_count--;
    }

    /**
     * Fork an existing node in the tree so that it becomes a child of the given node
     * x along with z. The boolean indicates which side the existing node will go to.
     *
     *      yield left               yield right
     *      |        |               |        |
     *      y   ->   x               y   ->   x
     *              / \      or              / \
     *             y  [z]                  [z]  y
     */
    void insert_node_leaf_fork(node_t* y, node_t* x, node_t* z, bool yield_right) {
        assert(y->is_leaf() && (!z || z->is_leaf()));
        adopt_node(y->parent, x, y == y->parent->link[1]);
        adopt_node(x, y, yield_right);
        adopt_node(x, z, !yield_right);
        rebalance_after_insert(x);
        x->balance = z ? 0 : (yield_right ? +1 : -1);
        node_count += 1 + !!z;
    }

    /**
     * Contract a fork anywhere in the tree, erasing the node and one of its subtrees.
     *
     *       |        |                  |        |
     *       y   ->   x                  y   ->   x
     *      / \      / \       or       / \      / \
     *     x   w    .. ..              w   x    .. ..
     *    / \ / \                     / \ / \
     *   .. ... ..                   .. ... ..
     */
    void contract_fork(node_t* y, bool keep_right) {
        node_t* x = y->link[keep_right];
        node_t* w = y->link[!keep_right];
        bool is_right = y == y->parent->link[1];
        adopt_node(y->parent, x, is_right);
        rebalance_after_erase(x);
        node_count -= 1 + w->subtree_size();
        drop_subtree(w);
        drop_node(y);
    }

    void pretty_print() const {
        printf("======== count: %02d ========\n", int(node_count));
        print_tree_preorder(head->link[0], "", false);
        printf("===========================\n");
    }

    void debug() const {
        assert(head && !head->link[1] && head->balance == 0 && head->parent == head);
        size_t cnt = 0;
        debug_node(head->link[0], head, cnt);
        assert(cnt == node_count);
    }

  private:
    void print_tree_preorder(node_t* n, std::string prefix, bool bar) const {
        static const char* line[2] = {u8"└", u8"├"};
        static const char* pad[2] = {"    ", u8" |  "};
        if (!n) {
            printf("%s %s\n", prefix.data(), line[bar]);
            return;
        }
        printf(u8"%s %s── %s\n", prefix.data(), line[bar], print_node(n).data());
        if (n->link[0] || n->link[1]) {
            prefix += pad[bar];
            print_tree_preorder(n->link[0], prefix, true);
            print_tree_preorder(n->link[1], prefix, false);
        }
    }

    static inline std::string print_node(node_t* node) noexcept {
        std::string s;
        s += std::to_string(node->data);
        s += "(" + std::to_string(node->balance) + ")";
        s += u8"  ╴  ╴  ╴  ╴ ";
        if (node->parent != node->parent->parent)
            s += "  ^(" + std::to_string(node->parent->data) + ")";
        if (node->link[0])
            s += "  <(" + std::to_string(node->link[0]->data) + ")";
        if (node->link[1])
            s += "  >(" + std::to_string(node->link[1]->data) + ")";
        return s;
    }

    int debug_node(const node_t* y, const node_t* parent, size_t& cnt) const {
        if (!y)
            return 0;
        cnt++;
        assert(y->parent == parent);
        assert(-1 <= y->balance && y->balance <= +1);
        int l = debug_node(y->link[0], y, cnt);
        int r = debug_node(y->link[1], y, cnt);
        assert(y->balance == r - l);
        return 1 + std::max(l, r);
    }
};
