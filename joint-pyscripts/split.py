import argparse
import glob, os

def main(inpF, ext='lp'):
  for j, cF in enumerate(glob.glob(inpF + '/*.%s'%ext)):
    idx = cF.strip('envi.%s'%ext)
    newFolder = inpF.replace('unlabeled', 'unlabeled%d'%(j%9))
    os.system('mkdir -p %s'%newFolder)
    os.system('cp %s %s/'%(cF, newFolder))
    
  return

if __name__ == '__main__':
  main('/Users/subrahma/proj/maverick-setup/tmp/psulu_all/psulu_19/env/unlabeled/', 'yaml')


