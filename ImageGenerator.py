import numpy
"""
	NAME : code_to_music.c
	AUTHOR : HUGO LECA
	LAST MODIFICATION : 16/05/2021

"""

from PIL import Image

data = numpy.zeros((1024, 1024, 3), dtype=numpy.uint8)
#tab_music = numpy.array([1, 1, 0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1])
#tab_music = numpy.array([1,     0, 1 , 1, 1, 1     , 1, 1,     0, 1, 1, 1, 1,        0, 1, 1])
#tab_music = numpy.array([1,     1, 0 , 0, 0, 0     , 1, 0,     0, 1, 0, 1, 1,        0, 1, 1])
tab_music = numpy.array( [1,     0, 1 , 0, 0, 0     , 0, 0,     0, 0, 0, 0, 0,        0, 0, 1])
#tab_music = numpy.array([1,     0, 0 , 0, 0, 0     , 0, 0,     0, 0, 0, 0, 0,        0, 0, 1])


#constante globales
BIT_WIDTH = 12

#algo remplissage
for i in range (1024):
	for j in range (len(tab_music)):
		if tab_music[j] == 1:
			data[i, j*BIT_WIDTH + 250 : j*BIT_WIDTH + (BIT_WIDTH) + 250] = [0, 0, 0]
		if tab_music[j] == 0:
			data[i, j*BIT_WIDTH + 250 : j*BIT_WIDTH + (BIT_WIDTH) + 250] = [255, 255, 255]
		data[i, (len(tab_music))*BIT_WIDTH + 251 : 1024] = [255, 255, 255]
		data[i, 0:249] = [255, 255, 255]

		


image = Image.fromarray(data)

image.save("Image3.jpeg")
image.show()

