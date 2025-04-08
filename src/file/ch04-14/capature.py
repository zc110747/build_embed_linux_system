import cv2

# 获取摄像头视频流(1对于/dev/video1)
cam = cv2.VideoCapture(1)

while (cam.isOpened()):

    # 读取图像并显示
    retval, img = cam.read()
    cv2.imshow("Video", img)
    
    # 等待按键结束
    key = cv2.waitKey(1)  
    if key == 32:
        break
    
capture.release()
cv2.destroyAllWindows()