from __future__ import division
import argparse
import numpy as np
import csv, pickle, os
#import matplotlib.pyplot as plt

# Keras
import keras
from keras import backend
from keras.callbacks import EarlyStopping
from keras.layers import Activation, Dense, Input, Lambda
from keras.layers.merge import Add
from keras.layers.core import Dropout
from keras.layers.normalization import BatchNormalization
from keras.layers import Activation, RepeatVector
from keras import regularizers
from keras.models import Sequential, Model
from keras.models import model_from_json
from keras import backend as be
from sklearn.model_selection import GroupKFold

# Sampling
from imblearn.over_sampling import RandomOverSampler

# svm dataset
from sklearn.datasets import load_svmlight_file
from sklearn.preprocessing import StandardScaler, normalize

_GROUPS_ = 2.5
_SEG1_ = 18
_SEG2_ = 54

def getRankNet(inp_dim, l1, l2, l3):
  # Defines the model for ranknet
  ranknet = Sequential()

  # Batch input normalization
  #ranknet.add(BatchNormalization(input_shape=(inp_dim, ), \
  #                                mode=0, momentum=0.99, weights=None))

  # Layer 2
  #ranknet.add(Dense(l1, input_shape=(inp_dim, ), kernel_regularizer=regularizers.l1(10**-3)))
  ranknet.add(Dense(l1, input_shape=(inp_dim, ), kernel_regularizer=regularizers.l2(10**-4)))
  #ranknet.add(BatchNormalization(momentum=0.99))
  ranknet.add(Activation('relu'))
  ranknet.add(Dropout(0.3))

  # Layer 2
  ranknet.add(Dense(l2, kernel_regularizer=regularizers.l2(10**-4)))
  #ranknet.add(BatchNormalization(momentum=0.99))
  ranknet.add(Activation('relu'))
  ranknet.add(Dropout(0.3))

  # Layer 2
  ranknet.add(Dense(l3, kernel_regularizer=regularizers.l2(10**-4)))
  ###ranknet.add(BatchNormalization(momentum=0.99))
  ranknet.add(Activation('relu'))
  ranknet.add(Dropout(0.3))

  ## Layer 2
  #ranknet.add(Dense(l4, init='uniform', kernel_regularizer=regularizers.l2(10**-3)))
  ##ranknet.add(BatchNormalization(momentum=0.99))
  #ranknet.add(Activation('relu'))
  ##ranknet.add(Dropout(0.3))

  # Final Layer
  ranknet.add(Dense(1))
  #ranknet.add(Activation('relu'))

  return ranknet

def setsModel(input_dim=2, l1=16, latent=8):
  model = Sequential()
  model.add(Dense(l1, input_dim=input_dim, \
                  kernel_regularizer=regularizers.l2(10**-3), \
                                      activation='relu'))
  model.add(Dense(latent, activation='relu'))
  return model

def chunks(l, n):
  """Yield successive n-sized chunks from l."""
  for i in xrange(0, len(l), n):
     yield l[i:i + n]

def transformSets(inp, inp_size, dim, out_size, model):
  '''
  Transforms the input using deepsets model
  '''
  model_output = []
  num = int(inp_size/dim)
  for i in range(num):
    in_ = Lambda(lambda x: x[:, dim*i:dim*(i+1)])(inp) # Get each obstacle centers
    model_output.append(model(in_))
  merged = keras.layers.Add()(model_output)
  merged = Lambda(lambda x: x/num, input_shape=(out_size, ))(merged)
  return merged

def runObstModel(inp, inp_dim, latent_dim, obstModel):
  # Split input into different pieces
  inp_1 = Lambda(lambda x: x[:, :_SEG1_])(inp) # general features
  inp_2 = Lambda(lambda x: x[:, _SEG1_:_SEG2_])(inp) # Obstacle certers
  inp_3 = Lambda(lambda x: x[:, _SEG2_:])(inp) # bounds on the time steps

  # Transform obstacles
  obstinp = transformSets(inp_2, _SEG2_-_SEG1_, 2, latent_dim, obstModel)
  merged = keras.layers.concatenate([inp_1, obstinp], axis=-1)
  merged = keras.layers.concatenate([merged, inp_3], axis=-1)
  return merged
 
