from searchnet import getRankModel, load_weight, getRank
from prunenet import load_svmlight
from sklearn.metrics import precision_score, recall_score
from keras.layers import Dense
import numpy as np
import argparse
import warnings

warnings.simplefilter("ignore")

def main():
  args = firstPassCommandLine()
  dataF1 = args.dataF + '.1'
  dataF2 = args.dataF + '.2'
  modelF = args.modelF

  # Load sample weights
  data1, labels1 = load_svmlight(dataF1)
  data2, labels2 = load_svmlight(dataF2)
  assert labels1[0] > labels2[1], "First data file not label 1"
  assert data1.shape[0] == data2.shape[0], "Input data of different shape"
  data1 = np.array(data1.todense())
  data2 = np.array(data2.todense())

  # Read model
  _, getScore, _ = getRankModel(data1.shape[1])
  getScore.add(Dense(1))
  getScore.add(Dense(1))
  getScore.load_weights(modelF)

  # Save mod
  ## Generate scores from document/query features.
  relS = getScore.predict(data1, batch_size=250, verbose=1)
  irrS = getScore.predict(data2, batch_size=250, verbose=1)
  allScore = np.hstack((relS.flatten(), irrS.flatten()))
  maxScore = np.max(allScore)
  minScore = np.min(allScore)
  print('Max: %f Min: %f'%(maxScore, minScore))
  scaleFactor = 1/(maxScore-minScore)

  idx = np.where(relS < irrS)
  print(idx)
  
  getRank(relS, irrS, 'data')
  return

def firstPassCommandLine():

  # Creating the parser for the input arguments
  parser = argparse.ArgumentParser(description='Pruning network')

  # Positional argument - Input XML file
  parser.add_argument('-dataF', '--t', type=str, \
                       default='./trj/train.trj.1.norm',
                       help='Prune Data File', dest='dataF')
  parser.add_argument('-selectmodel', '--m', type=str, \
                       default='./policy/data/psulu/ts_10/train/nn_psulu/searchPolicy.0_keras.h5', \
                       help='Select model file name', dest='modelF')

  # Parse input
  args = parser.parse_args()
  return args

if __name__ == '__main__':
  main()
