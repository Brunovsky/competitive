#ifndef BS_TREE_HPP
#define BS_TREE_HPP

#include "rb_tree.hpp"

/**
 * STL-friendly tree template for (multi)sets and (multi)maps.
 *   - Parametrized by type T and comparison functor.
 *   - Does not use allocators (for simplicity).
 *
 *   - Red black core
 *   - AVL core
 *
 * Done:
 *   - Tree core
 *   - Single and range insert and erase
 *   - All iterators
 *   - Support comparison operators
 *   - Support equal_range(), lower_bound() and upper_bound()
 *   - Support emplace(), emplace_hint(), and insert_hint()
 *   - Support merge() and extract()
 *   - Support inserters (avl_inserter_*)
 *   - Copying and moving
 *   - Define set, multiset wrappers
 *
 * TODO:
 *   - Define map, multimap wrappers
 */

/**
 * set, multiset, map, multimap tags and traits for bs_tree and friends.
 */
enum bs_tree_tag { set_tag, map_tag };

/**
 * Forward declarations
 */
template <typename T>
using Tree = rb_tree<T>;

template <typename T, typename Compare, bs_tree_tag tag>
struct bst_traits;

template <typename T, typename Compare, bs_tree_tag tag>
struct bs_tree;

template <typename BSTree>
struct bst_node_handle_methods;

template <typename BSTree>
struct bst_iterator;

template <typename BSTree>
struct bst_const_iterator;

template <typename BSTree>
struct bst_node_handle;

template <typename BSTree>
struct bst_insert_return_type;

template <typename BSTree>
struct bst_inserter_unique_iterator;

template <typename BSTree>
struct bst_inserter_multi_iterator;

/**
 * Define BST traits for sets and maps
 * Sets store immutable elements themselves
 * Maps store pairs where the key is immutable but the mapped value is mutable
 */
template <typename Key, typename Compare>
struct bst_traits<Key, Compare, set_tag> {
    using key_type = Key;
    using value_type = Key;
    using key_compare = Compare;
    using value_compare = Compare;

    using reference = const value_type&;
    using const_reference = const value_type&;
    using pointer = const value_type*;
    using const_pointer = const value_type*;

  protected:
    static constexpr inline const Key& get_key(const value_type& elem) noexcept {
        return elem;
    }
};
template <typename Key, typename T, typename Compare>
struct bst_traits<std::pair<const Key, T>, Compare, map_tag> {
    using key_type = Key;
    using value_type = std::pair<const Key, T>;
    using mapped_type = T;
    using key_compare = Compare;

    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;

  protected:
    static constexpr inline const Key& get_key(const value_type& elem) noexcept {
        return elem.first;
    }
};

/**
 * Node handle methods:
 *   value(), key() and mapped()
 * needs specialization according to standard
 */
template <typename Key, typename Compare>
struct bst_node_handle_methods<bs_tree<Key, Compare, set_tag>> {
  protected:
    using BSTree = bs_tree<Key, Compare, set_tag>;
    using self_t = bst_node_handle<bs_tree<Key, Compare, set_tag>>;

  public:
    using value_type = typename BSTree::value_type;

    value_type& value() const noexcept {
        return self_t::y->data;
    }
};

template <typename V, typename Compare>
struct bst_node_handle_methods<bs_tree<V, Compare, map_tag>> {
  protected:
    using BSTree = bs_tree<V, Compare, map_tag>;
    using self_t = bst_node_handle<bs_tree<V, Compare, map_tag>>;

  public:
    using key_type = typename BSTree::key_type;
    using mapped_type = typename BSTree::mapped_type;

    key_type& key() const {
        return self_t::y->data.first;
    }
    mapped_type& mapped() const {
        return self_t::y->data.second;
    }
};

/**
 * Non-const iterator for bs_tree
 * Works for all 4 container types
 */
template <typename BSTree>
struct bst_iterator {
  private:
    friend BSTree;
    friend bst_const_iterator<BSTree>;
    using node_t = typename BSTree::node_t;
    using self_t = bst_iterator<BSTree>;
    node_t* y;

  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = typename BSTree::value_type;
    using reference = typename BSTree::reference;
    using pointer = typename BSTree::pointer;
    using difference_type = ptrdiff_t;

