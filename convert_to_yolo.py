import os
import yaml
from PIL import Image

# ====== 目录设置 ======
ANNOT_DIR = "annotations"
IMAGE_DIR = "images"
OUTPUT_DIR = "yolo_labels"

os.makedirs(OUTPUT_DIR, exist_ok=True)

# 类别映射：crop=0（小麦），weed=1（杂草）
CLASS_MAP = {
    "crop": 0,
    "weed": 1
}

print("Starting conversion...")

for file in os.listdir(ANNOT_DIR):
    if not file.endswith(".yaml"):
        continue

    yaml_path = os.path.join(ANNOT_DIR, file)

    # --- 读取 YAML ---
    with open(yaml_path, "r", encoding="utf-8") as f:
        data = yaml.safe_load(f)

    # --- 取文件名 ---
    image_name = data["filename"]
    image_path = os.path.join(IMAGE_DIR, image_name)

    # --- 读取图片尺寸（用于归一化） ---
    if not os.path.exists(image_path):
        print(f"[Warning] 找不到图片：{image_path}")
        continue

    img = Image.open(image_path)
    img_w, img_h = img.size

    # 输出 YOLO 标签名
    txt_name = os.path.splitext(image_name)[0] + ".txt"
    txt_path = os.path.join(OUTPUT_DIR, txt_name)

    with open(txt_path, "w", encoding="utf-8") as out:

        for ann in data["annotation"]:
            cls_type = ann["type"]
            pts_x = ann["points"]["x"]
            pts_y = ann["points"]["y"]

            # --- 检查 annotation 是否有效 ---
            if not isinstance(pts_x, list) or not isinstance(pts_y, list):
                print(f"[Error] {image_name} 中有无效 annotation（不是点集）→ 已跳过")
                continue

            if len(pts_x) < 2 or len(pts_y) < 2:
                print(f"[Error] {image_name} 中 annotation 点数太少 → 已跳过")
                continue

            # --- polygon → bbox ---
            min_x = min(pts_x)
            max_x = max(pts_x)
            min_y = min(pts_y)
            max_y = max(pts_y)

            bbox_x = (min_x + max_x) / 2
            bbox_y = (min_y + max_y) / 2
            bbox_w = max_x - min_x
            bbox_h = max_y - min_y

            # --- YOLO 归一化 ---
            yolo_x = bbox_x / img_w
            yolo_y = bbox_y / img_h
            yolo_w = bbox_w / img_w
            yolo_h = bbox_h / img_h

            # --- 类别编号 ---
            cls_id = CLASS_MAP.get(cls_type, None)
            if cls_id is None:
                print(f"[Warning] 未知类别: {cls_type}")
                continue

            # 写入一行 YOLO 格式
            out.write(f"{cls_id} {yolo_x:.6f} {yolo_y:.6f} {yolo_w:.6f} {yolo_h:.6f}\n")

    print(f"[OK] {image_name} → {txt_name}")

print("Conversion finished! YOLO 标签已生成到 yolo_labels/ 文件夹")
