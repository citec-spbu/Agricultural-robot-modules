import os
import random
import shutil

# 输入文件夹
IMAGE_DIR = "images"
LABEL_DIR = "yolo_labels"

# 输出目录（YOLO 标准结构）
OUT = "dataset"
IMG_OUT = os.path.join(OUT, "images")
LBL_OUT = os.path.join(OUT, "labels")

# 创建结构
for sub in ["train", "val"]:
    os.makedirs(os.path.join(IMG_OUT, sub), exist_ok=True)
    os.makedirs(os.path.join(LBL_OUT, sub), exist_ok=True)

# 划分比例
train_ratio = 0.8

# 获取全部图片文件
all_images = sorted([f for f in os.listdir(IMAGE_DIR) if f.endswith(".png")])

# 随机洗牌
random.shuffle(all_images)

# 计算数量
train_count = int(len(all_images) * train_ratio)
train_files = all_images[:train_count]
val_files = all_images[train_count:]

def move_files(file_list, split):
    for img_name in file_list:
        # 图片
        src_img = os.path.join(IMAGE_DIR, img_name)
        dst_img = os.path.join(IMG_OUT, split, img_name)

        # 标签（同名 .txt）
        label_name = img_name.replace(".png", ".txt")
        src_lbl = os.path.join(LABEL_DIR, label_name)
        dst_lbl = os.path.join(LBL_OUT, split, label_name)

        shutil.copy(src_img, dst_img)
        shutil.copy(src_lbl, dst_lbl)

    print(f"[OK] {split} 集合完成，共 {len(file_list)} 张")

# 执行拷贝
move_files(train_files, "train")
move_files(val_files, "val")

print("Dataset split finished. 数据集划分完成！")