    bst_iterator() : y(nullptr) {}
    explicit bst_iterator(node_t* y) : y(y) {}

    explicit operator bool() const noexcept {
        return y != nullptr;
    }
    reference operator*() const noexcept {
        return y->data;
    }
    pointer operator->() const noexcept {
        return &y->data;
    }
    self_t& operator++() noexcept {
        y = node_t::increment(y);
        return *this;
    }
    self_t operator++(int) noexcept {
        self_t z = *this;
        y = node_t::increment(y);
        return z;
    }
    self_t& operator--() noexcept {
        y = node_t::decrement(y);
        return *this;
    }
    self_t operator--(int) noexcept {
        self_t z = *this;
        y = node_t::decrement(y);
        return z;
    }
    friend bool operator==(const self_t& lhs, const self_t& rhs) noexcept {
        return lhs.y == rhs.y;
    }
    friend bool operator!=(const self_t& lhs, const self_t& rhs) noexcept {
        return lhs.y != rhs.y;
    }
};

/**
 * Const iterator for bs_tree
 * Works for all 4 container types
 */
template <typename BSTree>
struct bst_const_iterator {
  private:
    friend BSTree;
    friend bst_iterator<BSTree>;
    using node_t = typename BSTree::node_t;
    using self_t = bst_const_iterator<BSTree>;
    const node_t* y;

  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = typename BSTree::value_type;
    using reference = typename BSTree::const_reference;
    using pointer = typename BSTree::const_pointer;
    using difference_type = ptrdiff_t;

    bst_const_iterator() : y(nullptr) {}
    explicit bst_const_iterator(const node_t* y) : y(y) {}
    bst_const_iterator(bst_iterator<BSTree> it) : y(it.y) {}

    explicit operator bool() const noexcept {
        return y != nullptr;
    }
    reference operator*() const noexcept {
        return y->data;
    }
    pointer operator->() const noexcept {
        return &y->data;
    }
    self_t& operator++() noexcept {
        y = node_t::increment(y);
        return *this;
    }
    self_t operator++(int) noexcept {
        self_t z = *this;
        y = node_t::increment(y);
        return z;
    }
    self_t& operator--() noexcept {
        y = node_t::decrement(y);
        return *this;
    }
    self_t operator--(int) noexcept {
        self_t z = *this;
        y = node_t::decrement(y);
        return z;
    }
    friend bool operator==(const self_t& lhs, const self_t& rhs) noexcept {
        return lhs.y == rhs.y;
    }
    friend bool operator!=(const self_t& lhs, const self_t& rhs) noexcept {
        return lhs.y != rhs.y;
    }
};

/**
 * The handle abstraction for extracted nodes
 */
template <typename BSTree>
struct bst_node_handle : bst_node_handle_methods<BSTree> {
  private:
    friend BSTree;
    using node_t = typename BSTree::node_t;
    using self_t = bst_node_handle;
    node_t* y;

  public:
    bst_node_handle() : y(nullptr) {}
    explicit bst_node_handle(node_t* y) : y(y) {}

    bst_node_handle(self_t&& other) : y(other.y) {
        other.y = nullptr;
    }
    bst_node_handle& operator=(self_t&& other) {
        delete y;
        y = other.y;
        other.y = nullptr;
    }

    ~bst_node_handle() {
        delete y;
    }

    bool empty() const noexcept {
        return y == nullptr;
    }
    explicit operator bool() const noexcept {
        return y != nullptr;
    }
    void swap(self_t& other) noexcept {
        swap(y, other.y);
    }
    friend void swap(self_t& lhs, self_t& rhs) noexcept {
        lhs.swap(rhs);
    }
};

/**
 * Return type for unique insertion using node handles
 */
template <typename BSTree>
struct bst_insert_return_type {
    using iterator = typename BSTree::iterator;
    using node_type = typename BSTree::node_type;

    iterator position;
    bool inserted;
    node_type node;
};

/**
 * Binary search tree built on top of the AVL tree core
 * Can generate all 4 types of containers (set, multiset, map and multimap)
 */
