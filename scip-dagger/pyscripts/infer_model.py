from __future__ import division
import argparse
from searchnet import getRankModel, load_svmlight
from keras.layers import Activation, Dense, Input, Lambda
import numpy as np

def main(inpdim, modelF, data1, data2, numPolicies, batch=250):
  _, getScore, _ = getRankModel(inpdim)
  for k in range(numPolicies):
     _, getScore, _ = getRankModel(inpdim)
     # Normalization layers
     getScore.add(Dense(1, input_shape=(1,)))
     getScore.add(Dense(1, input_shape=(1,)))

     cfname = modelF.replace(".h5", "_%d.h5"%k)
     getScore.load_weights(cfname)
     if k == 0:
       relS = getScore.predict(data1, batch_size=batch, verbose=1).T[0]
       irrS = getScore.predict(data2, batch_size=batch, verbose=1).T[0]
     else:
       relS = relS + getScore.predict(data1, batch_size=batch, verbose=1).T[0]
       irrS = irrS + getScore.predict(data2, batch_size=batch, verbose=1).T[0]

  relS = relS/numPolicies
  irrS = irrS/numPolicies
  count = np.where(relS > irrS)[0].shape[0]
  print(np.where(relS < irrS))
  print(count/relS.shape[0])
  return
 

def firstPassCommandLine():

  # Creating the parser for the input arguments
  parser = argparse.ArgumentParser(description='Pruning network')

  # Positional argument - Input XML file
  parser.add_argument('-searchTestPrefix', '--t', type=str, \
                       default='./sample-data/input104.search.trj',
                       help='search Testing Input prefix', dest='searchInpTest')
  parser.add_argument('-selectmodel', '--m', type=str, \
                       default='/Users/subrahma/proj/maverick-setup/co-training/scip-dagger/policy/Users/subrahma/proj/maverick-setup/co-training/data/psulu_small_a/mps/train/nn_psulu_0.25_cotrain_search_mps_1/searchPolicy.0_keras.h5', \
                       help='Select model file name', dest='searchF')
  parser.add_argument('-batchsize', '--b', type=int, default=250, \
                       help='Batch Size', dest='batchSize')
  parser.add_argument('-inpdim', '--i', type=int, default=1188, \
                       help='Input Dimension', dest='inpdim')
  parser.add_argument('-num', '--n', type=int, default=2, \
                       help='Num of policies', dest='num')

  # Parse input
  args = parser.parse_args()
  return args

if __name__ == '__main__':
  args = firstPassCommandLine()
  testfeats1, testlabels1 = load_svmlight(args.searchInpTest + '.1')
  testfeats2, testlabels2 = load_svmlight(args.searchInpTest + '.2')
  main(args.inpdim, args.searchF, testfeats1, testfeats2, args.num, args.batchSize)
