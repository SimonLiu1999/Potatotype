import os
import sys
#from random import randint

from Settings import *

def resource_path(path):
    """获取资源路径。

    对于由 Python 文件直接运行的窗口，资源路径应为当前文件夹路径 + 资源的相对路径。\n
    而对于从打包后的 exe 文件所运行的窗口，资源会被存放到 windows 临时文件夹下，
    所以资源路径为临时文件夹路径 + 资源相对路径。

    Args:
        path: 资源文件所在相对路径。

    Returns:
        拼接后的资源文件绝对路径。
    """
    if getattr(sys, 'frozen', False):
        base_path = sys._MEIPASS
    else:
        base_path = os.path.abspath('.')
    return os.path.join(base_path, path)