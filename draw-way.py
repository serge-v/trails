#!/usr/bin/python

import os, sys

A = 2.03160181179
D = 0.017
B = 0.02
E = -2.03160181179
C = 568000.0
F = 4561000.0

def latlon_to_utm(p):
    f = os.popen('echo %f %f | cs2cs +proj=latlong +datum=NAD83 +to +proj=utm +datum=NAD83 +zone=18' % (p[0], p[1]))
    pair = f.read().strip().split('\t')
    x = pair[0]
    y = pair[1].split(' ')[0]
    f.close()
    return map(float, (x, y))

def utm_to_px(p):
    x = (E * p[0] - B * p[1] + B * F - E * C) / (A * E - D * B)
    y = (-D * p[0] + A * p[1] + D * C - A * F) / (A * E - D * B)
    return (x, y)

lines = "-stroke red -draw ' \n"

for i in range(0, 6):
    for j in range(0, 9):
        pt = (568000 + i * 1000, 4562000 - j * 1000)
        (x, y) = utm_to_px(pt)
        lines += 'line %d,%d %d,%d line %d,%d %d,%d \n' % (x-20, y, x+20, y, x, y-20, x, y+20)

pt = latlon_to_utm((-74.125, 41.125))
print 'pt:', pt
sys.exit(1)
(x, y) = utm_to_px(pt)
lines += 'line %d,%d %d,%d line %d,%d %d,%d \n' % (x-20, y, x+20, y, x, y-20, x, y+20)

school = latlon_to_utm((-74.1864, 41.165))
(x, y) = utm_to_px(school)
lines += 'line %d,%d %d,%d line %d,%d %d,%d \n' % (x-20, y, x+20, y, x, y-20, x, y+20)
lines += "' "

#annotations = ''
#annotations += "-annotate +%d+%d '%s' " % (last_x, last_y, text)

cmd = "convert 2.png -fill none -strokewidth 4 %s 3.png" % (lines)

print cmd
#sys.exit(1)
rc = os.system(cmd)
print rc
if rc != 0:
    sys.exit(rc)
#rc = os.system('convert out.png -crop 2500x1800+2500+3400 1.png')
#if rc != 0:
#    sys.exit(rc)

#convert o41074b2-2013.png -crop 3000x4200+3534+3620 2.png
