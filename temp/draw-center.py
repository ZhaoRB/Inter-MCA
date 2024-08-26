import xml.etree.ElementTree as ET

import cv2

calibrationFilePath = (
    "/Users/riverzhao/Project/Codec/0_lvc_codec/Inter-MCA/config/new-seq/tlct.xml"
)
imageFilePath = (
    "/Users/riverzhao/Project/Codec/0_lvc_codec/Inter-MCA/data/raw/miniGarden.bmp"
)

image = cv2.imread(imageFilePath)

tree = ET.parse(calibrationFilePath)
root = tree.getroot()

# 转换成float了，因为cv.circle
diameter = int(float(root.find("diameter").text))
centers = root.find("centers")

points = ["ltop", "rtop", "lbot"]
for point in points:
    x = float(centers.find(point).find("x").text)
    y = float(centers.find(point).find("y").text)
    cv2.circle(image, (int(x), int(y)), diameter // 2, (0, 0, 255), 2)

output_path = "/Users/riverzhao/Project/Codec/0_lvc_codec/Inter-MCA/data/center/new-seq/three-points.png"
cv2.imwrite(output_path, image)
