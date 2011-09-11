#!/usr/bin/python
# coding: utf-8

import re, sys, os

from PIL import Image, ImageDraw, ImageFont
FONT_NAME = "/usr/share/fonts/truetype/ttf-dejavu/DejaVuSans.ttf" ##"LucidaSansRegular.ttf"

CMDS = ['LOAD', 'CPU', 'BLOCK', 'UNBLOCK', 'EXIT']
LOAD, CPU, BLOCK, UNBLOCK, EXIT = range(5)

COLOR_GRAY = ('#a0a0a0', '#d0d0d0')
COLOR_BLACK = '#000000'
COLORS = [
	('#c0ffc0', '#e7ffe7'), # Verde claro
	('#0000ff', '#b7b7f7'), # Azul
	('#ff0000', '#f7b7b7'), # Rojo
	('#d0d0d0', '#f4f4f4'), # Gris
	('#00e000', '#b7efb7'), # Verde
	('#ffff00', '#f7f7b7'), # Amarillo
	('#c0c0ff', '#e7e7ff'), # Azul claro
]

STATES = ['READY', 'BLOCKED', 'RUNNING', 'UNLOADED']
READY, BLOCKED, RUNNING, UNLOAD, RUNNINGBLOCKED = range(5)

def iround(m):
	"""Rounds the number to the nearest number with only one significative digit."""
	n = 1
	while (m + n/2) / n >= 10:
		n*= 10
	while (m + n/2) / n >= 5:
		n*= 5
	rm = m - (m/n)*n
	return ((m+rm)/n)*n

def parseData(fin):
	ln = 0
	res = []
	for l in fin:
		ln += 1
		vls = l.split()
		if l and l[0] == '#':
			continue
		if len(vls) != 3 or vls[0] not in CMDS:
			sys.stderr.write('Warning: Ignoring line %d: %s' % (ln, l))
			continue
		res.append((CMDS.index(vls[0]), int(vls[1]), int(vls[2])))
	return res

def dataGantt(data):
	g = dict()
	for cmd, tm, pid in data:
		if pid not in g: g[pid] = []
		g[pid].append((tm,cmd))
	return g

