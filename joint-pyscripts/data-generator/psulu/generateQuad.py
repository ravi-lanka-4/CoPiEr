import glob, yaml, tempfile
from PuLPpSulu import main
import argparse, os

def createQuad(inpF, configF):
    env = inpF + '/env/'
    with open(configF, 'r') as F:
        param = yaml.load(F)

    for cFolder in glob.glob(env + '/*'):
        cOut = cFolder.replace('/env/', '/mps/')
        os.system('mkdir -p %s'%cOut)

        for cF in glob.glob(cFolder + '/*yaml'):
            envF = cF
            param['environment'] = "[%s]"%(envF)
            param['waypoints'] = 23
            filename = os.path.basename(envF)
            filename = filename.replace('envi', 'input')
            filename = filename.replace('yaml', 'mps')

            # Write to temp file
            with tempfile.NamedTemporaryFile(delete=False) as tf:
               cparamF = tf.name
               yaml.dump(param, tf, default_flow_style=False)
               
            main(cparamF, create=True)
            os.system('mv MILP1.mps %s/%s'%(cOut, filename))
            print(filename)

    return

def firstPassCommandLine():
    
    # Creating the parser for the input arguments
    parser = argparse.ArgumentParser(description='Path Planning based on MILP')

    # Positional argument - Input XML file
    parser.add_argument('-i', type=str, default='/Users/subrahma/proj/maverick-setup/tmp/psulu_all/psulu_21/',
                        help='Input Folder', dest='inputFolder')
    parser.add_argument('-c', type=str, default='./config/param.yaml',
                        help='Config file', dest='configFile')

    # Parse input
    args = parser.parse_args()
    return args

if __name__ == '__main__':
    args = firstPassCommandLine()
    createQuad(args.inputFolder, args.configFile)

