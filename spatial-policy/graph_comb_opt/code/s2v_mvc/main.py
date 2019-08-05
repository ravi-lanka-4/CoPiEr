from __future__ import division
import numpy as np
import networkx as nx
import cPickle as cp
import random
import ctypes
import os
import sys
from tqdm import tqdm
import pickle
import glob
#import faulthandler

sys.path.append( '%s/mvc_lib' % os.path.dirname(os.path.realpath(__file__)) )
from mvc_lib import MvcLib
#faulthandler.enable()

def readOpt(solF):
    # Read optimal solution file
    opt = []
    with open(solF, 'r') as F:
      data = F.readlines()
      print(solF)
      for cdata in data:
          if '#' == cdata[0]:
            continue
          elif 'objective' in cdata:
            continue
          elif 'obj' in cdata:
            idx = cdata.find(' ')
            var = cdata[:idx]
            cdata = cdata[idx:]
            cdata = cdata.strip('\t').strip(' ')
            val = cdata.split('(')[0]
            var = int(var.strip('v'))
            val = float(val)
            if val > 0.6:
              opt.append(var)
          else:
            (var, val) = cdata.split(' ')
            var = int(var.strip('v'))
            val = int(val.strip('\n'))
            if val == 1:
              opt.append(var)

    return opt

def sup_gen_new_graphs(opt, prob=0.5, clear=False):
    print 'generating new training graphs'
    sys.stdout.flush()
    if clear:
      print('Clearning supervised data points')
      api.ClearTrainGraphs()

    dataF = opt['supDataF']
    solFolder = opt['supSolPrefix']

    for cF in glob.glob(dataF + '/*gpickle'):
        try:
          with open(cF, 'rb') as F:
              g = pickle.load(F)

          # Read optimal solution
          solF = solFolder + '/' + os.path.basename(cF).replace('.gpickle', '.sol')
          optSol = readOpt(solF)
          api.InsertGraph(g, False, optSol, prob)
        except:
          #print('Sol file missing: %s; skipping'%solF)
          pass

    return

def gen_new_graphs(opt, clear=False):
    print 'generating new training graphs'
    sys.stdout.flush()
    if clear:
      print('Clearning data points')
      api.ClearTrainGraphs()
    dataF = opt['dataF']

    for cF in glob.glob(dataF + '/*gpickle'):
        with open(cF, 'rb') as F:
            g = pickle.load(F)

        api.InsertGraph(g, is_test=False)

    return

def PrepareValidData(opt):
    print 'generating validation graphs'
    sys.stdout.flush()
    dataF = opt['validF']
    n_valid = 0
    for cF in glob.glob(dataF + '/*gpickle'):
        with open(cF, 'rb') as F:
            g = pickle.load(F)
        api.InsertGraph(g, is_test=True)
        n_valid = n_valid + 1

    return n_valid

def find_model_file(opt):
    max_n = int(opt['max_n'])
    min_n = int(opt['min_n'])
    log_file = None
    if max_n < 100:
	return None
    if min_n == 50 and max_n == 100:
        return None
    elif min_n == 100 and max_n == 200:
        n1 = 50
        n2 = 100
    else:
        n1 = min_n - 100
        n2 = max_n - 100

    log_file = '%s/log-%d-%d.txt' % (opt['save_dir'], n1, n2)
    if not os.path.isfile(log_file):
	return None
    best_r = -1000000
    best_it = -1
    with open(log_file, 'r') as f:
        for line in f:
            if 'average' in line:
                line = line.split(' ')
                it = int(line[1].strip())
                r = -float(line[-1].strip())
                if it > 10000 and r > best_r:
                    best_r = r
                    best_it = it
    if best_it < 0:
        return None
    print best_it, best_r
    return '%s/nrange_%d_%d_iter_%d.model' % (opt['save_dir'], n1, n2, best_it)
    
