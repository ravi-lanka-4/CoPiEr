'''Trains a simple convnet on the MNIST dataset.
Gets to 99.25% test accuracy after 12 epochs
(there is still a lot of margin for parameter tuning).
16 seconds per epoch on a GRID K520 GPU.
'''

from __future__ import print_function
import numpy as np
import random, csv
import argparse
import keras
import tqdm as T
from keras.callbacks import EarlyStopping
from keras.datasets import mnist
from keras.models import Sequential, Model
from keras.layers import Dense, Dropout, Flatten
from keras.layers import Conv2D, MaxPooling2D
from keras.layers.normalization import BatchNormalization
from keras import regularizers
from keras import backend as K
from keras.layers import Activation, Dense, Input, Lambda
from keras.utils import plot_model
from keras.backend import constant

def getModel(length, width, inp_shape1, l1=16, l2=16, outSize=64, addSig=True):
  # Create Model
  outSize=64
  model = Sequential()
  model.add(Dense(l1, input_dim=width, \
                    kernel_regularizer=regularizers.l2(10**-3), \
                    activation='relu'))
  #model.add(BatchNormalization(momentum=0.6))
  model.add(Dense(128, activation='relu'))
  #model.add(BatchNormalization(momentum=0.6))
  model.add(Dense(outSize, activation='relu'))
  #model.add(BatchNormalization(momentum=0.6))

  model1 = Sequential()
  model1.add(Dense(l1, input_shape=(outSize + inp_shape1, ), \
                      kernel_regularizer=regularizers.l2(10**-3)))
  model1.add(Activation('relu'))
  #model1.add(BatchNormalization(momentum=0.6))
  model1.add(Dense(l2, kernel_regularizer=regularizers.l2(10**-3)))
  model1.add(Activation('relu'))
  #model1.add(BatchNormalization(momentum=0.6))
  model1.add(Dense(1))
  if addSig:
    model1.add(Activation('sigmoid'))

  # Choose the index and shuffle the columns a bit -- maybe inefficient
  offset = int((length*width + inp_shape1)/2)

  len_ = length*width
  inp = Input((len_ + inp_shape1, ), dtype='float32')
  auxLen = int(inp_shape1/2)
  inp_1 = Lambda(lambda x: x[:,auxLen:offset])(inp)
  inp_2 = Lambda(lambda x: x[:,auxLen+offset:])(inp)
  inp1 = keras.layers.concatenate([inp_1, inp_2], axis=-1)

  model_output = []
  for i in range(length):
    in_ = Lambda(lambda x: x[:, (i*width):((i+1)*width)])(inp1)
    model_output.append(model(in_))
  merged = keras.layers.Add()(model_output)
  merged = Lambda(lambda x: x/length, input_shape=(outSize, ))(merged)

  # We can then concatenate the two vectors:
  inp_3 = Lambda(lambda x: x[:, :auxLen])(inp)
  inp_4 = Lambda(lambda x: x[:, offset:offset+auxLen])(inp)
  inp2 = keras.layers.concatenate([inp_3, inp_4], axis=-1)
  merged_vector = keras.layers.concatenate([merged, inp2], axis=-1)

  # Defines the model for ranknet
  out = model1(merged_vector)

  return (inp, out, model, model1)
   
def getData(inpF, step):
  datamap = {}
  for i in range(21):
    datamap[i] = []

  with open(inpF, 'r') as csvfile:
    reader = csv.reader(csvfile)
    for row in reader:
      crow = [float(l) for l in row]
      length = int(crow[0])
      width = int(crow[1])
      obstdata = crow[2:(length*width)+2]
      otherdata = crow[2+(length*width):-4]
      output = crow[-4:]
      datamap[length].append((length, width, obstdata, otherdata, output))

  # shuffle input
  for i in range(21):
    random.shuffle(datamap[i])

  length = datamap[step][0][0]
  width = datamap[step][0][1]
  obstdata = []
  otherdata = []
  output = []
  for i in range(len(datamap[step])):
    obstdata.append(datamap[step][i][2])
    otherdata.append(datamap[step][i][3])
    output.append(datamap[step][i][-1])

  return (length, width, obstdata, otherdata, output)

##############################
##############################
##############################
##############################

def main(inpF, nsteps, modelF=None, modelF1=None):

  (length, width, obstdata, otherdata, output) = getData(inpF, nsteps)

  batch_size = 128
  epochs = 500

  N = 5
  trainN = int(0.8*len(obstdata))
  testN = len(obstdata) - trainN
  input_shape = width
  inp_shape1 = len(otherdata[0])

  (inp, out, model, model1) = getModel(length, width, inp_shape1)

  # Build model.
  allModel = Model(inputs=inp, outputs=out)
  allModel.compile(loss=keras.losses.categorical_crossentropy,
                optimizer=keras.optimizers.Adadelta(),
                metrics=['categorical_crossentropy', 'accuracy'])

  # Fit data
  if (modelF == None) and (modelF1 == None):
    xnew_train = np.hstack((obstdata, otherdata))[:trainN]
    xnew_test = np.hstack((obstdata, otherdata))[trainN:]
    ynew_train = output[:trainN]
    ynew_test  = output[trainN:]

    early_stopping = EarlyStopping(monitor='val_acc', patience=20)
    allModel.fit(xnew_train, ynew_train,
                 batch_size=batch_size,
                 epochs=epochs,
                 verbose=1,
                 shuffle=True,
                 callbacks=[early_stopping],
                 validation_data=(xnew_test, ynew_test))
    model.save_weights('model.h5')
    model1.save_weights('model1.h5')
  else:
    model.load_weights(modelF)
    model1.load_weights(modelF1)
    xnew = np.hstack((obstdata, otherdata))
    ynew = output
    predacc = allModel.evaluate(xnew, ynew)[-1]
    print('Prediction Acc: %f'%predacc) 
    return predacc

def firstPassCommandLine():

  # Creating the parser for the input arguments
  parser = argparse.ArgumentParser(description='Pruning network')

  # Positional argument - Input XML file
  parser.add_argument('--i', type=str, default='/scratch/als690/newdata/step1.txt',
                       help='Input File', dest='inpF')
  parser.add_argument('--s', type=int, default=10,
                       help='Num of steps', dest='step')
  parser.add_argument('--m0', type=str, default='./model.h5',
                       help='First model', dest='modelF')
  parser.add_argument('--m1', type=str, default='./model1.h5',
                       help='second model', dest='modelF2')
  #parser.add_argument('--m0', type=str, default=None,
  #                     help='First model', dest='modelF')
  #parser.add_argument('--m1', type=str, default=None,
  #                     help='second model', dest='modelF1')

  # Parse input
  args = parser.parse_args()
  return args

if __name__ == '__main__':
  args = firstPassCommandLine()
  main(args.inpF, args.step, args.modelF, args.modelF1)

