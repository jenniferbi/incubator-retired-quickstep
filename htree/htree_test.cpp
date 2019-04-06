#include <vector>
#include "htree.hpp"

using std::vector;

int main() {
    std::cout << "\n\n\nTESTING CONSTRUCTION...\n";

    int num_attrs = 3;
    vector< vector<int> > tuples;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            for (int k = 0; k < 6; k++) {
                vector<int> tuple;
                tuple.push_back(i);
                tuple.push_back(j);
                tuple.push_back(k);
                tuples.push_back(tuple);
            }
        }
    }

    int bucket_total = 1;
    vector<int> num_buckets;
    for (int i = 0; i < num_attrs; i++) {
        int count = 3;
        num_buckets.push_back(count);
        bucket_total *= count;
    }
    auto p = construct_htree(tuples, num_buckets);
    p->print();

    std::cout << "\n\n\nTESTING SEARCH...\n";

    vector<interval> query_range;
    query_range.emplace_back(3, 4);
    query_range.emplace_back(2, 4);
    query_range.emplace_back(false, 1, false, 0); // no bounds on third attr

    auto results = p->search(query_range);
    for (shared_ptr< vector<interval> > rect : results) {
        std::cout << "{";
        for (interval i : *rect) {
            std::cout << " " << i << ", ";
        }
        std::cout << "}\n";
    }
    float bucket_count = buckets_overlapped(results, query_range);
    std::cout << "NUMBER OF HITS = " << tuples.size() << " / " <<
        bucket_total << " * " << bucket_count << " = " <<
        (int) ((float) tuples.size() / bucket_total * bucket_count) << "\n";
    return 0;
}
