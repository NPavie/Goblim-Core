import os,sys
from PIL import Image


cropping_request = False
renaming_request = False

if len(sys.argv) < 3:
	print ("Usage : " + sys.argv[0] + " <folderIn> <folderOut> [<resultsPrefix> <left> <up> <width> <height>]")
	sys.exit(1)
else:
	if len(sys.argv) > 3 :
		renaming_request = True
		print("png files will be prefixed with " + sys.argv[3])
	if len(sys.argv) > 4 :
		cropping_request = True
		if len(sys.argv) < 8 :
			print("Cropping request failed, treatment wil continue without cropping")
			print ("Usage : " + sys.argv[0] + " <folderIn> <folderOut> [<resultsPrefix> <left> <up> <width> <height>]")
			cropping_request = False
		else :
			print("Cropping images from coord x:" + sys.argv[4] + " and y:" + sys.argv[5] + \
				" with dimension w:" + sys.argv[6] + " and h:" + sys.argv[7]  )

	for fileName in os.listdir(sys.argv[1]):
		extension = fileName.split('.',1)[-1]
		fileLastName = fileName.split('/',1)[-1]
		if "png" in extension or "jpg" in extension or "ppm" in extension or "pgm" in extension:
			imageFile = Image.open(sys.argv[1]+"/"+fileName)
			newImage = imageFile

			if cropping_request is True :		
				box = (int(sys.argv[4]), int(sys.argv[5]), int(sys.argv[4]) + int(sys.argv[6]), int(sys.argv[5]) + int(sys.argv[7]))
				newImage = imageFile.crop(box)
			
			# Vertical mirror image
			pixels = newImage.load()
			height = newImage.size[1]
			for y in range(height//2) :
				for x in range(newImage.size[0]):
					temp = pixels[x,y]
					pixels[x,y] = pixels[x, height-1-y]
					pixels[x, height-1-y] = temp
					
			outputFileName = sys.argv[2]
			if outputFileName[-1] != '/' and  outputFileName[-1] != '\\':
				outputFileName = outputFileName + '/'
			fileLastNameNoExt = fileLastName.split('.',1)[0]
			if renaming_request is True :
				outputFileName = outputFileName + sys.argv[3] + fileLastNameNoExt + ".png"
			else :
				outputFileName = outputFileName + fileLastNameNoExt + ".png"
			print("Saving to " + sys.argv[2] + "" )
			newImage.save(outputFileName)