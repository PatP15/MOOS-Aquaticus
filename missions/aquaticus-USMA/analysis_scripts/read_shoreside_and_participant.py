import subprocess
from collections import OrderedDict

class TrialStats:
    SCENARIO=""
    RELIABLE="TRUE"
    BLUE_FLAG_GRABS=0
    BLUE_FLAG_SCORES=0
    RED_FLAG_GRABS=0
    RED_FLAG_SCORES=0
    TIMES_TAGGED=0
    SPEECH_COMMANDED=0
    TIME_FROM_AQUATICUS_GAME_START=0.0

#this function is independent of the class TrialStats in the sense that it works on an OrderedDict
#with variable types of TrialStats
#create a function that given the time of an event, will return the key to the appropriate scenario
#this function will be useful for going through the participant's logs and aligning events with scenarios
def find_proper_scenario_given_event_time(od,event_time):
    #scan through the events finding the appropriate key to return
    for key,value in od.items():
        print(key)
        #we check if at the last element and return that scenario's key

        #else we return the current scenario key if time is <= current key and < next key



trialDictionary = OrderedDict() 

#creating a class for keeping trial information
print "Hello, Python!"
subprocess.call('which aloggrep',shell=True)
subprocess.call("aloggrep -q C001_LOG_SHORESIDE_20_3_2019_____12_57_12.alog AQUATICUS_GAME* GROUP SCENARIO ROUND SELF_AUTHORIZE* RELIABLE SAY_MOOS| egrep 'AQUATICUS_GAME|GROUP|SCENARIO|ROUND|SELF_AUTHORIZE|RELIABLE|grab|goal' > shoreside_compact.txt",shell=True)

#populate important fields
GROUP=""
ROUND=""
SCENARIO=""
SELF_AUTHORIZE=""
RELIABLE=""
TOTAL_FLAG_GRABS_BLUE=0
TOTAL_FLAG_SCORES_BLUE=0
TOTAL_FLAG_GRABS_RED=0
TOTAL_FLAG_SCORES_RED=0
WIN_OR_LOSS=""
SHORESIDE_GAME_START=0.0
file = open("shoreside_compact.txt","r")
for line in file:
    fields = line.split()
    #0 is time, 1 is variable, 2 is source app, 3 is value
    print(fields[0] + " " + fields[1] + " " + fields[3])
    if(fields[1] == "GROUP"):
        GROUP=fields[3]
    elif(fields[1] == "ROUND"):
        ROUND=fields[3]
    elif(fields[1] == "SAY_MOOS"):
        if "red" in fields[3]:
            if "grab" in fields[3]:
                TOTAL_FLAG_GRABS_RED = TOTAL_FLAG_GRABS_RED + 1
                latestKey = next(reversed(trialDictionary))
                trialDictionary[latestKey].RED_FLAG_GRABS += 1
            elif "goal" in fields[3]:
                TOTAL_FLAG_SCORES_RED = TOTAL_FLAG_SCORES_RED + 1
                latestKey = next(reversed(trialDictionary))
                trialDictionary[latestKey].RED_FLAG_SCORES += 1
        elif "blue" in fields[3]:
            if "grab" in fields[3]:
                TOTAL_FLAG_GRABS_BLUE = TOTAL_FLAG_GRABS_BLUE + 1
                latestKey = next(reversed(trialDictionary))
                trialDictionary[latestKey].BLUE_FLAG_GRABS += 1
            elif "goal" in fields[3]:
                TOTAL_FLAG_SCORES_BLUE = TOTAL_FLAG_SCORES_BLUE + 1
                latestKey = next(reversed(trialDictionary))
                trialDictionary[latestKey].BLUE_FLAG_SCORES += 1
    elif(fields[1] == "SELF_AUTHORIZE_BLUE_TWO"):
        SELF_AUTHORIZE = fields[3]
    elif(fields[1] == "SCENARIO"):
        trialDictionary[fields[3]] = TrialStats()
        latestKey = next(reversed(trialDictionary))
        trialDictionary[latestKey].TIME_FROM_AQUATICUS_GAME_START = fields[0] - SHORESIDE_GAME_START
    elif(fields[1]=="AQUATICUS_GAME"):
        if(fields[3]=="play"):
            SHORESIDE_GAME_START=fields[0]



subprocess.call("aloggrep C001_P917_LOG_DONA_BLUE_ONE_20_3_2019_____12_57_11.alog AQUATICUS_GAME* uDialogManager uSpeechRec -q -nc -nr -gl -ac > participant_compact.txt")

file = open("participant_compact.txt","r")
for line in file:
    fields = line.split()
    #0 is time, 1 is variable, 2 is source app, 3 is value
    print(fields[0] + " " + fields[1] + " " + fields[3])
    if(fields[1] == "GROUP"):
        GROUP=fields[3]
    elif(fields[1] == "ROUND"):
        ROUND=fields[3]

#test for win or loss with respect to BLUE TEAM
if TOTAL_FLAG_SCORES_BLUE > TOTAL_FLAG_SCORES_RED:
    WIN_OR_LOSS = "WIN"
elif TOTAL_FLAG_SCORES_BLUE < TOTAL_FLAG_SCORES_RED:
    WIN_OR_LOSS = "LOSE"
elif TOTAL_FLAG_SCORES_BLUE ==  TOTAL_FLAG_SCORES_RED:
    if TOTAL_FLAG_GRABS_BLUE > TOTAL_FLAG_GRABS_RED:
        WIN_OR_LOSS = "WIN"
    elif TOTAL_FLAG_GRABS_BLUE < TOTAL_FLAG_GRABS_RED:
        WIN_OR_LOSS = "LOSE"
    elif TOTAL_FLAG_GRABS_BLUE == TOTAL_FLAG_GRABS_RED:
        WIN_OR_LOSS = "TIE"

print("GROUP: " + GROUP +" ROUND: " + ROUND)
print("Blue Robot Higher Level Autonomy: " + SELF_AUTHORIZE)
print("Blue Team Result: " + WIN_OR_LOSS)
print("Blue Team: Grabs " + str(TOTAL_FLAG_GRABS_BLUE) + " Scores: " + str(TOTAL_FLAG_SCORES_BLUE))
print("Red Team: Grabs " + str(TOTAL_FLAG_GRABS_RED) + " Scores: " + str(TOTAL_FLAG_SCORES_RED))

for key, value in trialDictionary.items():
    print(key)
    print("Blue flag scores: " + str(value.BLUE_FLAG_SCORES))
    print("Blue flag grabs: " + str(value.BLUE_FLAG_GRABS))
    print("Red flag scores: " + str(value.RED_FLAG_SCORES))
    print("Red flag grabs: " + str(value.RED_FLAG_GRABS))
    print("Reliable: " + value.RELIABLE)
    print("Speech Commanded: " + str(value.SPEECH_COMMANDED))
    print("Times tagged: " + str(value.TIMES_TAGGED))

