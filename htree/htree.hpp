#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

using std::vector;
using std::shared_ptr;
using std::make_shared;

class interval;
typedef vector<interval> bucket;

class htree_element;
class htree_node;

float overlap_proportion(const bucket &h_bkt, const bucket &query_bkt);
float buckets_overlapped(
    const vector< shared_ptr<bucket> > &h_bkts, const bucket &query_bkt);

int get_min_attr_value(
    vector< vector<int> >::iterator begin,
    vector< vector<int> >::iterator end,
    unsigned int attr_index);

int get_min_attr_value(
    vector< vector<int> >::iterator begin,
    vector< vector<int> >::iterator end,
    unsigned int attr_index);

shared_ptr<htree_node> construct_htree(
        vector< vector<int> > &tuples, vector<int> &num_buckets);

void construct_hsubtree(
        shared_ptr<htree_node> parent,
        vector<interval> &current_bkt,
        vector< vector<int> >::iterator tuples_begin,
        vector< vector<int> >::iterator tuples_end,
        vector<int> &num_buckets, int attr_index);

class interval
{
public:

    bool has_min;
    int min;
    bool has_max;
    int max;

    interval(int min, int max) :
        has_min(true), min(min), has_max(true), max(max) {};

    interval(bool has_min, int min, bool has_max, int max) :
        has_min(has_min), min(min), has_max(has_max), max(max) {};

    bool contains(int val) const {
        return (!has_min || min <= val) && (!has_max || val <= max);
    }

    // Assuming uniform distribution within the interval, calculate the
    // proportion of overlap area to total area.
    float overlap_proportion(interval i) const {
        // This function should only be used on intervals in the htree, which
        // should not be boundless in either dibktion.
        assert(has_min && has_max);
        int left_bound = min;
        if (i.has_min) {
            left_bound = std::max(min, i.min);
        }
        int right_bound = max;
        if (i.has_max) {
            right_bound = std::min(max, i.max);
        }
        // if no overlap
        if (right_bound - left_bound < 0) {
            return 0.0;
        }
        // We assume int values, so the interval [x,y] contains y-x+1 points.
        return (float) (right_bound - left_bound + 1) / (max - min + 1);
    }
};

float overlap_proportion(const bucket &h_bkt, const bucket &query_bkt) {
    float ratio = 1.0;
    for (int i = 0; i < h_bkt.size(); i++) {
        ratio *= h_bkt[i].overlap_proportion(query_bkt[i]);
    }
    return ratio;
}

float buckets_overlapped(
    const vector< shared_ptr<bucket> > &h_bkts, const bucket &query_bkt)
{
    float count = 0.0;
    for (int i = 0; i < h_bkts.size(); i++) {
        count += overlap_proportion(*h_bkts[i], query_bkt);
    }
    return count;
}

std::ostream& operator<<(std::ostream &os, const interval i) {
    os << "[" << i.min << ", " << i.max << "]";
    return os;
}

class htree_element
{
private:

    interval bounds;
    shared_ptr< bucket > bkt; // used in leaf nodes
    shared_ptr<htree_node> child;

public:

    const interval& get_bounds() const {
        return bounds;
    }

    const shared_ptr< bucket >& get_bucket() const {
        return bkt;
    }

    const shared_ptr<htree_node>& get_child() const {
        return child;
    }

    bool contains(int val) const {
        return bounds.contains(val);
    }

    htree_element(interval i, shared_ptr<htree_node> child) :
        bounds(i), child(child), bkt(nullptr) {};

    htree_element( interval i, shared_ptr<htree_node> child,
            shared_ptr< bucket > bkt) :
        bounds(i), child(child), bkt(bkt) {};

    void print(int indent_level = 0) const;

};

class htree_node
{
private:
    // leaf nodes are level 0; root is N-1, where N = number of attributes
    unsigned int level;
    vector<htree_element> elements;

    // Note that there may be multiple elements with the same bound of width 1,
    // e.g. if all of the values in the column are equal.
    vector<htree_element>::iterator find_min_overlapping(interval i) {
        assert(elements.size() > 0); // no node should be empty
        auto begin = elements.begin();
        int left_bound = i.min;
        // if left_bound <= first interval, simply return the first
        if (i.has_min && begin->get_bounds().min < left_bound) {
            while(!begin->contains(left_bound)) {
                begin++;
            }
        }
        return begin;
    }

