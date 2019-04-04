#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

using std::vector;
using std::shared_ptr;
using std::make_shared;

class htree_element;
class htree_node;

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

shared_ptr< htree_node > construct_htree(
        vector< vector<int> > &tuples, vector<int> &num_buckets);

void construct_hsubtree(
        shared_ptr<htree_node> parent,
        vector< vector<int> >::iterator tuples_begin,
        vector< vector<int> >::iterator tuples_end,
        vector<int> &num_buckets, int attr_index);

class htree_element
{
public:

    int min;
    int max;
    shared_ptr< htree_node > child;

    htree_element(int min, int max, shared_ptr< htree_node > child) :
        min(min), max(max), child(child) {};

    void print(int indent_level = 0);

};

class htree_node
{
public:

    unsigned int level; // leaf is 0; root is N-1, where N is the number of attributes
    vector< htree_element > elements;

    htree_node(unsigned int level) : level(level) {};

    bool is_internal(); // level  > 0
    bool is_leaf(); // level == 0

    void insert(int min, int max, shared_ptr< htree_node > child) {
        elements.emplace_back(min, max, child);
    }

    // TODO: implement find()
    vector< htree_element > find(int min, int max) {
        vector< htree_element > results;
        return results;
    }

    void print(int indent_level = 0);
};

void htree_element::print(int indent_level) {
    std::string indent = "";
    for (int i = 0; i < indent_level; i++) {
        indent += "\t";
    }
    std::cout << indent << "[" << min << ", " << max << "]";
    if (child != nullptr) {
        std::cout << indent << "{\n";
        child->print(indent_level + 1);
        std::cout << indent << "}\n";
    } else {
        std::cout << "\n";
    }
}

void htree_node::print(int indent_level) {
    for (htree_element elt : elements) {
        elt.print(indent_level);
    }
}

shared_ptr< htree_node > construct_htree(
        vector< vector<int> > &tuples, vector<int> &num_buckets)
{
    if (tuples.size() == 0) {
        return nullptr;
    }
    // initialize root node
    shared_ptr< htree_node > root = make_shared< htree_node >(tuples[0].size() - 1);

    // recursive helper
    construct_hsubtree(root, tuples.begin(), tuples.end(), num_buckets, 0);

    return root;
}

void construct_hsubtree(
        shared_ptr<htree_node> parent,
        vector< vector<int> >::iterator tuples_begin,
        vector< vector<int> >::iterator tuples_end,
        vector<int> &num_buckets, int attr_index)
{
    //std::cout << "working on index: " << attr_index << "\n";
    unsigned int bucket_capacity =
        std::ceil((float) (tuples_end - tuples_begin) / num_buckets[attr_index]);
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
        if (parent->level > 0) {
            shared_ptr< htree_node > node = make_shared< htree_node >(
                parent->level - 1);
            parent->insert(min, max, node);
            construct_hsubtree(
                node, range_begin, range_end, num_buckets, attr_index + 1);
        } else {
            // leaf nodes point have null children
            parent->insert(min, max, nullptr);
        }
        range_begin = range_end;
    }
}


