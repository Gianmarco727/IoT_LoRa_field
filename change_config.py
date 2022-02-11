import os
import signal

f = open(r"config.txt", "w")

while(1):
    ID=input('Enter the ID: \n')

    print ("setting parameters for device n. %s" %ID)
    if(int(ID)<3):
        break
    else:
        print("wrong ID selected choose between 1 and 2")


lat="{:.6f}".format(float(input('Enter the lat: \n')))

log="{:.6f}".format(float(input('Enter the long: \n')))
freq=int(input('Enter freq: \n'))

print(str(ID)+'#'+ str(lat) +'/' + str(log) + '&'+ str(freq))

f.write(str(ID)+'#'+ str(lat) +'/' + str(log) + '&'+ str(freq))
	
f.close()

print('done setting, press enter');

