#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QColor>

#define MAP_WIDTH (128)
#define MAP_HEIGHT (64)
#define TILE_WIDTH (3)
#define TILE_HEIGHT (3)
#define GLYPH_WIDTH (8)
#define GLYPH_HEIGHT (8)

#define OBJECTS_MAX (63)

#define PLAYER_OBJ (0)
#define OBJECT_MIN (1)
#define OBJECT_MAX (63)
#define ROBOT_MIN (1)
#define ROBOT_MAX (28)
#define ROBOT_HOVERBOT_LR (2)
#define ROBOT_HOVERBOT_UD (3)
#define ROBOT_HOVERBOT_ATTACK (4)
#define ROBOT_EVILBOT (9)
#define ROBOT_ROLLERBOT_UD (17)
#define ROBOT_ROLLERBOT_LR (18)
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

namespace C {
extern const QColor colorWalkable;
extern const QColor colorHoverable;
extern const QColor colorMovable;
extern const QColor colorDestructible;
extern const QColor colorShootThrough;
extern const QColor colorPushOnto;
extern const QColor colorSearchable;
extern const QColor colorDarken;
}

#endif // CONSTANTS_H