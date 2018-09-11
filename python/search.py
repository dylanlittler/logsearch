#!/usr/bin/env python3

######## IMPORTS ########
import argparse
import subprocess
import os
import fileinput
########################


### -------- Argument Parsing ------- ###
parser = argparse.ArgumentParser()
parser.add_argument("--logic", help="-o to search exclusively", type=str)
parser.add_argument("--logdir", help="/path/to/logdir", nargs='?',
                    default="/var/log/", type=str)
parser.add_argument("terms", help="search terms", nargs='+', type=str)
args = parser.parse_args()
### -------------------------------- ###



def ls_cmd(log_dir):
    ls = subprocess.Popen(["ls", log_dir], stdout=subprocess.PIPE)
    listing = ls.communicate()
    return listing[0].decode('ascii').splitlines()


def append_path(log_dir, files):
    file_list = []
    if log_dir[-1] != '/':
        log_dir = log_dir + '/'
    for f in files:
        file_list.append(log_dir + f)
    return file_list


def check_logic(cur_file):
    match = False
    with fileinput.FileInput(cur_file) as f:
        for line in f:
            for term in args.terms:
                if term in line:
                    match = True
                    print("'" + term + "'", "found in",
                          f.filename(), "\n", line, f.lineno())
    return match



def check_match_inclusive(log, search_terms):
    match = True
    for term in search_terms:
        if term not in log:
            match = False
            break
    return match

                
def list_files(files):
    global matching_files
    global matches
    for f in files:
        if os.path.isdir(f):
            sub_files = append_path(f, ls_cmd(f))
            list_files(sub_files)
        try:
            with open(f, 'r') as cur_file:
                if args.logic == 'o':
                    if check_logic(f):
                        matches += 1
                        matching_files.append(f)
                else:
                    if check_match_inclusive(cur_file.read(), args.terms):
                        matching_files.append(f)
                        check_logic(f)
                        matches += 1
        except (IsADirectoryError, PermissionError, UnicodeDecodeError):
            continue
    return matching_files

    
log_files = ls_cmd(args.logdir)
logs = append_path(args.logdir, log_files)

matches = 0
matching_files = []
matched = list_files(logs)
print("\n{} matches found.".format(matches))
print("Matching files: ")
for f in matched:
    print(f)
