#!/usr/bin/python

import pexpect
import sys
import os
import shutil
import argparse

# Lab 3 tests use the following two constants
NTHREADS=128
LOOPS=10

class Core:
        def set_timeout(self, timeout):
            self.program.timeout = timeout
            if self.verbose > 0:
                print 'This test has a timeout of ' + str(timeout) + ' seconds'

        def start_program(self, path, timeout = 10):
                if self.verbose > 0:
                        print 'STARTING PROGRAM: ' + path
                self.program = pexpect.spawn(path, [], timeout)
                # pexpect copies all input and output to this file
                self.program.logfile = open('tester.log', 'a')

        def __init__(self, message, total):
                parser = argparse.ArgumentParser(description='TestUnit')
                parser.add_argument('-v', '--verbose', action='store_true', 
                                    help='verbose mode')
                parser.add_argument('nr_times', metavar = 'N', type = int,
                                    nargs = '?', choices = xrange(1, 100),
                                    default = 3,
                                    help = 'Nr of times to run test')
                args = vars(parser.parse_args())

                self.nr_times = args['nr_times']

                if (args['verbose']):
                        self.verbose = 1
                else:
                        self.verbose = 0
                self.message = message
                print message
                self.cwd = os.getcwd()
                self.errors = {'EOF' : -1, 'TIMEOUT' : -2,
                               'INCONSISTENT' : -3, 'LOOPING' : -4,
                               'BUG' : -5 }
                self.mark = 0
                self.total = total

        def __del__(self):
                if hasattr(self, 'program'):
                        self.program.logfile.close()
                if (self.mark > self.total):
                        print 'mark = ' + str(self.mark) + ' is greater than ' \
                                'total = ' + str(self.total)
                        print '\nPLEASE REPORT THIS TO THE INSTRUCTOR OR A TA\n'
                print 'Mark for ' + self.message + ' is ' + \
                        str(self.mark) + ' out of ' + str(self.total)
                marker = open('tester.out', 'a')
                marker.write(self.message + ', ' + str(self.mark) + \
                             ', ' + str(self.total) + '\n')
                marker.close()
                if (self.mark == self.total):
                        print 'PASS'
                else:
                        print 'FAIL'

        def send_command(self, cmd):
                if self.verbose > 0:
                        print 'SENDING: ' + cmd
                # send the command character by character
                cmd_char = list(cmd)
                for i in cmd_char:
                        self.program.send(i)

        def look(self, result, mark = 0):
                try:
                        if self.verbose > 0:
                                print 'EXPECTING: ' + str(result)
                        index = self.program.expect(result)
                        if self.verbose > 0:
                                print 'FOUND: ' + self.program.match.group(0)
                except pexpect.TIMEOUT, e:
                        print 'ERROR: TIMEOUT: not found: ' + str(result)
                        return self.errors['TIMEOUT']
                except pexpect.EOF:
                        print 'ERROR: EOF: not found: ' + str(result)
                        return self.errors['EOF']
                except IOError as e:
                        print "ERROR: I/O error: " + e.strerror
                        print 'ERROR: I/O error: not found: ' + str(result)
                        return self.errors['EOF']
                except Exception, e:
                        print 'ERROR: unexpected problem', sys.exc_info()[0]
                        print '\nPLEASE REPORT THIS TO THE INSTRUCTOR OR A TA\n'
                        return self.errors['BUG']
                self.add_mark(mark)
                return index

        # look anchored by '^' and "\r\n"
        def lookA(self, result, mark = 0):
                return self.look('^' + result + '\r\n', mark)

        def add_mark(self, mark):
                self.mark += mark

        def reset_mark(self):
                self.mark = 0

        def get_mark(self):
                return self.mark

        # def program(self):
        #         return self.program

        # def verbose(self):
        #         return self.verbose
