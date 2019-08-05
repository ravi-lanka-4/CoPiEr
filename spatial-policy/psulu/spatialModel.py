from __future__ import division
from __future__ import print_function
import keras, glob, imageio
from scipy import misc
import csv, os, argparse
import numpy as np
from keras import regularizers
from keras.datasets import mnist
from keras.models import Sequential
from keras.layers import Dense, Dropout, Flatten, Input, Lambda
from keras.layers import Conv2D, MaxPooling2D
from keras import backend as be
from keras import backend as K
from keras.preprocessing.image import ImageDataGenerator, array_to_img, img_to_array, load_img
from keras.models import Sequential, Model
from dataGenerator import DataGenerator
from keras.callbacks import EarlyStopping
from keras.layers.normalization import BatchNormalization

preTrain = False

# Global setting
HOME=os.path.join(os.environ['COTRAIN_DATA'], 'psulu')
BASE=os.environ['COTRAIN_HOME']
SCRATCH=os.environ['COTRAIN_SCRATCH']

def spatialModel(img_rows, img_cols, stateVecLen=2):
  currentState = Input(shape=(stateVecLen,))
  imgInput = Input(shape=(img_rows, img_cols, 3))
  if not preTrain:
      conv1 = Conv2D(128, kernel_size=(3, 3), activation='relu',
                        kernel_regularizer=regularizers.l2(10**-5))(imgInput)
      pool1 = MaxPooling2D(pool_size=(2, 2))(conv1)
      conv2 = Conv2D(128, kernel_size=(3, 3), activation='relu',
                        kernel_regularizer=regularizers.l2(10**-5))(pool1)
      pool2 = MaxPooling2D(pool_size=(2, 2))(conv2)
      conv3 = Conv2D(128, kernel_size=(3, 3), activation='relu',
                        kernel_regularizer=regularizers.l2(10**-5))(pool2)
      pool3 = MaxPooling2D(pool_size=(2, 2))(conv3)
      flat = Flatten()(pool3)
  else:
      model.add(Dense(1024, activation='relu', input_shape=(24576,)))
  
  stateVec = Lambda(lambda x: x)(currentState)
  inp = keras.layers.concatenate([flat, stateVec], axis=-1)
  
  # model.add(Dense(1024, activation='relu',
  #                   kernel_regularizer=regularizers.l2(10**-5)))
  dense1 = Dense(128, activation='relu',
                        kernel_regularizer=regularizers.l2(10**-5))(inp)
  dense2 = Dense(64, activation='relu',
                        kernel_regularizer=regularizers.l2(10**-5))(dense1)
  out = Dense(2, activation='tanh')(dense2)
  
  model = Model(inputs = [imgInput, currentState], outputs=out)
  return model

def evaluate(model, modelF, gen):
    model.load_weights(modelF)
    out = []
    predict = []
    predicted = np.zeros((80, 18, 2))
    true = np.zeros((80, 18, 2))
    fname = np.empty((80,), dtype=object)
    outdict = {}
    for i in range(36):
        (x, y, fnames) = gen[i]
        cout = model.predict(x)
        for xx, yy, cf in zip(cout, y, fnames):
            ss = cf.strip('./output//valid//input.lp.png')
            [idx, step] = ss.split('.lp_')
            idx = int(idx) - 80
            step = int(step)
            predicted[idx, step] = xx
            true[idx, step] = yy

            outdict[cf] = xx.tolist()

            # file name
            fname[idx] = cf.split('_')[0]

        out.extend(cout.tolist())
        predict.extend(y.tolist())

    return outdict

def readInp(trainF, testF, validF):
    partition = {}
    iteritems = zip([trainF, testF, validF], ['train', 'test', 'valid'])
    for folder, str_ in iteritems:
      cF = os.path.basename(folder)
      print('Processing %s'%folder)
      with open(folder + '/target.txt', 'rb') as f:
        reader = csv.reader(f)
        partition[str_] = list(reader)
        for j, val in enumerate(partition[str_]):
            val[1:] = [float(cval) for cval in val[1:]]

    # Generators
    partition['train'] = np.array(partition['train'])
    partition['test'] = np.array(partition['test'])
    partition['valid'] = np.array(partition['valid'])
    return partition

