#!/usr/bin/python

import math, os

def utm(p):
    f = os.popen('echo %f %f | cs2cs +proj=latlong +datum=NAD83 +to +proj=utm +datum=NAD83 +zone=18' % (p[0], p[1]))
    pair = f.read().strip().split('\t')
    x = pair[0]
    y = pair[1].split(' ')[0]
    f.close()
    return map(float, (x, y))

b1 = {

'NW': ((1025, 750), (-74.250, 41.375)),
'NE': ((6170, 750), (-74.125, 41.375)),

'SW': ((1020, 7580), (-74.250, 41.250)),
'SE': ((6175, 7580), (-74.125, 41.250)),

}

print 'NE:', utm(b1['NE'][1])

size = (7200, 8700)

def utm_dist(utm1, utm2):
    p1 = utm(utm1[1])
    p2 = utm(utm2[1])

    dx = p1[0] - p2[0]
    dy = p1[1] - p2[1]
    
    d = math.sqrt(dx * dx + dy * dy)
    return d

def px_dist(px1, px2):
    p1 = px1[0]
    p2 = px2[0]

    dx = p1[0] - p2[0]
    dy = p1[1] - p2[1]
    
    d = math.sqrt(dx * dx + dy * dy)
    return d

utm_d =  utm_dist(b1['NE'], b1['NW'])
px_d = px_dist(b1['NE'], b1['NW'])

print utm_d
print px_d
print utm_d / px_d
