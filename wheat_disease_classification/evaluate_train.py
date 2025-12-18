import os
import torch
import numpy as np
from torchvision import models, transforms, datasets
from sklearn.metrics import accuracy_score, precision_recall_fscore_support, confusion_matrix
from tqdm import tqdm

MODEL_PATH = "resnet18_wheat.pth"
DATA_DIR = "data/train"  # 这里改成 train
IMG_SIZE = 224

# 读取类别名称
with open("class_names.txt", "r") as f:
    CLASS_NAMES = [line.strip() for line in f.readlines()]

def load_model():
    model = models.resnet18(weights=None)
    model.fc = torch.nn.Linear(model.fc.in_features, len(CLASS_NAMES))
    model.load_state_dict(torch.load(MODEL_PATH, map_location="cpu"))
    model.eval()
    return model

transform = transforms.Compose([
    transforms.Resize((IMG_SIZE, IMG_SIZE)),
    transforms.ToTensor()
])

dataset = datasets.ImageFolder(DATA_DIR, transform=transform)
loader = torch.utils.data.DataLoader(dataset, batch_size=32, shuffle=False)

model = load_model()
preds = []
labels = []

with torch.no_grad():
    for images, lbls in tqdm(loader, desc="Evaluating TRAIN"):
        outputs = model(images)
        pred = outputs.argmax(1).numpy()
        preds.extend(pred)
        labels.extend(lbls.numpy())

preds = np.array(preds)
labels = np.array(labels)

acc = accuracy_score(labels, preds)
precision, recall, f1, _ = precision_recall_fscore_support(labels, preds, average='weighted')
cm = confusion_matrix(labels, preds)

print("\n========== TRAIN METRICS ==========")
print(f"Accuracy:  {acc:.4f}")
print(f"Precision: {precision:.4f}")
print(f"Recall:    {recall:.4f}")
print(f"F1-score:  {f1:.4f}")
print("===================================\n")

print("Confusion Matrix (TRAIN):")
print(cm)
print("\nClass order:", CLASS_NAMES)
