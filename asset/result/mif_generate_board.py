from PIL import Image, ImageFilter

filename = "../board_new"
depth = "120000"

img = Image.open(filename+".png")

# modify opacity value
width, height = img.size
if img.mode == "RGBA":
	for y in range(height):
		for x in range(width):
			# Get the RGB values of the pixel at position (x, y)
			pix = img.getpixel((x, y))
			if pix[3] < 50:
				pix = (0,0,0,0)
			else:
				pix = (pix[0],pix[1],pix[2],255)
			img.putpixel((x,y),pix)

# quantized_image = img.quantize(colors=15,kmeans=3)
# quantized_image.show()
# quantized_image = img.quantize(colors=15,method=None)
# # convert colors
# if quantized_image.mode == "P":
#     # Get the palette
#     palette = quantized_image.getpalette()
#     # Determine the number of colors in the palette
#     num_colors = len(palette) // 3
    
#     # Convert palette values to RGB values
#     rgb_palette = [palette[i*3:i*3+3] for i in range(num_colors)]
    
#     print("RGB Palette:", rgb_palette)
#     with open(filename+".plt",'w') as file:
#         for idx,color in enumerate(rgb_palette):
#             file.write("8'h%02x : {VGA_R, VGA_G, VGA_B} = 24'h%02x%02x%02x;\n"%(idx,color[0],color[1],color[2]))
# else:
#     print("Image does not have a palette.")
    
# quantized_image.save(filename+"_quantized.png")
# b1 = 0
# with open(filename+".mif","w") as file:
# 	file.write('''WIDTH = 8;
# DEPTH = %s;
# ADDRESS_RADIX = HEX;
# DATA_RADIX = HEX;
# CONTENT BEGIN\n\n'''%depth)
# 	for idx,byte in enumerate(quantized_image.getdata()):
# 			if idx%2==0:
# 				b1 = byte<<4
# 			else:
# 				file.write("%x : %x;\n"%(int(idx/2),b1|byte))
# 	file.write("\nEND;")

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
		file.write("4'h%01x : {VGA_R, VGA_G, VGA_B} = 24'h%02x%02x%02x;\n"%(idx,color[0],color[1],color[2]))

b1 = 0
with open(filename+".mif","w") as file:
	file.write('''WIDTH = 8;
DEPTH = %s;
ADDRESS_RADIX = HEX;
DATA_RADIX = HEX;
CONTENT BEGIN\n\n'''%depth)
	for idx,byte in enumerate(vals):
		if idx%2==0:
			b1 = byte<<4
		else:
			file.write("%x : %x;\n"%(int(idx/2),b1|byte))
	file.write("\nEND;")