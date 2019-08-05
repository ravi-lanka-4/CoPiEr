from sklearn import preprocessing
from sklearn.datasets import dump_svmlight_file, load_svmlight_file
import numpy as np
import os, argparse
import csv, warnings
# import matplotlib.pyplot as plt

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

def normalizeData(inpF, outF, normF):
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

  # normalize data
  data_minmax = min_max_scaler.fit_transform(data)

  if prune:
      dump_svmlight(data_minmax, label, outF)
  else:
      dump_svmlight(data_minmax[:marker], label, outF + '.1')
      dump_svmlight(data_minmax[marker:], clabel, outF + '.2')

  # Write norm params
  min_ = min_max_scaler.data_min
  max_ = min_max_scaler.data_max_
  writeCSV(zip(min_, max_), normF)

def mean_variance(inpF):
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
  
  return np.mean(data, 0), np.var(data, 0)
  
def main():
  args = firstPassCommandLine()
  inpF  = args.inpF
  meanF = args.meanF
  varF = args.varF
  mean, variance = mean_variance(inpF)
  np.savetxt(meanF, mean)
  np.savetxt(varF, variance)
  # print(variance)

  # example data
  # x = np.arange(0, len(mean))
  # y = mean

  # example variable error bar values
  # yerr = variance

  # First illustrate basic pyplot interface, using defaults where possible.
  # fig = plt.figure()
  # ax = fig.add_subplot(111)
  # ax.errorbar(x, y, yerr=yerr, fmt='o', ecolor='g', markersize=5)
  # plt.title("Mean and Variance of Node and Problem Features")

  # plt.show()

def firstPassCommandLine():

  # Creating the parser for the input arguments
  parser = argparse.ArgumentParser(description='Pruning network')

  # Positional argument - Input XML file
  parser.add_argument('-inpF', '--i', type=str, \
                       default='./sample-data/kill.train.dat',
                       help='Input data file/Prefix', dest='inpF')
  parser.add_argument('-meanF', '--m', type=str, \
                       default='./sample-data/kill.train.dat',
                       help='Mean data file', dest='meanF')
  parser.add_argument('-varF', '--v', type=str, \
                       default='./sample-data/kill.train.dat',
                       help='Variance data file', dest='varF')

  # Parse input
  args = parser.parse_args()
  return args

if __name__ == '__main__':
  main()
