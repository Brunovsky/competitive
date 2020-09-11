#ifndef SPLAY_TREE_HPP
#define SPLAY_TREE_HPP

#include <cassert>
#include <functional>
#include <iostream>
#include <limits>

// *****

/**
 * Splay node
 * This same class is used to represent the head node. The node is the tree's head
 * iff it does not hold data iff the parent pointer is this (itself).
 */
template <typename T>
struct splay_node {
    using node_t = splay_node<T>;

    node_t* parent = nullptr;
    node_t* link[2] = {};
    union {
        int8_t _dummy;
        T data;
    };

    ~splay_node() {
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
    splay_node(const splay_node&) = delete;
    splay_node(splay_node&&) = delete;
    splay_node& operator=(const splay_node&) = delete;
    splay_node& operator=(splay_node&&) = delete;

    // hide this to prevent default-constructed data from creating head nodes
    struct splay_head_tag_t {};
    splay_node(splay_head_tag_t _tag) : parent(this) { (void)_tag; }

    splay_node(T data) : data(std::move(data)) {}
    template <typename... Args>
    splay_node(Args&&... args) : data(std::forward<Args>(args)...) {}

  public:
    static node_t* new_empty() { return new splay_node(splay_head_tag_t{}); }

    template <typename... Args>
    static node_t* make(Args&&... args) {
        return new splay_node(std::forward<Args>(args)...);
    }
};

/**
 * Splay binary search tree core
 * Tree is completely open
 */
template <typename T>
struct splay_tree {
    using node_t = splay_node<T>;

    // The real tree's root is head->link[0]. head is never nullptr.
    node_t* head;
    node_t* min_node;
    node_t* max_node;
    size_t node_count;

    splay_tree() noexcept
        : head(node_t::new_empty()), min_node(head), max_node(head), node_count(0) {}

    // Move constructor
    splay_tree(splay_tree&& other) noexcept : splay_tree() { swap(other); }
    // Copy constructor
    splay_tree(const splay_tree& other) noexcept
        : head(node_t::new_empty()), node_count(other.node_count) {
        adopt_node(head, deep_clone_node(other.head->link[0]), 0);
        update_minmax();
    }
    // Move assignment
    splay_tree& operator=(splay_tree&& other) noexcept {
        if (head != other.head) {
            clear();
            swap(other);
        }
        return *this;
    }
    // Copy assignment
    splay_tree& operator=(const splay_tree& other) noexcept {
        if (head != other.head) {
            delete head->link[0];
            adopt_node(head, deep_clone_node(other.head->link[0]), 0);
            update_minmax();
            node_count = other.node_count;
        }
        return *this;
    }

    ~splay_tree() noexcept { delete head; }

    inline void clear() noexcept {
        delete head->link[0];
        head->link[0] = nullptr;
        min_node = max_node = head;
        node_count = 0;
    }
    inline void swap(splay_tree& other) noexcept {
        std::swap(head, other.head);
        std::swap(min_node, other.min_node);
        std::swap(max_node, other.max_node);
        std::swap(node_count, other.node_count);
    }
    friend inline void swap(splay_tree& lhs, splay_tree& rhs) noexcept { lhs.swap(rhs); }

    inline node_t* minimum() noexcept { return min_node; }
    inline const node_t* minimum() const noexcept { return min_node; }
    inline node_t* maximum() noexcept { return max_node; }
    inline const node_t* maximum() const noexcept { return max_node; }

  private:
    inline void update_minmax() {
        if (head->link[0]) {
            min_node = node_t::minimum(head->link[0]);
            max_node = node_t::maximum(head->link[0]);
        } else {
            min_node = max_node = head;
        }
    }
    static inline void drop_node(node_t* node) {
        node->link[0] = node->link[1] = nullptr;
        delete node;
    }
    static inline void adopt_node(node_t* parent, node_t* child, bool side) {
        parent->link[side] = child;
        if (child)
            child->parent = parent;
    }
    static inline void clear_node(node_t* node) {
        node->link[0] = node->link[1] = nullptr;
        node->parent = nullptr;
    }
    static node_t* deep_clone_node(const node_t* node) {
        if (!node)
            return nullptr;
        node_t* clone = node_t::make(node->data);
        adopt_node(clone, deep_clone_node(node->link[0]), 0);
        adopt_node(clone, deep_clone_node(node->link[1]), 1);
        return clone;
    }

    /**
     *       y                     x
     *      / \                   / \
     *    [a]  x        ->       y  [c]
     *        / \               / \
     *      [b] [c]           [a] [b]
     */
    node_t* rotate_left(node_t* y) {
        node_t* x = y->link[1];
        bool yside = y == y->parent->link[1];
        adopt_node(y->parent, x, yside);
        adopt_node(y, x->link[0], 1);
        adopt_node(x, y, 0);
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
        bool yside = y == y->parent->link[1];
        adopt_node(y->parent, x, yside);
        adopt_node(y, x->link[1], 0);
        adopt_node(x, y, 1);
        return x;
    }

    /**
     * Splay node y, standard operation in splay tree.
     * Performs height(y) rotations.
     */
    void splay(node_t* y) {
        node_t* p = y->parent;
        node_t* g = p->parent;
        while (g != head) {
            bool yp = y == p->link[1];
            bool pg = p == g->link[1];
            if (yp && pg)
                rotate_left(g), rotate_left(p);
            else if (yp && !pg)
                rotate_left(p), rotate_right(g);
            else if (!yp && pg)
                rotate_right(p), rotate_left(g);
            else /* if (!yp && !pg) */
                rotate_right(g), rotate_right(p);
            p = y->parent;
            g = p->parent;
        }
        if (p != head) {
            if (y == p->link[1])
                rotate_left(p);
            else
                rotate_right(p);
        }
        assert(head->link[0] == y);
    }

