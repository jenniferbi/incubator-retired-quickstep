
#ifndef QUICKSTEP_HTREE_HTREE_HPP_
#define QUICKSTEP_HTREE_HTREE_HPP_

#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

/*  _   _     _____
 * | | | |   |_   _| __ ___  ___
 * | |_| |_____| || '__/ _ \/ _ \
 * |  _  |_____| || | |  __/  __/
 * |_| |_|     |_||_|  \___|\___|
 *  FIGLET: H-Tree
 *
 */

namespace quickstep {

using std::vector;
using std::shared_ptr;
using std::make_shared;

/*
 * An htree_node contains a vector of htree_elements. An htree_element contains
 * a key, which is an integer interval. Leaf elements will contain the entire
 * bucket, which has each dimension defined by the key of an ancestor (or the
 * leaf element itself). */
class interval;
typedef vector<interval> bucket;
class htree_element;
class htree_node;

float overlap_proportion(
    const interval &h_interval, const interval &query_interval);
float overlap_proportion(const bucket &h_bkt, const bucket &query_bkt);
float buckets_overlapped(
    const vector< shared_ptr<bucket> > &h_bkts, const bucket &query_bkt);

shared_ptr<htree_node> construct_htree(
    vector< vector<int> > &tuples, vector<int> &num_buckets);

void construct_hsubtree(
    shared_ptr<htree_node> parent,
    vector<interval> &current_bkt,
    vector< vector<int> >::iterator tuples_begin,
    vector< vector<int> >::iterator tuples_end,
    vector<int> &num_buckets, int attr_index);

int get_min_attr_value(
    vector< vector<int> >::iterator begin,
    vector< vector<int> >::iterator end,
    unsigned int attr_index);

int get_min_attr_value(
    vector< vector<int> >::iterator begin,
    vector< vector<int> >::iterator end,
    unsigned int attr_index);

interval get_attr_interval(
    vector< vector<int> >::iterator begin,
    vector< vector<int> >::iterator end,
    unsigned int attr_index);


/*  _                _        _
 * | |__  _   _  ___| | _____| |_ ___
 * | '_ \| | | |/ __| |/ / _ \ __/ __|
 * | |_) | |_| | (__|   <  __/ |_\__ \
 * |_.__/ \__,_|\___|_|\_\___|\__|___/
 *  FIGLET: buckets
 */

// An integer range. If it is unbounded from the left or right, has_min and
// has_max will be set to false respectively.
class interval
{
public:

    bool has_min, has_max;
    int min, max;

    interval(int min, int max) :
        has_min(true), has_max(true), min(min), max(max) {};

    interval(bool has_min, int min, bool has_max, int max) :
        has_min(has_min), has_max(has_max), min(min), max(max) {};

    bool contains(int val) const {
        return (!has_min || min <= val) && (!has_max || val <= max);
    }

};

std::ostream& operator<<(std::ostream &os, const interval i) {
    os << "[" << i.min << ", " << i.max << "]";
    return os;
}

// Calculate the proportion |this union query_interval| / |this|.
float overlap_proportion(
    const interval &h_interval, const interval &query_interval)
{
    // h_interval should be an interval in an htree, which should not be
    // boundless in either direction (however, query_interval may be
    // unbounded).
    assert(h_interval.has_min && h_interval.has_max);
    int left_bound = h_interval.min;
    if (query_interval.has_min) {
        left_bound = std::max(h_interval.min, query_interval.min);
    }
    int right_bound = h_interval.max;
    if (query_interval.has_max) {
        right_bound = std::min(h_interval.max, query_interval.max);
    }
    // No overlap
    if (right_bound - left_bound < 0) {
        return 0.0;
    }
    // We assume int values, so the interval [x,y] contains y-x+1 points.
    return (float) (right_bound - left_bound + 1) /
        (h_interval.max - h_interval.min + 1);
}

// Calculate the proportion |h_bkt union query_bkt| / |h_bkt|.
float overlap_proportion(const bucket &h_bkt, const bucket &query_bkt) {
    float ratio = 1.0;
    for (int i = 0; i < h_bkt.size(); i++) {
        ratio *= overlap_proportion(h_bkt[i], query_bkt[i]);
    }
    return ratio;
}

// Used for selectivity/cardinality estimation. Estimate the number of buckets
// (including partial buckets) that overlap with the query range, assuming a
// uniform distribution within each bucket.
float buckets_overlapped(
    const vector< shared_ptr<bucket> > &h_bkts, const bucket &query_bkt)
{
    float sum = 0.0;
    for (int i = 0; i < h_bkts.size(); i++) {
        sum += overlap_proportion(*h_bkts[i], query_bkt);
    }
    return sum;
}


/*                  _
 *  _ __   ___   __| | ___  ___
 * | '_ \ / _ \ / _` |/ _ \/ __|
 * | | | | (_) | (_| |  __/\__ \
 * |_| |_|\___/ \__,_|\___||___/
 *  FIGLET: nodes
 */

class htree_element
{
private:

