#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QColor>
#include <QImage>

#define MAP_BYTES (8962)
#define MAP_WIDTH (128)
#define MAP_HEIGHT (64)
#define TILE_WIDTH (3)
#define TILE_HEIGHT (3)
#define GLYPH_WIDTH (8)
#define GLYPH_HEIGHT (8)
#define TILE_COUNT (256)

#define PLAYER_OBJ (0)
#define UNITTYPE_NONE (0)
#define UNITTYPE_PLAYER (1)
#define OBJECT_NONE (-1)
#define OBJECT_MIN (0)
#define OBJECT_MAX (63)
#define ROBOT_MIN (1)
#define ROBOT_MAX (28)
#define ROBOT_HOVERBOT_LR (2)
#define ROBOT_HOVERBOT_UD (3)
#define ROBOT_HOVERBOT_ATTACK (4)
#define ROBOT_EVILBOT (9)
#define ROBOT_ROLLERBOT_UD (17)
#define ROBOT_ROLLERBOT_LR (18)
#define MAP_FEATURE_MIN (32)
#define MAP_FEATURE_MAX (47)
#define HIDDEN_OBJECT_MIN (48)
#define HIDDEN_OBJECT_MAX (63)
#define OBJECT_TRANSPORTER (7)
#define OBJECT_DOOR (10)
#define OBJECT_TRASH_COMPACTOR (16)
#define OBJECT_ELEVATOR (19)
#define OBJECT_WATER_RAFT (22)
#define OBJECT_KEY (128)
#define OBJECT_TIME_BOMB (129)
#define OBJECT_EMP (130)
#define OBJECT_PISTOL (131)
#define OBJECT_PLASMA_GUN (132)
#define OBJECT_MEDKIT (133)
#define OBJECT_MAGNET (134)

extern const QImage::Format IMAGE_FORMAT;

namespace C {
extern const QColor colorWalkable;
extern const QColor colorHoverable;
extern const QColor colorMovable;
extern const QColor colorDestructible;
extern const QColor colorShootThrough;
extern const QColor colorPushOnto;
extern const QColor colorSearchable;
extern const QColor colorDarken;
extern const QColor colorGrid;
extern const QColor colorAreaSelection;
extern const QColor colorTileSelection;
}

#endif // CONSTANTS_H