def drawGantt(data, fout, rg=3, width=1024, height=600):
	xmarks = 10
	subxmarks = 5

	n = len(data)
	xmin = min(min(tm for tm,_ in data[pid]) for pid in data)
	xmax = max(max(tm for tm,_ in data[pid]) for pid in data)+1

	# Inicializa Grafico
	xstp = iround((xmax+xmarks-1)/xmarks)
	xgap = xstp*xmarks
	ystp = 30
	if ystp*n > height: ystp = height/n
	
	gw,gh = width-60,n*ystp
	gx,gy = 5+20+ 30 , 5 + 30 + 5 + gh
	iw,ih = gx+gw+8, gy+5+17+5

	# Nueva IMG
	img = Image.new("RGB", (iw,ih), (255,255,255))
	draw = ImageDraw.Draw(img)
	font = ImageFont.truetype(FONT_NAME, 12)
	
	# Marcas en Y:
	pids = sorted(pid for pid in data if pid >= 0)
	if -1 in data: pids.append(-1)
	pids.reverse()
	for i in xrange(len(pids)):
		ly = gy - i*ystp - ystp/2
		draw.line((gx-2,ly,gx-4,ly), fill=COLOR_BLACK)
		txt = str(pids[i] if pids[i] >= 0 else 'IDLE')
		tw,th = draw.textsize(txt, font=font)
		draw.text((gx-2 -5 -tw, ly - th/2), txt, font=font, fill=COLOR_BLACK)
	
	# Marcas en X:
	for i in xrange(2*subxmarks*xmarks):
		x = xmin - xmin%xstp + xstp * i / subxmarks
		if x < xmin: continue
		if x > xmax: break
		lx = gx + x*gw / xmax
		draw.line((lx,gy+1,lx,gy+3), fill=COLOR_BLACK if i%subxmarks==0 else COLOR_GRAY[0])
		if x > xmin: draw.line((lx,gy-1,lx,gy-gh), fill=COLOR_BLACK if i%subxmarks==0 else COLOR_GRAY[0])
		if i%subxmarks==0:
			txt = str(x)
			tw,th = draw.textsize(txt, font=font)
			draw.text((lx-tw/2, gy+3), txt, font=font, fill=COLOR_BLACK)
	
	# Ejes
	draw.line((gx,gy,gx+gw,gy), fill=COLOR_BLACK)
	draw.line((gx-1,gy,gx-1,gy-gh), fill=COLOR_BLACK)
	
	# Leyenda
	lx,ly = gx,gy-gh-25
	bzs = 15
	for i in xrange(len(STATES)):
		draw.rectangle((lx,ly,lx+bzs,ly+bzs), fill=COLORS[i][0])
		draw.rectangle((lx+1,ly+1,lx+bzs-1,ly+bzs-1), fill=COLORS[i][1])
		lx += bzs + 3
		vl = STATES[i]
		tw,th = draw.textsize(vl, font=font)
		draw.text((lx, ly+(bzs-th)/2), vl, font=font, fill=COLOR_BLACK)
		lx += tw + 10
		
	
	# Grafico
	for i in xrange(len(pids)):
		sts = [UNLOAD for j in xrange(xmax+2)]
		p = 0
		l = data[pids[i]]
		blk = False
		ldd = False
		for j in xrange(xmax+1):
			cpu = False
			while p < len(l) and l[p][0] <= j:
				ev = l[p][1] 
				if ev == BLOCK: blk = True
				elif ev == UNBLOCK: blk = False
				elif ev == LOAD: ldd = True
				elif ev == EXIT: ldd = False
				elif ev == CPU: cpu = True
				p += 1
			if not ldd and pids[i] != -1: st = UNLOAD
			elif cpu and blk: st = RUNNINGBLOCKED
			elif cpu and not blk: st = RUNNING
			elif not cpu and blk: st = BLOCKED
			else: st = READY
			sts[j] = st
		
		ly = gy - i*ystp - ystp/2
		uy = ly-ystp/3
		by = ly+ystp/3
		for j in xrange(xmax):
			x = gx + j*gw / xmax
			nx = gx + (j+1)*gw / xmax
			st = sts[j]
			extend = (j>0) and (sts[j-1] == sts[j])
			if st != UNLOAD or True:
				if st == RUNNINGBLOCKED:
					extend = (j>0) and (sts[j-1] == BLOCKED or sts[j-1] == sts[j])
					draw.rectangle((x,uy-1,nx-1,ly), fill=COLORS[BLOCKED][0])
					draw.rectangle((x+1-(2 if extend else 0),uy,nx-2,ly-1), fill=COLORS[BLOCKED][1])
					extend = (j>0) and (sts[j-1] == RUNNING or sts[j-1] == sts[j])
					draw.rectangle((x,ly+1,nx-1,by+1), fill=COLORS[RUNNING][0])
					draw.rectangle((x+1-(2 if extend else 0),ly+2,nx-2,by), fill=COLORS[RUNNING][1])
				else:
					draw.rectangle((x,uy-1,nx-1,by+1), fill=COLORS[st][0])
					draw.rectangle((x+1-(2 if extend else 0),uy,nx-2,by), fill=COLORS[st][1])
				if st == RUNNING and j>0 and sts[j-1] == RUNNINGBLOCKED:
					draw.rectangle((x+1-2,ly+2,nx-2,by), fill=COLORS[st][1])
				if st == BLOCKED and j>0 and sts[j-1] == RUNNINGBLOCKED:
					draw.rectangle((x+1-2,uy,nx-2,ly-1), fill=COLORS[st][1])
					

	
	if isinstance(fout, str):
		dr = os.path.dirname(fout)
		if dr != "" and not os.path.isdir(dr):
			os.makedirs(dr, 0755)
	
	img.save(fout, "PNG")
	#img.resize(((png_box+1)*n-1, png_box), Image.ANTIALIAS).save(pngfn, "PNG")


def main(argv):
	if len(argv) <= 1:
		fin = sys.stdin
		fout = sys.stdout
	else:
		fin = open(argv[1], 'r')
		fout = argv[1]+'.png'
	
	data = parseData(fin)
	gantt = dataGantt(data)
	#for x in gantt: print gantt[x]
	drawGantt(gantt, fout)

if __name__ == "__main__":
	main(sys.argv)
