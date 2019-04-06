#include <vector>
#include <cmath>
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "gtest/gtest.h"
#include "histogram/HTree.hpp"

namespace quickstep {

using std::vector;

class mock_htree_node {
public:
    interval key;
    vector<mock_htree_node> children;
    vector<interval> leaf_elements;

    mock_htree_node(interval key, vector<mock_htree_node> children) :
        key(key), children(children) {}

    mock_htree_node(interval key, vector<interval> leaf_elements) :
        key(key), leaf_elements(leaf_elements) {}

    void print(std::ostream &os, unsigned int level=0) {
        std::string indent = "";
        for (int i = 0; i < level; i++) {
            indent += "\t";
        }
        // if leaf
        if (children.size() == 0) {
            os << indent << "{ ";
            for (interval range : leaf_elements) {
                os << range << ", ";
            }
            os << "}\n";
        } else {
            os << indent << key << " {\n";
            for (mock_htree_node child : children) {
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

void test_overlap_calculation(
    float expected, bucket query, vector<bucket> hits)
{
    vector< shared_ptr<bucket> > ptrs;
    for (bucket bkt : hits) {
        shared_ptr<bucket> p = std::make_shared<bucket>(bkt);
        ptrs.push_back(p);
    }
    EXPECT_EQ(expected, buckets_overlapped(ptrs, query));
}

void search_test(
    shared_ptr<htree_node> tree, bucket query, vector<bucket> expected_hits)
{
    vector< shared_ptr<bucket> > hits = tree->search(query);
    EXPECT_TRUE(hits.size() == expected_hits.size());
    for (auto bkt : hits) {
        auto it = std::find(expected_hits.begin(), expected_hits.end(), *bkt);
        EXPECT_TRUE(it != expected_hits.end());
    }
}

void construction_test(
    vector< vector<int> > tuples, vector<int> num_buckets,
    vector<mock_htree_node> expected_tree)
{
    std::ostringstream result_stream;
    std::ostringstream expected_stream;

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
        { { {0, 1}, {0, 1}, {false, -1, false, -1} } },
        {
            { { {0, 0}, {0, 0}, {0, 0} } },
            { { {0, 0}, {1, 1}, {3, 3} } },
            { { {1, 1}, {0, 0}, {7, 7} } },
            { { {2, 2}, {0, 0}, {0, 0} } } // does not overlap
        });
}

TEST(HTreeTest, HTreeTest_Overlap_Partial) {
    test_overlap_calculation(1.75,
        { { {0, 1}, {0, 1}, {false, -1, false, -1} } },
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
                { {0, 1}, {
                    { {0, 1},
                        { {0, 1}, {0, 1}, {0, 1} }
                    },
                    { {0, 2},
                        { {0, 1}, {0, 1}, {0, 2} }
                    },
                    { {1, 2},
                        { {0, 1}, {0, 1}, {1, 2} }
                    },
                } },
                { {0, 1}, {
                    { {0, 1},
                        { {0, 1}, {0, 1}, {0, 1} }
                    },
                    { {0, 2},
                        { {0, 1}, {0, 1}, {0, 2} }
                    },
                    { {1, 2},
                        { {0, 1}, {0, 1}, {1, 2} }
                    },
                } },
            } },
            { {2, 3}, {
                { {0, 1}, {
                    { {0, 1},
                        { {2, 3}, {0, 1}, {0, 1} }
                    },
                    { {0, 2},
                        { {2, 3}, {0, 1}, {0, 2} }
                    },
                    { {1, 2},
                        { {2, 3}, {0, 1}, {1, 2} }
                    },
                } },
                { {0, 1}, {
                    { {0, 1},
                        { {2, 3}, {0, 1}, {0, 1} }
                    },
                    { {0, 2},
                        { {2, 3}, {0, 1}, {0, 2} }
                    },
                    { {1, 2},
                        { {2, 3}, {0, 1}, {1, 2} }
                    },
                } },
            } },
            { {4, 5}, {
                { {0, 1}, {
                    { {0, 1},
                        { {4, 5}, {0, 1}, {0, 1} }
                    },
                    { {0, 2},
                        { {4, 5}, {0, 1}, {0, 2} }
                    },
                    { {1, 2},
                        { {4, 5}, {0, 1}, {1, 2} }
                    },
                } },
                { {0, 1}, {
                    { {0, 1},
                        { {4, 5}, {0, 1}, {0, 1} }
                    },
                    { {0, 2},
                        { {4, 5}, {0, 1}, {0, 2} }
                    },
                    { {1, 2},
                        { {4, 5}, {0, 1}, {1, 2} }
                    },
                } },
            } },
        });

}

} //namespace quickstep