    /**
     * Remove the element at the root of the tree.
     */
    void splice() {
        node_t* root = head->link[0];
        if (!root->link[0])
            adopt_node(head, root->link[1], 0);
        else if (!root->link[1])
            adopt_node(head, root->link[0], 0);
        else {
            node_t* n = root->link[0];
            while (n->link[1])
                n = rotate_left(n);
            adopt_node(n, root->link[1], 1);
            adopt_node(head, n, 0);
        }
    }

    /**
     * Update min max pointers before insert
     */
    void insert_minmax(node_t* parent, node_t* y, bool side) {
        if (node_count > 0) {
            if (!side && parent == min_node)
                min_node = y;
            else if (side && parent == max_node)
                max_node = y;
        } else {
            min_node = max_node = y;
        }
    }

    /**
     * Update min max pointers before erase
     */
    void erase_minmax(node_t* y) {
        if (node_count > 1) {
            if (y == min_node)
                min_node = y->link[1] ? y->link[1] : y->parent;
            else if (y == max_node)
                max_node = y->link[0] ? y->link[0] : y->parent;
        } else {
            min_node = max_node = head;
        }
    }

  public:
    /**
     * Insert node y into the tree as a child of parent on the given side.
     * parent must not have a child on that side and y must be a free node.
     *
     *    parent         parent                 parent        parent
     *     /       ->     /  \         or           \    ->    /  \
     *   [l]            [l]   y                     [r]       y   [r]
     */
    void insert_node(node_t* parent, node_t* y, bool side) {
        insert_minmax(parent, y, side);
        adopt_node(parent, y, side);
        splay(y);
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
    void insert_node_after(node_t* parent, node_t* y) {
        if (parent->link[1])
            insert_node(node_t::increment(parent), y, 0);
        else
            insert_node(parent, y, 1);
    }

    /**
     * Insert node y before node, so that decrementing node afterwards gives y.
     * Usually this will insert y as the left child of node.
     * y must be a free node.
     *
     *  parent          parent                    parent          parent
     *       \    ->    /  \          or        /  \    ->    /  \
     *       [r]       y   [r]                 l   [r]       l   [r]
     *                                          \             \
     *                                          ...           ...
     *                                            \             \
     *                                --parent --> x             x
     *                                            /             / \
     *                                          [l]           [l]  y
     */
    void insert_node_before(node_t* parent, node_t* y) {
        if (parent->link[0])
            insert_node(node_t::decrement(parent), y, 1);
        else
            insert_node(parent, y, 0);
    }

    /**
     * Remove node y from the tree and destroy it.
     */
    void erase_node(node_t* y) {
        erase_minmax(y);
        splay(y);
        splice();
        drop_node(y);
        node_count--;
    }

    /**
     * Remove node y from the tree but do not destroy it.
     */
    void yank_node(node_t* y) {
        erase_minmax(y);
        splay(y);
        splice();
        clear_node(y);
        node_count--;
    }

    void pretty_print() const {
        int h = get_height(head->link[0]);
        printf("===== count: %02d ===== height: %02d =====\n", int(node_count), h);
        print_tree_preorder(head->link[0], "", false);
        printf("======================================\n");
    }

    void debug() const {
        assert(head && !head->link[1] && head->parent == head);
        // assert(min_node == node_t::minimum(head));
        // assert(max_node == (head->link[0] ? node_t::maximum(head->link[0]) : head));
        size_t cnt = 0;
        debug_node(head->link[0], head, cnt);
        assert(cnt == node_count);
    }

  private:
    void print_tree_preorder(const node_t* n, std::string prefix, bool bar) const {
        static const char* line[2] = {u8"└──", u8"├──"};
        static const char* pad[2] = {"    ", u8" |  "};
        if (!n) {
            printf("%s %s\n", prefix.data(), line[bar]);
            return;
        }
        printf(u8"%s %s %s\n", prefix.data(), line[bar], print_node(n).data());
        if (n->link[0] || n->link[1]) {
            prefix += pad[bar];
            print_tree_preorder(n->link[0], prefix, true);
            print_tree_preorder(n->link[1], prefix, false);
        }
    }

    static inline std::string print_node(const node_t* node) noexcept {
        using std::to_string;
        std::string s;
        s += to_string(node->data);
        s += u8"  ╴  ╴  ╴  ╴ ";
        if (node->parent != node->parent->parent)
            s += "  ^(" + to_string(node->parent->data) + ")";
        if (node->link[0])
            s += "  <(" + to_string(node->link[0]->data) + ")";
        if (node->link[1])
            s += "  >(" + to_string(node->link[1]->data) + ")";
        return s;
    }

    int debug_node(const node_t* y, const node_t* parent, size_t& cnt) const {
        if (!y)
            return 0;
        cnt++;
        (void)parent, assert(y->parent == parent);
        int hl = debug_node(y->link[0], y, cnt);
        int hr = debug_node(y->link[1], y, cnt);
        return 1 + std::max(hl, hr);
    }

    int get_height(const node_t* y) const {
        return y ? 1 + std::max(get_height(y->link[0]), get_height(y->link[1])) : 0;
    }
};

#endif // SPLAY_TREE_HPP
