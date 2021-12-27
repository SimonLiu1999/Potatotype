from enum import IntEnum
from enum import Enum

class CommandEnum(IntEnum):
    TASK_SCREEN = 1
    TASK_CALCULATION = 2
    ACK = 3
    PRINT = 4
    LOGIN = 5
    BROADCAST = 6
    LOGOUT = 7

class EventEnum(IntEnum):
    network_layer_ready = 1
    frame_arrival = 2
    timeout = 3
    ack_timeout = 4

class frame_kind(IntEnum):
    data = 0
    ack = 1

class Status(IntEnum):
	WAITHEAD1=0
	WAITHEAD2=1
	WAITHEAD3=2
	GETLEN1=3
	GETLEN2=4
	LENCHECK=5
	READDATA=6
	SUMCHECK=7

class Head():
    HEAD1 = 0x11
    HEAD2 = 0x22
    HEAD3 = 0x33