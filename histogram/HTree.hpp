
#ifndef QUICKSTEP_HTREE_HTREE_HPP_
#define QUICKSTEP_HTREE_HTREE_HPP_

#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

#include "types/TypedValue.hpp"
#include "types/TypeID.hpp"

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

class HypedValue; // wrapper for TypedValue, used in the entries of the H-Tree.

/*
 * An htree_node contains a vector of htree_elements. An htree_element contains
 * a key, which is an interval. Leaf elements will contain the entire bucket,
 * which has each dimension defined by the key of an ancestor (or the leaf
 * element itself). */
template <typename T> class interval;
template <typename T> class bucket;
template <typename T> class htree_element;
template <typename T> class htree_node;

template <typename T>
double overlap_proportion(
    const interval<T> &h_interval, const interval<T> &query_interval);

template <typename T>
double overlap_proportion(const bucket<T> &h_bkt, const bucket<T> &query_bkt);

template <typename T>
double buckets_overlapped(
    const vector< shared_ptr< bucket<T> > > &h_bkts,
    const bucket<T> &query_bkt);

template <typename T>
shared_ptr< htree_node<T> > construct_htree(
    vector< vector<T> > &tuples, vector<int> &num_buckets);

template <typename T>
void construct_hsubtree(
    shared_ptr< htree_node<T> > parent,
    vector< interval<T> > &current_bkt,
    typename vector< vector<T> >::iterator tuples_begin,
    typename vector< vector<T> >::iterator tuples_end,
    vector<int> &num_buckets, int attr_index);

template <typename T>
T get_min_attr_value(
    typename vector< vector<T> >::iterator begin,
    typename vector< vector<T> >::iterator end,
    unsigned int attr_index);

template <typename T>
T get_max_attr_value(
    typename vector< vector<T> >::iterator begin,
    typename vector< vector<T> >::iterator end,
    unsigned int attr_index);

template <typename T>
interval<T> get_attr_interval(
    typename vector< vector<T> >::iterator begin,
    typename vector< vector<T> >::iterator end,
    unsigned int attr_index);



/*  _   _                      ___     __    _
 * | | | |_   _ _ __   ___  __| \ \   / /_ _| |_   _  ___
 * | |_| | | | | '_ \ / _ \/ _` |\ \ / / _` | | | | |/ _ \
 * |  _  | |_| | |_) |  __/ (_| | \ V / (_| | | |_| |  __/
 * |_| |_|\__, | .__/ \___|\__,_|  \_/ \__,_|_|\__,_|\___|
 *        |___/|_|
 *  FIGLET: HypedValue
 *
 *  A wrapper for the TypedValue class with comparison/arithmetic operators.
 */
class HypedValue {
    TypedValue v;

public:
    HypedValue(TypedValue v) : v(v) {}

    HypedValue(const HypedValue &h) : v(h.v) {}

    friend bool operator==(const HypedValue &h1, const HypedValue &h2);
    friend bool operator!=(const HypedValue &h1, const HypedValue &h2);
    friend bool operator<(const HypedValue &h1, const HypedValue &h2);
    friend bool operator<=(const HypedValue &h1, const HypedValue &h2);
    friend bool operator>(const HypedValue &h1, const HypedValue &h2);
    friend bool operator>=(const HypedValue &h1, const HypedValue &h2);
    /* shouldn't need this.
    friend double operator+(const HypedValue &h1, const HypedValue &h2);
    friend double operator-(const HypedValue &h1, const HypedValue &h2);
    */
    friend double width(const HypedValue &h1, const HypedValue &h2);
    friend std::ostream& operator<<(std::ostream &os, const HypedValue &h2);

};

