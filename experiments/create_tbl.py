#!/usr/bin/python

import os

table_dir = "test_tables/"

class Table(object):
    def __init__(self, n_cols, n_rows):
        self.n_cols = n_cols
        self.n_rows = n_rows

    def write_table(self, filename):
        if not os.path.exists(table_dir):
            os.makedirs(table_dir)
        with open(table_dir + filename, "w") as f:
            for i in range(0, self.n_rows):
                f.write('|'.join([str(i)] * self.n_cols) + "\n")

# We care about varying the selectivity of particular filters.
# A simple way to do this for single-attribute conditions is to have the values
# of that attribute uniformly distributed, so we can select for portions of the
# range
# e.g. in the range [0,100], attr < 50 would have a selectivity of .5
#
# This will be trickier when we start thinking about correlation.
#
# We may also want to consider the case of equijoin. To achieve selecivity p,q on the left/right tables respectively,
# we want unique matching keys such that probability of getting a matching key is p,q.

t1 = Table( 1, 1 )
t2 = Table( 1, 1000000 )
t1.write_table("t1.tbl")
t2.write_table("t2.tbl")
