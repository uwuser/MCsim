#!/usr/bin/python
"""
Copyright (c) <2012>, <Georgia Institute of Technology> All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted 
provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions 
and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this list of 
conditions and the following disclaimer in the documentation and/or other materials provided 
with the distribution.

Neither the name of the <Georgia Institue of Technology> nor the names of its contributors 
may be used to endorse or promote products derived from this software without specific prior 
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.
"""



import os
import argparse


"""
Argument parsing
""" 
def process_options():
  parser = argparse.ArgumentParser(description='patch.py')
  parser.add_argument('-create', action='store_true', dest='create', help='disable output compression')
  parser.add_argument('-apply', action='store_true', dest='apply', help='disable output compression')

  return parser


"""
Main function
"""
def main():
  # parse arguments
  parser = process_options()
  args = parser.parse_args()


  ## apply internal patch
  if args.apply:
    if not os.path.exists('macsim-internal.patch'):
      print('macsim-internal.patch does not exist.')
      return
    os.system('patch -p0 -i macsim-internal.patch')
  ## create internal patch
  elif args.create:
    if os.path.exists('macsim-internal.patch.temp'):
      os.system('rm -f macsim-internal.patch.temp')

    os.system('svn diff > macsim-internal.patch.temp')

    if os.path.exists('macim-internal.patch'):
      os.system('rm -f macsim-internal.patch')
    os.system('mv macsim-internal.patch.temp macsim-internal.patch')


if __name__ == '__main__':
  main()
