#!/usr/bin/python
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
SST-MacSim Unit Text
Author: Jaekyu Lee (jq.lee17@gmail.com)
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""


import os
import sys
import subprocess
import glob
import difflib
import re


"""
SST-MacSim unit test main routine
"""
def main():
  current_dir = os.getcwd()

  """
  sst.x existence check
  """
  sstx_exist = False
  for path in os.environ['PATH'].split(':'):
    if os.path.exists('%s/sst.x' % (path)):
      sstx_exist = True
      break

  if not sstx_exist:
    print('sst.x does not exist.... exit')
    sys.exit(-1)


  """
  Run traces
  """
  trace_dir = '%s/traces' % current_dir
  golden_result_dir = '%s/outputs' % current_dir
  os.chdir(trace_dir)
  for trace in os.listdir('.'):
    os.chdir('%s/%s' % (trace_dir, trace))
    """
    Create trace_file_list
    """
    trace_file_list = open('trace_file_list', 'w')
    trace_file_list.write('1\n')
    trace_file_list.write('%s/kernel_config.txt\n' % os.getcwd())
    trace_file_list.close()


    """
    Run trace
    """
    os.system('sst.x macsim.sdl > /dev/null')


    """
    Comparison with Golden results
    """
    match_fail = False
    golden_dir = '%s/%s' % (golden_result_dir, trace)
    test_dir = '%s/results' % os.getcwd()

    file_list = glob.glob('%s/*.stat.out' % golden_dir)
    for filename in file_list:
      test_output = '%s/%s' % (test_dir, os.path.basename(filename))
      golden_output = '%s/%s' % (golden_dir, os.path.basename(filename))

      if not os.path.exists(test_output):
        match_fail = True
        break

      diff = difflib.unified_diff(open(test_output).readlines(), open(golden_output).readlines())
      for line in diff:
        line = line.rstrip('\n')
        if line[0] == '-' and line[1] != '-':
          if re.match('-EXE_TIME', line):
            continue

          match_fail = True
          break

      if match_fail:
        break
  
    if match_fail:
      print('result of %s does not match... failed test' % trace)
    else:
      print('test (%s) successful...' % trace)


    """
    cleanup
    """
    os.system('rm -f trace_file_list NULL trace_debug.out')
    os.system('rm -rf results')


if __name__ == '__main__':
  main()


