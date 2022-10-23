from PIL import Image

import numpy as np
import math

def luminance(vec):
    return np.dot(np.array([0.2126, 0.7152, 0.0722]), vec)

expected = Image.open('./orig.bmp')
real = Image.open('./result.bmp')

expected_image = np.array(expected.convert('RGB'), dtype='int64')
real_image = np.array(real.convert('RGB'), dtype='int64')

res = 0
mse = 0
for i in range(expected_image.shape[0]):
    for j in range(expected_image.shape[1]):
        mse += (luminance(expected_image[i][j]) - luminance(real_image[i][j]))**2
mse /= (expected_image.shape[0] * expected_image.shape[1])

print(20 * math.log(255, 10) - 10 * math.log(mse, 10))