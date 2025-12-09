import asyncio
import time
import httpx
import pytest
from tests.test_basic_endpoints import tiny_png_base64

PNG_1x1 = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR4nGNgYAAAAAMAASsJTYQAAAAASUVORK5CYII="

async def post_detect(client, i):
    payload = {
        "latitude": 50.0 + i * 0.001,
        "longitude": 30.0 + i * 0.001,
        "rotation_angle": float(i % 360),
        "img_base64": tiny_png_base64() if i % 2 == 0 else PNG_1x1
    }
    r = await client.post("http://127.0.0.1:8000/detect/", json=payload)
    return i, r

async def post_fertilizer(client, i):
    payload = {"field_id": i, "soil_data": {"value": i}}
    r = await client.post("http://127.0.0.1:8000/fertilizer/", json=payload)
    return i, r


@pytest.mark.asyncio
async def test_both_endpoints_concurrent_small_burst():
    async with httpx.AsyncClient(timeout=30) as client:
        tasks = []
        for i in range(20):
            tasks.append(post_detect(client, i))
            tasks.append(post_fertilizer(client, i))
        results = await asyncio.gather(*tasks)
        for _, r in results:
            assert r.status_code == 200


@pytest.mark.asyncio
async def test_100_requests_in_1s_like_burst():
    async with httpx.AsyncClient(timeout=60) as client:
        start = time.perf_counter()
        tasks = [post_detect(client, i) for i in range(100)]
        results = await asyncio.gather(*tasks)
        elapsed = time.perf_counter() - start
        ok = sum(1 for _, r in results if r.status_code == 200)
        assert ok >= 90
        print(f"100 detect requests processed in {elapsed:.2f}s")