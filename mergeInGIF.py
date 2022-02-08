import os,sys
from PIL import Image, ImageSequence
from images2gif import writeGif

filesize = (256,256)

if len(sys.argv) < 3:
	print ("Usage : " + sys.argv[0] + " <folderIn> <output>")
	sys.exit(1)
else:
	frameList = []
	for fileName in os.listdir(sys.argv[1]):
		
		extension = fileName.split('.',1)[-1]
		fileLastName = fileName.split('/',1)[-1]
		if "png" in extension or "jpg" in extension or "ppm" in extension or "pgm" in extension:
			current = Image.open(sys.argv[1]+"/"+fileName)
			filesize = current.size	
			frameList.append(Image.open(sys.argv[1]+"/"+fileName))

	print( "test : " + sys.argv[2] + " "  + str(filesize) + " " + str(len(frameList)) )
	temp = Image.new("RGB",filesize)
	temp.save(sys.argv[2])
	writeGif(sys.argv[2],frameList,duration=0.75, dither=0)


