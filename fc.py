#!/usr/bin/python3
#Fork compiler toolchain script
import argparse
import re, os

def main():
  #Parse arguments
  parser = argparse.ArgumentParser(description='Fork toolchain command line parser...')
  parser.add_argument('files',metavar='filename',type=str,nargs='+',help='files to process')
  regex_delete = re.compile("(^\s*//.*)|(^\s*$)")
  args = parser.parse_args()
  files = args.files
  #Preprocessing
  temp_files = [x + '.wrapper_tmp_file' for x in files]
  for file_in,file_out in zip(files,temp_files):
    f_in = open(file_in,'r')
    f_out = open(file_out,'w')
    for line in f_in:
      if not regex_delete.match(line):
        f_out.write(line)
    f_in.close()
    f_out.close()
  #Call parser on temp_files
  for file in temp_files:
    os.system("./parser {}".format(file))
  #Postprocessing
  for file in temp_files:
    os.remove(file)

if __name__=='__main__':
  print('Running Fork Compiler...')
  main()