template <typename T, typename Compare = std::less<T>, bs_tree_tag tag = set_tag>
struct bs_tree : protected Tree<T>, public bst_traits<T, Compare, tag> {
  private:
    using Traits = bst_traits<T, Compare, tag>;
    using node_t = typename Tree<T>::node_t;
    friend bst_iterator<bs_tree>;
    friend bst_const_iterator<bs_tree>;
    friend bst_node_handle<bs_tree>;
    friend bst_insert_return_type<bs_tree>;

    using Traits::get_key;
    using Tree<T>::head;
    using Tree<T>::drop_node;
    using Tree<T>::insert_node;
    using Tree<T>::insert_node_after;
    using Tree<T>::insert_node_before;
    using Tree<T>::erase_node;
    using Tree<T>::yank_node;
    using Tree<T>::maximum;
    using Tree<T>::minimum;

  protected:
    using Key = typename Traits::key_type;
    Compare comp;

    template <typename K1 = Key, typename K2 = Key>
    inline bool compare(const K1& lhs, const K2& rhs) const noexcept {
        return comp(lhs, rhs);
    }

  public:
    using key_type = typename Traits::key_type;
    using value_type = typename Traits::value_type;

    using iterator = bst_iterator<bs_tree>;
    using const_iterator = bst_const_iterator<bs_tree>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    using size_type = typename Tree<T>::size_type;
    using difference_type = typename Tree<T>::difference_type;

    using reference = typename Traits::reference;
    using const_reference = typename Traits::const_reference;
    using pointer = typename Traits::pointer;
    using const_pointer = typename Traits::const_pointer;

    using node_type = bst_node_handle<bs_tree>;
    using insert_return_type = bst_insert_return_type<bs_tree>;

    bs_tree() {}

    explicit bs_tree(const Compare& comp) : comp(comp) {}

    bs_tree(const bs_tree& other) = default;
    bs_tree(bs_tree&& other) = default;
    bs_tree& operator=(const bs_tree& other) = default;
    bs_tree& operator=(bs_tree&& other) = default;

    using Tree<T>::clear;
    using Tree<T>::debug;
    using Tree<T>::empty;
    using Tree<T>::max_size;
    using Tree<T>::size;

    void swap(bs_tree& other) {
        using std::swap;
        Tree<T>::swap(other);
        swap(comp, other.comp);
    }
    friend inline void swap(bs_tree& lhs, bs_tree& rhs) noexcept {
        lhs.swap(rhs);
    }

