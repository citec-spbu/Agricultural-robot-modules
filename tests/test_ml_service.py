import asyncio
import base64
import io
import pytest
import httpx
from PIL import Image


def tiny_png_base64():
    img = Image.new("RGB", (2, 2), (255, 255, 255))
    buf = io.BytesIO()
    img.save(buf, format="PNG")
    return "data:image/png;base64," + base64.b64encode(buf.getvalue()).decode()


@pytest.fixture(scope="session")
def base_url():
    return "http://127.0.0.1:8000"


@pytest.mark.asyncio
async def test_detect_ok(base_url):
    payload = {
        "latitude": 54.123,
        "longitude": 30.123,
        "rotation_angle": 90.0,
        "img_base64": tiny_png_base64()
    }
    async with httpx.AsyncClient(timeout=30) as client:
        r = await client.post(f"{base_url}/detect/", json=payload)

    assert r.status_code == 200

    data = r.json()
    assert "task_id" in data
    assert isinstance(data["task_id"], str)
    assert len(data["task_id"]) == 36  # UUID4

    assert data["ml_result_base64"].startswith("data:image/png;base64,")
    assert data["latitude"] == 54.123
    assert data["longitude"] == 30.123
    assert data["rotation_angle"] == 90.0


@pytest.mark.asyncio
async def test_detect_burst_100(base_url):
    async with httpx.AsyncClient(timeout=60) as client:
        async def call(i):
            payload = {
                "latitude": 50.0 + i * 0.01,
                "longitude": 30.0 + i * 0.01,
                "rotation_angle": 0.0,
                "img_base64": tiny_png_base64()
            }
            r = await client.post(f"{base_url}/detect/", json=payload)
            return r.status_code

        codes = await asyncio.gather(*[call(i) for i in range(100)])
        ok_count = sum(1 for c in codes if c == 200)
        assert ok_count >= 90, f"Only {ok_count}/100 requests succeeded"