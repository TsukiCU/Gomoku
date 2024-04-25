from PIL import Image
strs=['''
 █████  
██   ██ 
███████ 
██   ██ 
██   ██ 
''',
'''
██████  
██   ██ 
██████  
██   ██ 
██████  
''',
'''
 ██████ 
██      
██      
██      
 ██████ 
''',
'''
██████  
██   ██ 
██   ██ 
██   ██ 
██████  
''',
'''
███████ 
██      
█████   
██      
███████ 
''',
'''
███████ 
██      
█████   
██      
██      
''',
'''
 █████  
██      
██  ███ 
██   ██ 
 █████  
''',
'''
██   ██ 
██   ██ 
███████ 
██   ██ 
██   ██ 
''',
'''
███████ 
  ███   
  ███   
  ███   
███████ 
''',
'''
     ██ 
     ██ 
     ██ 
██   ██ 
 █████  
''',
'''
██   ██ 
██  ██  
█████   
██  ██  
██   ██ 
''',
'''
██      
██      
██      
██      
███████ 
''',
'''
██   ██ 
███ ███ 
██ █ ██ 
██   ██ 
██   ██ 
''',
'''
██   ██ 
███  ██ 
██ █ ██ 
██  ███ 
██   ██ 
''',
'''
 █████  
██   ██ 
██   ██ 
██   ██ 
 █████  
''',
'''
██████  
██   ██ 
██████  
██      
██      
''',
'''
 █████  
██   ██ 
██   ██ 
 ██████ 
    ██  
''',
'''
██████  
██   ██ 
██████  
██   ██ 
██   ██ 
''',
'''
███████ 
██      
███████ 
     ██ 
███████ 
''',
'''
███████ 
  ███   
  ███   
  ███   
  ███   
''',
'''
██   ██ 
██   ██ 
██   ██ 
██   ██ 
 █████  
''',
'''
██   ██ 
██   ██ 
██   ██ 
 ██ ██  
  ███   
''',
'''
██   ██ 
██   ██ 
██   ██ 
██ █ ██ 
 ██ ██  
''',
'''
██   ██ 
 ██ ██  
  ███   
 ██ ██  
██   ██ 
''',
'''
██  ██  
██  ██  
 ████   
  ██    
  ██    
''',
'''
███████ 
   ███  
  ███   
 ███    
███████ 
''',
'''
 █████  
██  ███ 
██ █ ██ 
███  ██ 
 █████  
''',
'''
  ██    
████    
  ██    
  ██    
██████  
''',
'''
██████  
     ██ 
 █████  
██      
███████ 
''',
'''
██████  
     ██ 
 █████  
     ██ 
██████  
''',
'''
██   ██ 
██   ██ 
███████ 
     ██ 
     ██ 
''',
'''
███████ 
██      
███████ 
     ██ 
███████ 
''',
'''
 █████  
██      
██████  
██   ██ 
 █████  
''',
'''
███████ 
     ██ 
    ██  
   ██   
   ██   
''',
'''
 █████  
██   ██ 
 █████  
██   ██ 
 █████  
''',
'''
 █████  
██   ██ 
 ██████ 
     ██ 
 █████  
''',
'''
        
        
        
        
        
''',
'''
  ██    
  ██    
  ██    
        
  ██    
''',
'''
█████   
    ██  
  ██    
        
  ██    
''',
'''
        
        
        
██      
 █      
''',
'''
        
        
        
        
██      
''',
'''
        
  ██    
██████  
  ██    
        
''',
'''
        
        
█████   
        
        
''',
'''
        
 ██  ██ 
   ██   
 ██  ██ 
        
''',
'''
    ██  
   ██   
  ██    
 ██     
██      
'''
]
val = ['a','b','c','d','e','f','g','h','i','j','k','l','m','n',
       'o','p','q','r','s','t','u','v','w','x','y','z','0','1',
       '2','3','4','5','6','7','8','9',' ','!','?',',','.','+','-','*','/']
imgs = []
seg_width = 8
seg_height = 16
arrs=[]
for str,val in zip(strs,val):
	arr=[]
	wid=0
	for c in str:
		if c==' ':
			arr.append(0)
		elif c=='█':
			arr.append(1)
		elif c=='\n' and wid==0:
			print(val+" width %d"%len(arr))
			wid=len(arr)

	img = Image.new(size=(8*seg_width,5*seg_height),mode="RGB")
	for y in range(0,5):
		for yy in range(0,seg_height):
			for x in range(0,8):
				for xx in range(0,seg_width):
					if arr[y*8+x]==0:
						pix=(255,255,255)
					else:
						pix=(0,0,0)
					img.putpixel((x*seg_width+xx,y*seg_height+yy),pix)
	if val==' ':
		val="space"
	elif val=='!':
		val="excl"
	elif val=='?':
		val="ques"
	elif val==',':
		val='comm'
	elif val=='.':
		val="dot"
	elif val=='+':
		val="plus"
	elif val=='-':
		val="min"
	elif val=='/':
		val="div"
	elif val=='*':
		val="time"
	img.save(val+".png")
	imgs.append(img)
	arrs+=arr

sentence = 'HELLO, WORLD! KLMNOPQRSTUVWXYZ 1234567890 !?.+-3*5/'
length = len(sentence)
img_sentence = Image.new(mode="RGB",size=(8*seg_width*length,5*seg_height))
for idx,c in enumerate(sentence):
	i = ord(c)-ord('A')
	if ord(c) >= ord('0') and ord(c) <= ord('9'):
		i=ord(c)-ord('0')+26
	if c==' ':
		i=36
	elif c=='!':
		i=37
	elif c=='?':
		i=38
	elif c==',':
		i=39
	elif c=='.':
		i=40
	elif c=='+':
		i=41
	elif c=='-':
		i=42
	elif c=='*':
		i=43
	elif c=='/':
		i=44
	img_sentence.paste(imgs[i],(idx*8*seg_width,0,(idx+1)*8*seg_width,5*seg_height))
img_sentence.show()

b=0
fb = open("font.bin","wb")
with open("font.mif","w") as file:
	file.write('''WIDTH = 8;
DEPTH = %s;
ADDRESS_RADIX = HEX;
DATA_RADIX = HEX;
CONTENT BEGIN\n\n'''%(len(arrs)//8))
	for idx,byte in enumerate(arrs):
		b|=(byte<<(idx%8))
		if idx%8==7:
			file.write("%x : %x;\n"%(int(idx/8),b))
			fb.write(bytearray([b]))
			b=0
	file.write("\nEND;")
fb.close()