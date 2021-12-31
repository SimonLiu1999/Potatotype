'''
这个模块使用了回退n协议进行无差错的串口通信
'''
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
from Constant import Status
import sys
from queue import Empty, Queue

class Serial_Ack():
    def __init__(self):
        self.portx = "COM6" #端口，GNU / Linux上的/ dev / ttyUSB0 等 或 Windows上的 COM3 等
        self.bps = 115200 #波特率，标准值之一：50,75,110,134,150,200,300,600,1200,1800,2400,4800,9600,19200,38400,57600,115200
        self.timex = 5 #超时设置,None：永远等待操作，0为立即返回请求结果，其他值为等待超时时间(单位为秒）
        self.MAX_SEQ = 3 # 不超过7
        self.next_frame_to_send = 0 # 标志发送到了哪，属于发送窗口
        self.ack_expected = 0   # 标志发送出去的帧被确认到了哪，属于发送窗口
        self.frame_expected = 0 # 用于接收时检查是否是正确顺序的帧，属于接收窗口
        self.event = None 
        self.nbuffered = 0 # 目前buffer中还需要保留的个数，即窗口中未被确认的个数
        self.buffer = [0 for x in range(0,self.MAX_SEQ+1)] #长MAX_SEQ+1的list，作为buffer
        self.network_layer_enable = False

        self.timer_list = [-1 for x in range(0,self.MAX_SEQ+1)]
        self.ack_timer = -1
        self.queue_from_network_layer = Queue(maxsize = 3)
        self.queue_to_network_layer = Queue(maxsize = 3)
        self.queue_from_uart = Queue(maxsize = 3)
        self.queue_to_uart = Queue(maxsize = 3)
        self.queue_event = Queue(maxsize = 3)
        try:
            self.ser=serial.Serial(self.portx, self.bps, timeout=self.timex) # 打开串口，并得到串口对象
        except Exception as e:
            print("---打开串口失败---:",e)
            self.ser.close()#关闭串口
            sys.exit()
        

    def run(self):
        self.__timer_beat(0.0001) # 打开100us定时器
        thread = threading.Thread(target=self.__UartTx, args=()) #打开传输进程
        thread.setDaemon(True)
        thread.start()
        thread = threading.Thread(target=self.__UartRx, args=()) #打开接收进程
        thread.setDaemon(True)
        thread.start()
        self.__enable_network_layer()

        while True:
            event = self.__wait_for_event()
            if event == EventEnum.network_layer_ready:
                self.buffer[self.next_frame_to_send] = self.queue_from_network_layer.get()
                self.nbuffered = self.nbuffered + 1
                self.__send_data(frame_kind.data, self.next_frame_to_send, self.frame_expected, self.buffer)
                self.next_frame_to_send = self.__inc(self.next_frame_to_send)
            elif event == EventEnum.frame_arrival:
                r = self.queue_from_uart.get()
                if self.getkind(r) == frame_kind.data:
                    if self.getseq(r) == self.frame_expected:
                        self.queue_to_network_layer.put(self.getdata(r))
                        self.frame_expected = self.__inc(self.frame_expected)
                        self.__start_ack_timer()
                    while self.__between(self.ack_expected, self.getack(r), self.next_frame_to_send):
                        self.nbuffered = self.nbuffered - 1
                        self.__stop_timer(self.ack_expected)
                        self.ack_expected = self.__inc(self.ack_expected) 
                elif self.getkind(r) == frame_kind.ack:
                    while self.__between(self.ack_expected, self.getack(r), self.next_frame_to_send):
                        self.nbuffered = self.nbuffered - 1
                        self.__stop_timer(self.ack_expected)
                        self.ack_expected = self.__inc(self.ack_expected) 
            elif event == EventEnum.timeout:
                self.next_frame_to_send = self.ack_expected #开始重传
                print("超时重传")
                for i in range(self.nbuffered):
                    self.__send_data(frame_kind.data, self.next_frame_to_send, self.frame_expected, self.buffer)
                    self.next_frame_to_send = self.__inc(self.next_frame_to_send)
            elif event == EventEnum.ack_timeout:
                self.__send_data(frame_kind.ack, 0, self.frame_expected, self.buffer)
            if self.nbuffered < self.MAX_SEQ:
                self.__enable_network_layer()
            else:
                self.__disable_network_layer()
    
    def __between(self, a, b, c):
        if (a<=b and b<c) or (c<a and a<=b) or (b<c and c<a):
            return True
        else:
            return False

    def getkind(self,packet):
        pass
    def getseq(self,packet):
        pass
    def getack(self,packet):
        pass
    def getdata(self,packet):
        pass

    def __UartTx(self):
        while True:
            packet = self.queue_to_uart.get(block = True)
            self.ser.write(packet)

    def __UartRx(self):
        # 循环接收数据线程,死循环
        state = Status.WAITHEAD1
        while True:
            if self.ser.in_waiting:
                r_bytes = self.ser.read(self.ser.in_waiting).decode(encoding="utf-8",errors = "replace")
                print(bytes,end="")
                for r_byte in r_bytes:
                    state = self.__state_machine_decoder(state, r_byte)

    def __state_machine_decoder(self, state, r_byte):
        #解码后:
        pass
        self.queue_from_uart.put(packet)
        self.queue_event.put(EventEnum.frame_arrival)


    def __enable_network_layer(self):
        self.network_layer_enable = True
        if (not self.queue_from_network_layer.empty()):
            self.queue_event.put(EventEnum.network_layer_ready)

    def __disable_network_layer(self):
        self.network_layer_enable = False

    def __wait_for_event(self):
        while True:
            event = self.queue_event.get()
            if event == EventEnum.network_layer_ready and self.network_layer_enable == False:
                continue
            else:
                return event

    def __send_data(self,frame_kind, frame_nr, frame_expected, buffer):
        packet_to_uart = self.__make_uart_packet(frame_kind, frame_nr, (frame_expected+self.MAX_SEQ)%(self.MAX_SEQ+1), buffer)
        self.queue_to_uart.put(packet_to_uart)
        self.__start_timer(frame_nr)
        self.__stop_ack_timer()

    def __inc(self,num):
        return (num+1) % (self.MAX_SEQ + 1)

    def __make_uart_packet(self,frame_kind, frame_nr, ack, buffer):
        a=b'\x11\x22\x33' #帧头
        a=bytearray(a)
        ctr = ((frame_kind<<6) + (frame_nr<<3) + (ack))%256# 控制位
        a.append(ctr)
        datalen = len(buffer[frame_nr])
        a.append(datalen//256) #高8位
        a.append(datalen%256)  #低8位
        a.append((datalen//256 + datalen%256 + ctr)%256) #校验和1
        sum=0
        for i in buffer[frame_nr]:
            a.append(i)
            sum+=i
        a.append(sum%256) #校验和2
        return a

    def __start_timer(self, frame_nr):
        self.timer_list[frame_nr] = 5000 #500ms定时器 

    def __stop_timer(self, frame_nr):
        self.timer_list[frame_nr] = -1

    def __start_ack_timer(self):
        self.ack_timer = 10 #1ms定时器 

    def __stop_ack_timer(self,):
        self.ack_timer = -1

    def __timer_beat(self, inc):
        for timer in self.timer_list:
            if timer > 0:
                timer = timer -1
                if timer == 0:
                    timer = -1
                    print("timeout!")
                    self.queue_event.put(EventEnum.timeout)
        if self.ack_timer > 0:
            self.ack_timer = self.ack_timer - 1
            if self.ack_timer == 0:
                self.ack_timer = -1 
                print("ack_timeout!")
                self.queue_event.put(EventEnum.ack_timeout)
        t = Timer(inc, self.__timer_beat, (inc,))
        t.start()
    
    #def send_from_network(self):
        #数据包加入队列
        #if ser.network_layer_enable == True:
        #   加入event队列



