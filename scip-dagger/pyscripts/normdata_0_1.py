from sklearn import preprocessing
from sklearn.datasets import dump_svmlight_file, load_svmlight_file
import numpy as np
import os, argparse
import csv, warnings

warnings.filterwarnings("ignore",category=DeprecationWarning)

def writeCSV(data, normF):
  print('Writing %s'%normF)
  with open(normF, "wb") as csv_file:
      writer = csv.writer(csv_file, delimiter=',')
      for line in data:
         writer.writerow(line)
  return

def dump_svmlight(data, label, fname, zero_based):
  print('Writing %s'%fname)
  dump_svmlight_file(data, label, fname, zero_based) 

def load_svmlight(fname):
  print('Loading %s'%fname)
  return load_svmlight_file(fname)

def normalizeData(inpF, outF, normF, numProbFeat):
  min_max_scaler = preprocessing.MinMaxScaler()
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

  numNodeFeat = data.shape[1] - numProbFeat
  # normalize data (only node features)
  data_minmax = min_max_scaler.fit_transform(data[:, :numNodeFeat])
  # concatenate node features with problem features
  data_minmax = np.hstack((data_minmax, data[:, numNodeFeat:]))

  if prune:
      dump_svmlight(data_minmax, label, outF, False)
  else:
      dump_svmlight(data_minmax[:marker], label, outF + '.1', False)
      dump_svmlight(data_minmax[marker:], clabel, outF + '.2', False)

  # Write norm params, concatenating with zeros for min and ones for max for problem features
  # Hence, normalization will not change the problem features at all
  min_ = np.concatenate((min_max_scaler.data_min_, np.zeros(numProbFeat)))
  max_ = np.concatenate((min_max_scaler.data_max_, np.ones(numProbFeat)))
  writeCSV(zip(min_, max_), normF)

def main():
  args = firstPassCommandLine()
  inpF  = args.inpF
  outF  = args.outF
  normF = args.normF
  numProbFeat = args.numProbFeat
  normalizeData(inpF, outF, normF, numProbFeat)

def firstPassCommandLine():

  # Creating the parser for the input arguments
  parser = argparse.ArgumentParser(description='Pruning network')

  # Positional argument - Input XML file
  parser.add_argument('-inpF', '--i', type=str, \
                       default='./sample-data/kill.train.dat',
                       help='Input data file/Prefix', dest='inpF')
  parser.add_argument('-outF', '--o', type=str, \
                       default='./sample-data/kill.norm.train.dat',
                       help='Output data file/Prefix', dest='outF')
  parser.add_argument('-normF', '--n', type=str, \
                       default='./sample-data/kill.normparam.dat',
                       help='Norm params', dest='normF')
  parser.add_argument('-numFeat', '--f', type=int, default=0, 
                       help='Num Problem Features', dest='numProbFeat')

  # Parse input
  args = parser.parse_args()
  return args

if __name__ == '__main__':
  main()
