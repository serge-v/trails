#!/usr/bin/python

import math, os

c = [
('SW',  1036,    7532,    563000,    4553000),
('SE',  5958,    7579,    573000,    4553000),

('NW',  1096,    1135,    563000,    4566000),
('NE',  6017,    1180,    573000,    4566000)
]

size = (7200, 8700)

crop = [
    (568000, 4561000),
    (573000, 4553000)
]

utm_dx = c[1][3] - c[0][3]
utm_dy = c[1][4] - c[0][4]

px_dx = c[1][1] - c[0][1]
px_dy = c[1][2] - c[0][2]

utm_d = math.sqrt(utm_dx * utm_dx + utm_dy * utm_dy)
px_d = math.sqrt(px_dx * px_dx + px_dy * px_dy)

print utm_dx, utm_dy, utm_d
print px_dx, px_dy, px_d
mppx = utm_d / px_d
print mppx

start_utm_x = 563000 - 1036 * mppx
start_utm_y = 4566000 + 1135 * mppx
print 'start:', start_utm_x, start_utm_y

crop_x = (568000 - start_utm_x) / mppx
crop_y = (start_utm_y - 4561000) / mppx

print crop_x, crop_y

cmd = 'convert o41074b2-2013.png -crop 3000x3000+%d+%d 2.png' % (crop_x, crop_y)
print cmd

rc = os.system(cmd)
print rc
