#!/usr/bin/env python3

import math
import snowy
import numpy as np
from scipy import interpolate

width, height = 6000, 2000
warpx, warpy = height / 10, height / 10
falloff_stretch = (0.5, 1.1)


def generate_fBm(width, height, freq, layers, wrapx=False, wrapy=False):
    "Generate 2D fractional brownian motion"
    noise = snowy.generate_noise
    n = np.zeros([height, width, 1])
    ampl = 1
    for f in range(layers):
        seed = int(f)
        n += ampl * noise(width, height, freq, seed, wrapx, wrapy)
        freq *= 2
        ampl /= 2
    return n


print("Creating a stretched falloff function.")
w, h = falloff_stretch
x, y = np.linspace(-w, w, width), np.linspace(-h, h, height)
u, v = np.meshgrid(x, y, sparse=True)
falloff = np.clip(1 - (u * u + v * v), 0, 1)
falloff = snowy.reshape(falloff / 2)

print("Adding layers of gradient noise and scale with falloff.")
noise = generate_fBm(width, height, 6, 4)
island_noise = falloff * (falloff / 2 + noise)

print("Applying a warping operation to create the landmass mask.")

i, j = np.arange(width, dtype="i2"), np.arange(height, dtype="i2")
coords = np.dstack(np.meshgrid(i, j, sparse=False))

warpx = warpx * np.cos(noise * math.pi * 2)
warpy = warpy * np.sin(noise * math.pi * 2)
coords += np.int16(np.dstack([warpx, warpy]))

warped = snowy.dereference_coords(island_noise, coords)

mask = warped < 0.1

print("Computing the distance field.")
elevation = snowy.generate_sdf(mask)
elevation /= np.amax(elevation)

print("Quantizing.")
bins = np.arange(-1.0, 1.0, 0.2)
elevation = -1.0 + 2.0 * np.float64(np.digitize(elevation, bins)) / len(bins)
elevation = np.where(elevation < 0, 0, elevation)
snowy.save(elevation, "elevation.exr")

print("Computing ambient occlusion.")
occlusion = snowy.compute_skylight(elevation)
occlusion = 0.25 + 0.75 * occlusion

print("Generating normal map.")
normals = snowy.resize(snowy.compute_normals(elevation), width, height)

print("Applying diffuse lighting.")
lightdir = np.float64([0.5, -0.5, 1])
lightdir /= np.linalg.norm(lightdir)
lambert = np.sum(normals * lightdir, 2)
lighting = snowy.reshape(lambert) * occlusion

print("Applying color gradient.")
yvals = snowy.load("gradient.png")[0, :, :3]
water_color = np.copy(yvals[126])
yvals[0:128] = water_color
apply_lut = interpolate.interp1d(np.arange(len(yvals)), yvals, axis=0)
el = elevation * 0.2 + 0.49
el = np.clip(255 * el, 0, 255)
albedo = apply_lut(snowy.unshape(el))

print("Saving to disk.")
final = albedo * lighting
snowy.save(final, "terrain.png")
snowy.show(final)
