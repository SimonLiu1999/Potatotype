from abc import abstractmethod
import pygame
import pygame.image
import threading
import json
from Text import Text
from Settings import *
from utils import resource_path
from Button import Button
from Constant import ButtonEnum
import numpy as np

class AbstractInterface(object):
    """界面超类。"""

    @abstractmethod
    def draw(self):
        """绘制界面抽象方法

        绘制背景、按钮等界面上所存在的元素。
        """
        pass

    @abstractmethod
    def check_buttons(self, _mouse_pos):
        """检查按钮点击抽象方法。

        检查是否有按钮被点击。

        Args:
            _mouse_pos: 鼠标点击的坐标

        Returns:
            有按钮被点击时，返回 ButtonEnum 中对应按钮类型，\n
            否则返回 ButtonEnum 中无按钮类型。
        """
        pass

    @abstractmethod
    def reset(self):
        """重置页面抽象方法。

        用于重置页面，将页面上元素恢复到初始状态。
        """

class ConnectInterface(AbstractInterface):
    '''联网界面'''
    def __init__(self, _windows):
        """
            Args:
            _windows: 由 Pygame 创建的当前窗口
        """
        self.__windows = _windows
        self.text = "127.0.0.1"
        self.text_enable = False
        self.__textfont = pygame.font.Font("C:\\Windows\\Fonts\\simfang.ttf", 32)

        # 加载背景图并将其缩放至适合窗口的大小。
        self.__background_img = pygame.transform.scale(
            pygame.image.load('./resource/image/background.jpg'),
            [WINDOW_WIDTH, WINDOW_HEIGHT])

        # 创建首页上标题。
        self.__title_text = Text("C:\\Windows\\Fonts\\simfang.ttf", TITLE_HEIGHT, '欢迎使用土豆服务器',
                                 WHITE_COLOR, (TITLE_X, TITLE_Y))

        # 创建首页上按钮。         
        self.__connect_button = Button('连接', BUTTON_COLOR, True,
                                     (TITLE_X - BUTTON_WIDTH // 2,
                                      TITLE_Y + TITLE_HEIGHT))

        # 创建输入字符区域。
        self.__textrect = pygame.Rect(0, 0, 200, 50)
        self.__textrect.topleft = (TITLE_X - 100 , TITLE_Y + 200)

    def draw(self):
        self.__draw_background()
        self.__draw_button(self.__connect_button)
        self.__draw_text()

    def __draw_background(self):
        """绘制背景。"""
        pygame.draw.rect(self.__windows, WHITE_COLOR,
                         pygame.Rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT))
        self.__windows.blit(self.__background_img, (0, 0))
        self.__windows.blit(*self.__title_text.text_element)

    def __draw_button(self, _button):
        """绘制首页按钮。"""
        self.__windows.fill(_button.color, _button.rect)
        self.__windows.blit(*_button.text_element)

    def __draw_text(self):
        self.__windows.fill((255,255,255), self.__textrect)
        text_surface = self.__textfont.render(self.text, True, (0,0,0))
        self.__windows.blit(text_surface,(TITLE_X - 100 , TITLE_Y + 200))

    def check_buttons(self, _mouse_pos):
        if self.__connect_button.clicked(_mouse_pos):
            print("CONNECT_BUTTON clicked")
            return ButtonEnum.CONNECT_BUTTON
        if self.__textrect.collidepoint(_mouse_pos):
            print("TEXT_REGION clicked")
            return ButtonEnum.TEXT_REGION
        return ButtonEnum.NO_BUTTON
            
    def reset():
        pass

