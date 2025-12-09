import base64
import io
from PIL import Image
import pytest
import httpx

BASE = "http://127.0.0.1:8000"


def tiny_png_base64(color=(120, 180, 120)):
    img = Image.new("RGB", (64, 64), color)
    buf = io.BytesIO()
    img.save(buf, format="PNG")
    b64 = base64.b64encode(buf.getvalue()).decode()
    return f"data:image/png;base64,{b64}"


@pytest.mark.asyncio
async def test_detect_ok():
    async with httpx.AsyncClient(timeout=30) as client:
        payload = {
            "latitude": 55.7558,
            "longitude": 37.6173,
            "rotation_angle": 0.0,
            "img_base64": tiny_png_base64()
        }
        r = await client.post(f"{BASE}/detect/", json=payload)
        assert r.status_code == 200
        data = r.json()
        assert "task_id" in data
        assert len(data["task_id"]) == 36  # UUID4
        assert "ml_result_base64" in data
        assert data["latitude"] == 55.7558
        assert data["rotation_angle"] == 0.0


@pytest.mark.asyncio
async def test_fertilizer_ok():
    async with httpx.AsyncClient(timeout=30) as client:
        payload = {"field_id": 42, "soil_data": {"N": 10, "P": 5, "K": 8}}
        r = await client.post(f"{BASE}/fertilizer/", json=payload)
        assert r.status_code == 200
        data = r.json()
        assert data["result"]["data"]["soil_data"]["N"] == 10