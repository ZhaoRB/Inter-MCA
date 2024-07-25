# %% [markdown]
# # center 分析

import math

# %%
import os
import xml.etree.ElementTree as ET

import cv2
import numpy as np

projectPath = "/Users/riverzhao/Project/Codec/0_lvc_codec/Inter-MCA"

calibrationFilePath = os.path.join(projectPath, "./config/calibration.xml")
imageFilePath = os.path.join(projectPath, "./data/raw/cars.bmp")

image = cv2.imread(imageFilePath)
print(image.shape, image.shape[1])

tree = ET.parse(calibrationFilePath)
root = tree.getroot()

diameterNode = root.find("diameter")
diameter = int(diameterNode.text)
centersNode = root.find("centers")
rowNode = centersNode.find("rows")
rowNum = int(rowNode.text)
colNode = centersNode.find("cols")
colNum = int(colNode.text)


# %% [markdown]
# ## 计算所有的中心点

# %%
# 创建一个numpy数组存储所有的center points
allCenterPoints = np.zeros((rowNum, colNum, 2))

allCenterPoints[0, 0, 0] = float(centersNode.find("ltop").find("x").text)
allCenterPoints[0, 0, 1] = float(centersNode.find("ltop").find("y").text)

allCenterPoints[0, colNum - 1, 0] = float(centersNode.find("rtop").find("x").text)
allCenterPoints[0, colNum - 1, 1] = float(centersNode.find("rtop").find("y").text)

allCenterPoints[rowNum - 1, 0, 0] = float(centersNode.find("lbot").find("x").text)
allCenterPoints[rowNum - 1, 0, 1] = float(centersNode.find("lbot").find("y").text)

xBias = diameter / 2 * math.sqrt(3)
yBias = diameter / 2

allCenterPoints[0, 1, 0] = allCenterPoints[0, 0, 0] + xBias
allCenterPoints[0, 1, 1] = allCenterPoints[0, 0, 1] + yBias

ltop = allCenterPoints[0, 0, :]
lbot = allCenterPoints[rowNum - 1, 0, :]
ltopOdd = allCenterPoints[0, 1, :]
rtop = allCenterPoints[0, colNum - 1, :]

print(rtop)
print(ltopOdd)
colGap = (rtop - ltopOdd) / (colNum / 2 - 1)
rowGap = (lbot - ltop) / (rowNum - 1)

print("colGap", colGap)
print("rowGap", rowGap)

for i in range(int(colNum - 3)):
    allCenterPoints[0, i + 2, :] = allCenterPoints[0, i, :] + colGap

for r in range(int(rowNum - 1)):
    for c in range(int(colNum)):
        allCenterPoints[r + 1, c, :] = allCenterPoints[r, c, :] + rowGap

np.save("centerPoints.npy", allCenterPoints)

# %% [markdown]
# ## 画圆

# %%
image_copy = image.copy()

for row in allCenterPoints:
    for point in row:
        center = (round(point[0]), round(point[1]))
        color = (0, 0, 255)
        cv2.circle(image_copy, center, diameter // 2, color, 2)

output_path = os.path.join(projectPath, "./data/center/all_center_cars.png")
cv2.imwrite(output_path, image_copy)

del image_copy


# %% [markdown]
# ## Crop

# %%
radius = int(diameter / 2 / math.sqrt(2))
image_height = image.shape[0]
image_width = image.shape[1]

image_copy = image.copy()
for row in allCenterPoints:
    for point in row:
        x_center, y_center = round(point[0]), round(point[1])

        # 计算正方形区域的边界
        x_start = max(x_center - radius, 0)
        x_end = min(x_center + radius, image_width)
        y_start = max(y_center - radius, 0)
        y_end = min(y_center + radius, image_height)

        # 将正方形区域内的像素置为 0
        image[y_start:y_end, x_start:x_end, :] = 0

        top_left = (x_start, y_start)
        bottom_right = (x_end, y_end)
        cv2.rectangle(image_copy, top_left, bottom_right, color, 2)

output_path = os.path.join(projectPath, "./data/center/mask_cars.png")
cv2.imwrite(output_path, image)

del image_copy
