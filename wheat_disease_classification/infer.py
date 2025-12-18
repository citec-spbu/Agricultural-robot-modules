import sys
import torch
from torchvision import models, transforms
from PIL import Image

# 读取类别
with open("class_names.txt", "r") as f:
    CLASS_NAMES = [line.strip() for line in f.readlines()]

def load_model(model_path="resnet18_wheat.pth"):
    model = models.resnet18(weights=None)
    model.fc = torch.nn.Linear(model.fc.in_features, len(CLASS_NAMES))
    model.load_state_dict(torch.load(model_path, map_location="cpu"))
    model.eval()
    return model

def predict(image_path):
    model = load_model()

    tfms = transforms.Compose([
        transforms.Resize((224, 224)),
        transforms.ToTensor()
    ])

    img = Image.open(image_path).convert("RGB")
    img = tfms(img).unsqueeze(0)

    with torch.no_grad():
        outputs = model(img)
        pred = outputs.argmax(1).item()

    return CLASS_NAMES[pred]


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python infer.py <image_path>")
        exit()

    image_path = sys.argv[1]
    result = predict(image_path)
    print("Prediction:", result)
