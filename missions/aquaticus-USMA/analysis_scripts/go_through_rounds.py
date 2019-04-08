#here we assume script is being run at the participant level
#thus, the script needs to go into the corresponding
#SHORESIDE and PARTICIPANT folders and grab their .alog files
#assumes we have class_trial.py in same directory
import sys
import subprocess
import os
from collections import OrderedDict
from class_trial import TrialStats
from return_ordered_dictionary import ordered_dictionary_of_trials 

print "Hello, Python!"
shoreside_dir_name=""
cwd = os.getcwd()
print("Current working directory: " + cwd)
list_of_dirs = os.listdir(cwd)
print(list_of_dirs)
for curr_dir in list_of_dirs:
    if(os.path.isdir(curr_dir)):
        if('r' in curr_dir):
            print(curr_dir)
            os.chdir(curr_dir)
            catching_od = ordered_dictionary_of_trials()

            for key, value in catching_od.items():
                print("Trial: " + key)
                print("Blue flag scores: " + str(value.BLUE_FLAG_SCORES))
                print("Blue flag grabs: " + str(value.BLUE_FLAG_GRABS))
                print("Red flag scores: " + str(value.RED_FLAG_SCORES))
                print("Red flag grabs: " + str(value.RED_FLAG_GRABS))
                print("Reliable: " + value.RELIABLE)
                print("Times tagged: " + str(value.TIMES_TAGGED))
                print("Speech Commanded: " + str(value.SPEECH_COMMANDED))
                print("DIALOG_ERROR: " + str(value.DIALOG_ERROR))
                print("COMMAND_CANCELED: " + str(value.COMMAND_CANCELED))

    os.chdir(cwd)