class ChooseInterface(AbstractInterface):
    '''选择界面'''
    def __init__(self, _windows):
        self.__windows = _windows

        self.__background_img = pygame.transform.scale(
            pygame.image.load('./resource/image/background.jpg'),
            [WINDOW_WIDTH, WINDOW_HEIGHT])

        self.__title_text = Text("C:\\Windows\\Fonts\\simfang.ttf", TITLE_HEIGHT, '选择功能',
            WHITE_COLOR, (TITLE_X, TITLE_Y))

        # 创建按钮。         
        self.__chat_button = Button('聊天', BUTTON_COLOR, True,
                                     (TITLE_X - BUTTON_WIDTH // 2,
                                      TITLE_Y + TITLE_HEIGHT))

        self.__gobang_button = Button('五子棋', BUTTON_COLOR, True,
                                     (TITLE_X - BUTTON_WIDTH // 2,
                                      TITLE_Y + TITLE_HEIGHT + 60))

    def draw(self):
        self.__draw_background()
        self.__draw_button(self.__chat_button)
        self.__draw_button(self.__gobang_button)

    def check_buttons(self, _mouse_pos):
        if self.__chat_button.clicked(_mouse_pos):
            print("CHAT_BUTTON clicked")
            return ButtonEnum.CHAT_BUTTON
        if self.__gobang_button.clicked(_mouse_pos):
            print("GOBANG_BUTTON clicked")
            return ButtonEnum.GOBANG_BUTTON
        return ButtonEnum.NO_BUTTON

    def __draw_button(self, _button):
        self.__windows.fill(_button.color, _button.rect)
        self.__windows.blit(*_button.text_element)

    def __draw_background(self):
        """绘制首页背景。"""
        pygame.draw.rect(self.__windows, WHITE_COLOR,
                         pygame.Rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT))
        self.__windows.blit(self.__background_img, (0, 0))
        self.__windows.blit(*self.__title_text.text_element)

