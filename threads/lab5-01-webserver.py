#!/usr/bin/python

import tester
import sys
import random
import re

def file_line_count(fname, values_array):
    try:
        with open(fname) as f:
            for i, l in enumerate(f):
                l = l.rstrip('\n')
                line = re.split(r'[, ]+', l, 1)
                try:
                    f = float(line[1])
                except ValueError:
                    print 'error: ' + fname + ': could not convert ' + \
                        line[1] + ' to float'
                    return 0

                if (i >= len(values_array)):
                    print 'error: ' + fname + ': too many values '
                    return 0

                if (int(line[0]) != values_array[i]):
                    print 'error: ' + fname + ': expected ' + \
                        str(values_array[i]) + ', found ' + line[0]
                    return 0
                pass
    except IOError as e:
        print 'error: ' + fname + ': ' + e.strerror
        return 0
    return i + 1

def main():
    test = tester.Core('webserver test', 15)
    # start server at some random port. this may cause collisions.
    random.seed(None)
    port = random.randint(2049, 65534)
    print 'starting server at port ' + str(port)
    # timeout is 3600, 60 minutes
    test.start_program('./run-experiment ' + str(port), 3600)

    test.look("Threads experiment done.\r\n")
    count = file_line_count('plot-threads.out', \
                                [0, 1, 2, 4, 8, 16, 32, 64, 128])
    test.look("Requests experiment done.\r\n")
    count += file_line_count('plot-requests.out', [1, 2, 4, 8, 16, 32])

    test.look("Cachesize experiment done.\r\n")
    cache_count = file_line_count('plot-cachesize.out', \
                                      [16384, 65536, 262144, \
                                           1048576, 4194304, 16777216])

    # give a maximum of 3 marks for first two experiments, 
    # and a max of 12 marks for the cache experiments
    newcount = int(round(count/5 + 2 * cache_count))

    test.add_mark(newcount)
    test.start_program('./plot-experiment')

if __name__ == '__main__':
	main()