if __name__ == '__main__':
    api = MvcLib(sys.argv)
    
    opt = {}
    for i in range(1, len(sys.argv), 2):
        opt[sys.argv[i][1:]] = sys.argv[i + 1]

    model_file=opt['modelF']
    if os.path.isfile(model_file) and (int(opt['citer']) != 0):
        print 'loading', model_file
        sys.stdout.flush()
        api.LoadModel(model_file)

    # preparation validation
    n_valid = PrepareValidData(opt)
 
    # Supervised or RL
    eps_start = 1.0
    eps_end = 0.05
    eps_step = int(opt['eps_step']) #10000.0

    # Now add unlabeled data
    sup_gen_new_graphs(opt, prob=float(opt['sup_prob']), clear=True)

    # Pre start with supervised data
    init_iter=200
    init_samp=32

    if (int(opt['citer']) != 0):
      opt['citer'] = int(opt['citer']) + init_iter

    elif (int(opt['citer']) == 0) and (float(opt['sup_prob']) != 0):
      print('**********************')
      print('**    Co-training   **')
      print('**********************')

      # startup only for the first iteration
      for iter in range(init_iter):
        eps = eps_end + max(0., (eps_start - eps_end) * (eps_step - iter - int(opt['citer'])) / eps_step)

        if iter % init_samp == 0:
          api.lib.PlayGame(init_samp, ctypes.c_double(eps), True)

        if iter % 300 == 0:
            frac = 0.0
            for idx in range(n_valid):
                frac += api.lib.Test(idx)
            print 'iter', iter , 'eps', eps, 'average size of vc: ', frac / n_valid
            sys.stdout.flush()

        if iter == 0:
          api.TakeSnapshot()

        # Perform supervised learning on these labeled data points
        api.lib.Fit()

      gen_new_graphs(opt)
      opt['citer'] = int(opt['citer']) + init_iter # Offset initial iter with supervised training iterations

    elif (int(opt['citer']) == 0) and int(opt['behavior_cloning']):
      # Behavior cloning
      print('**********************')
      print('** Behavior cloning **')
      print('**********************')

      assert int(opt['sup_prob']) == 0, "Supervised probability not zero; so forcing IL regularizer for cotraining"

      # startup only for the first iteration
      for iter in range(init_iter):
        eps = eps_end + max(0., (eps_start - eps_end) * (eps_step - iter - int(opt['citer'])) / eps_step)
        if iter % init_samp == 0:
          api.lib.PlayGame(init_samp, ctypes.c_double(eps), True)

        if iter % 300 == 0:
            frac = 0.0
            for idx in range(n_valid):
                frac += api.lib.Test(idx)
            print 'iter', iter , 'eps', eps, 'average size of vc: ', frac / n_valid
            sys.stdout.flush()

        if iter == 0:
          api.TakeSnapshot()

        # Perform supervised learning on these labeled data points
        api.lib.Fit()

      # Now add unlabeled data
      gen_new_graphs(opt)
      opt['citer'] = int(opt['citer']) + init_iter # Offset initial iter with supervised training iterations

    elif (int(opt['citer']) == 0):
      # RL only
      print('**********************')
      print('****** RL only *******')
      print('**********************')

      # No work; just increase maximum iterations
      opt['max_iter'] = int(opt['max_iter']) + init_iter

    best_model = None
    best_test_error = None

    for iter in range(int(opt['max_iter'])):
        eps = eps_end + max(0., (eps_start - eps_end) * (eps_step - iter - int(opt['citer'])) / eps_step)
        if iter % 10 == 0:
            api.lib.PlayGame(10, ctypes.c_double(eps), False)

        if iter % 300 == 0:
            frac = 0.0
            for idx in range(n_valid):
                frac += api.lib.Test(idx)
            print 'iter', iter + int(opt['citer']), 'eps', eps, 'average size of vc: ', frac / n_valid
            sys.stdout.flush()
            model_path = '%s/nrange_%d_iter_%d.model' % (opt['save_dir'], int(opt['citer']), iter)
            api.SaveModel(model_path)
            
            if (best_model == None) or (best_test_error > float(frac/n_valid)):
              best_model = model_path
              best_test_error = float(frac/n_valid)

        if iter % 1000 == 0:
            api.TakeSnapshot()

        api.lib.Fit()

    frac = 0.0
    for idx in range(n_valid):
        frac += api.lib.Test(idx)
    print 'iter', iter + int(opt['citer']), 'eps', eps, 'average size of vc: ', frac / n_valid
    sys.stdout.flush()
    model_path = '%s/nrange_%d_iter_%d.model' % (opt['save_dir'], int(opt['citer']), iter)
    api.SaveModel(model_path)
    
    if (best_model == None) or (best_test_error > float(frac/n_valid)):
      best_model = model_path
      best_test_error = float(frac/n_valid)

    # To do: Jialin? Should get the best model and save it here!
    #api.SaveModel(model_file)

    # Save the best model in the set of current iteration
    # Used for inference
    best_model_file = model_file.replace('_best.model', '_best%d.model'%(int(opt['citer'])))
    if best_model != None:
      os.system('cp %s %s'%(best_model, best_model_file))

    # Used for next iteration of cotraining so only uses  model_path
    os.system('cp %s %s'%(best_model, model_file))
    print('cp %s %s'%(best_model, model_file))
    

