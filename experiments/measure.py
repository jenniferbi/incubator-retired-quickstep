#!/usr/bin/python
from subprocess import Popen, PIPE
import matplotlib.pyplot as plt
import re
import os
import numpy as np

DEVNULL = open(os.devnull, 'wb')

# regex for est and true selectivities
truesel = re.compile("\d+(?= rows)")
estsel = re.compile("(?<=Selectivity = )[0|1].\d+")

tbl="hi2"
# execute quickstep queries
ranges = range(0,1000,50)
queries  = [ "SELECT * FROM %s where x > %d;" % (tbl,r) for r in ranges]
est_results = [] 
true_results = [] 
for query in queries:
    p = Popen(["../build/quickstep_cli_shell", "--visualize_plan=true"], stdin=PIPE, stdout=DEVNULL, stderr=PIPE)
    _,res = p.communicate(input=query)
    # parse output file
    
    true = float(truesel.findall(res)[0])/500000
    est = estsel.findall(res)[1] ## the second one is selection op?
    est = float(est)
    est_results.append(est)
    true_results.append(true)
    print "est",est
    print "true",true

# execute queries on baseline 
tbl = tbl + "_nohist"
queries  = [ "SELECT * FROM %s where x > %d;" % (tbl,r) for r in ranges]
trivest_results = [] 
for query in queries:
    p = Popen(["../build/quickstep_cli_shell", "--visualize_plan=true"], stdin=PIPE, stdout=DEVNULL, stderr=PIPE)
    _,res = p.communicate(input=query)
    # parse output file
    
    true = float(truesel.findall(res)[0])/500000
    est = estsel.findall(res)[1] ## the second one is selection op?
    est = float(est)
    trivest_results.append(est)

true_results = np.array(true_results)
est_results= np.array(est_results)
trivest_results= np.array(trivest_results)
print (true_results - est_results)

# plot error
#plt.plot(ranges, trivtrue_results - trivest_results, 'ro', label='simple cost model')
plt.plot(ranges, true_results - est_results, 'b.', label='histogram')
plt.xlabel("A value")
plt.ylabel("True selectivity - estimated selectivity")
#plt.legend(loc='upper left')
plt.tight_layout()
plt.show()

# overall error
hist_error = np.sum(np.abs(true_results - est_results))/np.sum(true_results)
cm_error = np.sum(np.abs(true_results - trivest_results))/np.sum(true_results)
print "error: ", hist_error, cm_error
# plot

