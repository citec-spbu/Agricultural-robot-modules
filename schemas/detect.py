# schemas/detect.py
from pydantic import BaseModel
from typing import Optional

class DetectRequest(BaseModel):
    latitude: float
    longitude: float
    rotation_angle: float
    img_base64: str

class DetectResult(BaseModel):
    task_id: str
    latitude: float
    longitude: float
    rotation_angle: float
    ml_result_base64: str