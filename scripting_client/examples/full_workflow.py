from planetaryimager import PlanetaryImager

planetary_imager = PlanetaryImager()

# display available cameras
print(planetary_imager.cameras)

# connect the first available camera
planetary_imager.imager.open(planetary_imager.cameras[0])

