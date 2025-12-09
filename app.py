import os
import uuid
import json
import time
import asyncio
from fastapi import FastAPI, HTTPException
from contextlib import asynccontextmanager
from redis.asyncio import Redis, from_url

from config.redis_settings import (
    REDIS_HOST,
    QUEUE_DETECT,
    QUEUE_FERTILIZER,
)
from config.app_settings import REQUEST_TIMEOUT

from schemas.detect import DetectRequest, DetectResult
from schemas.fertilizer import FertilizerRequest, FertilizerResult

from workers.manager import start_all_workers, stop_all_workers
from utils.utils import check_redis, clear_queues, _validate_base64_image


@asynccontextmanager
async def lifespan(app: FastAPI):
    await check_redis()
    app.state.redis = await from_url(REDIS_HOST, decode_responses=True)
    await clear_queues(app.state.redis)
    await start_all_workers(app.state.redis)
    try:
        yield
    finally:
        await stop_all_workers()
        await app.state.redis.close()


app = FastAPI(lifespan=lifespan)

RESPONSE_TIMEOUT_SEC = float(os.getenv("RESPONSE_TIMEOUT_SEC", REQUEST_TIMEOUT))


async def wait_for_result(task_id: str, timeout: float, redis):
    deadline = time.time() + timeout

    while time.time() < deadline:
        data = await redis.get(task_id)
        if data:
            return json.loads(data)
        await asyncio.sleep(0.05)  # короткая асинхронная пауза

    raise HTTPException(status_code=504, detail="Timeout waiting for result")



@app.post("/detect/", response_model=DetectResult)
async def detect(request: DetectRequest):
    redis = app.state.redis
    task_id = str(uuid.uuid4())
    task = {"task_id": task_id, "type": "detect", "data": request.model_dump()}
    await redis.rpush(QUEUE_DETECT, json.dumps(task))
    result = await wait_for_result(task_id, RESPONSE_TIMEOUT_SEC, redis)
    
    return {
        "task_id": task_id,
        "latitude": result["latitude"],
        "longitude": result["longitude"],
        "rotation_angle": result["rotation_angle"],
        "ml_result_base64": result["ml_result_base64"]
    }


@app.post("/fertilizer/", response_model=FertilizerResult)
async def fertilizer(request: FertilizerRequest):
    redis = app.state.redis
    task_id = str(uuid.uuid4())
    task = {"task_id": task_id, "type": "fertilizer", "data": request.model_dump()}
    await redis.rpush(QUEUE_FERTILIZER, json.dumps(task))
    result = await wait_for_result(task_id, RESPONSE_TIMEOUT_SEC, redis)
    return {"task_id": task_id, "result": result}
