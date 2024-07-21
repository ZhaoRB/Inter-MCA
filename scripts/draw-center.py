import os
import xml.etree.ElementTree as ET

import cv2

currentDir = os.path.dirname(os.path.abspath(__file__))
calibrationFilePath = os.path.join(currentDir, "../config/calibration.xml")
imageFilePath = os.path.join(currentDir, "../data/raw/cars.bmp")

image = cv2.imread(imageFilePath)

tree = ET.parse("../config/calibration.xml")
root = tree.getroot()

# 转换成float了，因为cv.circle
diameter = int(float(root.find("diameter").text))
centers = root.find("centers")

points = ["ltop", "rtop", "lbot"]
for point in points:
    x = float(centers.find(point).find("x").text)
    y = float(centers.find(point).find("y").text)
    cv2.circle(image, (int(x), int(y)), diameter // 2, (0, 0, 255), 2)

output_path = os.path.join(currentDir, "../data/center/center_cars.png")
cv2.imwrite(output_path, image)