    interval key;
    // Internal elements point to a child node (null for leaf elements).
    shared_ptr<htree_node> child;
    // Leaf elements hold a bounding bucket defined by the keys of all elements
    // along the path from the root (null for internal elements).
    shared_ptr<bucket> bkt;

public:

    const interval& get_key() const {
        return key;
    }

    const shared_ptr<htree_node>& get_child() const {
        return child;
    }

    const shared_ptr<bucket>& get_bucket() const {
        return bkt;
    }

    bool contains(int val) const {
        return key.contains(val);
    }

    // internal element
    htree_element(interval key, shared_ptr<htree_node> child) :
        key(key), child(child), bkt(nullptr) {};

    // leaf element
    htree_element(interval key, shared_ptr<bucket> bkt) :
        key(key), child(nullptr), bkt(bkt) {};

    void print(int indent_level = 0) const;

};

class htree_node
{
private:
    // Leaf nodes are level 0; root is N-1, where N = number of attributes.
    unsigned int level;
    vector<htree_element> elements;

    // Note that there may be multiple elements with the same bound of width 1,
    // e.g. if all of the values in the column are equal.
    vector<htree_element>::iterator find_min_overlapping(interval bounds) {
        assert(elements.size() > 0); // no node should be empty
        auto begin = elements.begin();
        int left_bound = bounds.min;
        // If left_bound <= first interval, simply return the first.
        if (bounds.has_min && begin->get_key().min < left_bound) {
            while(!begin->contains(left_bound)) {
                begin++;
            }
        }
        return begin;
    }

    vector<htree_element>::iterator find_max_overlapping(interval bounds) {
        assert(elements.size() > 0); // no node should be empty
        auto end = elements.end() - 1;
        int right_bound = bounds.max;
        // If right_bound >= last interval, simply return the last.
        if (bounds.has_max && right_bound < end->get_key().max) {
            while(!end->contains(right_bound)) {
                end--;
            }
        }
        return end + 1; // keep exclusive end
    }

    // Recursive search helper
    void search_subhtree(
        const bucket &query_range,
        unsigned int attr_index,
        vector< shared_ptr<bucket> > &results)
    {
        // attr_index increases as level decreases.
        assert(attr_index + level + 1 == query_range.size());
        interval bounds = query_range[attr_index];

        // Get "slice" of overlapping htree_elements.
        auto begin = find_min_overlapping(bounds);
        auto end = find_max_overlapping(bounds);

        while (begin < end) {
            if (is_leaf()) {
                results.push_back(begin->get_bucket());
            } else {
                begin->get_child()->search_subhtree(
                    query_range, attr_index + 1, results);
            }
            begin++;
        }
    }

public:

    htree_node(unsigned int level) : level(level) {};

    bool is_internal() const {
        return level > 0;
    }

    bool is_leaf() const {
        return level == 0;
    }

    unsigned int get_level() const {
        return level;
    }

    void insert(interval key, shared_ptr<htree_node> child) {
        elements.emplace_back(key, child);
    }

    void insert(interval key, shared_ptr<bucket> bkt)
    {
        elements.emplace_back(key, bkt);
    }

    // Returns a list of pointers to buckets that overlap with the query range.
    vector< shared_ptr<bucket> > search(const bucket &query_range) {
        assert(level == query_range.size() - 1); // must start search at root
        vector< shared_ptr<bucket> > results;
        search_subhtree(query_range, 0 /* attr_index */, results);
        return results;
    }

