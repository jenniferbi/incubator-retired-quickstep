#!/usr/bin/python
from subprocess import Popen, PIPE
import matplotlib.pyplot as plt
import re
import os
import sys
import math
import numpy as np

def execute_queries(tbl, dims):
# execute quickstep queries
    qtype = 1 # do gaussians first
    c_v = 0.01 # 1 percent of data range
    num_queries = 20
    low = -1000
    high = 1000
    ranges = [] 
    queries = []
    step = math.pow(math.pow((high - low),dims)*c_v , 1.0/dims)
    step = int(step/2.0)
    print "%s: step: %d" %(tbl, step)

    x,y,z,a,b,c,d,e=0,0,0,0,0,0,0,0

    if qtype == 1: # GAUSS
        ranges = np.random.multivariate_normal([0]*dims, 34*34*np.identity(dims), size=num_queries)
    elif qtype == 2: # UNIFORM
        ranges = np.random.uniform([low]*dims, [high]*dims, size=num_queries)

    if dims == 2:
        queries  = [ """SELECT * FROM %s where x > %d and x < %d
                        and y > %d and y < %d;"""
                    % (tbl,x-step, x+step, y-step, y+step) for (x,y) in ranges]
    elif dims == 3:
        queries  = [ """SELECT * FROM %s where x > %d and x < %d
                        and y > %d and y < %d and z > %d and z < %d;"""
                    % (tbl,x-step, x+step, y-step, y+step, z-step, z+step) for (x,y,z) in ranges]
    elif dims == 5:
        queries  = [ """SELECT * FROM %s where a > %d and a < %d
                        and b > %d and b < %d and c > %d and c < %d
                        and d > %d and d < %d and e > %d and e < %d;"""
                    % (tbl,a-step,a+step,b-step,b+step,c-step,c+step,
                        d-step,d+step,e-step,e+step) for (a,b,c,d,e) in ranges]
    #print queries
    est_results = [] 
    true_results = [] 
    for query in queries:
        p = Popen(["../build/quickstep_cli_shell", "--visualize_plan=true"], stdin=PIPE, stdout=DEVNULL, stderr=PIPE)
        _,res = p.communicate(input=query)
        # parse output file
        try:
            l = truesel.findall(res.splitlines()[-1])
            true = float(l[0])
        except:
            print res.splitlines()[-1]
            print l
            sys.exit()
        est = estsel.findall(res)[1] ## the second one is selection op?
        est = float(est)*500000
        est_results.append(est)
        true_results.append(true)
        print "est",est
        print "true",true

    true_results = np.array(true_results)
    est_results= np.array(est_results)
    print "true_results",true_results
    #print "est_results",est_results
    # overall error
    #error = np.sum(np.abs(true_results - est_results))/np.sum(true_results)
    print(len(queries))
    error = 100*np.sum(np.divide(np.abs(true_results - est_results),true_results))/len(queries)
    print tbl, " error: ", error

    return error 

def generate_bar():
    barwidth = 0.25
    x = ["Gauss2d", "Gauss3d", "Unif"]
    x_pos = [i for i, _ in enumerate(x)]
    x_pos2 = [i + barwidth for i in x_pos]
    x_pos3 = [i + barwidth for i in x_pos2]

    errA = execute_queries("low2", 2);
    errB = execute_queries("low3", 3);
    errs = [errA, errB]#, errD]
    plt.bar(x_pos, errs, color = '#1c7f27', edgecolor='white', width = barwidth, label='cov=0')

    errA = execute_queries("med2", 2);
    errB = execute_queries("med3", 3);
    errs = [errA, errB]
    plt.bar(x_pos2, errs, color = '#1459c9', edgecolor='white', width = barwidth, label='cov=0.5')

    errA = execute_queries("hi2", 2);
    errB = execute_queries("hi3", 3);
    errs = [errA, errB]
    plt.bar(x_pos3, errs, color = '#d8b62f', edgecolor='white', width = barwidth, label='cov=0.9')

    plt.ylabel("Average error")

    plt.xticks([i + barwidth for i in range(len(x_pos))], x)

    plt.tight_layout()
    plt.legend()
    plt.show()

DEVNULL = open(os.devnull, 'wb')

# regex for est and true selectivities
truesel = re.compile("\d+")
#truesel = re.compile("\d+((?= rows)|(?= row))")
estsel = re.compile("(?<=Selectivity = )[0|1].\d+")

generate_bar()
"""


# plot error
plt.plot(ranges, true_results - est_results, 'b.', label='histogram')
plt.xlabel("A value")
plt.ylabel("True selectivity - estimated selectivity")
#plt.legend(loc='upper left')
plt.tight_layout()
plt.show()

"""
