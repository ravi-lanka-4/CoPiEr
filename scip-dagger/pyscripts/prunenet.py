from __future__ import division
import argparse
import numpy as np
import csv, pickle, os
#import matplotlib.pyplot as plt
from searchnet import load_weight
import pprint as P

# Keras
from keras import backend
from keras.callbacks import EarlyStopping
from keras.layers import Activation, Dense, Input, Lambda
from keras.layers.merge import Add
from keras.layers.core import Dropout
from keras.layers.normalization import BatchNormalization
from keras import regularizers
from keras.models import Sequential, Model
from keras.models import model_from_json
from keras import backend as be
from kerasify import export_model

# Sampling
from imblearn.over_sampling import RandomOverSampler

# svm dataset
from sklearn.datasets import load_svmlight_file
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import precision_score, recall_score

def getPruneModel(inp_dim, l1=16, l2=16, l3=16):
  # Defines the model for ranknet
  model = Sequential()
  #model.add(BatchNormalization(input_shape=(inp_dim, ), epsilon=1e-04, \
  #                                mode=0, momentum=0.9, weights=None))

  model.add(Dense(l1, input_dim=inp_dim, activation='relu'))
  model.add(Dropout(0.3))
  model.add(Dense(l2, activation='relu'))
  model.add(Dropout(0.3))
  model.add(Dense(l3, activation='relu'))
  model.add(Dense(1, activation='sigmoid'))

  model.compile(loss='binary_crossentropy',
                optimizer='adadelta',
                metrics=['accuracy'])

  return model

def load_svmlight(fname):
  print('Loading %s'%fname)
  return load_svmlight_file(fname)

