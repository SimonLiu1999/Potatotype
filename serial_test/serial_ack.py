import serial #导入模块
import threading
import serial.tools.list_ports
from PIL import Image
import numpy as np
import time
from Constant import CommandEnum
import sys
from queue import Empty, Queue

class Serial_Ack():
    def __init__(self):
        self.portx="COM7" #端口，GNU / Linux上的/ dev / ttyUSB0 等 或 Windows上的 COM3 等
        self.bps=115200 #波特率，标准值之一：50,75,110,134,150,200,300,600,1200,1800,2400,4800,9600,19200,38400,57600,115200
        self.timex=5 #超时设置,None：永远等待操作，0为立即返回请求结果，其他值为等待超时时间(单位为秒）
        self.queue_packetToTx = Queue(maxsize = 5)
        self.queue_ackToTx = Queue(maxsize = 1)
        self.queue_Rxpacket = Queue(maxsize = 5)

        self.rx_expected_num = 0 # 采用一位滑动窗口协议进行无差错传输，取值为0，1
        self.next_send_num = 0 # 取值为0，1

        try:
            self.ser=serial.Serial(self.portx, self.bps, timeout=self.timex) # 打开串口，并得到串口对象
        except Exception as e:
            print("---打开串口失败---:",e)
            self.ser.close()#关闭串口
            sys.exit()

    def run(self):
        thread = threading.Thread(target=self.__UartTx, args=(self.ser,)) #打开传输进程
        thread.setDaemon(True)
        thread.start()
        thread = threading.Thread(target=self.__UartRx, args=(self.ser,)) #打开接收进程
        thread.setDaemon(True)
        thread.start()

    def __UartTx(self, ser):
        packet = self.queue_packetToTx.get(block = True)
        seq = self.next_send_num
        ack = 1 - self.rx_expected_num
        packet = self.__add_num_to_packet(packet,seq,ack) #加的号是self.tx_sending_num
        ser.write(packet)
        while True:
            try:
                enable = self.queue_ackToTx.get(block = True, timeout=0.2) # 用于指示收到帧(不是收到ack)
            except Empty as e: # 超时重传
                print("ack timeout! " + str(e))
                ser.write(packet)
            else: # 收到发送使能
                if (self.ackflag == 1): #收到确认，需要更新需要发的包
                    packet = self.queue_packetToTx.get(block = False)
                if (self.dataflag == 1): #收到数据，需要立即发一个包出去

                        

                
                    pass
                packet = self.queue_packetToTx.get(block = True) # 低速下堵在这????
                seq = self.next_send_num
                ack = 1 - self.rx_expected_num
                packet = self.__add_num_to_packet(packet, seq, ack) #加的号是self.tx_sending_num
                ser.write(packet)

    def __add_num_to_packet(self, packet, seq, ack):
        command = packet[6]
        packet[6] = (command & 0x7f) | ((seq<<7) & 0x80) | ((ack<<6) & 0x40) #把command最高位置为seq，第二高位置为ack
        return packet

    def __UartRx(self, ser):
        # 循环接收数据线程,死循环
        while True:
            if ser.in_waiting:
                STRGLO = ser.read(ser.in_waiting).decode(encoding="utf-8",errors = "replace")
                print(STRGLO,end="")
                if "收到一帧":
                    self.dataflag = 0
                    self.ackflag = 0
                    command = "取出command"
                    seq = (command >> 7) & 0x01
                    ack = (command >> 6) & 0x01
                    if(seq == self.rx_expected_num): #收到正确数据
                        """数据送往上层"""
                        self.rx_expected_num = 1 - self.rx_expected_num
                        self.dataflag = 1
                    if(ack == self.next_send_num):
                        self.next_send_num = 1 - self.next_send_num
                        self.ackflag = 1
                    self.queue_ackToTx.put(1) #使能





img=Image.open("tsinghualogo.jpg")
img = img.resize((152,152))
img = img.convert('L')
threshold = 200
table = []
for i in range(256):
    if i < threshold:
        table.append(0)
    else:
        table.append(1)
# 图片二值化
img = img.point(table, '1')

data = np.asarray(img)      #转换为矩阵
data=data.flatten()
Black=bytearray()
Yellow=bytearray()
for i in range(0,len(data)//8):
    b=0
    for j in range(0,8):
        b=b<<1
        b+=data[i*8+j]
    Black.append(b)
    Yellow.append(0xff)

def calc(str):
    a=b'\x11\x22\x33\x02\x00'
    a=bytearray(a)
    a.append(len(str))
    sum=0
    for i in str:
        a.extend(bytes(i, encoding="utf8"))
        sum+=ord(i)
    a.append(sum%256)
    ser.write(a)
    return

def calc_new(str):
    a=b'\x11\x22\x33' #帧头
    a=bytearray(a)
    datalen = len(str) + 1
    a.append(datalen//256) #高8位
    a.append(datalen%256)  #低8位
    a.append((datalen//256 + datalen%256)%256) #datalen校验和 
    sum=0
    a.append(CommandEnum.TASK_CALCULATION)
    sum+=CommandEnum.TASK_CALCULATION
    for i in str:
        a.extend(bytes(i, encoding="utf8"))
        sum+=ord(i)
    a.append(sum%256)
    ser.write(a)
    return

def RefreshScreen(ser,Black,Yellow):
    a=b'\x11\x22\x33\x16\x91'
    a=bytearray(a)
    a.append((0x16+0x91)%256)
    sum=0
    a.append(CommandEnum.TASK_SCREEN)
    sum+=CommandEnum.TASK_SCREEN
    for i in range(0,2888):
        a.append(Black[i])
        sum+=Black[i]
    for i in range(0,2888):
        a.append(Yellow[i])
        sum+=Yellow[i]
    a.append(sum%256)
    ser.write(a)
    return




try:
  #端口，GNU / Linux上的/ dev / ttyUSB0 等 或 Windows上的 COM3 等
  portx="COM7"
  #波特率，标准值之一：50,75,110,134,150,200,300,600,1200,1800,2400,4800,9600,19200,38400,57600,115200
  bps=115200
  #超时设置,None：永远等待操作，0为立即返回请求结果，其他值为等待超时时间(单位为秒）
  timex=5
  # 打开串口，并得到串口对象
  ser=serial.Serial(portx,bps,timeout=timex)
  threading.Thread(target=ReadData, args=(ser,)).start()
  # 写数据
  #print("写总字节数:",result)

  #ser.close()#关闭串口

except Exception as e:
    print("---异常---：",e)
    ser.close()#关闭串口

#time.sleep(10)


if __name__ == '__main__':
    calc("(1/7+2/7+3/7+1/7+2/7+3/7+(1/7+(2/7+3/7+(1/7+2/7+3/7+1/7))+2/7+3/7)+1/7+(2/7+(3/7)*5+3))**2")
    calc_new("1/7")
    #time.sleep(10)
    RefreshScreen(ser,Black,Yellow)