#!/usr/bin/python

import sys

def usage():
  print 'usage: {0:s} <multiplication table fn> <show coeffs: no/yes> <sought terms lhs> <sought terms rhs> <context before> <context after>'.format(sys.argv[0])
  exit(1)

if len(sys.argv) != 7:
  usage()

fn = sys.argv[1]
f_coeff = sys.argv[2].lower()
search_lhs = set(sys.argv[3].lower().split())
search_rhs = set(sys.argv[4].lower().split())

search_lhs = search_lhs if len(search_lhs) > 0 else None
search_rhs = search_rhs if len(search_rhs) > 0 else None

n_cb = int(sys.argv[5])
n_ca = int(sys.argv[6])

if f_coeff == 'yes':
  f_coeff = True
elif f_coeff == 'no':
  f_coeff = False
else:
  usage()

f = open(fn)

for i in range(2):
  t = f.readline()
  t = t.rstrip()
  t = t == 'Multiplication table:'
  if t:
    break
if not t:
  print 'Did not find expected header'
  exit(1)

lhs = ''
rhs = ''

buffer = [[False, ''] for i in range(n_cb + 1 + n_ca)]

def h1():
  global lhs, rhs, f_coeff

  t = False
  s = ''

  if lhs != '' and rhs != '':
    rhs = rhs.replace('+', ' + ')

    lhs = ' '.join(lhs.split())
    rhs = ' '.join(rhs.split())

    lhs_terms = lhs.replace('(', '').replace(')', '').split('*')

    rhs_terms = rhs.replace('+', '').split()
    rhs_terms = filter(lambda x: x[0] == 'b', rhs_terms)

    nt = len(rhs_terms)

    if not f_coeff:
      rhs = ' + '.join(rhs_terms)

    t0 = search_lhs == None and search_rhs == None
    t1 = search_lhs != None and search_rhs != None
    t2 = search_lhs != None and set(lhs_terms) & search_lhs == search_lhs
    t3 = search_rhs != None and set(rhs_terms) & search_rhs == search_rhs 
    t = t0 or (not t1 and t2) or (not t1 and t3) or (t1 and t2 and t3)
        
    s = '{0:d} : {1:s} = {2:s}'.format(nt, lhs, rhs)

    lhs = ''
    rhs = ''

  return t, s


def h2():
  global lhs, rhs, f_coeff, context_before, context_after

  if buffer[n_cb][0]:
    if n_cb == 0 and n_ca == 0:
      print '{0:s}'.format(buffer[n_cb][1])
    else:
      for i in range(len(buffer)):
        if buffer[i][1] != '':
          print '{0:+d} : {1:s}'.format(i - n_cb, buffer[i])
      print '-' * 60


def h3():
  global buffer

  buffer = buffer[1:]
  buffer += [h1()]

  h2()

 
for line in f:
  line = line.rstrip()
  
  if line == '':
    continue

  if line[0] != ' ':
    h3()

    lhs = line
  else:
    if rhs != '':
      rhs += ' + ' + line
    else:
      rhs = line

if lhs != '' and rhs != '':
  h3()
