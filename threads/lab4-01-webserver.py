#!/usr/bin/python

import tester
import sys
import random
import re

def file_line_count(fname, values, times):
    try:
        with open(fname) as f:
            for i, l in enumerate(f):
                l = l.rstrip('\n')
                line = re.split(r'[, ]+', l, 1)
                try:
                    time_value = float(line[1])
                except ValueError:
                    print 'error: ' + fname + ': could not convert ' + \
                        line[1] + ' to float'
                    return 0

                if (i >= len(values)):
                    print 'error: ' + fname + ': too many values '
                    return 0

                if (int(line[0]) != values[i]):
                    print 'error: ' + fname + ': expected ' + \
                        str(values[i]) + ', found ' + line[0]
                    return 0
                times[i] = time_value
        f.closed
    except IOError as e:
        print 'error: ' + fname + ': ' + e.strerror
        return 0
    return i + 1

interval = 0.05
min_interval = 1 - interval
max_interval = 1 + interval

def main():
    test = tester.Core('webserver test', 30)
    # start server at some random port. this may cause collisions.
    random.seed(None)
    port = random.randint(2049, 65534)
    print 'starting server at port ' + str(port)
    ### timeout is 3600, 60 minutes
    test.start_program('./run-experiment ' + str(port) + " nocache", 3600)

    test.look("Threads experiment done.\r\n")
    threads = [0, 1, 2, 4, 8, 16, 32, 64, 128]
    times = [0] * len(threads)
    times_ratios = [1, 1, 0.5, 0.25, 0.15, 0.125, 0.125, 0.125, 0.125]
    count = file_line_count('plot-threads.out', threads, times)
    if (count == len(threads)):
        for i in range(len(threads)):
            if (abs(times[i]/times[0] - times_ratios[i]) < interval):
                count += 1
            else:
                expected_value_min = (times[0] * times_ratios[i]) * min_interval
                expected_value_max = (times[0] * times_ratios[i]) * max_interval
                print 'plot-threads.out: run time with ' + str(threads[i]) + \
                    ' threads is ' + str(times[i]) + \
                    '. We expect it to lie between (' + \
                    str(expected_value_min) + ', ' + \
                    str(expected_value_max) + ').'
    test.add_mark(count)    

    test.look("Requests experiment done.\r\n")
    requests = [1, 2, 4, 8, 16, 32]
    times = [0] * len(requests)
    times_ratios = [1, 1, 1, 1, 1, 1]
    count = file_line_count('plot-requests.out', requests, times)
    if (count == len(requests)):
        for i in range(len(requests)):
            if (abs(times[i]/times[0] - times_ratios[i]) < interval):
                count += 1
            else:
                expected_value_min = (times[0] * times_ratios[i]) * min_interval
                expected_value_max = (times[0] * times_ratios[i]) * max_interval
                print 'plot-requests.out: run time with ' + str(requests[i]) + \
                    ' buffers is ' + str(times[i]) + \
                    '. We expect it to lie between (' + \
                    str(expected_value_min) + ', ' + \
                    str(expected_value_max) + ').'
    test.add_mark(count)

    test.start_program('./plot-experiment')

if __name__ == '__main__':
	main()
