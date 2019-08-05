import argparse, glob, os
from spatialModel import spatialModel
from createSpatialInstance import main as createSpatialInst
from keras import backend as be
import numpy as np

# input image dimensions
img_rows, img_cols = 128, 96
modelF = 'cnn_spat_model'

# Global setting
HOME=os.path.join(os.environ['COTRAIN_DATA'], 'psulu')
BASE=os.environ['COTRAIN_HOME']
SCRATCH=os.environ['COTRAIN_SCRATCH']

def main(inpFolder, solFolder, paramF, enviFolder, outPrefix, model, augNum):

  if model:
    allF = glob.glob(inpFolder + '/*')
  else:
    allF = glob.glob(solFolder + '/*')

  for csubF in allF: 
    subF = csubF.replace('sol/', 'lpfiles/')
    if os.path.isfile(csubF):
      cFolder = os.path.basename(os.path.dirname(subF))
      allF = [subF.replace('.sol', '.lp')]
    else:
      cFolder = os.path.basename(subF)
      allF = glob.glob(csubF + '/*')
      allF = [cF.replace('sol/', 'lpfiles/') for cF in allF]
      print('Processing %s'%cFolder)

    for cFile in allF:
      # Prepare input parameters
      print('Converting %s'%cFile)
      fname = os.path.basename(cFile)
      solF = os.path.join(solFolder, fname.replace('.lp','.sol'))
      enviF = os.path.join(enviFolder, 'envi' + fname.strip('input.lp') + '.yaml')

      assert os.path.isfile(solF) or model, "Sol file %s missing"%solF
      assert os.path.isfile(enviF), "Envi file %s missing"%enviF
      assert os.path.isfile(paramF), "Param file %s missing"%paramF

      # Execute 
      if not model:
        outP = outPrefix + '/'+ fname 
      else:
        outP = outPrefix + '/pred_sols/' + fname 

      os.system('mkdir -p %s'%(os.path.dirname(outP)))
      createSpatialInst(cFile, solF, paramF, enviF, outP, model)

      # Data augment
      testorvalid = ('test' in cFolder) or ('valid' in cFolder) or ('unlabeled' in cFolder)
      if not testorvalid: 
        if not model:
          for j in range(augNum):
            numObst = np.random.randint(3, 6)
            createSpatialInst(cFile, solF, paramF, enviF, outP, \
                                model=model, augment=True, numObst=numObst, suffix='%d'%j)

  be.clear_session()
  return

def firstPassCommandLine():

  # Creating the parser for the input arguments
  parser = argparse.ArgumentParser(description='Generate data for spatial model')

  # Positional argument - Input XML file
  parser.add_argument('--i', type=str, default='lpfiles/',
                       help='Input LP File', dest='inpFolder')
  parser.add_argument('--s', type=str, default='sol',
                       help='Solution File', dest='solFolder')
  parser.add_argument('--p', type=str, default='joint-pyscripts/data-generator/psulu/config/param.yaml',
                       help='Param File', dest='paramF')
  parser.add_argument('--e', type=str, default='/Users/subrahma/proj/psulu/ts/env/psulu/ts_15/',
                       help='Environment File', dest='enviFolder')
  parser.add_argument('--o', type=str, default='./12obst_15ts/',
                       help='Output Folder', dest='outPrefix')
  parser.add_argument('--m', type=bool, default=False,
                       help='Use Model', dest='model')
  parser.add_argument('--f', type=str, default='cnn_spat_model',
                       help='Model file', dest='modelF')
  parser.add_argument('--n', type=int, default=1,
                       help='Augmentation number', dest='augNum')

  # Parse input
  args = parser.parse_args()
  return args

if __name__ == '__main__':
  args = firstPassCommandLine()
  model = None

  # append only if its not a full path
  paramF = os.path.join(BASE, args.paramF)
  if not os.path.isdir(args.inpFolder):
      inpFolder  = os.path.join(HOME, args.inpFolder)
      solFolder  = os.path.join(HOME, args.solFolder)
      enviFolder = os.path.join(HOME, args.enviFolder)
      outPrefix  = os.path.join(SCRATCH, args.outPrefix)
      modelF     = os.path.join(SCRATCH, args.modelF)
  else:
      inpFolder  = args.inpFolder
      solFolder  = args.solFolder
      enviFolder = args.enviFolder
      outPrefix  = args.outPrefix
      modelF     = args.modelF

  if args.model:
      model = spatialModel(img_rows, img_cols)
      if '.h5' in modelF:
        model.load_weights(modelF)
      else:
        model.load_weights(modelF + '.h5')

  main(inpFolder, solFolder, paramF, enviFolder, outPrefix, model, args.augNum)

