-- 1. Сессии
CREATE TABLE IF NOT EXISTS Sessions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    start_timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    description TEXT DEFAULT 'session'
);

-- 2. Поля
CREATE TABLE IF NOT EXISTS Fields (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL DEFAULT 'Поле №1',
    boundary_json TEXT DEFAULT '{}',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    session_id INTEGER,
    FOREIGN KEY(session_id) REFERENCES Sessions(id)
);

-- 3. Спецификации сенсоров
CREATE TABLE IF NOT EXISTS Sensor_specs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    spec_json TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 4. Точки
CREATE TABLE IF NOT EXISTS Points (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    field_id INTEGER NOT NULL,
    session_id INTEGER NOT NULL,
    latitude REAL NOT NULL,
    longitude REAL NOT NULL,
    data_json TEXT NOT NULL DEFAULT '{}',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(field_id) REFERENCES Fields(id),
    FOREIGN KEY(session_id) REFERENCES Sessions(id)
);

-- 5. Наблюдения
CREATE TABLE IF NOT EXISTS Observations (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    point_id INTEGER NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(point_id) REFERENCES Points(id)
);

-- 6. Результаты ML
CREATE TABLE IF NOT EXISTS ML_results (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    observation_id INTEGER NOT NULL,
    module_name TEXT NOT NULL DEFAULT 'ML_module',
    results_json TEXT NOT NULL DEFAULT '{}',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(observation_id) REFERENCES Observations(id)
);

-- 7. Рекомендации
CREATE TABLE IF NOT EXISTS Recommendations (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    observation_id INTEGER NOT NULL,
    text TEXT NOT NULL DEFAULT 'Action',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(observation_id) REFERENCES Observations(id)
);
