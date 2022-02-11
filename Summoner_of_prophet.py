from prophetV2 import prophet
import time


list=[]
k=0

list = [item for item in input(
        "Enter the name of the measure you want to predict (i.e. temp moist hum), divided by a space : \n").split()]
print("then i'm going to do a prediction over:"+ ' '.join(map(str, list)))


start=str(input('Enter the starting time (i.e. -1h,-3d, ecc): \n'))
periods=str(input('Enter the length of the prediction period in minutes: \n'))
t=int(input('Enter the interval for the next prediction in seconds \n'))

while (True):
    if("temp" in list):
        k = 1
        prophet("temperature",start, periods)
    if ("moist" in list):
        k = 1
        prophet("moisture", start, periods)
    if ("hum" in list):
        k = 1
        prophet("humidity", start, periods)
    if (k==0) :
        print("mismatch in asked prophecy try again")
        break
    else :
        print("Going to sleep for "+ str(t) + " seconds")
        time.sleep(t)

