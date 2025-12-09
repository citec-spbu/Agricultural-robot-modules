# models/detect_model.py
import base64
import io
import re
import torch
import numpy as np
from PIL import Image
from typing import Dict
from config.ml_models_settings import DETECT_MODEL_PATH
from models.deeplab_mnv3 import build_model
from utils.augmentations import get_valid_augs

#--- Валидация base64 (скопирована из utils.py) ---
_DATAURL_RE = re.compile(r"^data:image/[^;]+;base64,", re.IGNORECASE)

def _validate_base64_image(s: str) -> None:
    if not isinstance(s, str) or not s.strip():
        raise ValueError("Image field must be a non-empty string")

    s = s.strip()

    if s.lower().startswith("http://") or s.lower().startswith("https://"):
        return  # Разрешаем URL

    b64 = _DATAURL_RE.sub("", s)
    try:
        base64.b64decode(b64, validate=True)
    except Exception as e:
        raise ValueError("Invalid base64 image") from e
# --- Конец валидации ---

# Глобальная загрузка модели (один раз при импорте)
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
model = build_model(num_classes=3).to(device)
model.load_state_dict(torch.load(DETECT_MODEL_PATH, map_location=device))
model.eval()

augs = get_valid_augs(size=512)  # Размер входа модели

COLOR_MAP = np.array([
    [0,   0,   0],   # 0 — фон      → чёрный
    [0, 255,   0],   # 1 — пшеница  → зелёный
    [255, 0,   0]    # 2 — сорняк   → красный
], dtype=np.uint8)

async def predict(data: Dict) -> Dict:
    # Валидация base64
    img_base64 = data.get("img_base64")
    _validate_base64_image(img_base64)

    # Декодирование base64 в изображение
    img_bytes = base64.b64decode(img_base64.split(",")[-1])  # Удаляем data:image/...;base64, если есть
    orig_img = Image.open(io.BytesIO(img_bytes)).convert("RGB")
    w, h = orig_img.size

    # Предобработка
    img_np = np.array(orig_img)
    augmented = augs(image=img_np)
    tensor = augmented["image"].unsqueeze(0).to(device)

    # Инференс
    with torch.no_grad():
        logits = model(tensor)["out"]
        pred = torch.argmax(logits, dim=1).cpu().numpy()[0]  # (512, 512)

    # Ресайз к оригинальному размеру
    pred_pil = Image.fromarray(pred.astype(np.uint8))
    pred_resized = pred_pil.resize((w, h), Image.NEAREST)
    pred_np = np.array(pred_resized)  # Маска с значениями 0,1,2

    colored_mask = Image.fromarray(COLOR_MAP[pred_np])

    # Кодирование маски в base64 (как PNG)
    buf = io.BytesIO()
    colored_mask.save(buf, format="PNG")
    ml_result_base64 = "data:image/png;base64," + base64.b64encode(buf.getvalue()).decode()

    # Формирование ответа
    response = {
        "latitude": data["latitude"],
        "longitude": data["longitude"],
        "rotation_angle": data["rotation_angle"],
        "ml_result_base64": ml_result_base64
    }
    return response