def getRankModel(inp_dim, l1=64, l2=64, l3=64, latent_dim=8):

  # Models used
  ranknet = getRankNet(_SEG1_ + latent_dim + (inp_dim-_SEG2_), l1, l2, l3)
  obstModel = setsModel(latent=latent_dim)
 
  # Relevant document score.
  node1 = Input(shape = (inp_dim, ), dtype = "float32")
  transNode1 = runObstModel(node1, inp_dim, latent_dim, obstModel)
  node1score = ranknet(transNode1)

  node2 = Input(shape = (inp_dim, ), dtype = "float32")
  transNode2 = runObstModel(node2, inp_dim, latent_dim, obstModel)
  node2score = ranknet(transNode2)

  # Subtract scores.
  negscore2= Lambda(lambda x: -1*x, output_shape = (1, ))(node2score)
  diff = Add()([node1score, negscore2])

  # Pass difference through sigmoid function.
  #prob = Lambda(lambda x: (x+1)/2, output_shape = (1, ))(diff)
  prob = Activation("sigmoid")(diff)

  # Build model.
  model = Model(inputs = [node1, node2], outputs=prob)
  model.compile(optimizer = "adadelta", loss = "binary_crossentropy", \
                 metrics=['accuracy'])
  outnet = Model(ranknet.layers[0].input, ranknet.layers[-2].output)

  return model, ranknet, outnet

def load_svmlight(fname):
  print('Loading %s'%fname)
  data, labels = load_svmlight_file(fname)
  numFeats = int(labels[0])
  newdata = np.zeros((data.shape[0]-1, numFeats))
  newdata[:, :data.shape[1]] = data[1:, :].toarray()
  labels = labels[1:]
  return (newdata, labels)

def load_weight(fname):
  print('Loading %s'%fname)
  weight = []
  with open(fname, 'rb') as csvfile:
     spamreader = csv.reader(csvfile, delimiter='\n')
     for row in spamreader:
         weight.append(float(row[0]))
  return np.array(weight)

def combine_modules(inpdim, numPolicies, modelF):
  print('Input Dimension: %d'%inpdim)
  inp = Input((inpdim, ))

  out = []
  for k in range(numPolicies):
     _, getScore, _ = getRankModel(inpdim)
     cfname = modelF.replace(".h5", "_%d.h5"%k)

     # Normalization layers
     getScore.add(Dense(1, input_shape=(1,)))
     getScore.add(Dense(1, input_shape=(1,)))

     print('Loading Model %s'%cfname)
     getScore.load_weights(cfname)
     cout = getScore(inp)
     out.append(cout)
     
  if numPolicies > 1:
    ensembleOut = keras.layers.average(out)
  else:
    ensembleOut = cout
  ensemble = Model(inputs=inp, outputs=ensembleOut)
  
  finalfname = modelF.replace(".h5", ".h5")
  jsonfname = finalfname.replace('.h5', '.json')
  ensemble.save(finalfname)

  # Well using python3; but the limitation of the library. 
  dir_ = os.path.dirname(os.path.abspath(__file__))
  os.system('python3 %s/keras_export/convert_model.py %s %s'%(dir_, finalfname, jsonfname))
  return

