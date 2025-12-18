import os
import torch
from torch import nn, optim
from torch.utils.data import DataLoader
from torchvision import datasets, transforms, models
from sklearn.metrics import accuracy_score
from tqdm import tqdm

# 你的已划分数据文件夹结构：
# data/train/CLASS_NAME
# data/val/CLASS_NAME
DATA_DIR = "data"

BATCH_SIZE = 16
LR = 1e-4
EPOCHS = 10
MODEL_PATH = "resnet18_wheat.pth"

# 图像增强：适度增强避免过拟合
train_tfms = transforms.Compose([
    transforms.Resize((224, 224)),
    transforms.RandomHorizontalFlip(),
    transforms.RandomRotation(10),
    transforms.ToTensor(),
])

val_tfms = transforms.Compose([
    transforms.Resize((224, 224)),
    transforms.ToTensor(),
])

# 加载数据集（你已经划分好 train/val，无需任何其他操作）
train_ds = datasets.ImageFolder(os.path.join(DATA_DIR, "train"), transform=train_tfms)
val_ds = datasets.ImageFolder(os.path.join(DATA_DIR, "val"), transform=val_tfms)

train_loader = DataLoader(train_ds, batch_size=BATCH_SIZE, shuffle=True)
val_loader = DataLoader(val_ds, batch_size=BATCH_SIZE, shuffle=False)

# 获取类别名称（自动）
class_names = train_ds.classes
print("Classes:", class_names)

# 保存类别名称用于推理
with open("class_names.txt", "w") as f:
    for name in class_names:
        f.write(name + "\n")

# 使用 GPU（你是 CUDA = YES）
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
print("Using device:", device)

# 加载 ResNet18（ImageNet 预训练权重）
model = models.resnet18(weights=models.ResNet18_Weights.IMAGENET1K_V1)
model.fc = nn.Linear(model.fc.in_features, len(class_names))
model.to(device)

criterion = nn.CrossEntropyLoss()
optimizer = optim.Adam(model.parameters(), lr=LR)

# 训练循环
for epoch in range(EPOCHS):
    model.train()
    epoch_loss = 0

    for images, labels in tqdm(train_loader, desc=f"Epoch {epoch+1}/{EPOCHS}"):
        images, labels = images.to(device), labels.to(device)

        optimizer.zero_grad()
        outputs = model(images)
        loss = criterion(outputs, labels)
        loss.backward()
        optimizer.step()

        epoch_loss += loss.item()

    print(f"Epoch {epoch+1} Loss: {epoch_loss / len(train_loader):.4f}")

    # 验证阶段
    model.eval()
    preds, truths = [], []

    with torch.no_grad():
        for images, labels in val_loader:
            images = images.to(device)
            outputs = model(images)
            pred = outputs.argmax(1).cpu().numpy()

            preds.extend(pred)
            truths.extend(labels.numpy())

    acc = accuracy_score(truths, preds)
    print(f"Val Accuracy: {acc:.4f}")

# 保存模型
torch.save(model.state_dict(), MODEL_PATH)
print("模型已保存：", MODEL_PATH)
