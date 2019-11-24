#!/usr/bin/env python3

"""
When the controller is in orbit mode, the orientation of the camera is defined by a Y-axis
rotation followed by an X-axis rotation.

φ :: X axis rotation :: constrained   :: applied first  :: how far from equator
θ :: Y axis rotation :: unconstrained :: applied second :: how far from prime meridian

If home_vector is (0,0,1) then eyepos works like this:

x = sin(θ) * cos(φ)
y = sin(φ)          // the "axis" of planet is the Y axis
z = cos(θ) * cos(φ) // because we want Z=1 when θ=φ=0

θ = atan2(x,z)
φ = asin(y)

Theta is a CCW angle seen from above (i.e. 0,1,0) starting from (0,0,1)
Phi is a CCW angle seen from the left (i.e. -1,0,0) starting from (0,0,1)

"""

import math

def get_xyz(theta, phi):
    x = math.sin(theta) * math.cos(phi)
    y = math.sin(phi)
    z = math.cos(theta) * math.cos(phi)
    print(f'theta={theta:+3.2f}, phi={phi:+3.2f} :: {x:+3.2f}, {y:+3.2f}, {z:+3.2f}')
    return x, y, z

def get_angles(x, y, z):
    theta = math.atan2(x, z)
    phi = math.asin(y)
    print(f'theta={theta:+3.2f}, phi={phi:+3.2f} :: {x:+3.2f}, {y:+3.2f}, {z:+3.2f}')
    return theta, phi

xyz = get_xyz(theta=0.0, phi=0.0)
get_angles(*xyz)

print()

xyz = get_xyz(theta=math.pi, phi=0.0)
get_angles(*xyz)

print()

xyz = get_xyz(theta=math.pi, phi=math.pi/4)
get_angles(*xyz)

print()

xyz = get_xyz(theta=math.pi/2, phi=math.pi/4)
get_angles(*xyz)

print()

xyz = get_xyz(theta=3*math.pi/2, phi=math.pi/4)
get_angles(*xyz)
