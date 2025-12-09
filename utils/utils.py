import sys
import re
import base64
from fastapi import HTTPException
from redis.asyncio import Redis, from_url

from config.redis_settings import REDIS_HOST
from workers.validators import QueueName


_DATAURL_RE = re.compile(r"^data:image/[^;]+;base64,", re.IGNORECASE)


def _validate_base64_image(s: str) -> None:
    """
    Проверка, что пришла либо http(s) URL строка, либо base64-строка
    (с префиксом data:image/...;base64, или без него).
    """
    if not isinstance(s, str) or not s.strip():
        raise HTTPException(status_code=400, detail="Image field must be a string")

    s = s.strip()

    # 1. Разрешаем обычные http/https ссылки
    if s.lower().startswith("http://") or s.lower().startswith("https://"):
        return

    # 2. Иначе считаем, что это base64 (возможно с dataURL-префиксом)
    b64 = _DATAURL_RE.sub("", s)
    try:
        base64.b64decode(b64, validate=True)
    except Exception:
        raise HTTPException(status_code=400, detail="Invalid base64 image")


async def clear_queues(redis: Redis):
    for queue in QueueName:
        await redis.delete(queue.value)
    print("🧹 Все очереди очищены")


async def check_redis():
    try:
        redis = await from_url(REDIS_HOST)
        pong = await redis.ping()
        if not pong:
            print("❌ Redis сервер не доступен")
            sys.exit(1)
        print("✅ Redis подключен")
        return redis
    except Exception as e:
        print(f"❌ Redis сервер не доступен: {e}")
        sys.exit(1)
