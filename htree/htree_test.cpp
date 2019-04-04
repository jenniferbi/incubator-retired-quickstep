#include <vector>
#include "htree.hpp"

using std::vector;

int main() {
    int num_attrs = 3;
    vector< vector<int> > tuples;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                vector<int> tuple;
                tuple.push_back(i);
                tuple.push_back(j);
                tuple.push_back(k);
                tuples.push_back(tuple);
            }
        }
    }

    vector<int> num_buckets;
    for (int i = 0; i < num_attrs; i++) {
            num_buckets.push_back(3);
    }
    auto p = construct_htree(tuples, num_buckets);
    p->print();
    return 0;
}
