# 土豆服务器 Potatotype
这是一个用土豆电池驱动的服务器
This is a server powered by potato battery



代码分为三部分

The code is divided into three parts



client_python_pc 是用python写的跑在pc上的客户端，用户可以通过这个程序登录土豆服务器。

client_python_pc is a client program written in python that runs on the PC, and users can log in to the potato server through this program.



gateway_python_pc是用python写的跑在pc上的网关，用于将tcp连接转换为土豆服务器的接口。

gateway_python_pc is a gateway program  written in python that runs on the pc and is used to convert the tcp connection to the interface of the potato server.



Embedded End是c语言写的跑在土豆服务器上的服务器端程序，烧写在stm32中。

Embedded End is a server-side program written in C and running on the potato server, it is burned in stm32.



本人代码水平不高，大部分代码都是边学边写，因此存在诸多不规范和已知bug，在寒假期间还会逐步完善。

