#include "constants.h"


const QImage::Format IMAGE_FORMAT = QImage::Format_ARGB32_Premultiplied;

namespace C {
const QColor colorWalkable(0, 255, 0, 128);
const QColor colorHoverable(255, 255, 0, 128);
const QColor colorMovable(128, 128, 255, 128);
const QColor colorDestructible(255, 0, 0, 128);
const QColor colorShootThrough(255, 128, 0, 128);
const QColor colorPushOnto(0, 255, 255, 128);
const QColor colorSearchable(255, 0, 255, 128);
const QColor colorDarken(128, 128, 128, 128);
}
