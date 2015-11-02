#!/usr/bin/python3
#Fork compiler toolchain script
import argparse
import re, os
from sys import exit

def main():
  #Parse arguments
  parser = argparse.ArgumentParser(description='Fork toolchain command line parser...')
  parser.add_argument('-v',action='store_true',help='Use valgrind')
  parser.add_argument('files',metavar='filename',type=str,nargs='+',help='files to process')
  regex_delete = re.compile("(^\s*//.*)|(^\s*$)")
  args = parser.parse_args()
  files = args.files
  #Check that parser exists
  if not os.path.exists("./parser"):
    print("Parser binary not found in current directory.")
    exit(1)
  #Preprocessing
  temp_files = [x + '.wrapper_tmp_file' for x in files]
  for file_in,file_out in zip(files,temp_files):
    try:
      f_in = open(file_in,'r')
      f_out = open(file_out,'w')
    except (FileNotFoundError,PermissionError) as e:
      print(e)
      exit(2)
    for line in f_in:
      if not regex_delete.match(line):
        f_out.write(line)
    f_in.close()
    f_out.close()
  #Call parser on temp_files
  for file in temp_files:
    if args.v:
      print("Please ignore GC_INIT() uninitialized memory.")
      os.system("valgrind --vgdb=no ./parser {}".format(file))
    else:
      os.system("""echo "./parser {0} 3>&1 1>&2 2>&3 | tee {1}.llvm" | bash """.format(file,file[0:-20]))
  #Postprocessing
  for file in temp_files:
    os.remove(file)

if __name__=='__main__':
  print('Running Fork Compiler...')
  main()
