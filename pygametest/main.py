import sys
import pygame
from Potatotype import Potatotype

def main():
    """Potatotype主入口。
    初始化并启动。

    """
    pygame.init()
    potatotype = Potatotype()
    while True:
        potatotype.run()

    sys.exit(0) 

if __name__ == '__main__':
    main()