#include <vector>
#include <cmath>
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "gtest/gtest.h"
#include "histogram/HTree.hpp"

namespace quickstep {

using std::vector;

template <typename T>
class mock_htree_node {
public:
    interval<T> key;
    vector< mock_htree_node<T> > children;
    vector< interval<T> > leaf_elements;

    mock_htree_node(interval<T> key, vector< mock_htree_node<T> > children) :
        key(key), children(children) {}

    mock_htree_node(interval<T> key, vector< interval<T> > leaf_elements) :
        key(key), leaf_elements(leaf_elements) {}

    void print(std::ostream &os, unsigned int level=0) {
        std::string indent = "";
        for (int i = 0; i < level; i++) {
            indent += "\t";
        }
        // if leaf
        if (children.size() == 0) {
            os << indent << "{ ";
            for (interval<T> range : leaf_elements) {
                os << range << ", ";
            }
            os << "}\n";
        } else {
            os << indent << key << " {\n";
            for (mock_htree_node<T> child : children) {
                child.print(os, level + 1);
            }
            os << indent << "}\n";
        }
    }
};

vector< vector<int> > generate_uniform_tuples(
    int num_attrs, vector<int> dimensions)
{
    unsigned long num_tuples = 1;
    for (int d : dimensions) {
        num_tuples *= d;
    }
    vector< vector<int> > tuples;
    for (unsigned long i = 0; i < num_tuples; i++) {
        vector<int> tuple;
        unsigned divisor = num_tuples;
        for (int j = 0; j < num_attrs; j++) {
            int length = dimensions[j];
            divisor /= length;
            int attr_j = (int) (i / divisor) % length;
            tuple.push_back(attr_j);
        }
        tuples.push_back(tuple);
    }
    return tuples;
}

template <typename T>
vector< vector<HypedValue> >
generate_hyped_values(vector< vector<int> > &tuples) {
    vector< vector<HypedValue> > hyped_tuples;
    for (vector<int> tuple : tuples) {
        vector<HypedValue> row;
        for (int x : tuple) {
            row.push_back(HypedValue{ TypedValue{static_cast<T>(x)} });
        }
        hyped_tuples.push_back(row);
    }
    return hyped_tuples;
}

template <typename T>
class compare {

    int attr_index;

public:
    compare(int attr_index) : attr_index(attr_index) {}

