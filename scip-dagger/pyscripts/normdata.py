from sklearn import preprocessing
from sklearn.datasets import dump_svmlight_file, load_svmlight_file
import numpy as np
import os, argparse
import csv, warnings

warnings.filterwarnings("ignore",category=DeprecationWarning)

def writeCSV(data, normF):
  print('Writing %s'%normF)
  with open(normF, "w") as csv_file:
      writer = csv.writer(csv_file, delimiter=',')
      for line in data:
         writer.writerow(line)
  return

def readCSV(normInpF):
  norm_min = []
  norm_max = []
  print('Reading %s'%normInpF)
  with open(normInpF, "r") as csv_file:
      reader = csv.reader(csv_file, delimiter=',')
      for line in reader:
         norm_min.append(float(line[0]))
         norm_max.append(float(line[1]))
      norm_min = np.array(norm_min)
      norm_max = np.array(norm_max)
  return norm_min, norm_max

def dump_svmlight(data, label, fname, numFeats):
  print('Writing %s'%fname)
  print('Feat Length %d'%numFeats)
  allzeros = np.zeros((1, numFeats))
  data = np.vstack((allzeros, data))
  label = np.hstack((numFeats, label))
  dump_svmlight_file(data, label, fname) 
  return

def load_svmlight(fname):
  print('Loading %s'%fname)
  return load_svmlight_file(fname)

def override(data):
  '''
  Override the normalization of data in range 
  '''
  flag = []
  for idx in range(data.shape[1]):
    cdata = data[:, idx]
    lidx = np.where(cdata < -2)[0]
    gidx = np.where(cdata > 2)[0]
    if (len(lidx) > 0) or (len(gidx) > 0):
        flag.append(0)
    else:
        flag.append(1)

  return flag

def normalizeData(inpF, outF, normF, normInpF, dump=True):
  min_max_scaler = preprocessing.MinMaxScaler(feature_range=(-1, 1))
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

  # Normalize everything -- reduces issues
  numNodeFeat = data.shape[1]

  # input norm file not specified, normalize data to [-1, 1]
  if normInpF is None:
      # normalize data (only node features) to [-1, 1] 
      data_minmax = min_max_scaler.fit_transform(data)
      flag = override(data)
      idx = np.where(flag)[0]
      min_max_scaler.data_min_[idx] = -1
      min_max_scaler.data_max_[idx] = 1
      for j, idx in enumerate(flag):
          if idx == 1:
              data_minmax[:, j] = data[:, j]

      # Write norm params (we pad with zeros for problem features mins and maxes 
      writeCSV(zip(min_max_scaler.data_min_, min_max_scaler.data_max_), normF)

  # input norm file specified, use it to normalize
  else:
      data_min, data_max = readCSV(normInpF)
      data_minmax = []
      # Normalize data (only node features) using input normalization file
      for i in range(numNodeFeat):
          norm_denom = data_max[i] - data_min[i]
          if norm_denom == 0:
              norm_denom = 1
          norm_elem = ((2 * (data[:, i] - data_min[i]))/norm_denom) - 1
          data_minmax.append(norm_elem)

      # Note that we built a list of columns of normalized data, so we need to transpose
      data_minmax = np.transpose(np.array(data_minmax))

  if dump:
      if prune:
          dump_svmlight(data_minmax, label, outF,  numNodeFeat)
      else:
          dump_svmlight(data_minmax[:marker], label, outF + '.1', numNodeFeat)
          dump_svmlight(data_minmax[marker:], clabel, outF + '.2', numNodeFeat)
  else:
      if prune:
          return data_minmax, label
      else:
          return data_minmax[:marker], label, data_minmax[marker:], clabel 

def main():
  args = firstPassCommandLine()
  inpF  = args.inpF
  outF  = args.outF
  normF = args.normF
  normInpF = args.normInpF
  normalizeData(inpF, outF, normF, normInpF)

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
                       default=None,
                       help='Output norm params', dest='normF')
  parser.add_argument('-normInpF', '--p', type=str, \
                       default=None,
                       help='Input norm params', dest='normInpF')

  # Parse input
  args = parser.parse_args()
  return args

if __name__ == '__main__':
  main()
