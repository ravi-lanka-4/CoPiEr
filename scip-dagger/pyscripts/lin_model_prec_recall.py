from __future__ import division
import argparse
import numpy as np
import csv, pickle, os

# svm dataset
from sklearn.datasets import load_svmlight_file
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import accuracy_score, precision_score, recall_score

def load_svmlight(fname):
  print('Loading %s'%fname)
  return load_svmlight_file(fname)

def main():
  args = firstPassCommandLine()
  preds = args.preds
  train_data = args.trainD

  _, train_labels = load_svmlight(train_data)
  pred_labels = np.loadtxt(preds)
  print("Accuracy = " + str(accuracy_score(train_labels, pred_labels)))
  print("Precision = " + str(precision_score(train_labels, pred_labels)))
  print("Recall = " + str(recall_score(train_labels, pred_labels)))

def firstPassCommandLine():

  # Creating the parser for the input arguments
  parser = argparse.ArgumentParser(description='Linear model precision recall')

  # Positional argument - Input XML file
  parser.add_argument('-predictions', '--p', type=str, \
                       default='./sample-data/search.valid.dat',
                       help='Linear model predictions', dest='preds')
  parser.add_argument('-train_data', '--t', type=str, \
                       default='./sample-data/search.valid.dat',
                       help='Linear model training data', dest='trainD')

  # Parse input
  args = parser.parse_args()
  return args

if __name__ == '__main__':
  main()