    inline iterator begin() noexcept {
        return iterator(minimum());
    }
    inline iterator end() noexcept {
        return iterator(head);
    }
    inline const_iterator begin() const noexcept {
        return const_iterator(minimum());
    }
    inline const_iterator end() const noexcept {
        return const_iterator(head);
    }
    inline const_iterator cbegin() const noexcept {
        return const_iterator(minimum());
    }
    inline const_iterator cend() const noexcept {
        return const_iterator(head);
    }
    inline reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }
    inline reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }
    inline const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }
    inline const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }
    inline const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(end());
    }
    inline const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(begin());
    }

    friend bool operator==(const bs_tree& lhs, const bs_tree& rhs) noexcept {
        return lhs.size() == rhs.size() &&
               std::equal(lhs.begin(), lhs.end(), rhs.begin());
    }
    friend bool operator!=(const bs_tree& lhs, const bs_tree& rhs) noexcept {
        return !(lhs == rhs);
    }
    friend bool operator<(const bs_tree& lhs, const bs_tree& rhs) noexcept {
        return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                            rhs.end());
    }
    friend bool operator>(const bs_tree& lhs, const bs_tree& rhs) noexcept {
        return rhs < lhs;
    }
    friend bool operator<=(const bs_tree& lhs, const bs_tree& rhs) noexcept {
        return !(rhs < lhs);
    }
    friend bool operator>=(const bs_tree& lhs, const bs_tree& rhs) noexcept {
        return !(lhs < rhs);
    }

  private:
    template <typename K = Key>
    node_t* find_node(const K& key) {
        node_t* x = head->link[0];
        while (x) {
            bool lesser = compare(key, get_key(x->data));
            if (!lesser && !compare(get_key(x->data), key))
                return x;
            x = x->link[!lesser];
        }
        return head;
    }
    template <typename K = Key>
    const node_t* find_node(const K& key) const {
        const node_t* x = head->link[0];
        while (x) {
            bool lesser = compare(key, get_key(x->data));
            if (!lesser && !compare(get_key(x->data), key))
                return x;
            x = x->link[!lesser];
        }
        return head;
    }
    template <typename K = Key>
    node_t* lower_bound_node(const K& key) {
        node_t* x = head->link[0];
        node_t* y = head;
        while (x) {
            if (!compare(get_key(x->data), key))
                y = x, x = x->link[0];
            else
                x = x->link[1];
        }
        return y;
    }
    template <typename K = Key>
    const node_t* lower_bound_node(const K& key) const {
        const node_t* x = head->link[0];
        const node_t* y = head;
        while (x) {
            if (!compare(get_key(x->data), key))
                y = x, x = x->link[0];
            else
                x = x->link[1];
        }
        return y;
    }
    template <typename K = Key>
    node_t* upper_bound_node(const K& key) {
        node_t* x = head->link[0];
        node_t* y = head;
        while (x) {
            if (compare(key, get_key(x->data)))
                y = x, x = x->link[0];
            else
                x = x->link[1];
        }
        return y;
    }
    template <typename K = Key>
    const node_t* upper_bound_node(const K& key) const {
        const node_t* x = head->link[0];
        const node_t* y = head;
        while (x) {
            if (compare(key, get_key(x->data)))
                y = x, x = x->link[0];
            else
                x = x->link[1];
        }
        return y;
    }
    template <typename K = Key>
    std::pair<node_t*, node_t*> equal_range_node(const K& key) {
        node_t* x = head->link[0];
        node_t* y = head;
        while (x) {
            if (compare(get_key(x->data), key))
                x = x->link[1];
            else if (compare(key, get_key(x->data)))
                y = x, x = x->link[0];
            else {
                node_t* xu = x->link[1];
                node_t* yu = y;
                y = x, x = x->link[0];
                // lower bound [x, y]
                while (x) {
                    if (!compare(get_key(x->data), key))
                        y = x, x = x->link[0];
                    else
                        x = x->link[1];
                }
                // upper bound [xu, yu]
                while (xu) {
                    if (compare(key, get_key(xu->data)))
                        yu = xu, xu = xu->link[0];
                    else
                        xu = xu->link[1];
                }
                return {y, yu};
            }
        }
        return {y, y};
    }
    template <typename K = Key>
    std::pair<const node_t*, const node_t*> equal_range_node(const K& key) const {
        const node_t* x = head->link[0];
        const node_t* y = head;
        while (x) {
            if (compare(get_key(x->data), key))
                x = x->link[1];
            else if (compare(key, get_key(x->data)))
                y = x, x = x->link[0];
            else {
                const node_t* xu = x->link[1];
                const node_t* yu = y;
                y = x, x = x->link[0];
                // lower bound [x, y]
                while (x) {
                    if (!compare(get_key(x->data), key))
                        y = x, x = x->link[0];
                    else
                        x = x->link[1];
                }
                // upper bound [xu, yu]
                while (xu) {
                    if (compare(key, xu->data))
                        yu = xu, xu = xu->link[0];
                    else
                        xu = xu->link[1];
                }
                return {y, yu};
            }
        }
        return {y, y};
    }

  public:
    template <typename K = Key>
    iterator find(const K& key) {
        return iterator(find_node(key));
    }
    template <typename K = Key>
    const_iterator find(const K& key) const {
        return const_iterator(find_node(key));
    }
    template <typename K = Key>
    iterator lower_bound(const K& key) {
        return iterator(lower_bound_node(key));
    }
    template <typename K = Key>
    const_iterator lower_bound(const K& key) const {
        return const_iterator(lower_bound_node(key));
    }
    template <typename K = Key>
    iterator upper_bound(const K& key) {
        return iterator(upper_bound_node(key));
    }
    template <typename K = Key>
    const_iterator upper_bound(const K& key) const {
        return const_iterator(upper_bound_node(key));
    }
    template <typename K = Key>
    std::pair<iterator, iterator> equal_range(const K& key) {
        auto nodes = equal_range_node(key);
        return {iterator(nodes.first), iterator(nodes.second)};
    }
    template <typename K = Key>
    std::pair<const_iterator, const_iterator> equal_range(const K& key) const {
        auto nodes = equal_range_node(key);
        return {const_iterator(nodes.first), const_iterator(nodes.second)};
    }
    template <typename K = Key>
    bool contains(const K& key) const {
        const node_t* x = head->link[0];
        while (x) {
            bool lesser = compare(key, get_key(x->data));
            if (!lesser && !compare(get_key(x->data), key))
                return true;
            x = x->link[!lesser];
        }
        return false;
    }
    template <typename K = Key>
    size_t count(const K& key) const {
        auto range = equal_range(key);
        return std::distance(range.first, range.second);
    }

  private:
    std::pair<iterator, bool> try_insert_node_unique(node_t* node) {
        node_t* y = head->link[0];
        node_t* parent = head;
        bool lesser = true;
        while (y) {
            lesser = compare(get_key(node->data), get_key(y->data));
            if (!lesser && !compare(get_key(y->data), get_key(node->data)))
                return {iterator(y), false};
            parent = y;
            y = y->link[!lesser];
        }
        insert_node(parent, node, !lesser);
        return {iterator(node), true};
    }
    std::pair<iterator, bool> insert_node_unique(node_t* node) {
        auto res = try_insert_node_unique(node);
        if (!res.second)
            drop_node(node);
        return res;
    }
    std::pair<iterator, bool> try_insert_node_hint_unique(node_t* node, node_t* hint) {
        if (hint == head) {
            if (size() > 0 && compare(get_key(maximum()->data), get_key(node->data))) {
                insert_node(maximum(), node, 1);
                return {iterator(node), true};
            }
            return try_insert_node_unique(node); // bad hint
        } else if (compare(get_key(node->data), get_key(hint->data))) {
            if (hint == minimum()) {
                insert_node(minimum(), node, 0);
                return {iterator(node), true};
            }
            node_t* prev = node_t::decrement(hint);
            if (compare(get_key(prev->data), get_key(node->data))) {
                insert_node_before(hint, node);
                return {iterator(node), true};
            }
            return try_insert_node_unique(node); // bad hint
        } else if (compare(get_key(hint->data), get_key(node->data))) {
            return try_insert_node_unique(node); // bad hint
        } else {
            return {iterator(hint), false};
        }
    }
    std::pair<iterator, bool> insert_node_hint_unique(node_t* node, node_t* hint) {
        auto res = try_insert_node_hint_unique(node, hint);
        if (!res.second)
            drop_node(node);
        return res;
    }
    iterator insert_node_multi(node_t* node) {
        node_t* y = head->link[0];
        node_t* parent = head;
        bool lesser = true;
        while (y) {
            lesser = compare(get_key(node->data), get_key(y->data));
            parent = y;
            y = y->link[!lesser];
        }
        insert_node(parent, node, !lesser);
        return iterator(node);
    }
    iterator insert_node_hint_multi(node_t* node, node_t* hint) {
        if (hint == head) {
            if (size() > 0 && compare(get_key(maximum()->data), get_key(node->data))) {
                insert_node(maximum(), node, 1);
                return iterator(node);
            }
            return insert_node_multi(node); // bad hint
        } else if (compare(get_key(node->data), get_key(hint->data))) {
            if (hint == minimum()) {
                insert_node(minimum(), node, 0);
                return iterator(node);
            }
            node_t* prev = node_t::decrement(hint);
            if (compare(get_key(prev->data), get_key(node->data))) {
                insert_node_before(hint, node);
                return iterator(node);
            }
            return insert_node_multi(node);
        } else if (compare(get_key(hint->data), get_key(node->data))) {
            return insert_node_multi(node);
        } else {
            insert_node_before(hint, node);
            return iterator(node);
        }
    }

  public:
    insert_return_type insert_unique(node_type&& nh) {
        if (nh.y == nullptr)
            return {end(), false, node_type()};
        auto res = try_insert_node_unique(nh.y);
        if (res.second)
            nh.y = nullptr;
        return {res.first, res.second, std::move(nh)};
    }
    iterator insert_hint_unique(const_iterator hint, node_type&& nh) {
        if (nh.y == nullptr)
            return end();
        node_t* hint_node = const_cast<node_t*>(hint.y);
        auto res = try_insert_node_hint_unique(nh.y, hint_node);
        if (res.second)
            nh.y = nullptr;
        return res.first;
    }
    std::pair<iterator, bool> insert_unique(const T& data) {
        node_t* node = new node_t(data);
        return insert_node_unique(node);
    }
    iterator insert_hint_unique(const_iterator hint, const T& data) {
        node_t* node = new node_t(data);
        node_t* hint_node = const_cast<node_t*>(hint.y);
        return insert_node_hint_unique(node, hint_node).first;
    }
    std::pair<iterator, bool> insert_unique(T&& data) {
        node_t* node = new node_t(std::move(data));
        return insert_node_unique(node);
    }
    iterator insert_hint_unique(const_iterator hint, T&& data) {
        node_t* node = new node_t(std::move(data));
        node_t* hint_node = const_cast<node_t*>(hint.y);
        return insert_node_hint_unique(node, hint_node).first;
    }
    template <typename InputIt>
    void insert_unique(InputIt first, InputIt last) {
        for (auto it = first; it != last; ++it) {
            insert_unique(*it);
        }
    }
    void insert_unique(std::initializer_list<T> ilist) {
        insert_unique(ilist.begin(), ilist.end());
    }

    iterator insert_multi(node_type&& nh) {
        auto it = insert_node_multi(nh.y);
        nh.y = nullptr;
        return it;
    }
    iterator insert_hint_multi(const_iterator hint, node_type&& nh) {
        if (nh.y == nullptr)
            return end();
        node_t* hint_node = const_cast<node_t*>(hint.y);
        auto it = insert_node_hint_multi(nh.y, hint_node);
        nh.y = nullptr;
        return it;
    }
    iterator insert_multi(const T& data) {
        node_t* node = new node_t(data);
        return insert_node_multi(node);
    }
    iterator insert_hint_multi(const_iterator hint, const T& data) {
        node_t* node = new node_t(data);
        node_t* hint_node = const_cast<node_t*>(hint.y);
        return insert_node_hint_multi(node, hint_node);
    }
    iterator insert_multi(T&& data) {
        node_t* node = new node_t(std::move(data));
        return insert_node_multi(node);
    }
    iterator insert_hint_multi(const_iterator hint, T&& data) {
        node_t* node = new node_t(std::move(data));
        node_t* hint_node = const_cast<node_t*>(hint.y);
        return insert_node_hint_multi(node, hint_node);
    }
    template <typename InputIt>
    void insert_multi(InputIt first, InputIt last) {
        for (auto it = first; it != last; ++it) {
            insert_multi(*it);
        }
    }
    void insert_multi(std::initializer_list<T> ilist) {
        insert_multi(ilist.begin(), ilist.end());
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace_unique(Args&&... args) {
        node_t* node = new node_t(std::forward<Args>(args)...);
        return insert_node_unique(node);
    }
    template <typename... Args>
    iterator emplace_hint_unique(const_iterator hint, Args&&... args) {
        node_t* node = new node_t(std::forward<Args>(args)...);
        node_t* hint_node = const_cast<node_t*>(hint.y);
        return insert_node_hint_unique(node, hint_node).first;
    }

    template <typename... Args>
    iterator emplace_multi(Args&&... args) {
        node_t* node = new node_t(std::forward<Args>(args)...);
        return insert_node_multi(node);
    }
    template <typename... Args>
    iterator emplace_hint_multi(const_iterator hint, Args&&... args) {
        node_t* node = new node_t(std::forward<Args>(args)...);
        node_t* hint_node = const_cast<node_t*>(hint.y);
        return insert_node_hint_multi(node, hint_node);
    }

    bool erase_unique(const Key& key) {
        node_t* y = find_node(key);
        if (y != head) {
            erase_node(y);
            return true;
        }
        return false;
    }
    size_t erase_multi(const Key& key) {
        std::pair<iterator, iterator> range = equal_range(key);
        size_t s = size();
        for (auto it = range.first; it != range.second;) {
            node_t* y = it.y;
            ++it;
            erase_node(y);
        }
        return s - size();
    }
    void erase(iterator pos) {
        assert(pos.y != head);
        erase_node(pos.y);
    }
    void erase(iterator first, iterator last) {
        for (auto it = first; it != last;) {
            erase(it++);
        }
    }
    template <typename Pred>
    friend size_type erase_if(bs_tree& bst, Pred pred) {
        auto s = bst.size();
        for (auto it = bst.begin(), last = bst.end(); it != last;) {
            if (pred(*it))
                it = bst.erase(it);
            else
                ++it;
        }
        return s - bst.size();
    }

    node_type extract(const_iterator pos) {
        assert(pos.y != head);
        node_t* y = const_cast<node_t*>(pos.y);
        yank_node(y);
        return node_type(y);
    }
    node_type extract(const Key& key) {
        node_t* y = find_node(key);
        if (y != head) {
            yank_node(y);
            return node_type(y);
        }
        return node_type();
    }

  protected:
    template <typename CmpFn2>
    void merge_unique(bs_tree<T, CmpFn2, tag>& src) {
        for (auto it = src.begin(); it != src.end();) {
            node_t* node = it.y;
            ++it;
            node_t* x = head->link[0];
            node_t* y = head;
            bool lesser = true;
            while (x) {
                lesser = compare(get_key(node->data), get_key(x->data));
                if (!lesser && !compare(get_key(x->data), get_key(node->data)))
                    goto skip;
                y = x;
                x = x->link[!lesser];
            }
            src.yank_node(node);
            insert_node(y, node, !lesser);
        skip:;
        }
    }
    template <typename CmpFn2>
    void merge_unique(bs_tree<T, CmpFn2, tag>&& src) {
        merge_unique(src);
    }
    template <typename CmpFn2>
    void merge_multi(bs_tree<T, CmpFn2, tag>& src) {
        for (auto it = src.begin(); it != src.end();) {
            node_t* node = it.y;
            ++it;
            assert(node && node->parent);
            node_t* x = head->link[0];
            node_t* y = head;
            bool lesser = true;
            while (x) {
                lesser = compare(get_key(node->data), get_key(x->data));
                y = x;
                x = x->link[!lesser];
            }
            src.yank_node(node);
            insert_node(y, node, !lesser);
        }
    }
    template <typename CmpFn2>
    void merge_multi(bs_tree<T, CmpFn2, tag>&& src) {
        merge_multi(src);
    }
};