    vector<htree_element>::iterator find_max_overlapping(interval i) {
        assert(elements.size() > 0); // no node should be empty
        auto end = elements.end() - 1;
        int right_bound = i.max;
        // if right_bound >= last interval, simply return the last
        if (i.has_max && right_bound < end->get_bounds().max) {
            while(!end->contains(right_bound)) {
                end--;
            }
        }
        return end + 1; // keep exclusive end
    }

    void search_subhtree(
        const bucket &query_range,
        unsigned int attr_index,
        vector< shared_ptr< bucket > > &results)
    {
        assert(attr_index + level + 1 == query_range.size());
        interval bounds = query_range[attr_index];
        auto begin = find_min_overlapping(bounds);
        auto end = find_max_overlapping(bounds);
        while (begin < end) {
            if (level == 0) {
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

    void insert(interval i, shared_ptr<htree_node> child) {
        elements.emplace_back(i, child);
    }

    void insert(interval i, shared_ptr<htree_node> child,
        shared_ptr< bucket > bkt)
    {
        elements.emplace_back(i, child, bkt);
    }

    // returns a list of pointers to buckets (interval vectors)
    vector< shared_ptr <bucket > > search(
        const bucket &query_range);

    void print(int indent_level = 0) const;
};

vector< shared_ptr <bucket > > htree_node::search(
    const bucket &query_range)
{
    assert(level == query_range.size() - 1); // must start search at root
    vector< shared_ptr <bucket > > results;
    search_subhtree(query_range, 0, results);
    return results;
}

void htree_element::print(int indent_level) const {
    std::string indent = "";
    for (int i = 0; i < indent_level; i++) {
        indent += "\t";
    }
    if (child != nullptr) {
        std::cout << indent << bounds;
        std::cout << indent << "{\n";
        child->print(indent_level + 1);
        std::cout << indent << "}\n";
    } else {
        std::cout << indent << "{";
        // bkt is nullptr except in leaf elements
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

shared_ptr<htree_node> construct_htree(
        vector< vector<int> > &tuples, vector<int> &num_buckets)
{
    if (tuples.size() == 0) {
        return nullptr;
    }
    // initialize root node
    shared_ptr<htree_node> root = make_shared<htree_node>(tuples[0].size() - 1);

    // track bounds of the current path:
    // leaf nodes contain the dimensions of all ancestors
    vector<interval> current_bkt;

    // recursive helper
    construct_hsubtree(
        root, current_bkt, tuples.begin(), tuples.end(), num_buckets, 0);

    return root;
}

void construct_hsubtree(
        shared_ptr<htree_node> node,
        vector<interval> &current_bkt,
        vector< vector<int> >::iterator tuples_begin,
        vector< vector<int> >::iterator tuples_end,
        vector<int> &num_buckets, int attr_index)
{
    //std::cout << "working on index: " << attr_index << "\n";
    unsigned int bucket_capacity = std::ceil(
        (float) (tuples_end - tuples_begin) / num_buckets[attr_index]);
    //std::cout << "bucket cap: " << bucket_capacity << "\n";
    assert(bucket_capacity > 0);

    vector< vector<int> >::iterator range_begin = tuples_begin;
    while (range_begin < tuples_end) {
        vector< vector<int> >::iterator range_end =
            range_begin + bucket_capacity;
        if (range_end > tuples_end) {
            range_end = tuples_end;
        }
        int min = get_min_attr_value(range_begin, range_end, attr_index);
        int max = get_max_attr_value(range_begin, range_end, attr_index);
        interval bounds{ min, max };
        current_bkt.push_back(bounds);
        if (node->is_internal()) {
            shared_ptr<htree_node> child = make_shared<htree_node>(
                node->get_level() - 1);
            node->insert(bounds, child);
            construct_hsubtree(child, current_bkt, range_begin, range_end,
                num_buckets, attr_index + 1);
        } else {
            // leaf nodes have null children
            shared_ptr< bucket > bkt =
                make_shared< bucket >(current_bkt);
            node->insert(bounds, nullptr, bkt);
        }
        current_bkt.pop_back();
        range_begin = range_end;
    }
}


