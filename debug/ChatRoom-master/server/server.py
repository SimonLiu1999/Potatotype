import socket
import threading
import json
import numpy as np


class Server:
    """
    服务器类
    """

    def __init__(self):
        """
        构造
        """
        self.__socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.__chat_connections = {}
        self.__gobang_connections = {}
        self.__nicknames = {}
        self.totol_connect_num = 0
        self.black_player = None
        self.white_player = None
        self.qizi = np.zeros((15,15),dtype=np.int)

    def __user_thread(self, user_id):
        """
        用户子线程
        :param user_id: 用户id
        """
        connection = self.__chat_connections[user_id]
        nickname = self.__nicknames[user_id]
        print('[Server] 用户', user_id, nickname, '加入聊天室')
        self.__broadcast(message='用户 ' + str(nickname) + '(' + str(user_id) + ')' + '加入聊天室')

        # 侦听
        while True: 
            # noinspection PyBroadException
            try:
                buffer = connection.recv(1024).decode()
                # 解析成json数据
                obj = json.loads(buffer)
                # 如果是广播指令
                if obj['type'] == 'broadcast':
                    self.__broadcast(obj['sender_id'], obj['message'])
                elif obj['type'] == 'logout':
                    print('[Server] 用户', user_id, nickname, '退出聊天室')
                    self.__broadcast(message='用户 ' + str(nickname) + '(' + str(user_id) + ')' + '退出聊天室')
                    self.__chat_connections.pop(user_id)
                    self.__nicknames.pop(user_id)
                    
                    thread = threading.Thread(target=self.__waitForLogin, args=(connection,user_id))
                    thread.setDaemon(True)
                    thread.start()

                    break
                else:
                    print('[Server] 无法解析json数据包:', connection.getsockname(), connection.fileno())
            except Exception:
                print('[Server] 连接失效:', connection.getsockname(), connection.fileno())
                #self.__chat_connections[user_id].close()
                self.__chat_connections.pop(user_id)
                self.__nicknames.pop(user_id)
                break

    def __gobang_thread(self, user_id, identity):       
        """
        gobang子线程
        :param user_id: 用户id
        """
        connection = self.__gobang_connections[user_id]
        nickname = self.__nicknames[user_id]
        print('[Server] 用户', user_id, nickname, '加入gobang')
        self.__gobang_broadcast(message= identity +'' + str(nickname) + '(' + str(user_id) + ')' + '加入游戏', type="info")

        # 侦听
        while True: 
            # noinspection PyBroadException
            try:
                buffer = connection.recv(1024).decode()
                # 解析成json数据
                obj = json.loads(buffer)
                # 如果是广播指令
                if obj['type'] == 'xiaqi':
                    self.__gobang_broadcast(sender_id=obj['sender_id'], type='xiaqi',identity=obj['identity'], pos=obj['pos'])
                    x, y = obj['pos']
                    self.qizi[x][y] = 1 if obj['identity']=="black" else 2
                    winner = self.__check_winner(x,y,self.qizi[x][y])
                    if winner:
                        winner = 'black' if winner ==1 else 'white'
                        message = "winner is " + winner
                        self.__gobang_broadcast(sender_id=0, type='win',identity=obj['identity'], message=message)
                elif obj['type'] == 'logout':
                    print('[Server] 用户', user_id, nickname, '退出游戏')
                    if obj['identity'] != "audience":
                        self.__gobang_broadcast(sender_id=0, type='logout',identity=obj['identity'])
                        if identity == 'black':
                            self.black_player = None
                        if identity == 'white':
                            self.white_player = None
                    else:
                        self.__gobang_broadcast(sender_id=0,message= identity +'' + str(nickname) + '(' + str(user_id) + ')' + '退出观看游戏', type="info")

                    self.__gobang_connections.pop(user_id)
                    self.__nicknames.pop(user_id)
                    
                    thread = threading.Thread(target=self.__waitForLogin, args=(connection,user_id))
                    thread.setDaemon(True)
                    thread.start()
                    break
                else:
                    print('[Server] 无法解析json数据包:', connection.getsockname(), connection.fileno())
            except Exception as e:
                print(e)
                print(e.__traceback__.tb_lineno)
                print('[Server] 连接失效:', connection.getsockname(), connection.fileno())
                #self.__chat_connections[user_id].close()
                self.__gobang_connections.pop(user_id)
                self.__nicknames.pop(user_id)
                break

    def __broadcast(self, sender_id=0, message='', type='info',identity='', pos=(0,0)):
        """
        广播
        :param user_id: 用户id(0为系统)
        :param message: 广播内容
        """
        for user_id, conection in self.__chat_connections.items():
            if user_id != sender_id and conection:
                conection.send(json.dumps({
                    'sender_id': sender_id,
                    'sender_nickname': self.__nicknames[sender_id],
                    'message': message
                }).encode())

    def __check_winner(self, x, y, turn):
        #沿x方向
        cnt = 0
        curx,cury = x,y
        while (0<=curx and curx<15 and 0<=cury and cury<15 and self.qizi[curx][cury]==turn):
            cnt += 1
            curx -= 1
        curx,cury = x,y
        while(0<=curx and curx<15 and 0<=cury and cury<15 and self.qizi[curx][cury]==turn):
            cnt += 1
            curx += 1
        if(cnt>5):
            return turn
        #沿y方向
        cnt=0
        curx,cury = x,y
        while(0<=curx and curx<15 and 0<=cury and cury<15 and self.qizi[curx][cury]==turn) :
            cnt+=1
            cury-=1
        curx,cury = x,y
        while(0<=curx and curx<15 and 0<=cury and cury<15 and self.qizi[curx][cury]==turn) :
            cnt+=1
            cury+=1
        if(cnt>5):
            return turn
        #沿右上方向
        cnt=0
        curx,cury = x,y
        while (0<=curx and curx<15 and 0<=cury and cury<15 and self.qizi[curx][cury]==turn) :
            cnt+=1
            curx+=1
            cury-=1
        curx,cury = x,y
        while (0<=curx and curx<15 and 0<=cury and cury<15 and self.qizi[curx][cury]==turn) :
            cnt+=1
            curx-=1
            cury+=1
        if(cnt>5):
            return turn
        #沿左上方向
        cnt=0
        curx,cury = x,y
        while(0<=curx and curx<15 and 0<=cury and cury<15 and self.qizi[curx][cury]==turn) :
            cnt+=1
            curx-=1
            cury-=1
        curx,cury = x,y
        while(0<=curx and curx<15 and 0<=cury and cury<15 and self.qizi[curx][cury]==turn):
            cnt+=1
            curx+=1
            cury+=1
        if (cnt>5):
            return turn
        return 0 # 没赢

    def __gobang_broadcast(self, sender_id=0, message='', type='info',identity='', pos=(0,0)):
        """
        广播
        :param user_id: 用户id(0为系统)
        :param message: 广播内容
        """
        if type == "info":
            for user_id, conection in self.__gobang_connections.items():
                if user_id != sender_id and conection:
                    conection.send(json.dumps({
                        'sender_id': sender_id,
                        'sender_nickname': self.__nicknames[sender_id],
                        'type': type,
                        'message': message
                    }).encode())
        elif type == "xiaqi":
            for user_id, conection in self.__gobang_connections.items():
                if user_id != sender_id and conection:
                    conection.send(json.dumps({
                        'sender_id': sender_id,
                        'sender_nickname': self.__nicknames[sender_id],
                        'type': type,
                        'identity': identity,
                        'pos': pos
                    }).encode())
        elif type == "logout": # player logout
            for user_id, conection in self.__gobang_connections.items():
                if user_id != sender_id and conection:
                    conection.send(json.dumps({
                        'sender_id': sender_id,
                        'sender_nickname': self.__nicknames[sender_id],
                        'identity': identity,
                        'type': type,
                    }).encode())
        elif type == "win": # player win
            for user_id, conection in self.__gobang_connections.items():
                if user_id != sender_id and conection:
                    conection.send(json.dumps({
                        'sender_id': sender_id,
                        'sender_nickname': self.__nicknames[sender_id],
                        'message': message,
                        'identity': identity,
                        'type': type,
                    }).encode())
    def __waitForLogin(self, connection, user_id):
        # 尝试接受数据
        # noinspection PyBroadException
        try:
            buffer = connection.recv(1024).decode()
            # 解析成json数据
            obj = json.loads(buffer)
            # 如果是连接指令，那么则返回一个新的用户编号，接收用户连接
            if obj['type'] == 'login':
                self.__chat_connections.update({user_id: connection})
                self.__nicknames.update({user_id: obj['nickname']})
                connection.send(json.dumps({
                    'id': user_id
                }).encode())
                # 开辟一个新的线程
                thread = threading.Thread(target=self.__user_thread, args=(user_id,))
                thread.setDaemon(True)
                thread.start()

            elif obj['type'] == 'gobang_login':
                if self.black_player == None:
                    identity = "black"
                    self.black_player = user_id
                elif self.white_player == None:
                    identity = "white"
                    self.white_player = user_id
                else:
                    identity = "audience"
                self.__gobang_connections.update({user_id: connection})
                self.__nicknames.update({user_id: obj['nickname']})
                connection.send(json.dumps({
                    'id': user_id,
                    'identity': identity
                }).encode())
                # 开辟一个新的线程
                thread = threading.Thread(target=self.__gobang_thread, args=(user_id,identity))
                thread.setDaemon(True)
                thread.start()
            else:
                print('[Server] 无法解析json数据包:', connection.getsockname(), connection.fileno())
        except Exception as e:
            print(e)
            print(e.__traceback__.tb_lineno)
            print('[Server] 无法接受数据:', connection.getsockname(), connection.fileno())

    def start(self):
        """
        启动服务器
        """
        # 绑定端口
        self.__socket.bind(('183.172.175.161', 12345))
        # 启用监听
        self.__socket.listen(10)
        print('[Server] 服务器正在运行......')

        # 清空连接
        self.__chat_connections.clear()
        self.__nicknames.clear()

        self.__chat_connections.update({0: None})
        self.__nicknames.update({0: "System"})
        self.totol_connect_num += 1

        # 开始侦听
        while True:
            connection, address = self.__socket.accept()
            print('[Server] 收到一个新连接', connection.getsockname(), connection.fileno())
            user_id = self.totol_connect_num 
            self.totol_connect_num += 1

            thread = threading.Thread(target=self.__waitForLogin, args=(connection,user_id))
            thread.setDaemon(True)
            thread.start()
