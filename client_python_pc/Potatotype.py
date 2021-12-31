from Settings import *
import pygame
from Constant import ButtonEnum
from utils import resource_path
from Interface import ConnectInterface
from Interface import ChooseInterface
from Interface import GobangInterface
from Interface import ChatInterface
import sys
import socket

class Potatotype(object):
    '''Potatotype软件类，包含了软件主要的交互和运行流程
    '''

    def __init__(self):
        """交互初始化方法。"""
        self.__window, self.__clock = Potatotype.__init_windows()
        self.__socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        self.__connect_interface = ConnectInterface(self.__window)
        self.__choose_interface  = ChooseInterface(self.__window)
        self.__gobang_interface  = GobangInterface(self.__window, self.__socket)
        self.__chat_interface  = ChatInterface(self.__window, self.__socket)
        
        self.__interface = "connect"

        

    def run(self):
        """进行游戏方法。

        处理游戏中的各种交互事件：\n
        1. 关闭界面事件。\n
        2. 界面跳转事件。\n
        3. 落子事件。\n
        4. 投降与重新开始事件。
        """
        self.__handle_event()
        self.__draw_window()
        #pass
    
    def __draw_window(self):
        """渲染窗口方法。"""
        if self.__interface == "connect":
            self.__connect_interface.draw()
        elif self.__interface == "choose":
            self.__choose_interface.draw()
        elif self.__interface == "chat":
            self.__chat_interface.draw()
        elif self.__interface == "gobang":
            self.__gobang_interface.draw()

        #self.__window.draw()
        pygame.display.update()

    def __handle_event(self):
        for event in pygame.event.get():  # 遍历所有事件
            if event.type == pygame.QUIT:  # 如果单击关闭窗口，则退出
                sys.exit()
            elif event.type == pygame.KEYDOWN:
                self.__keydown(event)
            elif event.type == pygame.MOUSEBUTTONDOWN:
                self.__click(pygame.mouse.get_pos())
    

    def __click(self, _mouse_pos):
        if self.__interface == "connect":
            self.__connect_interface.text_enable = False
            btn = self.__connect_interface.check_buttons(_mouse_pos)
            if btn == ButtonEnum.CONNECT_BUTTON:
                # 使用socket连接
                addr = self.__connect_interface.text
                #addr ='183.172.175.161'
                try:
                    self.__socket.connect((addr,12345))
                    self.__interface = "choose" # 连接成功后进入选择界面
                except Exception:
                    print('[!] Server not found or not open，please try again')
                    self.__interface = "choose" # debug中
            elif btn == ButtonEnum.TEXT_REGION:
                self.__connect_interface.text_enable = True

        elif self.__interface == "choose":
            btn = self.__choose_interface.check_buttons(_mouse_pos)
            if btn == ButtonEnum.CHAT_BUTTON:
                self.__chat_interface.do_login()
                self.__interface = "chat"
            elif btn == ButtonEnum.GOBANG_BUTTON:
                self.__gobang_interface.do_login()
                self.__interface = "gobang"

        elif self.__interface == "chat":
            btn = self.__chat_interface.check_buttons(_mouse_pos)
            if btn == ButtonEnum.SEND_BUTTON:
                self.__chat_interface.do_send()
            elif btn == ButtonEnum.TEXT_REGION:
                self.__chat_interface.text_enable = True
            elif btn == ButtonEnum.BACK_BUTTON:
                self.__chat_interface.do_logout()
                self.__interface = "choose"
            elif btn == ButtonEnum.CLEAN_BUTTON:
                pass

        elif self.__interface == "gobang":
            btn = self.__gobang_interface.check_buttons(_mouse_pos)
            if btn == ButtonEnum.BACK_BUTTON:
                self.__gobang_interface.do_logout()
                self.__interface = "choose"
            elif btn == ButtonEnum.PLAY_REGION:
                x, y = _mouse_pos
                x, y = x // REC_SIZE, y // REC_SIZE
                self.__gobang_interface.make_one_step(x,y)

    def __keydown(self, _key_event):
        if self.__interface == "connect":
            if self.__connect_interface.text_enable:
                if _key_event.key == pygame.K_BACKSPACE:
                    self.__connect_interface.text = self.__connect_interface.text[:-1]
                elif _key_event.key >= pygame.K_0 and _key_event.key <= pygame.K_9:
                    self.__connect_interface.text += chr(_key_event.key)
                elif _key_event.key == pygame.K_PERIOD:   
                    self.__connect_interface.text += _key_event.unicode
        elif self.__interface == "chat":
            if self.__chat_interface.text_enable:
                if _key_event.key == pygame.K_BACKSPACE:
                    self.__chat_interface.type_text = self.__chat_interface.type_text[:-1]
                else:
                    self.__chat_interface.type_text += _key_event.unicode


    @staticmethod
    def __init_windows():
        window = pygame.display.set_mode([WINDOW_WIDTH, WINDOW_HEIGHT])
        pygame.display.set_caption("{} {}".format(SOFTWARE_NAME, SOFTWARE_VERSION))
        pygame.display.set_icon(
            pygame.image.load(resource_path('resource\image\head.ico')))

        # 初始化时钟
        clock = pygame.time.Clock()
        clock.tick(60) # 设定帧率为 60 帧/秒。
        return window, clock