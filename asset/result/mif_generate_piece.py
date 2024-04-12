from PIL import Image

filename = "black"
depth = "841"

img = Image.open(filename+".png")

# modify opacity value
width, height = img.size
colors = []
vals = []
for y in range(height):
	for x in range(width):
		# Get the RGB values of the pixel at position (x, y)
		pix = img.getpixel((x, y))
		if pix[3] < 50:
			pix = (0,0,0,0)
		else:
			pix = (pix[0],pix[1],pix[2],255)
		img.putpixel((x,y),pix)
		if pix not in colors:
			colors.append(pix)
		vals.append(colors.index(pix))
for color in colors:
	#print("%x %x %x"%(color))
	print(color)
print(len(colors))

with open(filename+".plt",'w') as file:
	for idx,color in enumerate(colors):
		file.write("8'h%02x : {VGA_R, VGA_G, VGA_B} = 24'h%02x%02x%02x;\n"%(idx,color[0],color[1],color[2]))

with open(filename+".mif","w") as file:
	file.write('''WIDTH = 8;
DEPTH = %s;
ADDRESS_RADIX = HEX;
DATA_RADIX = HEX;
CONTENT BEGIN\n\n'''%depth)
	for idx,byte in enumerate(vals):
		file.write("%x : %x;\n"%(idx,byte))
	file.write("\nEND;")

img.save(filename+"_modified.png")