def main():
  args = firstPassCommandLine()
  trainF    = args.pruneInpTrain
  validF    = args.pruneInpValid
  weightF   = args.pruneWeight
  validWeightF = args.pruneValidWeight
  testF     = args.pruneInpTest
  modelF    = args.pruneF
  nepoch    = args.nepoch
  batchSize = args.batchSize

  # Load sample weights
  weights   = load_weight(weightF)
  valid_weights = load_weight(validWeightF)

  # Load data in the svm rank file format
  trainfeats, trainlabels = load_svmlight(trainF)
  validfeats, validlabels = load_svmlight(validF)
  testfeats_exists = False

  #weights = weights[:len(trainlabels)] valid_weights = valid_weights[:len(validlabels)]

  if testF != None:
      testfeats, testlabels   = load_svmlight(testF)
      testfeats_exists = True
  else:
      testfeats  = None
      testlabels = None

  # convert to dense array
  trainfeats = np.array(trainfeats.todense())
  validfeats = np.array(validfeats.todense())
  if testfeats_exists:
      testfeats = np.array(testfeats.todense())

  # concatenate weights to data points for oversampling
  trainfeats_weights = np.hstack((trainfeats, np.reshape(weights, (len(weights), 1))))
  validfeats_weights = np.hstack((validfeats, np.reshape(valid_weights, (len(valid_weights), 1))))

  # Apply the random over-sampling
  ros = RandomOverSampler()
  trainfeats_weights_ros, trainlabels_ros = ros.fit_sample(trainfeats_weights, trainlabels)
  validfeats_weights_ros, validlabels_ros = ros.fit_sample(validfeats_weights, validlabels)
  trainfeats_ros = trainfeats_weights_ros[:, :-1]
  trainweights_ros = trainfeats_weights_ros[:, -1]
  validfeats_ros = validfeats_weights_ros[:, :-1]
  validweights_ros = validfeats_weights_ros[:, -1]

  if testfeats_exists:
     testfeats_ros, testlabels_ros = ros.fit_sample(testfeats, testlabels)

  # Reset labels (not needed anymore since for neural nets, we use labels of 0 and 1 in trj files)
  #trainlabels[np.where(trainlabels==-1)[0]] = 0
  #validlabels[np.where(validlabels==-1)[0]] = 0
  #if testfeats != None:
  #  testlabels[np.where(testlabels==-1)[0]] = 0

  # Create model
  INPUT_DIM = trainfeats.shape[1]
  pruneModel = getPruneModel(INPUT_DIM)
  pruneModel.summary()

  ## Train model.
  early_stopping = EarlyStopping(monitor='val_loss', patience=20)
  history = pruneModel.fit(trainfeats_ros, trainlabels_ros,
                           batch_size=batchSize, \
                           sample_weight=trainweights_ros,
                           epochs=nepoch, verbose=1,
                           validation_data=(validfeats_ros, validlabels_ros, validweights_ros),
                           callbacks=[early_stopping],
                           shuffle=True)
  # Evaluate on training
  train_preds = pruneModel.predict_classes(trainfeats)
  train_eval_ = pruneModel.evaluate(trainfeats, trainlabels, verbose=1)
  print("[Loss, accuracy] = " + str(train_eval_))
  print("Precision = " + str(precision_score(trainlabels, train_preds)))
  print("Recall = " + str(recall_score(trainlabels, train_preds)))

  train_preds = pruneModel.predict_classes(trainfeats)
  train_eval_ = pruneModel.evaluate(trainfeats, trainlabels, verbose=1)
  print("[Loss, accuracy] (randomly oversampled) = " + str(train_eval_))
  print("Precision (randomly oversampled) = " + str(precision_score(trainlabels, train_preds)))
  print("Recall (randomly oversampled) = " + str(recall_score(trainlabels, train_preds)))

  # Evaluate on test
  eval_ = pruneModel.evaluate(validfeats, validlabels, verbose=1)

  if testfeats_exists:
      eval_ = pruneModel.evaluate(testfeats, testlabels, verbose=1)
      print("[Test loss, test accuracy] = " + str(eval_))
      eval_ = pruneModel.evaluate(testfeats, testlabels, verbose=1)
      print("[Test loss, test accuracy] (randomly oversampled) = " + str(eval_))

  # Save mode
  print('Creating: ' + modelF)
  export_model(pruneModel, modelF)
  pruneModel.save(modelF[:-3] + "_keras.h5")
  del pruneModel 
  be.clear_session()
  return

def firstPassCommandLine():

  # Creating the parser for the input arguments
  parser = argparse.ArgumentParser(description='Pruning network')

  # Positional argument - Input XML file
  parser.add_argument('-pruneValidPrefix', '--v', type=str, \
                       default='./sample-data/kill.valid.dat',
                       help='Prune Valid Input File prefix', dest='pruneInpValid')
  parser.add_argument('-pruneTrainPrefix', '--t', type=str, \
                       default='./sample-data/kill.train.dat',
                       help='Prune Training Input prefix', dest='pruneInpTrain')
  parser.add_argument('-pruneTrainWeight', '--w', type=str, \
                       default='./sample-data/prune.weight',
                       help='Prune Training Input Sample Weight', dest='pruneWeight')
  parser.add_argument('-pruneValidWeight', '--x', type=str, \
                       default='./sample-data/prune.weight',
                       help='Prune Validation Input Sample Weight', dest='pruneValidWeight')
  parser.add_argument('-pruneTestPrefix', '--u', type=str, default=None,
                       help='Prune Test Input prefix', dest='pruneInpTest')
  parser.add_argument('-selectmodel', '--m', type=str, \
                       default='./models/model_keras_prune_1.h5', \
                       help='Select model file name', dest='pruneF')
  parser.add_argument('-nepoch', '--n', type=int, default=100, \
                       help='Number of epochs', dest='nepoch')
  parser.add_argument('-batchsize', '--b', type=int, default=250, \
                       help='Batch Size', dest='batchSize')

  # Parse input
  args = parser.parse_args()
  return args

if __name__ == '__main__':
  main()