class ChatInterface(AbstractInterface):
    '''聊天界面'''
    def __init__(self, _windows, _socket):
        self.__windows = _windows
        self.__socket = _socket
        self.__id = None
        self.__nickname = None
        self.__isLogin = False

        self.__clean_button = Button('清空', BUTTON_COLOR, False,
                                       (BOARD_WIDTH + 30, 430))
        self.__send_button = Button('发送', BUTTON_COLOR, True,
                                       (BOARD_WIDTH + 30, BUTTON_HEIGHT + 460))
        self.__back_button = Button('返回', BUTTON_COLOR, True,
                                    (BOARD_WIDTH + 30, 2 * BUTTON_HEIGHT + 490))
        
        self.type_text = ""
        self.__disp_text = []
        self.text_enable = False
        self.__textfont = pygame.font.Font("C:\\Windows\\Fonts\\simfang.ttf", 22)

        # 创建输入字符区域。
        self.__textrect = pygame.Rect(0, BOARD_HEIGHT // 4 * 3, BOARD_WIDTH, BOARD_HEIGHT - BOARD_HEIGHT // 4 * 3)
        # 创建显示字符区域。
        self.__disprect = pygame.Rect(0, 0, BOARD_WIDTH, BOARD_HEIGHT // 4 * 3)

    def __draw_background(self):
        # 绘制显示区。
        pygame.draw.rect(self.__windows, LIGHT_YELLOW,
                         self.__disprect)
        # 绘制输入区。            
        pygame.draw.rect(self.__windows, WHITE_COLOR,
                         self.__textrect)
        # 绘制右侧白色背景。
        pygame.draw.rect(self.__windows, ORANGE_COLOR,
                         (BOARD_WIDTH, 0, INFO_WIDTH, BOARD_HEIGHT))

    def __add_line(self,str):
        if self.__isLogin:
            if len(self.__disp_text) < 10:
                self.__disp_text.append(str)
            else:
                self.__disp_text.append(str)
                self.__disp_text = self.__disp_text[1:11]

    def __draw_text(self):
        for i in range(len(self.__disp_text)):
            text_surface = self.__textfont.render(self.__disp_text[i], True, (0,0,0))
            self.__windows.blit(text_surface,(0, 25*i))
        text_surface = self.__textfont.render(self.type_text, True, (0,0,0))
        self.__windows.blit(text_surface,(0, BOARD_HEIGHT // 4 * 3))
        

    def draw(self):
        self.__draw_background()
        self.__draw_button(self.__clean_button)
        self.__draw_button(self.__send_button)
        self.__draw_button(self.__back_button)
        self.__draw_text()

    def check_buttons(self, _mouse_pos):
        if self.__clean_button.clicked(_mouse_pos):
            return ButtonEnum.CLEAN_BUTTON
        if self.__send_button.clicked(_mouse_pos):
            return ButtonEnum.SEND_BUTTON
        if self.__back_button.clicked(_mouse_pos):
            return ButtonEnum.BACK_BUTTON
        if self.__textrect.collidepoint(_mouse_pos):
            print("TEXT_REGION clicked")
            return ButtonEnum.TEXT_REGION
        return ButtonEnum.NO_BUTTON
    
    def __draw_button(self, _button):
        """绘制游戏界面按钮。"""
        self.__windows.fill(_button.color, _button.rect)
        self.__windows.blit(*_button.text_element)

    def __receive_message_thread(self):
        """
        接受消息线程
        """
        while self.__isLogin:
            # noinspection PyBroadException
            try:
                buffer = self.__socket.recv(1024).decode()
                obj = json.loads(buffer)
                print('[' + str(obj['sender_nickname']) + '(' + str(obj['sender_id']) + ')' + ']', obj['message'])
                self.__add_line('[' + str(obj['sender_nickname']) + '(' + str(obj['sender_id']) + ')' + ']'+ obj['message'])
            except Exception as ex:
                print(ex)
                print('[Client] 无法从服务器获取数据2')

    def __send_message_thread(self, message):
        """
        发送消息线程
        :param message: 消息内容
        """
        self.__socket.send(json.dumps({
            'type': 'broadcast',
            'sender_id': self.__id,
            'message': message
        }).encode())

    def do_login(self):
        """
        登录聊天室
        """
        nickname = "tester"

        # 将昵称发送给服务器，获取用户id
        self.__socket.send(json.dumps({
            'type': 'login',
            'nickname': nickname
        }).encode())
        # 尝试接受数据
        # noinspection PyBroadException
        try:
            buffer = self.__socket.recv(1024).decode()
            obj = json.loads(buffer)
            if obj['id']:
                self.__nickname = nickname
                self.__id = obj['id']
                self.__isLogin = True
                self.__add_line('[Client] 成功登录到聊天室')

                # 开启子线程用于接受数据
                thread = threading.Thread(target=self.__receive_message_thread)
                thread.setDaemon(True)
                thread.start()
            else:
                 self.__add_line('[Client] 无法登录到聊天室')
        except Exception as ex:
            print(ex)
            print('[Client] 无法从服务器获取数据1')

    def do_send(self):
        """
        发送消息
        """
        message = self.type_text
        self.type_text = ""
        # 显示自己发送的消息
        self.__add_line('[' + str(self.__nickname) + '(' + str(self.__id) + ')' + ']' + message)
        # 开启子线程用于发送数据
        thread = threading.Thread(target=self.__send_message_thread, args=(message,))
        thread.setDaemon(True)
        thread.start()

    def do_logout(self, args=None):
        """
        登出
        :param args: 参数
        """
        self.__socket.send(json.dumps({
            'type': 'logout',
            'sender_id': self.__id
        }).encode())
        self.__isLogin = False
        self.__disp_text = []
        self.type_text = ""


class GobangInterface(AbstractInterface):
    '''五子棋界面'''
    def __init__(self, _windows, _socket):
        self.__windows = _windows
        self.__socket = _socket
        self.__nickname = None
        self.__id = None
        self.__identity = None
        self.__isLogin = False
        self.__is_my_turn = False
        self.qizi = np.zeros((15,15),dtype=np.int)

        self.__disp_text = []
        self.__textfont = pygame.font.Font("C:\\Windows\\Fonts\\simfang.ttf", 12)

        self.__play_region_rect = pygame.Rect(0, 0, BOARD_WIDTH, BOARD_HEIGHT)
        # 加载 AI 用头像
        self.__ai_img = pygame.transform.scale(pygame.image.load(
            resource_path('./resource/image/ai.jpg')), (100, 100))

        # 创建游戏界面上按钮。
        '''
        self.__restart_button = Button('Restart', BUTTON_COLOR, False,
                                       (BOARD_WIDTH + 30, 130))
        self.__give_up_button = Button('GiveUp', BUTTON_COLOR, True,
                                       (BOARD_WIDTH + 30, BUTTON_HEIGHT + 160))
        '''
        self.__back_button = Button('返回', BUTTON_COLOR, True,
                                    (BOARD_WIDTH + 30, 2 * BUTTON_HEIGHT + 190))

    def draw(self):
        self.__draw_background()
        self.__draw_button(self.__back_button)
        self.__draw_text()
        self.__draw_qizi()

    def check_buttons(self, _mouse_pos):
        if self.__back_button.clicked(_mouse_pos):
            print("back button clicked")
            return ButtonEnum.BACK_BUTTON
        if self.__play_region_rect.collidepoint(_mouse_pos):
            print("play region clicked")
            return ButtonEnum.PLAY_REGION
        return ButtonEnum.NO_BUTTON

    def do_logout(self):
        self.__socket.send(json.dumps({
            'type': 'logout',
            'sender_id': self.__id,
            'identity': self.__identity
        }).encode())
        self.__isLogin = False
        self.__disp_text = []

    
    def make_one_step(self, x, y):
        if self.__is_my_turn:
            # 更新自己的棋子
            self.qizi[x][y] = 1 if self.__identity == "black" else 2
            package = json.dumps({
                    'sender_id': self.__id,
                    'sender_nickname': self.__nickname,
                    'type': 'xiaqi',
                    'identity': self.__identity,
                    'pos': (x,y)
                }).encode()
            self.__is_my_turn = False
            # 开启子线程用于发送数据
            thread = threading.Thread(target=self.__send_message_thread, args=(package,))
            thread.setDaemon(True)
            thread.start()
    
    def __send_message_thread(self, package):
        """
        发送消息线程
        :param message: 消息内容
        """
        self.__socket.send(package)

    def do_login(self):
        """
        登录gobang
        """
        nickname = "tester"

        # 将昵称发送给服务器，获取用户id
        self.__socket.send(json.dumps({
            'type': 'gobang_login',
            'nickname': nickname
        }).encode())
        # 尝试接受数据
        # noinspection PyBroadException
        #try:
        buffer = self.__socket.recv(1024).decode()
        obj = json.loads(buffer)
        if obj['id']:
            self.__nickname = nickname
            self.__id = obj['id']
            self.__identity = obj['identity']
            self.__isLogin = True
            self.__add_line('[Client] 成功登录到聊天室')
            # 如果作为黑方登入，就先手
            if self.__identity == 'black':
                self.__is_my_turn = True
            # 开启子线程用于接受数据
            thread = threading.Thread(target=self.__receive_message_thread)
            thread.setDaemon(True)
            thread.start()
        else:
            self.__add_line('[Client] 无法登录到聊天室')
        #except Exception as ex:
        #    print(ex)
        #    print(ex.__traceback__.tb_lineno) 
        #    print('[Client] 无法从服务器获取数据1')
         #   self.__add_line('[Client] 无法从服务器获取数据1')
    
    def __receive_message_thread(self):
        """
        接受消息线程
        """
        while self.__isLogin:
            # noinspection PyBroadException
            try:
                buffer = self.__socket.recv(1024).decode()
                obj = json.loads(buffer)
                if obj['type'] == 'info':
                    self.__add_line('[' + str(obj['sender_nickname']) + '(' + str(obj['sender_id']) + ')' + ']'+ obj['message'])
                elif obj['type'] == 'xiaqi':
                    if self.__identity != 'audience':
                        self.__is_my_turn = True
                    x , y = obj['pos']
                    self.qizi[x][y] = 1 if obj['identity'] == 'black' else 2
                elif obj['type'] == 'logout':
                    #self.reset()
                    pass
                elif obj['type'] == 'win':
                    self.__add_line('[' + str(obj['sender_nickname']) + '(' + str(obj['sender_id']) + ')' + ']'+ obj['message'])
                    self.reset()
                #print('[' + str(obj['sender_nickname']) + '(' + str(obj['sender_id']) + ')' + ']', obj['message'])
            except Exception as ex:
                print(ex)
                self.__add_line('[Client] 无法从服务器获取数据2')
    
    def __add_line(self,str):
        if self.__isLogin:
            if len(self.__disp_text) < 10:
                self.__disp_text.append(str)
            else:
                self.__disp_text.append(str)
                self.__disp_text = self.__disp_text[1:11]

    def __draw_text(self):
        for i in range(len(self.__disp_text)):
            text_surface = self.__textfont.render(self.__disp_text[i], True, (0,0,0))
            self.__windows.blit(text_surface,(BOARD_WIDTH, BOARD_HEIGHT//2 + 15*i))

    def __draw_background(self):
        """绘制游戏界面背景。"""
        # 绘制棋盘。
        pygame.draw.rect(self.__windows, LIGHT_YELLOW,
                         self.__play_region_rect)
        # 绘制右侧白色背景。
        pygame.draw.rect(self.__windows, WHITE_COLOR,
                         (BOARD_WIDTH, 0, INFO_WIDTH, BOARD_HEIGHT))

        # 绘制棋盘上线。
        for y in range(CHESS_MAX_NUM):
            # 画横线。
            start_pos = REC_SIZE // 2, REC_SIZE // 2 + REC_SIZE * y
            end_pos = BOARD_WIDTH - REC_SIZE // 2, REC_SIZE // 2 + REC_SIZE * y
            if y == CHESS_MAX_NUM // 2:
                width = 2
            else:
                width = 1
            pygame.draw.line(self.__windows, BLACK_COLOR, start_pos,
                             end_pos, width)
        for x in range(CHESS_MAX_NUM):
            # 画竖线。
            start_pos = REC_SIZE // 2 + REC_SIZE * x, REC_SIZE // 2
            end_pos = REC_SIZE // 2 + REC_SIZE * x, BOARD_HEIGHT - REC_SIZE // 2
            if x == BOARD_HEIGHT // 2:
                width = 2
            else:
                width = 1
            pygame.draw.line(self.__windows, BLACK_COLOR, start_pos,
                             end_pos, width)
        
        # 绘制棋盘上方块。
        rec_size = 8
        pos = [(3, 3), (11, 3), (3, 11), (11, 11), (7, 7)]
        for x, y in pos:
            pygame.draw.rect(self.__windows, BLACK_COLOR,
                             (REC_SIZE // 2 + REC_SIZE * x - rec_size // 2,
                              REC_SIZE // 2 + REC_SIZE * y - rec_size // 2,
                              rec_size, rec_size))

    def __draw_qizi(self):
        player_color=[(0,0,0),(255,255,255)]
        radius = REC_SIZE//10*4
        for x in range(15):
            for y in range(15):
                if self.qizi[x][y]:
                    pos = (x*REC_SIZE + REC_SIZE // 2, y*REC_SIZE + REC_SIZE // 2)
                    pygame.draw.circle(self.__windows, player_color[self.qizi[x][y]-1], pos, radius)

    def reset(self):
        self.qizi = np.zeros((15,15),dtype=np.int)

    def __draw_button(self, _button):
        """绘制游戏界面按钮。"""
        self.__windows.fill(_button.color, _button.rect)
        self.__windows.blit(*_button.text_element)