template <typename BSTree>
struct bst_inserter_unique_iterator {
  private:
    using T = typename BSTree::value_type;
    BSTree* tree;

  public:
    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using reference = void;
    using pointer = void;
    using difference_type = void;
    using self_t = bst_inserter_unique_iterator;
    using container_type = BSTree;

    bst_inserter_unique_iterator(container_type& tree) : tree(&tree) {}

    self_t& operator*() {
        return *this;
    }
    self_t& operator++() {
        return *this;
    }
    self_t& operator++(int) {
        return *this;
    }
    self_t& operator=(const T& value) {
        tree->insert_unique(value);
        return *this;
    }
    self_t& operator=(T&& value) {
        tree->insert_unique(std::move(value));
        return *this;
    }
};

template <typename BSTree>
struct bst_inserter_multi_iterator {
  private:
    using T = typename BSTree::value_type;
    BSTree* tree;

  public:
    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using reference = void;
    using pointer = void;
    using difference_type = void;
    using self_t = bst_inserter_multi_iterator;
    using container_type = BSTree;

    bst_inserter_multi_iterator(container_type& tree) : tree(&tree) {}

    self_t& operator*() {
        return *this;
    }
    self_t& operator++() {
        return *this;
    }
    self_t& operator++(int) {
        return *this;
    }
    self_t& operator=(const T& value) {
        tree->insert_multi(value);
        return *this;
    }
    self_t& operator=(T&& value) {
        tree->insert_multi(std::move(value));
        return *this;
    }
};

template <typename BSTree>
bst_inserter_unique_iterator<BSTree> bst_inserter_unique(BSTree& tree) {
    return bst_inserter_unique_iterator<BSTree>(tree);
}

template <typename BSTree>
bst_inserter_multi_iterator<BSTree> bst_inserter_multi(BSTree& tree) {
    return bst_inserter_multi_iterator<BSTree>(tree);
}

#endif // BS_TREE_HPP
