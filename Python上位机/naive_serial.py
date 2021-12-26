from numpy.lib.utils import safe_eval
import serial #导入模块
import threading
from threading import Timer
import serial.tools.list_ports
from PIL import Image
import numpy as np
import time
from Constant import CommandEnum
from Constant import EventEnum
from Constant import frame_kind
from Constant import Head
from Constant import Status
import sys
from queue import Empty, Queue

class Naive_serial():
    def __init__(self):
        self.portx = "COM7" #端口，GNU / Linux上的/ dev / ttyUSB0 等 或 Windows上的 COM3 等
        self.bps = 115200 #波特率，标准值之一：50,75,110,134,150,200,300,600,1200,1800,2400,4800,9600,19200,38400,57600,115200
        self.timex = 5 #超时设置,None：永远等待操作，0为立即返回请求结果，其他值为等待超时时间(单位为秒）
        self.nbuffered = 0 # 目前buffer中还需要保留的个数，即窗口中未被确认的个数
        self.queue_from_network_layer = Queue(maxsize = 3)
        self.queue_to_network_layer = Queue(maxsize = 3)
        self.queue_from_uart = Queue(maxsize = 3)
        self.queue_to_uart = Queue(maxsize = 3)
        try:
            self.ser=serial.Serial(self.portx, self.bps, timeout=self.timex) # 打开串口，并得到串口对象
        except Exception as e:
            print("---打开串口失败---:",e)
            self.ser.close()#关闭串口
            sys.exit()
    
    def run(self):
        thread = threading.Thread(target=self.__UartTx, args=()) #打开传输进程
        thread.setDaemon(True)
        thread.start()
        thread = threading.Thread(target=self.__UartRx, args=()) #打开接收进程
        thread.setDaemon(True)
        thread.start()
        while True:
            data = self.queue_from_uart.get(block=True)
            datalen =  (data[0]<<8)+data[1]-1 # 除掉指令位长度
            cmd =data[2]
            data = data[3:]
            if cmd == CommandEnum.PRINT:
                print(data.decode(encoding="utf-8",errors = "replace"))
                
    

    def __UartRx(self):
        # 循环接收数据线程,死循环
        state = Status.WAITHEAD1
        time_now = time.time()
        time_last = time.time()
        while True:
            if self.ser.in_waiting:
                time_now = time.time()
                time_diff = time_now - time_last
                time_last = time_now
                if time_diff > 0.25: # 超时收到数据，上一帧丢弃，status重置后重新解码
                    print("接收数据超时!")
                    state = Status.WAITHEAD1
                r_bytes = self.ser.read(self.ser.in_waiting)#.decode(encoding="utf-8",errors = "replace")
                print(bytes,end="")
                for r_byte in r_bytes:
                    state = self.__state_machine_decoder(state, r_byte)
                

    def __UartTx(self):
        while True:
            packet = self.queue_to_uart.get(block = True)
            self.ser.write(packet)

    def __state_machine_decoder(self,state,r_byte):
        check_len_sum = 0
        datalen_high8 = 0
        datalen_low8 = 0
        datalen = 0
        i = 0
        checksum = 0
        data = None
        
        if state == Status.WAITHEAD1:
            if r_byte == Head.HEAD1:
                state = Status.WAITHEAD2
            else:
                state = Status.WAITHEAD1
        elif state == Status.WAITHEAD2:
            if r_byte == Head.HEAD2:
                state = Status.WAITHEAD3
            else:
                state = Status.WAITHEAD1
        elif state == Status.WAITHEAD3:
            if r_byte == Head.HEAD3:
                check_len_sum = 0
                state = Status.GETLEN1
            else:
                state = Status.WAITHEAD1
        elif state == Status.GETLEN1:
            datalen_high8 = r_byte
            check_len_sum += datalen_high8
            state = Status.GETLEN2
        elif state == Status.GETLEN2:
            datalen_low8 = r_byte
            check_len_sum = (check_len_sum + datalen_low8)%256
            datalen = (datalen_high8<<8) + datalen_low8
            state = Status.LENCHECK
        elif state == Status.LENCHECK:
            if r_byte == check_len_sum:
                i = 0; checksum = 0
                data = [0 for x in range(0,(datalen+2))]
                data[0] = datalen_high8
                data[1] = datalen_low8
                state = Status.READDATA
            else:
                print("checklen wrong!")
                state = Status.WAITHEAD1
        elif state == Status.READDATA:
            data[i+2] = r_byte
            checksum += r_byte
            i += 1
            if(i == datalen):
                state = Status.SUMCHECK
        elif state == Status.SUMCHECK:
            if int(r_byte) == (checksum%256):
                print("接收到一帧")
                self.queue_from_uart.put(data)
            else:
                print("checksum wrong!")
            state = Status.WAITHEAD1
        
        return state
        