    bool operator() (vector<T> u, vector<T> v) {
        return u[attr_index] < v[attr_index];
    }
};

template <typename T>
void sort_tuples(
    typename vector< vector<T> >::iterator begin,
    typename vector< vector<T> >::iterator end,
    vector<int> &num_buckets, int attr_index)
{
    if (attr_index == num_buckets.size()) return;

    std::sort(begin, end, compare<T>{attr_index});
    unsigned int bucket_size =
        std::ceil((float) (end - begin) / num_buckets[attr_index]);
    typename vector< vector<T> >::iterator range_begin = begin;
    while (range_begin < end) {
        typename vector< vector<T> >::iterator range_end =
            range_begin + bucket_size;
        if (range_end > end) {
            range_end = end;
        }
        sort_tuples<T>(range_begin, range_end, num_buckets, attr_index + 1);
        range_begin = range_end;
    }
}

template <typename T>
void sort_tuples(vector< vector<T> > &tuples, vector<int> &num_buckets) {
    sort_tuples<T>(tuples.begin(), tuples.end(), num_buckets, 0);
}

template <typename T>
void test_overlap_calculation(
    float expected, bucket<T> query, vector< bucket<T> > hits)
{
    vector< shared_ptr< bucket<T> > > ptrs;
    for (bucket<T> bkt : hits) {
        shared_ptr< bucket<T> > p = std::make_shared< bucket<T> >(bkt);
        ptrs.push_back(p);
    }
    EXPECT_EQ(expected, buckets_overlapped(ptrs, query));
}

template <typename T>
void search_test(
    shared_ptr< htree_node<T> > tree,
    bucket<T> query,
    vector< bucket<T> > expected_hits)
{
    vector< shared_ptr< bucket<T> > > hits = tree->search(query);
    EXPECT_TRUE(hits.size() == expected_hits.size());
    for (auto bkt : hits) {
        auto it = std::find(expected_hits.begin(), expected_hits.end(), *bkt);
        EXPECT_TRUE(it != expected_hits.end());
    }
}

template <typename T>
void construction_test(
    vector< vector<T> > tuples, vector<int> num_buckets,
    vector< mock_htree_node<T> > expected_tree)
{
    std::ostringstream result_stream;
    std::ostringstream expected_stream;

    sort_tuples(tuples, num_buckets);
    auto htree = construct_htree(tuples, num_buckets);
    htree->print(result_stream);

    for (auto node : expected_tree) {
        node.print(expected_stream);
    }

    std::string result = result_stream.str();
    std::string expected = expected_stream.str();
    bool pass = result.compare(expected) == 0;
    if (!pass) {
        std::cout << "RESULT:\n" << result << "\n";
        std::cout << "EXPECTED:\n" << expected << "\n";
    }
    EXPECT_TRUE(pass);
}

TEST(HTreeTest, HTreeTest_Overlap_Basic) {
    test_overlap_calculation(3.0,
        bucket<int>{ { {0, 1}, {0, 1}, {false, -1, false, -1} } },
        {
            { { {0, 0}, {0, 0}, {0, 0} } },
            { { {0, 0}, {1, 1}, {3, 3} } },
            { { {1, 1}, {0, 0}, {7, 7} } },
            { { {2, 2}, {0, 0}, {0, 0} } } // does not overlap
        });
}

TEST(HTreeTest, HTreeTest_Overlap_Partial) {
    test_overlap_calculation(1.75,
        bucket<int>{ { {0, 1}, {0, 1}, {false, -1, false, -1} } },
        {
            { { {-1, 0}, {0, 0}, {0, 0} } }, // overlap = .5
            { { {-1, 0}, {1, 2}, {3, 3} } }, // overlap = .25
            { { {1, 1}, {0, 0}, {7, 7} } }, // overlap = 1
            { { {2, 2}, {0, 0}, {0, 0} } } // does not overlap
        });
}

TEST(HTreeTest, HTreeTest_Search_Basic) {
    vector< vector<int> > tuples = generate_uniform_tuples(3, { 2, 2, 2 });
    vector<int> num_buckets = { 2, 2, 2 };
    auto htree = construct_htree(tuples, num_buckets);
    search_test(htree, { { {0, 1}, {0, 0}, {1, 1} } }, {
        { { {0, 0}, {0, 0}, {1, 1} } },
        { { {1, 1}, {0, 0}, {1, 1} } },
    });
}

TEST(HTreeTest, HTreeTest_Search_Unbounded_Query) {
    vector< vector<int> > tuples = generate_uniform_tuples(3, { 2, 2, 2 });
    vector<int> num_buckets = { 2, 2, 2 };
    auto htree = construct_htree(tuples, num_buckets);
    search_test(htree, { { { false, -1, false, -1}, {0, 0}, {1, 1} } }, {
        { { {0, 0}, {0, 0}, {1, 1} } },
        { { {1, 1}, {0, 0}, {1, 1} } },
    });
}

TEST(HTreeTest, HTreeTest_Search_No_Hits) {
    vector< vector<int> > tuples = generate_uniform_tuples(3, { 2, 2, 2 });
    vector<int> num_buckets = { 2, 2, 2 };
    auto htree = construct_htree(tuples, num_buckets);
    search_test(htree, { { {2, 2}, {0, 1}, {0, 1} } }, {} );
}

TEST(HTreeTest, HTreeTest_Search_Partial_Overlap) {
    vector< vector<int> > tuples = generate_uniform_tuples(3, { 3, 3, 3 });
    vector<int> num_buckets = { 1, 1, 1 };
    auto htree = construct_htree(tuples, num_buckets);
    search_test(htree, { { {1, 1}, {1, 1}, {1, 1} } }, {
        { { {0, 2}, {0, 2}, {0, 2} } },
    });
}

TEST(HTreeTest, HTreeTest_Hyped_Tree_Long) {
    vector< vector<int> > tuples = generate_uniform_tuples(3, { 3, 3, 3 });
    vector< vector<HypedValue> > hyped_tuples =
        generate_hyped_values<long>(tuples);
    vector<int> num_buckets = { 3, 3, 3 };
    sort_tuples(hyped_tuples, num_buckets);
    auto htree = construct_htree(hyped_tuples, num_buckets);

    HypedValue zero = HypedValue{TypedValue{static_cast<long>(0)}};
    HypedValue one = HypedValue{TypedValue{static_cast<long>(1)}};
    HypedValue two = HypedValue{TypedValue{static_cast<long>(2)}};
    bucket<HypedValue> query = {{ { one, one }, { one, two }, { zero, two } }};

    EXPECT_EQ(htree->estimateSelectivity(query), 6);
}

TEST(HTreeTest, HTreeTest_Hyped_Tree_Double) {
    vector< vector<int> > tuples = generate_uniform_tuples(4, { 4, 4, 4, 4 });
    vector< vector<HypedValue> > hyped_tuples =
        generate_hyped_values<double>(tuples);
    vector<int> num_buckets = { 4, 4, 4, 4 };
    sort_tuples(hyped_tuples, num_buckets);
    auto htree = construct_htree(hyped_tuples, num_buckets);

    HypedValue zero = HypedValue{TypedValue{static_cast<double>(0)}};
    HypedValue half = HypedValue{TypedValue{static_cast<double>(0.5)}};
    HypedValue one_plus_half = HypedValue{TypedValue{static_cast<double>(1.5)}};
    HypedValue two = HypedValue{TypedValue{static_cast<double>(2)}};
    bucket<HypedValue> query = {{
        { zero, half }, { half, one_plus_half }, { zero, zero }, { zero, two }
    }};

    EXPECT_EQ(htree->estimateSelectivity(query), 3);
}

TEST(HTreeTest, HTreeTest_Optimal_Partition_Construction) {
    vector< vector<int> > tuples = generate_uniform_tuples(3, { 3, 2, 19 });
    construction_test(tuples, { 3, 2, 3 },
        {
            { {0, 0}, {
                { {0, 0}, {
                    { {0, 6},
                        { {0, 0}, {0, 0}, {0, 6} }
                    },
                    { {7, 13},
                        { {0, 0}, {0, 0}, {7, 13} }
                    },
                    { {14, 18},
                        { {0, 0}, {0, 0}, {14, 18} }
                    },
                } },
                { {1, 1}, {
                    { {0, 6},
                        { {0, 0}, {1, 1}, {0, 6} }
                    },
                    { {7, 13},
                        { {0, 0}, {1, 1}, {7, 13} }
                    },
                    { {14, 18},
                        { {0, 0}, {1, 1}, {14, 18} }
                    },
                } },
            } },
            { {1, 1}, {
                { {0, 0}, {
                    { {0, 6},
                        { {1, 1}, {0, 0}, {0, 6} }
                    },
                    { {7, 13},
                        { {1, 1}, {0, 0}, {7, 13} }
                    },
                    { {14, 18},
                        { {1, 1}, {0, 0}, {14, 18} }
                    },
                } },
                { {1, 1}, {
                    { {0, 6},
                        { {1, 1}, {1, 1}, {0, 6} }
                    },
                    { {7, 13},
                        { {1, 1}, {1, 1}, {7, 13} }
                    },
                    { {14, 18},
                        { {1, 1}, {1, 1}, {14, 18} }
                    },
                } },
            } },
            { {2, 2}, {
                { {0, 0}, {
                    { {0, 6},
                        { {2, 2}, {0, 0}, {0, 6} }
                    },
                    { {7, 13},
                        { {2, 2}, {0, 0}, {7, 13} }
                    },
                    { {14, 18},
                        { {2, 2}, {0, 0}, {14, 18} }
                    },
                } },
                { {1, 1}, {
                    { {0, 6},
                        { {2, 2}, {1, 1}, {0, 6} }
                    },
                    { {7, 13},
                        { {2, 2}, {1, 1}, {7, 13} }
                    },
                    { {14, 18},
                        { {2, 2}, {1, 1}, {14, 18} }
                    },
                } },
            } },
        });

}

TEST(HTreeTest, HTreeTest_Suboptimal_Partition_Construction) {
    vector< vector<int> > tuples = generate_uniform_tuples(3, { 6, 2, 3 });
    construction_test(tuples, { 3, 2, 3 },
        {
            { {0, 1}, {
                { {0, 0}, {
                    { {0, 0},
                        { {0, 1}, {0, 0}, {0, 0} }
                    },
                    { {1, 1},
                        { {0, 1}, {0, 0}, {1, 1} }
                    },
                    { {2, 2},
                        { {0, 1}, {0, 0}, {2, 2} }
                    },
                } },
                { {1, 1}, {
                    { {0, 0},
                        { {0, 1}, {1, 1}, {0, 0} }
                    },
                    { {1, 1},
                        { {0, 1}, {1, 1}, {1, 1} }
                    },
                    { {2, 2},
                        { {0, 1}, {1, 1}, {2, 2} }
                    },
                } },
            } },
            { {2, 3}, {
                { {0, 0}, {
                    { {0, 0},
                        { {2, 3}, {0, 0}, {0, 0} }
                    },
                    { {1, 1},
                        { {2, 3}, {0, 0}, {1, 1} }
                    },
                    { {2, 2},
                        { {2, 3}, {0, 0}, {2, 2} }
                    },
                } },
                { {1, 1}, {
                    { {0, 0},
                        { {2, 3}, {1, 1}, {0, 0} }
                    },
                    { {1, 1},
                        { {2, 3}, {1, 1}, {1, 1} }
                    },
                    { {2, 2},
                        { {2, 3}, {1, 1}, {2, 2} }
                    },
                } },
            } },
            { {4, 5}, {
                { {0, 0}, {
                    { {0, 0},
                        { {4, 5}, {0, 0}, {0, 0} }
                    },
                    { {1, 1},
                        { {4, 5}, {0, 0}, {1, 1} }
                    },
                    { {2, 2},
                        { {4, 5}, {0, 0}, {2, 2} }
                    },
                } },
                { {1, 1}, {
                    { {0, 0},
                        { {4, 5}, {1, 1}, {0, 0} }
                    },
                    { {1, 1},
                        { {4, 5}, {1, 1}, {1, 1} }
                    },
                    { {2, 2},
                        { {4, 5}, {1, 1}, {2, 2} }
                    },
                } },
            } },
        });

}


} //namespace quickstep
