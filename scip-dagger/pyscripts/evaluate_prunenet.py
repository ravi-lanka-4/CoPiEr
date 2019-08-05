from prunenet import getPruneModel, load_svmlight
from sklearn.metrics import precision_score, recall_score
from searchnet import load_weight
import argparse
import numpy as np

def main():
  args = firstPassCommandLine()
  dataF    = args.dataF
  modelF    = args.modelF
  weightF   = args.weightF

  # Load sample weights
  weights   = load_weight(weightF)
  data, labels = load_svmlight(dataF)
  data = np.array(data.todense())

  # Read model
  model = getPruneModel(data.shape[1])
  model.load_weights(modelF)

  # Evaluate on training
  preds = model.predict_classes(data)
  #score = model.predict_proba(data)

  # Evaluate
  dataeval_ = model.evaluate(data, labels, verbose=1)
  print("\n")
  print("[Loss, accuracy] = " + str(dataeval_))
  print("Precision = " + str(precision_score(labels, preds)))
  print("Recall = " + str(recall_score(labels, preds)))


def firstPassCommandLine():

  # Creating the parser for the input arguments
  parser = argparse.ArgumentParser(description='Pruning network')

  # Positional argument - Input XML file
  parser.add_argument('-dataF', '--t', type=str, \
                       default='./trj/test.kill.trj.norm',
                       help='Prune Data File', dest='dataF')
  parser.add_argument('-weightF', '--w', type=str, \
                       default='./trj/test.kill.weight',
                       help='Weight Data File', dest='weightF')
  parser.add_argument('-selectmodel', '--m', type=str, \
                       default='./policy/data/psulu/ts_10/train/nn_psulu/killPolicy.1_keras.h5', \
                       help='Select model file name', dest='modelF')

  # Parse input
  args = parser.parse_args()
  return args

if __name__ == '__main__':
  main()
