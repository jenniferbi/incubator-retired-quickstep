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


# execute quickstep queries
ranges = range(0,1000,50)
queries  = [ "SELECT * FROM low2 where x > %d;" % r for r in ranges]
est_results = [] 
true_results = [] 
for query in queries:
    p = Popen(["../build/quickstep_cli_shell", "--visualize_plan=true"], stdin=PIPE, stdout=DEVNULL, stderr=PIPE)
    _,res = p.communicate(input=query)
    # parse output filei
    
    true = float(truesel.findall(res)[0])/500000
    est = estsel.findall(res)[1] ## the second one is selection op?
    est = float(est)
    est_results.append(est)
    true_results.append(true)
    print "est",est
    print "true",true

true_results = np.array(true_results)
est_results= np.array(est_results)
print (true_results - est_results)
# plot error
plt.plot(ranges, true_results - est_results, 'bo')
plt.show()
# overall error
error = np.sum(np.abs(true_results - est_results))/np.sum(true_results)

# plot

