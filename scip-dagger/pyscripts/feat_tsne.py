from sklearn import preprocessing
from sklearn.datasets import dump_svmlight_file, load_svmlight_file
from sklearn.manifold import TSNE
import numpy as np
import os, argparse
import csv, warnings
import matplotlib.pyplot as plt

warnings.filterwarnings("ignore",category=DeprecationWarning)

def writeCSV(data, normF):
  print('Writing %s'%normF)
  with open(normF, "wb") as csv_file:
      writer = csv.writer(csv_file, delimiter=',')
      for line in data:
         writer.writerow(line)
  return

def dump_svmlight(data, label, fname):
  print('Writing %s'%fname)
  dump_svmlight_file(data, label, fname) 

def load_svmlight(fname):
  print('Loading %s'%fname)
  return load_svmlight_file(fname)

def tsne(inpF):
  if os.path.isfile(inpF):
      # Pruning data
      data, label = load_svmlight(inpF)
      data = np.array(data.todense())
      prune = True
  else:
      # Searching data
      prune = False
      data, label = load_svmlight(inpF + '.1')
      data = np.array(data.todense())
      marker = len(label)
      cdata, clabel = load_svmlight(inpF + '.2')
      cdata = np.array(cdata.todense())

      # compile the 2 data sets
      data = np.vstack((data, cdata))
  model = TSNE()
  return model.fit_transform(data)

  
def main():
  args = firstPassCommandLine()
  inpF  = args.inpF
  outF  = args.outF
  tsne_embedding = tsne(inpF)
  np.save(outF)

  # First illustrate basic pyplot interface, using defaults where possible.
  '''
  fig = plt.figure()
  ax = fig.add_subplot(111)
  plt.scatter(tsne_embedding[:, 0], tsne_embedding[:, 1])
  plt.title("TSNE Embedding of Node Features")
  '''

  plt.show()

def firstPassCommandLine():

  # Creating the parser for the input arguments
  parser = argparse.ArgumentParser(description='Pruning network')

  # Positional argument - Input XML file
  parser.add_argument('-inpF', '--i', type=str, \
                       default='./sample-data/kill.train.dat',
                       help='Input data file/Prefix', dest='inpF')
  # Positional argument - Input XML file
  parser.add_argument('-outF', '--o', type=str, \
                       default='./sample-data/kill.train.dat',
                       help='Output data file', dest='outF')

  # Parse input
  args = parser.parse_args()
  return args

if __name__ == '__main__':
  main()
