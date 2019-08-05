import os, argparse, re

# Gurobi_cl usage: gurobi_cl ResultFile=coins.sol coins.lp
run_cmd = 'gurobi_cl ResultFile={0} {1}'

# args: Input Folder, Output Folder, file Extension
def generate_solution(inpFolder, outFolder, fext):
    # Generate Solution with the same heirarchy as that of the input folder
    inpFolder = os.path.abspath(inpFolder)
    outFolder = os.path.abspath(outFolder)
    common_prefix = os.path.commonprefix([inpFolder, outFolder])
    fprefix = outFolder + '/' + re.sub(common_prefix, '', inpFolder)
    
    # Iterate through every file and generate the result
    for path, folders, files in os.walk(inpFolder):
        for f in files:
            fname, cf_ext = os.path.splitext(f)
            if fext == cf_ext:
                # Check if the corresponding file exists in the solution folder
                cprefix = fprefix + re.sub(inpFolder, '', path)
                os.system('mkdir -p %s'%cprefix)

                # Run the output generator for the files with extension
                cfile = path + '/' + f
                outF =  cprefix + '/' + fname + '.sol'
                os.system(run_cmd.format(outF, cfile))

                # Convert exponent objective value to float representation
                with open(outF, 'r') as fp:
                    lines = fp.readlines()
                    lines[1] = lines[1].strip('# Objective value = \n')
                    lines[1] = '#Objective value = %f\n'%float(lines[1])

                with open(outF, 'w') as fp:
                    for line in lines[1:]:
                       fp.write(line)

def firstPassCommandLine():
    
    # Creating the parser for the input arguments
    parser = argparse.ArgumentParser(description='Path Planning based on MILP')

    # Positional argument - Input XML file
    parser.add_argument('-i', type=str, default='../data/psulu',
                        help='Input Folder to process the files', dest='inpFolder')
    parser.add_argument('-o', type=str, default='../solution/',
                        help='Output Folder', dest='outFolder')
    parser.add_argument('-e', type=str, default='.lp',
                        help='File Extension', dest='ext')

    # Parse input
    args = parser.parse_args()
    return args

if __name__ == '__main__':
    args = firstPassCommandLine()
    generate_solution(args.inpFolder, args.outFolder, args.ext)
