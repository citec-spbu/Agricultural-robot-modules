# Robot Simulator

Это сервис-имитация робота, который поддерживает движение по координатам, вращение, остановку и отправку данных о текущем состоянии. Сервис использует FastAPI и предоставляет REST API для управления роботом и получения его состояния.

## Функциональность

- Робот имеет координаты (`latitude`, `longitude`) и угол ориентации (`rotation_angle`) относительно начального положения.
- Поддерживает команды:
  - `move` — движение вперед/назад на указанное расстояние (м)
  - `rotate` — поворот на заданное количество градусов
  - `speed` — установка скорости движения (м/с)
  - `stop` — остановка движения
- Движение происходит **плавно**, координаты обновляются со временем, как если бы робот реально ехал.
- Робот отправляет данные:
  - `latitude` — широта
  - `longitude` — долгота
  - `rotation_angle` — угол ориентации
  - `img_base64` — изображение с камеры в Base64
- Каждое новое изображение подставляется каждые 1 метр движения. После достижения конца набора изображений счетчик начинается заново.

## Запуск
`uvicorn main:app'

Задать свой адрес и порт можно через флаги: `--host 0.0.0.0 --port 8000`

## API

### Получение состояния
`GET /collect`

Пример ответа

```
{
  "latitude": 54.123,
  "longitude": 30.123,
  "rotation_angle": 0.0,
  "img_base64": "<base64_string>"
}
```

### Получение спецификации
`GET /spec`

Пример ответа:

```
{
   {
      "name": "gps",
      "model": "GPS-A1",
      "observations": [
            {"description": "latitude",  "datatype": "double", "bytes": 8},
            {"description": "longitude", "datatype": "double", "bytes": 8},
      ],
   },
   {
      "name": "orientation",
      "model": "ROT-1",
      "observations": [
            {"description": "rotation_angle", "datatype": "double", "bytes": 8},
      ],
   },
   {
      "name": "camera",
      "model": "CamX",
      "observations": [
            {"description": "image", "datatype": "str", "bytes": IMAGE_SIZE_BYTES},
      ],
   }
}

```

### Остановка
`POST /stop`

Ответ:
`{"status": "stopped"}`


### Отправка команды
`POST /command`

### Виды команд
#### Движение вперед на m метров
```
{
  "cmd": "move",
  "data": {
    "distance_m": 100
  }
}
```

#### Поворот
```
{
  "cmd": "rotate",
  "data": {
    "delta_angle": -20
  }
}
```
