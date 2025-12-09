import pytest
import httpx

BASE = "http://127.0.0.1:8000"


@pytest.mark.asyncio
async def test_detect_422_missing_fields():
    async with httpx.AsyncClient(timeout=10) as client:
        r = await client.post(f"{BASE}/detect/", json={})
        assert r.status_code == 422

        r = await client.post(f"{BASE}/detect/", json={"latitude": 0, "longitude": 0, "rotation_angle": 0})
        assert r.status_code == 422  # нет img_base64


@pytest.mark.asyncio
async def test_detect_400_invalid_base64():
    async with httpx.AsyncClient(timeout=10) as client:
        payload = {
            "latitude": 0,
            "longitude": 0,
            "rotation_angle": 0,
            "img_base64": "invalid-base64-string!!!"
        }
        r = await client.post(f"{BASE}/detect/", json=payload)
        assert r.status_code == 400
        assert "Invalid base64 image" in r.json()["detail"]


@pytest.mark.asyncio
async def test_fertilizer_422_missing_soil_data():
    async with httpx.AsyncClient(timeout=10) as client:
        r = await client.post(f"{BASE}/fertilizer/", json={"field_id": 1})
        assert r.status_code == 422