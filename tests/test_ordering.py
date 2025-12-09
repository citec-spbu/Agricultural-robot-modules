import asyncio
import pytest
import httpx
from tests.test_basic_endpoints import tiny_png_base64


@pytest.mark.asyncio
async def test_order_of_100_requests(base_url="http://127.0.0.1:8000"):
    async with httpx.AsyncClient(timeout=60) as client:
        async def call(i):
            payload = {
                "latitude": 50.0 + i * 0.001,
                "longitude": 30.0 + i * 0.001,
                "rotation_angle": 0.0,
                "img_base64": tiny_png_base64()
            }
            r = await client.post(f"{base_url}/detect/", json=payload)
            return i, r.status_code

        N = 100
        results = await asyncio.gather(*[call(i) for i in range(N)])
        ok = [i for i, code in results if code == 200]
        assert len(ok) == N

        order_ok = all(ok[i] < ok[i + 1] for i in range(len(ok) - 1))
        assert order_ok, "Responses came out of order!"