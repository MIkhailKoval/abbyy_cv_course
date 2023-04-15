import cv2
import matplotlib.pyplot as plt

import numpy as np

IMAGES_COUNT = 12
DELTA = 10
DETECTORS = ['BRISK', 'ORB', 'SIFT']

NAME_TO_DETECTOR = {
    'BRISK': cv2.BRISK_create(),
    'SIFT': cv2.SIFT_create(),
    'ORB': cv2.ORB_create(),
}

def make_points(detector_name, image):
    detector = NAME_TO_DETECTOR[detector_name]
    points = detector.detect(image, None)
    return [point.pt for point in points]

def match_points(first_points, second_points):
    result = []
    first_mean_points = np.mean(first_points, axis = 0)
    second_mean_points = np.mean(second_points, axis = 0)
    mean_points1 = np.mean(first_points, axis=0)
    mean_points2 = np.mean(second_points, axis=0)
    for i in range(np.min([len(first_points), len(second_points)])):
        diff = second_points - np.array(first_points)[i, :]
        diff -= np.array([first_mean_points[0] - second_mean_points[0], first_mean_points[1] - second_mean_points[1]])
        dist = np.sum(np.array(np.abs(diff)), axis = 1)
        if np.min(np.array(dist)) <= DELTA:
            result.append(dist)
    return len(result)

def calculate_matching_statistic(detector_name, first_image, second_image):
    first_points = make_points(detector_name, first_image)
    second_points = make_points(detector_name, second_image)
    final_points = match_points(first_points, second_points)

    return final_points / len(first_points)

def calculate_repeatability():
    result = [[],[],[]]
    for k in range(len(DETECTORS)):
        detector = DETECTORS[k]
        for i in range(1, IMAGES_COUNT + 1):
            current_repeitability = 0.0
            for j in range(1, IMAGES_COUNT + 1):
                if i != j:
                    print(f"{i} with {j}, detector {detector}")
                    first_image = cv2.imread('samples/' + str(i) + '.tif')
                    second_image = cv2.imread('samples/' + str(j) + '.tif')
                    current_repeitability += calculate_matching_statistic(detector, first_image, second_image)
            result[k].append(current_repeitability)
    return result

result = calculate_repeatability()

for i in range(len(DETECTORS)):
    plt.figure(figsize=(15, 10))
    plt.plot(np.arange(1, 13), np.array(result[i]) / (IMAGES_COUNT - 1))
    plt.ylabel('repeatability')
    plt.xlabel('image number')
    plt.show()
