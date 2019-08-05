import numpy as np
from scipy import misc
from keras.applications import VGG16
from keras.utils import to_categorical
import keras
import os, tqdm
import PIL
from PIL import Image

class DataGenerator(keras.utils.Sequence):
    'Generates data for Keras'
    def __init__(self, list_IDs, currState, labels, batch_size=32, dim=(32,32,32), n_channels=1,
                 n_classes=10, shuffle=True, preTrain=False):
        'Initialization'
        self.dim = dim
        self.batch_size = batch_size
        self.stateVec = currState
        self.labels = labels
        self.list_IDs = list_IDs
        self.n_channels = n_channels
        self.n_classes = n_classes
        self.shuffle = shuffle
        self.on_epoch_end()
        self.preTrain = preTrain
        self._dim = 24576 # Pretrain dimensions
        self._splitdim = (6, 8, 512) # Pretrain split dimensions
        self.trueLabels = None
        self.model = None

        if self.preTrain:
          self.model = VGG16(weights='imagenet', include_top=False, input_shape=self.dim)
          self.feats = None
          self.Z = None
          self._getPretrained()

        return

    def _getPretrained(self):
        self.feats = np.empty((len(self.list_IDs),) + self.dim, dtype=float)
        statedim = self.stateVec.shape
        statedim[0] = len(self.list_IDs)
        self.Z = np.empty(statedim)

        for i, ID in tqdm.tqdm(enumerate(self.list_IDs)):
            img = misc.imread(ID, mode='RGB')
            img = misc.imresize(img, self.dim)
            img = img.astype(float)
            img = img/255
            self.feats[i,] = img 
            self.Z[i] = self.stateVec[i]

        self.feats = self.model.predict(self.feats, verbose=1)
        print('Extracted pretrained feats')
        return

    def __len__(self):
        'Denotes the number of batches per epoch'
        return int(np.floor(len(self.list_IDs) / self.batch_size))

    def __getitem__(self, index):
        'Generate one batch of data'
        # Generate indexes of the batch
        indexes = self.indexes[index*self.batch_size:(index+1)*self.batch_size]

        # Find list of IDs
        list_IDs_temp = [self.list_IDs[k] for k in indexes]

        # Generate data
        X, y  = self.__data_generation(list_IDs_temp, indexes)
        return X, y

    def on_epoch_end(self):
        'Updates indexes after each epoch'
        self.indexes = np.arange(len(self.list_IDs))
        if self.shuffle == True:
            np.random.shuffle(self.indexes)
        self.trueLabels = self.labels[self.indexes]
        return

    def __data_generation(self, list_IDs_temp, indexes):
        'Generates data containing batch_size samples' # X : (n_samples, *dim, n_channels)
        # Initialization
        # 24576 -> prod((6, 8, 512)) output of the pretrained model

        y = np.empty((self.batch_size, len(self.labels[0])), dtype=float)
        if not self.preTrain:
          X = np.empty((self.batch_size,) + self.dim, dtype=float)
          statedim = list(self.stateVec.shape)
          statedim[0] = self.batch_size
          Z = np.empty(tuple(statedim))

        # Generate data
        for i, ID in enumerate(list_IDs_temp):
            # Store sample
            if not self.preTrain:
              img = Image.open(ID)
              img = img.convert('RGB')
              img = img.resize(self.dim[:-1][::-1], PIL.Image.ANTIALIAS)
              img = np.array(img).astype(float)
              img = img/255
              X[i,] = img

            # Store class
        y = self.labels[indexes, :]
        Z = self.stateVec[indexes, :]

        if self.preTrain:
           X = self.feats[indexes, :, :, :]
           X = X.reshape((self.batch_size, self._dim))
           Z = self.stateVec[indexes]

        M = {}
        M['input_2'] = X
        M['input_1'] = Z
        return M, y