    void print(int indent_level = 0) const;
};

void htree_element::print(int indent_level) const {
    std::string indent = "";
    for (int i = 0; i < indent_level; i++) {
        indent += "\t";
    }
    if (child != nullptr) {
        // Internal element
        std::cout << indent << key << " {\n";
        child->print(indent_level + 1);
        std::cout << indent << "}\n";
    } else {
        // Leaf element
        std::cout << indent << "{";
        for (interval i : *bkt) {
            std::cout << " " << i << ", ";
        }
        std::cout << "}\n";
    }
}

void htree_node::print(int indent_level) const {
    for (htree_element elt : elements) {
        elt.print(indent_level);
    }
}

/*  _     _
 * | |__ | |_ _ __ ___  ___
 * | '_ \| __| '__/ _ \/ _ \
 * | | | | |_| | |  __/  __/
 * |_| |_|\__|_|  \___|\___|
 *                      _                   _   _
 *   ___ ___  _ __  ___| |_ _ __ _   _  ___| |_(_) ___  _ __
 *  / __/ _ \| '_ \/ __| __| '__| | | |/ __| __| |/ _ \| '_ \
 * | (_| (_) | | | \__ \ |_| |  | |_| | (__| |_| | (_) | | | |
 *  \___\___/|_| |_|___/\__|_|   \__,_|\___|\__|_|\___/|_| |_|
 *  FIGLET: htree construction
 */

// Given a vector of tuples and the number of buckets for each dimension,
// construct an h-tree.
shared_ptr<htree_node> construct_htree(
    vector< vector<int> > &tuples, vector<int> &num_buckets)
{
    if (tuples.size() == 0) {
        return nullptr;
    }
    // Initialize root node.
    shared_ptr<htree_node> root = make_shared<htree_node>(tuples[0].size() - 1);

    // Leaf nodes contain the keys of all ancestors, so we need to keep track
    // of the keys along the current path.
    vector<interval> current_bkt;

    construct_hsubtree(
        root, current_bkt, tuples.begin(), tuples.end(), num_buckets, 0);

    return root;
}

// Recursive helper
void construct_hsubtree(
        shared_ptr<htree_node> node,
        vector<interval> &current_bkt /* keys on current path */,
        vector< vector<int> >::iterator tuples_begin,
        vector< vector<int> >::iterator tuples_end,
        vector<int> &num_buckets /* number of buckets per dimension */,
        int attr_index)
{
    // Calculate the number of tuples to be represented within a bucket.
    unsigned int bucket_capacity = std::ceil(
        (float) (tuples_end - tuples_begin) / num_buckets[attr_index]);
    assert(bucket_capacity > 0);

    // Each loop adds an element to the given node. Each element represents a
    // partition of tuples with size <= bucket_capacity.
    vector< vector<int> >::iterator partition_begin = tuples_begin;
    while (partition_begin < tuples_end) {
        vector< vector<int> >::iterator partition_end =
            partition_begin + bucket_capacity;
        if (partition_end > tuples_end) {
            partition_end = tuples_end;
        }
        interval key = get_attr_interval(
            partition_begin, partition_end, attr_index);

        // Update current path.
        current_bkt.push_back(key);

        if (node->is_internal()) {
            // Add internal element.
            shared_ptr<htree_node> child = make_shared<htree_node>(
                node->get_level() - 1);
            node->insert(key, child);
            // Construct subtree. The child nodes will further partition the
            // current range of tuples.
            construct_hsubtree(
                child,
                current_bkt,
                partition_begin,
                partition_end,
                num_buckets,
                attr_index + 1);
        } else {
            // Add leaf element.
            shared_ptr<bucket> bkt = make_shared<bucket>(current_bkt);
            node->insert(key, bkt);
        }

        // Roll back current path.
        current_bkt.pop_back();

        partition_begin = partition_end;
    }
}

// Find the minimum value of the specified attribute.
int get_min_attr_value(
    vector< vector<int> >::iterator begin,
    vector< vector<int> >::iterator end,
    unsigned int attr_index)
{
    assert(begin < end);
    int min = (*begin)[attr_index];
    while (++begin < end) {
        min = std::min(min, (*begin)[attr_index]);
    }
    return min;
}

// Find the maximum value of the specified attribute.
int get_max_attr_value(
    vector< vector<int> >::iterator begin,
    vector< vector<int> >::iterator end,
    unsigned int attr_index)
{
    assert(begin < end);
    int max = (*begin)[attr_index];
    while (++begin < end) {
        max = std::max(max, (*begin)[attr_index]);
    }
    return max;
}

// Get the range of all values for the given attribute.
interval get_attr_interval(
    vector< vector<int> >::iterator begin,
    vector< vector<int> >::iterator end,
    unsigned int attr_index)
{
    int key_min = get_min_attr_value(begin, end, attr_index);
    int key_max = get_max_attr_value(begin, end, attr_index);
    return { key_min, key_max };
}


} // namespace quickstep
#endif  // QUICKSTEP_HTREE_HTREE_HPP_
