import os
import multiprocessing
import time

def open_window():
    os.system('C:\\Users\\huawei\\Anaconda3\\envs\\AI\\python.exe ./main.py')

if __name__ == "__main__":
    p1 = multiprocessing.Process(target = open_window, args = ())
    p2 = multiprocessing.Process(target = open_window, args = ())
    p3 = multiprocessing.Process(target = open_window, args = ())
    p1.start()
    p2.start()
    p3.start()
    while True:
        pass