bool operator==(const HypedValue &h1, const HypedValue &h2) {
    TypedValue v1 = h1.v;
    TypedValue v2 = h2.v;
    TypeID t = v1.getTypeID();
    assert(t == v2.getTypeID());
    switch(t) {
      case kInt:
        return v1.getLiteral<int>() == v2.getLiteral<int>();
      case kLong:
        return v1.getLiteral<std::int64_t>() == v2.getLiteral<std::int64_t>();
      case kFloat:
        return v1.getLiteral<float>() == v2.getLiteral<float>();
      case kDouble:
        return v1.getLiteral<double>() == v2.getLiteral<double>();
      case kDate:
        return v1.getLiteral<DateLit>() == v2.getLiteral<DateLit>();
      case kDatetime:
        return v1.getLiteral<DatetimeLit>() == v2.getLiteral<DatetimeLit>();
      case kDatetimeInterval:
        return v1.getLiteral<DatetimeIntervalLit>() ==
            v2.getLiteral<DatetimeIntervalLit>();
      case kYearMonthInterval:
        return v1.getLiteral<YearMonthIntervalLit>() ==
            v2.getLiteral<YearMonthIntervalLit>();
      default:
        return false; // unimplemented
    }
}

bool operator!=(const HypedValue &h1, const HypedValue &h2) {
    return !(h1 == h2);
}

bool operator<(const HypedValue &h1, const HypedValue &h2) {
    TypedValue v1 = h1.v;
    TypedValue v2 = h2.v;
    TypeID t = v1.getTypeID();
    assert(t == v2.getTypeID());
    switch(t) {
      case kInt:
        return v1.getLiteral<int>() < v2.getLiteral<int>();
      case kLong:
        return v1.getLiteral<std::int64_t>() < v2.getLiteral<std::int64_t>();
      case kFloat:
        return v1.getLiteral<float>() < v2.getLiteral<float>();
      case kDouble:
        return v1.getLiteral<double>() < v2.getLiteral<double>();
      case kDate:
        return v1.getLiteral<DateLit>() < v2.getLiteral<DateLit>();
      case kDatetime:
        return v1.getLiteral<DatetimeLit>() < v2.getLiteral<DatetimeLit>();
      case kDatetimeInterval:
        return v1.getLiteral<DatetimeIntervalLit>() <
            v2.getLiteral<DatetimeIntervalLit>();
      case kYearMonthInterval:
        return v1.getLiteral<YearMonthIntervalLit>() <
            v2.getLiteral<YearMonthIntervalLit>();
      default:
        return false; // unimplemented
    }
}

bool operator<=(const HypedValue &h1, const HypedValue &h2) {
    return h1 < h2 || h1 == h2;
}

bool operator>(const HypedValue &h1, const HypedValue &h2) {
    return !(h1 <= h2);
}

bool operator>=(const HypedValue &h1, const HypedValue &h2) {
    return h1 > h2 || h1 == h2;
}

/* Shouldn't need this.
double operator+(const HypedValue &h1, const HypedValue &h2) {
    TypedValue v1 = h1.v;
    TypedValue v2 = h2.v;
    TypeID t = v1.getTypeID();
    assert(t == v2.getTypeID());
    switch(t) {
      case kInt:
        return v1.getLiteral<int>() + v2.getLiteral<int>();
      case kFloat:
        return v1.getLiteral<float>() + v2.getLiteral<float>();
      case kLong:
        return v1.getLiteral<std::int64_t>() + v2.getLiteral<std::int64_t>();
      case kDouble:
        return v1.getLiteral<double>() + v2.getLiteral<double>();
      default:
        return 0.0; // unimplemented
    }
}

double operator-(const HypedValue &h1, const HypedValue &h2) {
    TypedValue v1 = h1.v;
    TypedValue v2 = h2.v;
    TypeID t = v1.getTypeID();
    assert(t == v2.getTypeID());
    switch(t) {
      case kInt:
        return v1.getLiteral<int>() - v2.getLiteral<int>();
      case kFloat:
        return v1.getLiteral<float>() - v2.getLiteral<float>();
      case kLong:
        return v1.getLiteral<std::int64_t>() - v2.getLiteral<std::int64_t>();
      case kDouble:
        return v1.getLiteral<double>() - v2.getLiteral<double>();
      default:
        return 0.0; // unimplemented
    }
}
*/

