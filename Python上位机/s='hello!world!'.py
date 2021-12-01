from PIL import Image
import numpy as np
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
a=bytearray()
for i in range(0,len(data)//8):
    b=0
    for j in range(0,8):
        b=b<<1
        b+=data[i*8+j]
    a.append(b)
c=bytearray([0,1,2])
c.extend(a)
print(data.shape)
img.show()