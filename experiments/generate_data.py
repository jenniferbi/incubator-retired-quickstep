#!/usr/bin/python
from subprocess import Popen, PIPE, STDOUT
import numpy as np
import matplotlib.pyplot as plt
import sys
# standard deviations
# range of data is [0, 1000) 
tables = [("low", 2), ("med", 2), ("hi", 2), ("low",3), ("med",3), ("hi", 3),
            ("low", 5), ("med", 5), ("hi", 5)]
#tables = [("unif", 2), ("unif", 3), ("unif", 5)]
for tbl,dims in tables:
    D = np.diag([340]*dims)
    corr = np.ones((dims,dims),dtype=float)
    # generate synthetic data
    if tbl == "low":
        corr = 0.1*corr
    elif tbl == "med":
        corr = 0.5*corr
    elif tbl == "hi":
        corr = 0.9*corr
    np.fill_diagonal(corr,1)
    cov = D * corr * D
    mean = [0]*dims
    data = np.random.multivariate_normal(mean, cov, size=500000)
    #data = np.random.uniform([-1000]*dims,[1000]*dims, size=(500000,dims))
    """
    arr = np.array([])
    data = np.random.zipf(2,size=(63500,dims)) # produces around 500000
    for d in range(0,dims):
        vals = np.random.uniform(-1000,1000, 63500)
        vals2 = np.random.uniform(-1000,1000, 63500)
        freq = data[:,d]
        for v,v2,f in zip(vals,vals2,freq):
    """        
    
    # uncomment to visualize one col 
    #plt.hist(data[:,0], 100)
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
    elif dims == 5:
        create_qs.append("(a INTEGER, b INTEGER, c INTEGER, d INTEGER, e INTEGER);");
    create_qs.append("COPY %s FROM '%s.csv' WITH (DELIMITER ',');" % (name,name));
    create_qs.append("\histogram %s\n" % name)

    ## Don't make histograms for these

for tbl,dims in tables:
    if dims == 5: 
        continue
    name = tbl + str(dims) + "_nohist"
    create_qs.append("CREATE TABLE %s " % name);
    if dims == 2:
        create_qs.append("(x INTEGER, y INTEGER);");
    elif dims == 3:
        create_qs.append("(x INTEGER, y INTEGER, z INTEGER);");
    #elif dims == 5:
    #    create_qs.append("(a INTEGER, b INTEGER, c INTEGER, d INTEGER, e INTEGER);");
    create_qs.append("COPY %s FROM '%s.csv' WITH (DELIMITER ',');" % (name,tbl+str(dims)));

create =" ".join(create_qs)
print create
#p = Popen(["../build/quickstep_cli_shell", "--initialize_db=true"], stdin=PIPE)
p = Popen(["../build/quickstep_cli_shell"], stdin=PIPE)

out,err = p.communicate(input=create)
#print out
