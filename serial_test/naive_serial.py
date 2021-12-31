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
        '''
        while True:
            data = self.queue_from_uart.get(block=True)
            datalen =  (data[0]<<8)+data[1]-1 # 除掉指令位长度
            cmd =data[2]
            data = data[3:]
            if cmd == CommandEnum.PRINT:
                print(data_decoder.decode(encoding="utf-8",errors = "replace"))
        '''

    

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
                #print(r_bytes)
                for r_byte in r_bytes:
                    state = self.__state_machine_decoder(state, r_byte)
                

    def __UartTx(self):
        while True:
            packet = self.queue_to_uart.get(block = True)
            self.ser.write(packet)

    def __state_machine_decoder(self,state,r_byte):
        global check_len_sum_decoder
        global datalen_high8_decoder
        global datalen_low8_decoder
        global datalen_decoder
        global i_decoder
        global checksum_decoder
        global data_decoder

        if state == Status.WAITHEAD1: # init
            check_len_sum_decoder=0
            datalen_high8_decoder=0
            datalen_low8_decoder=0
            datalen_decoder=0
            i_decoder=0
            checksum_decoder=0
            data_decoder = None
        
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
                check_len_sum_decoder = 0
                state = Status.GETLEN1
            else:
                state = Status.WAITHEAD1
        elif state == Status.GETLEN1:
            datalen_high8_decoder = r_byte
            check_len_sum_decoder += datalen_high8_decoder
            state = Status.GETLEN2
        elif state == Status.GETLEN2:
            datalen_low8_decoder = r_byte
            #print("datalenlow8%d"%datalen_low8_decoder)
            check_len_sum_decoder = (datalen_high8_decoder + datalen_low8_decoder)%256
            datalen_decoder = (datalen_high8_decoder<<8) + datalen_low8_decoder
            state = Status.LENCHECK
        elif state == Status.LENCHECK:
            #print("len%d"%check_len_sum_decoder)
            if r_byte == check_len_sum_decoder:
                i_decoder = 0; checksum_decoder = 0
                data_decoder = bytearray([0 for x in range(0,(datalen_decoder+2))])
                data_decoder[0] = datalen_high8_decoder
                data_decoder[1] = datalen_low8_decoder
                state = Status.READDATA
            else:
                print("checklen wrong!")
                state = Status.WAITHEAD1
        elif state == Status.READDATA:
            data_decoder[i_decoder+2] = r_byte
            checksum_decoder += r_byte
            i_decoder += 1
            if(i_decoder == datalen_decoder):
                state = Status.SUMCHECK
        elif state == Status.SUMCHECK:
            if int(r_byte) == (checksum_decoder%256):
                print("接收到一帧")
                self.queue_from_uart.put(data_decoder)
            else:
                print("checksum wrong!")
            state = Status.WAITHEAD1
        
        return state

if __name__ == '__main__':
    a = Naive_serial()
    a.run()