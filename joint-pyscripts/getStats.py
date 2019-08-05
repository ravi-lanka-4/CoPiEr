import numpy as np
import argparse
import os
import random

gap_ = lambda folder, x, y: 'find %s/ -name "*.%s" | sort | xargs grep -Rs "Gap                :" {} > %s'%(folder, y, x)
primal_ = lambda folder, x, y: 'find %s/ -name "*.%s" | sort | xargs grep -Rs "Primal Bound       : " {} > %s'%(folder, y, x)
obj_ = lambda folder, x, y: 'find %s/ -name "*.%s" | sort | xargs grep -Rs "#Objective*" {} > %s'%(folder, y, x)

#tmpFile = '/tmp/tmp%s.txt'%(str(random.random()).strip('.'))
SCRATCH=os.environ['COTRAIN_SCRATCH'].replace('//', '/')

def main(inpFolder, parse, ftype, flag=True, nnodes='', log='log'):

  cid = str(random.random()).strip('.')
  if os.path.isdir(inpFolder):
    if 'log' in log:
      if not flag:
        tmpFile = SCRATCH + 'retro/primal_%s_%s_'%(nnodes, cid)  + parse + '.txt'
        print(primal_(inpFolder, tmpFile, log))
        os.system(primal_(inpFolder, tmpFile, log))
      else:
        tmpFile = SCRATCH + 'retro/dual_%s_%s_'%(nnodes, cid) + parse + '.txt'
        print(gap_(inpFolder, tmpFile, log))
        os.system(gap_(inpFolder, tmpFile, log))
    else:
      tmpFile = SCRATCH + 'retro/spat_%s_%s_'%(nnodes, cid) + parse + '.txt'
      print(obj_(inpFolder, tmpFile, log))
      os.system(obj_(inpFolder, tmpFile, log))
  else:
    print('Error: Input file does not exist')

  dual = []
  alldata = {}
  with open(tmpFile, 'r') as F:
    cdata = F.readlines()
    for cc in cdata:
      try:
        cc=cc.replace('//', '/')
        fname = os.path.basename(cc.split(':')[0])
        orig = '/' + cc.split(':')[0].replace(SCRATCH, '').replace('/' + ftype + '/', '/sol/').split(parse)[0]
        val = float(cc.split(': ')[1].strip('\n ').strip('%').split('(')[0])
      except:
        val = 300

      dual.append(min(val, 300))
      alldata[fname] = val

  if ('lpfiles' in inpFolder) or ('gpickle' in inpFolder):
      ext = '.sol'
  else:
      ext = 'mps.sol'

  if not flag:
      opt = []
      for key, val in alldata.iteritems():
        fname = os.path.join(orig, key.replace('.log', ext))
        if val == 300:
            opt.append(300)
        else:
            with open(fname, 'r') as F:
                lines = F.readlines()
                optVal = None
                for cline in lines:
                    cc = cline.lower()
                    if 'objective value' in cc:
                        optVal = float(cc.strip('# objective value = '))
                        break

                assert optVal != None, "Opt value not found"
                opt.append(100*(np.abs(val-optVal)/np.abs(optVal)))
  else:
      opt = dual

  print(opt)
  opt = np.array(opt)
  idx = np.where(opt < 250)[0]
  goodopt = opt[idx].tolist()
  mean = np.mean(opt)
  goodmean = np.mean(goodopt)
  num = len(opt) - len(idx)
  return (mean, goodmean, num)

def firstPassCommandLine():

  # Creating the parser for the input arguments
  parser = argparse.ArgumentParser(description='Generate data for spatial model')

  # Positional argument - Input XML file
  parser.add_argument('--i', type=str, default="/Users/subrahma/proj/maverick-setup/co-training/scratch/Users/subrahma/proj/maverick-setup/co-training/data/psulu_test/lpfiles/test/nn_psulu_0.1_cotrain_search_lp_0/",
                       help='input file', dest='inp')
  parser.add_argument('--o', type=str, default="out.txt",
                       help='output file', dest='out')
  parser.add_argument('--g', action='store_true',
                       help='Flag for only optimality gap', dest='graph')
  parser.add_argument('--s', type=str, default="lpfiles",
                       help='problem type', dest='ftype')
  parser.add_argument('--n', type=int, default=None,
                       help='Number of nodes', dest='nnodes')

  # Parse input
  args = parser.parse_args()
  return args

if __name__ == '__main__':
  args = firstPassCommandLine()
  fname = os.path.basename(os.path.dirname(args.inp))
  if args.nnodes == None:
    nnodes = ''
  else:
    nnodes = str(args.nnodes)

  if not args.graph:
    dual_vals = main(args.inp, fname, args.ftype, nnodes=nnodes)
  else:
    dual_vals = (np.nan, np.nan, 0)

  if not args.graph:
    primal_vals = main(args.inp, fname, args.ftype, False, nnodes)
  else:
    primal_vals = main(args.inp, fname, args.ftype, False, nnodes, 'sol')

  row = dual_vals + primal_vals
  with open(args.out,'a') as f:
      f.write(fname + '\t')
      for val in row:
          f.write('%.4f'%val + '\t')
      f.write('\n')

