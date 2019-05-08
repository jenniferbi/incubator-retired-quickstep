#!/usr/bin/python
from subprocess import Popen, PIPE, STDOUT
import numpy as np
import matplotlib.pyplot as plt
import sys
# standard deviations
# range of data is [0, 1000) 
mean2d = [500, 500]
stddev2d = [340, 340]
D = np.diag(stddev2d)

# low correlation
lowcorr = np.array([[1, 0], [0, 1]])
lowcov = D * lowcorr * D

# medium correlation
medcorr = np.array([[1, 0.5], [0.5, 1]])
medcov = D * medcorr * D

# high correlation
hicorr = np.array([[1, 0.9], [0.9, 1]])
hicov = D * hicorr * D

tables = [("low", 2), ("med", 2), ("hi", 2)]

for tbl,dims in tables:
    # generate synthetic data
    if tbl == "low":
        cov = lowcov
    elif tbl == "med":
        cov = medcov
    elif tbl == "hi":
        cov = hicov
    if dims == 2:
        mean = mean2d
    #elif dims == 3:
    #    data = np.random.multivariate_normal(mean3d, lowcov, size=500000)
    #elif dims == 8:
    #    data = np.random.multivariate_normal(mean8d, lowcov, size=500000)
    data = np.random.multivariate_normal(mean, cov, size=500000)
    # uncomment to visualize one col 
    #plt.hist(data[:,1], 100)
    #plt.show()
    #sys.exit()

    #cast to ints
    np.savetxt(tbl + str(dims) + ".csv",  data.astype(int), fmt="%d", delimiter=",")

# create quickstep tables
create_qs = []
for tbl,dims in tables:
    name = tbl + str(dims)
    create_qs.append("CREATE TABLE %s " % name);
    if dims == 2:
        create_qs.append("(x INTEGER, y INTEGER);");
    elif dims == 3:
        create_qs.append("(x INTEGER, y INTEGER, z INTEGER);");
    elif dims == 8:
        create_qs.append("(a INTEGER, b INTEGER, c INTEGER, d INTEGER, e INTEGER, f INTEGER, h INTEGER);");
    create_qs.append("COPY %s FROM '%s.csv' WITH (DELIMITER ',');" % (name,name));
    create_qs.append("\histogram %s\n" % name)

    ## Don't make histograms for these
for tbl,dims in tables:
    name = tbl + str(dims) + "_nohist"
    create_qs.append("CREATE TABLE %s " % name);
    if dims == 2:
        create_qs.append("(x INTEGER, y INTEGER);");
    elif dims == 3:
        create_qs.append("(x INTEGER, y INTEGER, z INTEGER);");
    elif dims == 8:
        create_qs.append("(a INTEGER, b INTEGER, c INTEGER, d INTEGER, e INTEGER, f INTEGER, h INTEGER);");
    create_qs.append("COPY %s FROM '%s.csv' WITH (DELIMITER ',');" % (name,tbl+str(dims)));

create =" ".join(create_qs)
print create
p = Popen(["../build/quickstep_cli_shell", "--initialize_db=true"], stdin=PIPE)

out,err = p.communicate(input=create)
#print out
