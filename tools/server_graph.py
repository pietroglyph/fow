#!/usr/bin/python3

import requests, matplotlib, string, sys, time
import matplotlib.pyplot as plt

if len(sys.argv) < 2:
    print("Please specify the URL your want graph the data from as an argument to this command.")
    exit()

response = requests.get(sys.argv[1])
response.raise_for_status()

numpy.array(500)
plt.ion()

while True:
    cur_time = time.time()
    response = requests.get(sys.argv[1])
    ferries = response.text.split(":")
    
    plt.plot(cur_time, float(ferries.pop()))
    
    for ferry in ferries:
        print(ferry)
        vals = ferry.split(",")

    

    plt.pause(3)
