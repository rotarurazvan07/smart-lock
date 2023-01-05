import os
import pickle
import shutil
import subprocess
import threading
import time

import cv2
import numpy as np
import serial
from PIL import Image
from picamera import PiCamera
from picamera.array import PiRGBArray

class SerialManager():
    def __init__(self):
        self.connection = serial.Serial("/dev/serial0", baudrate=115200, timeout=1)
        self.serial_thread = threading.Thread(target=self.read_from_serial)
        self.serial_thread.start()

    def close_serial(self):
        self.connection.close()

    def read_from_serial(self):
        while True:
            if self.connection.inWaiting() > 0:
                try:
                    data = self.connection.readline().strip().decode('utf-8')
                    print("Received message from esp32: " + data)
                except UnicodeDecodeError:
                    continue

                if "AT+REGISTER=" in data:
                    name = data[data.find("=")+1:]
                    face_manager.register_face(name)
                elif "AT+READUSER=" in data:
                    face_manager.recognise_user()
                elif "AT+DELETEUSER=" in data:
                    name = data[data.find("=") + 1:]
                    face_manager.delete_face(name)

            time.sleep(10)

    def write_to_serial(self, data):
        print("Sending to esp32: " + data.decode('utf-8'))
        self.connection.write(data)

class FaceManager():
    def __init__(self):
        self.camera = None
        self.rawCapture = None
        self.faceCascade = cv2.CascadeClassifier("haarcascade_frontalface_alt2.xml")
        self.recognizer = cv2.face.LBPHFaceRecognizer_create()

    def init_camera(self):
        self.camera = PiCamera()
        self.camera.resolution = (640, 480)
        self.camera.rotation = 180
        self.camera.framerate = 30
        self.rawCapture = PiRGBArray(self.camera, size=self.camera.resolution)

    def delete_face(self, user_name):
        print("Deleting face name: " + user_name)
        dirName = "./dataset/" + user_name
        shutil.rmtree(dirName)
        print("Retraining model")
        self.train_model()
        print("Retraining complete")
        serial_manager.write_to_serial("AT+DELETEUSER=1\r\n".encode("utf-8"))

    def train_model(self):
        print("Starting training")

        imageDir = os.path.join(".", "dataset")

        currentId = 1
        labelIds = {}
        yLabels = []
        xTrain = []

        for root, dirs, files in os.walk(imageDir):
            for file in files:
                if file.endswith("png") or file.endswith("jpg"):
                    path = os.path.join(root, file)
                    label = os.path.basename(root)
                    print("Training: " + label + " : " + path)

                    if not label in labelIds:
                        labelIds[label] = currentId
                        currentId += 1

                    id_ = labelIds[label]
                    pilImage = Image.open(path).convert("L")
                    imageArray = np.array(pilImage, "uint8")
                    faces = self.faceCascade.detectMultiScale(imageArray, scaleFactor=1.05, minNeighbors=3, minSize=(30, 30))

                    for (x, y, w, h) in faces:
                        roi = imageArray[y:y + h, x:x + w]
                        xTrain.append(roi)
                        yLabels.append(id_)

        try:
            self.recognizer.train(xTrain, np.array(yLabels))
            self.recognizer.save("trainer.yml")

            with open("labels", "wb") as f:
                pickle.dump(labelIds, f)
                f.close()

            print("Training complete")
        except:
            # delete trainer.yml and labels
            os.remove("trainer.yml")
            os.remove("labels")
            shutil.rmtree("dataset")
            print("no faces to train")

    def register_face(self, user_name):
        self.init_camera()
        green_led_on()
        print("Starting registration")
        dirName = "./dataset/" + user_name
        os.makedirs(dirName)

        count = 0
        for frame in self.camera.capture_continuous(self.rawCapture, format="bgr", use_video_port=True):
            red_led_off()
            if count > 100:
                break

            frame = frame.array
            gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
            faces = self.faceCascade.detectMultiScale(gray, scaleFactor=1.05, minNeighbors=5, minSize=(200, 200))

            for (x, y, w, h) in faces:
                roiGray = gray[y:y + h, x:x + w]
                fileName = dirName + "/" + user_name + str(count) + ".jpg"
                print("Registration step: " + str(count) + "/100: " + fileName)
                cv2.imwrite(fileName, roiGray)
                count+=1
                red_led_on()

            if cv2.waitKey(1) & 0xFF == ord('q'): break

            self.rawCapture.truncate(0)

        self.camera.close()
        print("Registration complete")
        green_led_off()
        # ----------------------------------------------------------

        self.train_model()

        print("User " + user_name +" registered successfully")
        serial_manager.write_to_serial("AT+REGISTER=1\r\n".encode("utf-8"))

    def recognise_user(self):
        detections = dict()
        detections["unknown"] = 0
        timer_up = 0
        try:
            f = open('labels', 'rb')
            dicti = pickle.load(f)
            f.close()
            for k in dicti.keys():
                detections[k] = 0
        except:
            print("No registered users")
            serial_manager.write_to_serial(("AT+READUSER=" + "-1" + "\r\n").encode("utf-8"))
            return

        self.recognizer.read("trainer.yml")

        curr_time = time.time()
        prev_time = curr_time

        self.init_camera()
        green_led_on()

        for frame in self.camera.capture_continuous(self.rawCapture, format="bgr", use_video_port=True):
            red_led_off()
            frame = frame.array
            gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
            faces = self.faceCascade.detectMultiScale(gray, scaleFactor=1.05, minNeighbors=5, minSize=(200, 200))

            for (x, y, w, h) in faces:
                roiGray = gray[y:y + h, x:x + w]

                name = "unknown"
                id_, conf = self.recognizer.predict(roiGray)
                if conf <= 80:
                    for k, v in dicti.items():
                        if v == id_:
                            name = k

                print("Detected " + name + " with confidence " + str(conf))

                curr_time = time.time()

                if timer_up == 0:
                    prev_time = curr_time
                    timer_up = 1

                if curr_time > prev_time + 5:
                    timer_up = 0
                    curr_time = time.time()
                    prev_time = curr_time
                    for k, v in detections.items():
                        detections[k] = 0

                if timer_up == 1:
                    red_led_on()
                    detections[name] += 1
                    if detections[name] > 10:
                        if name == "unknown":
                            print(name + " is at the door")
                            serial_manager.write_to_serial(("AT+READUSER=" + "-1" + "\r\n").encode("utf-8"))
                            self.camera.close()
                            red_led_off()
                            green_led_off()
                            return
                        else:
                            print("Granted Access to  " + name)
                            serial_manager.write_to_serial(("AT+READUSER=" + name + "\r\n").encode("utf-8"))
                            self.camera.close()
                            red_led_off()
                            green_led_off()
                            return

            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

            self.rawCapture.truncate(0)

def red_led_on():
    subprocess.call("sudo bash -c \"echo 1 >/sys/class/leds/led1/brightness\"", shell=True)

def red_led_off():
    subprocess.call("sudo bash -c \"echo 0 >/sys/class/leds/led1/brightness\"", shell=True)

def green_led_on():
    subprocess.call("sudo bash -c \"echo 1 >/sys/class/leds/led0/brightness\"", shell=True)

def green_led_off():
    subprocess.call("sudo bash -c \"echo 0 >/sys/class/leds/led0/brightness\"", shell=True)

if __name__ == "__main__":
    subprocess.call("sudo bash -c \"echo none > /sys/class/leds/led0/trigger\"", shell=True)
    subprocess.call("sudo bash -c \"echo none > /sys/class/leds/led1/trigger\"", shell=True)

    serial_manager = SerialManager()
    face_manager = FaceManager()