def main():
  args = firstPassCommandLine()
  trainF    = args.searchInpTrain
  validF    = args.searchInpValid
  weightF   = args.searchWeight
  validWeightF = args.searchValidWeight
  testF     = args.searchInpTest
  modelF    = args.searchF
  nepoch    = args.nepoch
  batchSize = args.batchSize

  # Read weight for each data points
  weights = load_weight(weightF)

  # Load data in the svm rank file format
  trainfeats1, trainlabels1 = load_svmlight(trainF + '.1')
  trainfeats2, trainlabels1 = load_svmlight(trainF + '.2')

  #valid_weights = load_weight(validWeightF)
  #validfeats1, validlabels2 = load_svmlight(validF + '.1')
  #validfeats2, validlabels2 = load_svmlight(validF + '.2')

  # Permulation
  idx = np.random.permutation(trainfeats1.shape[0])
  trainfeats1 = trainfeats1[idx, :]
  trainfeats2 = trainfeats2[idx, :]
  weights = weights[idx]

  assert len(weights) == len(trainlabels1), "Length not equal"
  testfeats_exists = False

  if testF != None:
      testfeats1, testlabels1   = load_svmlight(testF + '.1')
      testfeats2, testlabels2   = load_svmlight(testF + '.2')
      testfeats_exists = True
  else:
      testfeats1 = None
      testfeats2 = None
      testlabels = None

  ## convert to dense array
  #trainfeats1 = np.array(trainfeats1.todense())
  #validfeats1 = np.array(validfeats1.todense())
  #trainfeats2 = np.array(trainfeats2.todense())
  #validfeats2 = np.array(validfeats2.todense())
  #
  #if testfeats_exists:
  #    testfeats1 = np.array(testfeats1.todense())
  #    testfeats2 = np.array(testfeats2.todense())

  # Create model
  INPUT_DIM = trainfeats1.shape[1]
  rankModel, getScore, outNet = getRankModel(INPUT_DIM)
  target = np.ones((trainfeats1.shape[0]))
  rankModel.summary()
  numParams = rankModel.count_params()
  numDatapts = trainfeats1.shape[0]
  numPolicies = int(np.round(numDatapts/(numParams*_GROUPS_)))
  numPolicies = -1
  print("###############################")
  print("###############################")
  print("###############################")
  print("###############################")

  if numPolicies <= 0:
    numPolicies = 1
    group = [range(len(trainfeats1))]
  else:
    group = list(chunks(range(0, len(trainfeats1)), int(numParams*_GROUPS_)))

  ## Train model.
  allModels = []
  for k, cidx in enumerate(group):
    if k != 0:
      # Create new models
      rankModel, getScore, outNet = getRankModel(INPUT_DIM)

    print('Data size: %d\n'%len(cidx))
    #early_stopping = EarlyStopping(monitor='val_acc', patience=25)

                            #validation_data=([validfeats1, validfeats2], \
                            #                  np.ones((validfeats1.shape[0])), valid_weights), \

                            #callbacks=[early_stopping])

    history = rankModel.fit([trainfeats1[cidx, :], trainfeats2[cidx, :]], target[cidx], \
                            sample_weight=weights[cidx],
                            validation_split=0.2,
                            batch_size=batchSize, \
                            epochs=nepoch, verbose=1, \
                            shuffle=True)

    # Add an additional module to normalize the score
    relS = getScore.predict(trainfeats1[cidx, :], batch_size=batchSize, verbose=1)
    irrS = getScore.predict(trainfeats2[cidx, :], batch_size=batchSize, verbose=1)
    allScore = np.hstack((relS.flatten(), irrS.flatten()))
    maxScore = np.max(allScore)
    minScore = np.min(allScore)
    scaleFactor = 1/(maxScore-minScore)
    print('Max: %f Min: %f'%(maxScore, minScore))
    getScore.add(Dense(1, input_shape=(1,), weights=[np.ones([1, 1]), -minScore*np.ones((1))]))
    getScore.add(Dense(1, input_shape=(1,), weights=[scaleFactor*np.ones([1, 1]), np.zeros((1))]))

    # Save mode
    ## Generate scores from document/query features.
    #relS = getScore.predict(validfeats1, batch_size=batchSize, verbose=1)
    #irrS = getScore.predict(validfeats2, batch_size=batchSize, verbose=1)
    #getRank(relS, irrS, 'valid')

    if testfeats_exists:
        relS = getScore.predict(testfeats1, batch_size=batchSize, verbose=1)
        irrS = getScore.predict(testfeats2, batch_size=batchSize, verbose=1)
        np.savetxt(modelF[:-3] + "rank1.txt", relS)
        np.savetxt(modelF[:-3] + "rank2.txt", irrS)
        print("Max score difference: " + str(np.max(relS-irrS)))
        print("Min score difference: " + str(np.min(relS-irrS)))
        print("Mean score difference: " + str(np.mean(relS-irrS)))
        print("Score difference std dev: " + str(np.std(relS-irrS)))
        getRank(relS, irrS, 'test')

    # Save mode
    getScore.save(modelF.replace(".h5", '_%d.h5'%k))
    del getScore
    del rankModel

    #_, getScore, _= getRankModel(INPUT_DIM)

  combine_modules(INPUT_DIM, numPolicies, modelF)
  be.clear_session()
  return


def getRank(relS, irrS, str_=None):
  # Rank given score
  avgRank = np.sum(relS > irrS)/relS.shape[0]
  if str_ != None:
    print('\n' + str_ + ' rank: %f'%avgRank)

  return avgRank

def firstPassCommandLine():

  # Creating the parser for the input arguments
  parser = argparse.ArgumentParser(description='Pruning network')

  # Positional argument - Input XML file
  parser.add_argument('-searchValidPrefix', '--v', type=str, \
                       default='./sample-data/input104.search.trj',
                       help='search Valid Input File prefix', dest='searchInpValid')
  parser.add_argument('-searchTrainPrefix', '--t', type=str, \
                       default='./sample-data/input104.search.trj',
                       help='search Training Input prefix', dest='searchInpTrain')
  parser.add_argument('-searchTrainWeight', '--w', type=str, \
                       default='./sample-data/input104.search.trj.weight',
                       help='search Training Input prefix', dest='searchWeight')
  parser.add_argument('-searchValidWeight', '--x', type=str, \
                       default='./sample-data/input104.search.trj.weight',
                       help='Search Validation Input Sample Weight', dest='searchValidWeight')
  parser.add_argument('-searchTestPrefix', '--u', type=str, default=None,
                       help='search Test Input prefix', dest='searchInpTest')
  parser.add_argument('-selectmodel', '--m', type=str, \
                       default='./models/model_keras_search_1.h5', \
                       help='Select model file name', dest='searchF')
  parser.add_argument('-nepoch', '--n', type=int, default=1000, \
                       help='Number of epochs', dest='nepoch')
  parser.add_argument('-batchsize', '--b', type=int, default=128, \
                       help='Batch Size', dest='batchSize')

  # Parse input
  args = parser.parse_args()
  return args

if __name__ == '__main__':
  main()
