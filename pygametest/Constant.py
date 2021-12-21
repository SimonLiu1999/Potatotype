from enum import IntEnum

class ButtonEnum(IntEnum):
    """按钮枚举类。

    为每种按钮编号，无按钮编号为 -1.
    """
    NO_BUTTON = -1,
    START_BUTTON = 1,
    MODULE_BUTTON = 2,
    EXIT_BUTTON = 3,
    RESTART_BUTTON = 4,
    GIVE_UP_BUTTON = 5,
    BACK_BUTTON = 6
    CONNECT_BUTTON = 7
    TEXT_REGION = 8
    CHAT_BUTTON = 9
    GOBANG_BUTTON = 10 
    SEND_BUTTON = 11
    CLEAN_BUTTON = 12
    PLAY_REGION = 13