double width(const HypedValue &h1, const HypedValue &h2) {
    if (h1 > h2) {
        return 0.0;
    }
    TypedValue v1 = h1.v;
    TypedValue v2 = h2.v;
    TypeID t = v1.getTypeID();
    assert(t == v2.getTypeID());
    // For integer values, the interval [x,y] contains y-x+1 points.
    switch(t) {
        case kInt:
            return v2.getLiteral<int>() - v1.getLiteral<int>() + 1;
        case kLong:
            return v2.getLiteral<std::int64_t>() -
                v1.getLiteral<std::int64_t>() + 1;
        case kFloat:
            return v2.getLiteral<float>() - v1.getLiteral<float>();
        case kDouble:
            assert(1 == 0);
            return v2.getLiteral<double>() - v1.getLiteral<double>();
        default:
            return 0.0; // unimplemented
    }
}

std::ostream& operator<<(std::ostream &os, const HypedValue &h) {
    os << "HypedValue{";
    switch(h.v.getTypeID()) {
        case kInt:
            os << h.v.getLiteral<int>();
        case kLong:
            os << h.v.getLiteral<std::int64_t>();
        case kFloat:
            os << h.v.getLiteral<float>();
        case kDouble:
            os << h.v.getLiteral<double>();
        default: // unimplemented
            break;
    }
    os << "}";
    return os;
}

template <typename T>
HypedValue make_hyped_value(T value) {
    return HypedValue{TypedValue{static_cast<T>(value)}};
}


/*  _                _        _
 * | |__  _   _  ___| | _____| |_ ___
 * | '_ \| | | |/ __| |/ / _ \ __/ __|
 * | |_) | |_| | (__|   <  __/ |_\__ \
 * |_.__/ \__,_|\___|_|\_\___|\__|___/
 *  FIGLET: buckets
 */

// A range of T-values. If it is unbounded from the left or right, has_min and
// has_max will be set to false respectively.
template <typename T>
class interval
{
public:

    bool has_min, has_max;
    T min, max;

    interval(T min, T max) :
        has_min(true), has_max(true), min(min), max(max) {};

    interval(bool has_min, T min, bool has_max, T max) :
        has_min(has_min), has_max(has_max), min(min), max(max) {};

    bool contains(T val) const {
        return (!has_min || min <= val) && (!has_max || val <= max);
    }

};

template <typename T>
std::ostream& operator<<(std::ostream &os, const interval<T> i) {
    os << "[" << i.min << ", " << i.max << "]";
    return os;
}

template <typename T>
bool operator==(const interval<T> a, const interval<T> b) {
    return ((!a.has_min && !b.has_min) || (a.min == b.min)) &&
        ((!a.has_max && !b.has_max) || (a.max == b.max));
}

template <typename T>
bool operator!=(const interval<T> a, const interval<T> b) {
    return !(a == b);
}


template <typename T>
class bucket {
private:
    vector< interval<T> > dimensions;

public:

    bucket(vector< interval<T> > dimensions) : dimensions(dimensions) {}

    interval<T> operator[](const int index) const {
        return dimensions[index];
    }

    int size() const {
        return dimensions.size();
    }

    const vector< interval<T> > &get_dimensions() const {
        return dimensions;
    }
};

