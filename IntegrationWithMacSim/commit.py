#!/usr/bin/python

"""
Author  : Jaekyu Lee (jq.lee17@gmail.com)
Date    : 8/9/2012
Purpose : This script file is intended for committing files to MacSim repository.
          This script will do
          1) Run regression test
          2) If the regression test is passed, commit files
"""


import os
import re
import sys
import glob
import argparse
import subprocess



"""
<Usage>

./commit.py -bin macsim_binary [-file file1 file2 ...] [-skip]

* You have to specify macsim binary for regression test unless -skip option is specified.
* You have to generate regression data on your own.
* You have to pass regression test to commit.
* You don't have to specify file list to commit.
  In this case, it will be just same as 'svn commit'.
* If you want to skip regression test, please use -skip option.
"""


"""
Process arguments
"""
def process_options():
  parser = argparse.ArgumentParser(description='GPU trace generation for MacSim')
  parser.add_argument('-bin', action='store', default='', dest='bin', 
      help='macsim binary to run regression test')
  parser.add_argument('-skip', action='store_true', dest='skip',
      help='skip regression test')
  parser.add_argument('-file', action='append', nargs='*', dest='file', 
      help='file to commit')

  return parser


"""
To perform regression test
"""
def regression_test():
  if args.bin == '':
    print('please specify macsim binary to run regression test')
    sys.exit(-1)

  if not os.path.exists(args.bin):
    print('%s does not exist' % args.bin)
    sys.exit(-1)

  if 'SIMDIR' not in os.environ:
    print('please set env. SIMDIR')
    sys.exit(-1)

  regression_path = os.environ['SIMDIR'] + '/tools/macsim_regression/data'
  if not os.path.exists(regression_path):
    print('you have to generate regression data from the clean repository version first')
    sys.exit(-1)

  cmd = 'regression_test.py --bin %s' % args.bin
  p = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, 
                       stderr=subprocess.PIPE, close_fds=True)     
  (fi, fo, fe) = (p.stdin, p.stdout, p.stderr)

  pattern = re.compile('Summary: total ([0-9]+) pass ([0-9]+)')
  regression_result = False
  for line in fo:
    match = pattern.match(line)
    if match != None and match.group(1) == match.group(2):
      regression_result = True
      print('pass!')

  if regression_result == False:
    print('regression test failed')
    sys.exit(-1)


"""
commit changes to MacSim repository
"""
def commit():
  if args.file == None:
    file_list = ''
  else:
    file_list = ' '.join(sum(args.file, []))
    
  os.system('svn commit %s' % file_list)

"""
main routine
"""
def main():
  global args
  
  parser = process_options()
  args = parser.parse_args()

  if not args.skip:
    regression_test()

  commit()


if __name__ == '__main__':
  main()
