# Wheat Leaf Disease Classification

Модель классификации болезней листьев пшеницы на основе ResNet18.

## Описание
В данном проекте реализована нейронная сеть для классификации изображений листьев пшеницы по типу заболевания.  
Модель обучена на датасете **Wheat Leaf Disease Dataset** и решает задачу многоклассовой классификации.

## Классы
Модель классифицирует изображения на 5 классов:
- Brown rust  
- Healthy  
- Loose Smut  
- Septoria  
- Yellow rust  
Порядок классов зафиксирован в файле `class_names.txt`.

## Модель
- Архитектура: **ResNet18**
- Предобучение: ImageNet
- Фреймворк: PyTorch
- Устройство обучения: GPU (CUDA)

## Метрики (validation)
- Accuracy: **0.9773**
- Precision: **0.9772**
- Recall: **0.9773**
- F1-score: **0.9772**

Модель также была проверена на обучающем наборе для контроля переобучения.

## Структура проекта
wheat_disease_classification/

├── train.py

├── infer.py

├── evaluate.py

├── evaluate_train.py

├── class_names.txt

├── requirements.txt

└── README.md

Обучение модели
```bash
python train.py
```
Оценка качества
Валидация:

``` bash
python evaluate.py
``` 
Обучающая выборка:
```bash
python evaluate_train.py
```
Инференс
Пример предсказания для одного изображения:
```bash
python infer.py path/to/image.png
```
Зависимости
Все используемые библиотеки перечислены в requirements.txt.
