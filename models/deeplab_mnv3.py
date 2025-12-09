# models/deeplab_mnv3.py
import torch.nn as nn
from torchvision.models.segmentation import deeplabv3_mobilenet_v3_large

def build_model(num_classes: int = 3, pretrained: bool = False):
    weights = "DEFAULT" if pretrained else None
    model = deeplabv3_mobilenet_v3_large(weights=weights)

    model.classifier[4] = nn.Conv2d(256, num_classes, kernel_size=1)

    model.aux_classifier = None

    return model