template <typename T>
bool operator==(const bucket<T> a, const bucket<T> b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (int i = 0; i < a.size(); i++) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

template <typename T>
bool operator!=(const bucket<T> a, const bucket<T> b) {
    return !(a == b);
}

// For int values, the interval [x,y] contains y-x+1 points.
int width(int x, int y) {
    return y - x + 1;
}

double width(double x, double y) {
    return y - x;
}

// Calculate the proportion |this union query_interval| / |this|.
template <typename T>
double overlap_proportion(
    const interval<T> &h_interval, const interval<T> &query_interval)
{
    // h_interval should be an interval in an htree, which should not be
    // boundless in either direction (however, query_interval may be
    // unbounded).
    assert(h_interval.has_min && h_interval.has_max);
    T left_bound = h_interval.min;
    if (query_interval.has_min) {
        left_bound = std::max(h_interval.min, query_interval.min);
    }
    T right_bound = h_interval.max;
    if (query_interval.has_max) {
        right_bound = std::min(h_interval.max, query_interval.max);
    }
    // No overlap
    if (left_bound > right_bound) {
        return 0.0;
    }
    // The bucket only includes one value, and that value is contained in the
    // query range.
    if (h_interval.min == h_interval.max) {
        return 1.0;
    }
    return (double) width(left_bound, right_bound) /
        width(h_interval.min, h_interval.max);
}

// Calculate the proportion |h_bkt union query_bkt| / |h_bkt|.
template <typename T>
double overlap_proportion(const bucket<T> &h_bkt, const bucket<T> &query_bkt) {
    double ratio = 1.0;
    for (int i = 0; i < h_bkt.size(); i++) {
        ratio *= overlap_proportion(h_bkt[i], query_bkt[i]);
    }
    return ratio;
}

// Used for selectivity/cardinality estimation. Estimate the number of buckets
// (including partial buckets) that overlap with the query range, assuming a
// uniform distribution within each bucket.
template <typename T>
double buckets_overlapped(
    const vector< shared_ptr< bucket<T> > > &h_bkts, const bucket<T> &query_bkt)
{
    double sum = 0.0;
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

template <typename T>
class htree_element
{
private:

    interval<T> key;
    // Internal elements point to a child node (null for leaf elements).
    shared_ptr< htree_node<T> > child;
    // Leaf elements hold a bounding bucket defined by the keys of all elements
    // along the path from the root (null for internal elements).
    shared_ptr< bucket<T> > bkt;

public:

    const interval<T>& get_key() const {
        return key;
    }

    const shared_ptr< htree_node<T> >& get_child() const {
        return child;
    }

    const shared_ptr< bucket<T> >& get_bucket() const {
        return bkt;
    }

    bool contains(T val) const {
        return key.contains(val);
    }

    // Internal element
    htree_element(interval<T> key, shared_ptr< htree_node<T> > child) :
        key(key), child(child), bkt(nullptr) {};

    // Leaf element
    htree_element(interval<T> key, shared_ptr< bucket<T> > bkt) :
        key(key), child(nullptr), bkt(bkt) {};

    void print(std::ostream &os, int indent_level = 0) const;

};

template <typename T>
class htree_node
{
private:
    // Leaf nodes are level 0; root is N-1, where N = number of attributes.
    unsigned int level;
    vector< htree_element<T> > elements;

    // Note that there may be multiple elements with the same bound of width 1,
    // e.g. if all of the values in the column are equal.
    typename vector< htree_element<T> >::iterator
    find_min_overlapping(interval<T> bounds) {
        assert(elements.size() > 0); // no node should be empty
        auto begin = elements.begin();
        T left_bound = bounds.min;
        // If left_bound <= first interval, simply return the first.
        if (bounds.has_min) {
            while(begin->get_key().max < left_bound) {
                begin++;
            }
        }
        return begin;
    }

    typename vector< htree_element<T> >::iterator
    find_max_overlapping(interval<T> bounds) {
        assert(elements.size() > 0); // no node should be empty
        auto end = elements.end() - 1;
        T right_bound = bounds.max;
        // If right_bound >= last interval, simply return the last.
        if (bounds.has_max) {
            while(right_bound < end->get_key().min) {
                end--;
            }
        }
        return end + 1; // keep exclusive end
    }

    // Recursive search helper
    void search_subhtree(
        const bucket<T> &query_range,
        unsigned int attr_index,
        vector< shared_ptr< bucket<T> > > &results)
    {
        // attr_index increases as level decreases.
        assert(attr_index + level + 1 == query_range.size());
        interval<T> bounds = query_range[attr_index];

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

    void insert(interval<T> key, shared_ptr< htree_node<T> > child) {
        elements.emplace_back(key, child);
    }

    void insert(interval<T> key, shared_ptr< bucket<T> > bkt)
    {
        elements.emplace_back(key, bkt);
    }

    // Returns a list of pointers to buckets that overlap with the query range.
    vector< shared_ptr< bucket<T> > > search(const bucket<T> &query_bkt) {
        assert(level == query_bkt.size() - 1); // must start search at root
        vector< shared_ptr< bucket<T> > > results;
        search_subhtree(query_bkt, 0 /* attr_index */, results);
        return results;
    }

    double estimateSelectivity(const bucket<T> &query_bkt) {
        return buckets_overlapped(search(query_bkt), query_bkt);
    }

    void print(std::ostream &os, int indent_level = 0) const;
};

template <typename T>
void htree_element<T>::print(std::ostream &os, int indent_level) const {
    std::string indent = "";
    for (int i = 0; i < indent_level; i++) {
        indent += "\t";
    }
    if (child != nullptr) {
        // Internal element
        os << indent << key << " {\n";
        child->print(os, indent_level + 1);
        os << indent << "}\n";
    } else {
        // Leaf element
        os << indent << "{";
        for (interval<T> i : bkt->get_dimensions()) {
            os << " " << i << ",";
        }
        os << " }\n";
    }
}

template <typename T>
void htree_node<T>::print(std::ostream &os, int indent_level) const {
    for (htree_element<T> elt : elements) {
        elt.print(os, indent_level);
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
template <typename T>
shared_ptr< htree_node<T> > construct_htree(
    vector< vector<T> > &tuples, vector<int> &num_buckets)
{
    if (tuples.size() == 0) {
        return nullptr;
    }
    // Initialize root node.
    shared_ptr< htree_node<T> > root =
        make_shared< htree_node<T> >(tuples[0].size() - 1);

    // Leaf nodes contain the keys of all ancestors, so we need to keep track
    // of the keys along the current path.
    vector< interval<T> > current_bkt;

    construct_hsubtree(
        root, current_bkt, tuples.begin(), tuples.end(), num_buckets, 0);

    return root;
}

// Recursive helper
template <typename T>
void construct_hsubtree(
        shared_ptr< htree_node<T> > node,
        vector< interval<T> > &current_bkt /* keys on current path */,
        typename vector< vector<T> >::iterator tuples_begin,
        typename vector< vector<T> >::iterator tuples_end,
        vector<int> &num_buckets /* number of buckets per dimension */,
        int attr_index)
{
    // Calculate the number of tuples to be represented within a bucket.
    unsigned int bucket_capacity = std::ceil(
        (float) (tuples_end - tuples_begin) / num_buckets[attr_index]);
    assert(bucket_capacity > 0);

    // Each loop adds an element to the given node. Each element represents a
    // partition of tuples with size <= bucket_capacity.
    typename vector< vector<T> >::iterator partition_begin = tuples_begin;
    while (partition_begin < tuples_end) {
        typename vector< vector<T> >::iterator partition_end =
            partition_begin + bucket_capacity;
        if (partition_end > tuples_end) {
            partition_end = tuples_end;
        }
        interval<T> key = get_attr_interval<T>(
            partition_begin, partition_end, attr_index);

        // Update current path.
        current_bkt.push_back(key);

        if (node->is_internal()) {
            // Add internal element.
            shared_ptr< htree_node<T> > child = make_shared< htree_node<T> >(
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
            shared_ptr< bucket<T> > bkt = make_shared< bucket<T> >(current_bkt);
            node->insert(key, bkt);
        }

        // Roll back current path.
        current_bkt.pop_back();

        partition_begin = partition_end;
    }
}

// Find the minimum value of the specified attribute.
template <typename T>
T get_min_attr_value(
    typename vector< vector<T> >::iterator begin,
    typename vector< vector<T> >::iterator end,
    unsigned int attr_index)
{
    assert(begin < end);
    T min = (*begin)[attr_index];
    while (++begin < end) {
        min = std::min(min, (*begin)[attr_index]);
    }
    return min;
}

// Find the maximum value of the specified attribute.
template <typename T>
T get_max_attr_value(
    typename vector< vector<T> >::iterator begin,
    typename vector< vector<T> >::iterator end,
    unsigned int attr_index)
{
    assert(begin < end);
    T max = (*begin)[attr_index];
    while (++begin < end) {
        max = std::max(max, (*begin)[attr_index]);
    }
    return max;
}

// Get the range of all values for the given attribute.
template <typename T>
interval<T> get_attr_interval(
    typename vector< vector<T> >::iterator begin,
    typename vector< vector<T> >::iterator end,
    unsigned int attr_index)
{
    T key_min = get_min_attr_value<T>(begin, end, attr_index);
    T key_max = get_max_attr_value<T>(begin, end, attr_index);
    return { key_min, key_max };
}


} // namespace quickstep
#endif  // QUICKSTEP_HTREE_HTREE_HPP_
