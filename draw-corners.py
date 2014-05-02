#!/usr/bin/python

import os, sys

file = 'o41074b2.png'

west = -74.250000
east = -74.125000  
north = 41.250000 
south = 41.125000

A = 2.43840000
D = 0.00000000
B = 0.00000000
E = -2.43840000
C = 562074.74737547
F = 4567438.17703877

def latlon_to_utm(p):
    f = os.popen('echo %f %f | cs2cs +proj=latlong +ellps=clrk66 +to +proj=utm +ellps=clrk66 +zone=18' % (p[0], p[1]))
    pair = f.read().strip().split('\t')
    x = pair[0]
    y = pair[1].split(' ')[0]
    f.close()
    return map(float, (x, y))

NW = latlon_to_utm((west, north))
NE = latlon_to_utm((east, north))
SW = latlon_to_utm((west, south))
SE = latlon_to_utm((east, south))
for a in NW, NE, SW, SE:
    print a
    
def utm_to_px(p):
    x = (E * p[0] - B * p[1] + B * F - E * C) / (A * E - D * B)
    y = (-D * p[0] + A * p[1] + D * C - A * F) / (A * E - D * B)
    return (x, y)

lines = "-stroke red -draw ' \n"
(x, y) = utm_to_px(NW)
crop_x = x
crop_y = y
lines += '    line %d,%d %d,%d line %d,%d %d,%d \n' % (x-20, y, x+20, y, x, y-20, x, y+20)
(x, y) = utm_to_px(NE)
lines += '    line %d,%d %d,%d line %d,%d %d,%d \n' % (x-20, y, x+20, y, x, y-20, x, y+20)
(x, y) = utm_to_px(SW)
lines += '    line %d,%d %d,%d line %d,%d %d,%d \n' % (x-20, y, x+20, y, x, y-20, x, y+20)
(x, y) = utm_to_px(SE)
lines += '    line %d,%d %d,%d line %d,%d %d,%d \n' % (x-20, y, x+20, y, x, y-20, x, y+20)
crop_w = x - crop_x
crop_h = y - crop_y

school = latlon_to_utm((-74.1864, 41.165))
(x, y) = utm_to_px(school)
lines += '    line %d,%d %d,%d line %d,%d %d,%d \n' % (x-20, y, x+20, y, x, y-20, x, y+20)
lines += "' "

#cmd = "convert %s -fill none -strokewidth 4 %s out.png" % (file, lines)
#print cmd

#rc = os.system(cmd)
#if rc != 0:
#    sys.exit(rc)
#rc = os.system('convert out.png -crop 2500x1800+2500+3400 1.png')
#if rc != 0:
#    sys.exit(rc)

cmd = "convert b2.png -crop %dx%d+%d+%d b2c.png" % (crop_w, crop_h, crop_x, crop_y)
rc = os.system(cmd)
if rc != 0:
    sys.exit(rc)
