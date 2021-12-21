import sys
import socket


class Webcom():
    def __init__(self):
        self.address = ""
        self.__socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)


    def connect(self, _address):
        self.address = (_address, 12345)
        self.__socket.connect(self.address)  # 尝试连接服务端,exception交给上级处理

    def close(self):
        self.__socket.close()


