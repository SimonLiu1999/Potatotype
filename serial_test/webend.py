from flask import Flask,jsonify,request,render_template
import serial #导入模块
import threading
import serial.tools.list_ports
from PIL import Image
import numpy as np
import time
import sys

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
    time.sleep(0.3)
    rbuf = ser.read(ser.in_waiting).decode("utf-8")
    rbuf=rbuf[10:]
    return rbuf

def RefreshScreen(ser,Black,Yellow):
    a=b'\x11\x22\x33\x01\x16\x90'
    a=bytearray(a)
    sum=0
    for i in range(0,2888):
        a.append(Black[i])
        sum+=Black[i]
    for i in range(0,2888):
        a.append(Yellow[i])
        sum+=Yellow[i]
    a.append(sum%256)
    ser.write(a)
    return



STRGLO="" #读取的数据
BOOL=True  #读取标志位

#读数代码本体实现
def ReadData(ser):
    global STRGLO,BOOL
    # 循环接收数据，此为死循环，可用线程实现
    while BOOL:
        if ser.in_waiting:
            STRGLO = ser.read(ser.in_waiting).decode("utf-8")
            print(STRGLO,end="")

try:
  #端口，GNU / Linux上的/ dev / ttyUSB0 等 或 Windows上的 COM3 等
  portx="COM3"
  #波特率，标准值之一：50,75,110,134,150,200,300,600,1200,1800,2400,4800,9600,19200,38400,57600,115200
  bps=115200
  #超时设置,None：永远等待操作，0为立即返回请求结果，其他值为等待超时时间(单位为秒）
  timex=5
  # 打开串口，并得到串口对象
  ser=serial.Serial(portx,bps,timeout=timex)
  #threading.Thread(target=ReadData, args=(ser,)).start()
  # 写数据
  #print("写总字节数:",result)

  #ser.close()#关闭串口

except Exception as e:
    print("---异常---：",e)
    ser.close()#关闭串口

app = Flask(__name__)
@app.route('/')
def hello():
    return render_template("index.html")


@app.route('/index', methods=['POST'])
def index():
    sentence = request.form['sentence']
    res = str(calc(sentence))

    return jsonify({'sentence': res})

if __name__ == '__main__':
    app.run() #127.0.0.1 回路 自己返回自己
'''
#time.sleep(10)
calc("(1/7+2/7+3/7+1/7+2/7+3/7+(1/7+(2/7+3/7+(1/7+2/7+3/7+1/7))+2/7+3/7)+1/7+(2/7+(3/7)*5+3))**2")
calc("1/7")
#time.sleep(10)
RefreshScreen(ser,Black,Yellow)'''