def main(trainF, testF, validF, modelF):
    batch_size = 32
    num_classes = 2
    epochs = 100
    patience = 5#int(epochs/20)

    targetIdx = range(3, 5)
    currentStateIdx = range(1, 3) # Includes number of time steps

    # input image dimensions
    img_rows, img_cols = 128, 96

    # Parameters
    params = {'dim': (img_rows, img_cols, 3),
              'batch_size': batch_size,
              'n_channels': 3,
              'preTrain': preTrain,
              'shuffle': True}

    # File name
    partition = readInp(trainF, testF, validF)
    trainF = partition['train'][:, 0]
    testF = partition['test'][:, 0]
    validF = partition['valid'][:, 0]
    trainTarget = partition['train'][:, targetIdx].astype(float)
    testTraget = partition['test'][:, targetIdx].astype(float)
    validTarget = partition['valid'][:, targetIdx].astype(float)

    # Current state position
    trainCurrState = partition['train'][:, currentStateIdx].astype(float)
    testCurrState = partition['test'][:, currentStateIdx].astype(float)
    validCurrState = partition['valid'][:, currentStateIdx].astype(float)

    training_generator = DataGenerator(trainF, trainCurrState, trainTarget, **params)
    valid_generator = DataGenerator(validF, validCurrState, validTarget, **params)

    #
    #testparams = {'dim': (img_rows, img_cols, 3),
    #              'batch_size': 64,
    #              'n_channels': 3,
    #              'shuffle': False}
    #test_generator = DataGenerator(testF, testTraget, **testparams)

    # Input that stores the previous step
    model = spatialModel(img_rows, img_cols)
    model.compile(loss=keras.losses.mse,
                  optimizer=keras.optimizers.Adam(lr=0.001),
                  metrics=['mse', 'mae'])

    #pre_dict = evaluate(model, modelF + '.h5', valid_generator)
    #truedict = dict(zip(validF, validTarget))
    #for i in range(36):
    #    true = np.zeros((19, 2), dtype=float)
    #    pred = np.zeros((19, 2), dtype=float)
    #    for j in range(1, 19):
    #       key = './output//valid//input%d.lp_%d.png'%(i+80, j-1)
    #       true[j, :] = truedict[key]
    #       pred[j, :] = pre_dict[key]
    #
    #    pred = np.cumsum(pred, axis=0)
    #    true = np.cumsum(true, axis=0)
    #    pred = np.array(pred)
    #    true = np.array(true)
    #
    #    import matplotlib.pyplot as plt
    #    plt.scatter(pred[:, 0], pred[:, 1], c='r')
    #    plt.scatter(true[:, 0], true[:, 1], c='g')
    #    plt.show()

    early_stopping = EarlyStopping(monitor='mean_squared_error', patience=patience)
                                  #validation_data=valid_generator,
                                  #callbacks=[early_stopping,],
    history = model.fit_generator(generator=training_generator,
                                  use_multiprocessing=False,
                                  epochs=epochs, verbose=1, workers=1)

    model.save(modelF)
    del model
    print('Done saving model: %s'%modelF)
    return

def firstPassCommandLine():

  # Creating the parser for the input arguments
  parser = argparse.ArgumentParser(description='Generate data for spatial model')

  # Positional argument - Input XML file
  parser.add_argument('--train', type=str, default=None,
                       help='Training Data', dest='trainFolder')
  parser.add_argument('--test', type=str, default=None,
                       help='Testing Data', dest='testFolder')
  parser.add_argument('--valid', type=str, default=None,
                       help='Valid Data', dest='validFolder')
  parser.add_argument('--f', type=str, default='cnn_spat_model.h5',
                       help='Model file', dest='modelF')

  # Parse input
  args = parser.parse_args()
  return args

def completePath(inp):
  if not os.path.isdir(inp):
    inp = os.path.join(SCRATCH, inp)

  return inp

if __name__ == '__main__':
  args = firstPassCommandLine()

  # Complete path based no whether it is a folder or not
  testFolder  = completePath(args.testFolder)
  trainFolder = completePath(args.trainFolder)
  validFolder = completePath(args.validFolder)
  modelF      = args.modelF

  main(trainFolder, testFolder, validFolder, modelF)

