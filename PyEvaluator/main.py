import sys
import getopt

import eval
import analyse

argv = sys.argv[1:]
opts, addr = getopt.getopt(argv, "ea:")
addr = addr[0]

data = []

try:
    with open(addr, "r") as route:
        print("Loading data")
        route.readline()
        line = route.readline().split()
        while (line):
            planetid = int(line[0])
            answer = True if line[1][0] == "D" else False
            groupid = int(line[2]) if (len(line) > 2) else 0
            data.append([planetid, answer, groupid])

            line = route.readline().split()
        print("Data loaded")
except:
    print("Unable to open %s." % addr)
else:  
    for opt, arg in opts:
        if(opt == "-e"):
            evaluator = eval.Evaluator(data)
            evaluator.evaluate()
            break
        if(opt == "-a"):
            analyser = analyse.Analyser(data, arg)
            analyser.analyse()
            break
