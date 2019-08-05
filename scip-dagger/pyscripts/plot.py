import csv, re
import glob, os
import pdb
import numpy as np
import matplotlib.pyplot as plt

inputFolder = '/Users/subrahma/proj/pdf/scip-dagger/final/'

def readFile(folder):
    '''
    Read file and get statistics
    '''
    ogaps = []
    unsolvedprob = []
    print('Processing: %s'%folder)
    for fname in glob.glob(folder + '/*.txt'):
        with open(fname, 'r') as F: 
            lines = F.readlines()
            ogap = []
            unsolved = []
            name = []
            switch = False
            for line in lines[1:]:
                if switch:
                    reline = re.sub( '\s+', ' ', line).strip()
                    reline = reline.split(' ')
                    ogap.append(float(reline[-3]))
                    unsolved.append(int(reline[-1]))
                else:
                    name.append(line)

                switch = switch != True

            ogaps.append(ogap)
            unsolvedprob.append(unsolved)

    meanOgap = np.mean(ogaps, axis=0)*100
    stdOgap = np.std(ogaps, axis=0)
    meanUnsolved = np.mean(unsolvedprob, axis=0)
    return (meanOgap, stdOgap, meanUnsolved, np.array(name))

def saveplot(legend, x, y, flag=[], stdY=None, outF=None):
    '''
    Save plot 
    '''
    if flag != []:
        flag = np.where(flag)[0]
        legend = legend[flag]
        y = y[:, flag]
        if stdY != None:
            stdY = stdY[:, flag]
    
    color = ['g', 'r', 'b']
    fig, ax = plt.subplots(1)
    if stdY != None:
       for yy, stdyy, nn, cc  in zip(y.T, stdY.T, legend, color):
          nn = nn.strip('\n')
          plt.errorbar(x, yy, stdyy, mfc=cc, marker='.', label=nn)
    else:
       for yy, nn, cc  in zip(y.T, legend, color):
          nn = nn.strip('\n')
          plt.plot(x, yy, color=cc, marker='.', label=nn)

    ax.set_xticks(x)
    ax.set_title('pSulu Optimality Gap')
    ax.set_xlabel('Way points')
    ax.set_ylabel('Optimality Gap')
    ax.legend()
    #ax.set_labels(legend)

    if outF != None:
        plt.savefig(outF)
    else:
        plt.show()

    return

if __name__ == '__main__':
    allOgap = []
    stdOgap = []
    allUnsolved = []
    scales = []
    for folder in glob.glob(inputFolder + '/*/'):
        scale = folder.strip(os.path.dirname(inputFolder))
        try: 
            scales.append(int(scale)) 
            (ogap, std, unsolved, name) = readFile(folder)
            allOgap.append(ogap)
            stdOgap.append(std)
            allUnsolved.append(unsolved)
        except:
            pass

    allOgap = np.array(allOgap)
    allUnsolved = np.array(allUnsolved)
    stdOgap = np.array(stdOgap)
    scales = np.array(scales)

    allSolved = 50-allUnsolved
    cumSolved = np.cumsum(allSolved, axis=0)

    # remove time limit
    flag = [1, 0, 1, 1] 

    # plot the results
    saveplot(name, scales, allOgap, flag, None, inputFolder + '/ogap.png')
    #saveplot(name, scales, cumSolved, flag, None, inputFolder + '/ogap.png')


