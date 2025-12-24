import time, os, base64
from threading import Lock
from geographiclib.geodesic import Geodesic
from .config import *

class Robot:
    def __init__(self, latitude=START_LATITUDE, longitude=START_LONGITUDE, rotation_angle=START_ANGLE):
        self.latitude = latitude
        self.longitude = longitude
        self.rotation_angle = rotation_angle 

        self.speed_mps = 0.0
        self.mode = "idle"  
        self.move_remaining_m = 0.0

        self.img_index = 0
        self.image_base64 = self.load_image(self.img_index)

        self.lock = Lock()
        self.last_update = time.time()
        self.distance_accum = 0.0

    def load_image(self, index):
        if not IMAGE_FILES:
            return ""
        index = index % len(IMAGE_FILES)  # зацикливаем
        path = os.path.join(IMAGE_FOLDER, IMAGE_FILES[index])
        with open(path, "rb") as f:
            return base64.b64encode(f.read()).decode()

    def update_position(self):
        """Обновление координат и смена изображения"""
        now = time.time()

        with self.lock:
            if self.speed_mps == 0 or self.mode not in ["moving", "speed"]:
                self.last_update = now
                return

            # время с последнего обновления позиции
            dt = now - self.last_update
            self.last_update = now

            ds = abs(self.speed_mps) * dt

            if self.mode == "moving":
                if ds >= self.move_remaining_m:
                    ds = self.move_remaining_m
                    self.mode = "idle"
                    self.speed_mps = 0
                    self.move_remaining_m = 0
                else:
                    self.move_remaining_m -= ds

            # знак движения (вперед/назад)
            sign = 1 if self.speed_mps >= 0 else -1
            ds_signed = ds * sign

            # вычисление новой позиции
            result = Geodesic.WGS84.Direct(
                self.latitude,
                self.longitude,
                self.rotation_angle,
                ds_signed
            )
            self.latitude = result["lat2"]
            self.longitude = result["lon2"]

            # обновление изображения каждые IMAGE_STEP_M метров
            self.distance_accum += abs(ds)
            while self.distance_accum >= IMAGE_STEP_M:
                self.distance_accum -= IMAGE_STEP_M
                self.img_index += 1
                self.image_base64 = self.load_image(self.img_index)


    def stop(self):
        """Останавливает робота"""
        with self.lock:
            self.speed_mps = 0
            self.mode = "idle"
            self.move_remaining_m = 0

    def move(self, distance_m, speed_mps=START_SPEED):
        with self.lock:
            self.move_remaining_m = abs(distance_m)
            self.speed_mps = speed_mps if distance_m >= 0 else -speed_mps
            self.mode = "moving" if distance_m != 0 else "idle"
            self.last_update = time.time()

    def rotate(self, delta_angle):
        with self.lock:
            self.rotation_angle = (self.rotation_angle + delta_angle) % 360

    def get_state(self):
        """Возвращает текущие данные робота"""
        self.update_position()
        return {
            "latitude": self.latitude,
            "longitude": self.longitude,
            "rotation_angle": self.rotation_angle,
            "img_base64": self.image_base64,
        }
