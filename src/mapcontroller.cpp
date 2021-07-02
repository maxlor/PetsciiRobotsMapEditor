#include "mapcontroller.h"
#include <QAction>
#include <QKeySequence>
#include "map.h"
#include "mapcommands.h"
#include "mapobject.h"


/** @class MapController
 * The controller in the map MVC tuple.
 * 
 * This class is responsible for all modifications of the map. It provides
 * methods to load and save the map, to modify the map's tiles and objects,
 * and has undo/redo capabilities.
 * 
 * #### Object Numbers ####
 * 
 * Attack of the PETSCII Robots has a fixed number of slots for objects, and
 * different slot ranges may only contain certain types of objects. Object
 * numbers (generally called \a objectNo) refer to this slot number.
 * 
 * Valid objects have numbers between #MapObject::IdMin and #MapObject::IdMax (inclusive).
 * #MapObject::IdNone is used to designate no object, or failure of an operation.
 * The range of valid object numbers comprises these groups:
 * 
 * * #MapObject::IdPlayer
 *   : the player object
 * * #MapObject::IdRobotMin – #MapObject::IdRobotMax
 *   : for the enemy robots
 * * Reserved objects for weapons fire
 * * #MapObject::IdMapFeatureMin – #MapObject::IdMapFeatureMax
 *   : for map features, i.e. doors, trash compactors, elevators and water rafts
 * * #MapObject::IdHiddenMin – #MapObject::IdHiddenMax
 *   : for hidden objects, i.e. keys, time bombs, EMPs, pistols, plasma guns,
 *     medkits and magnets
 */


MapController::MapController(QObject *parent) : QObject(parent) {
	_map = new Map(this);
	_undoStack.setUndoLimit(64);
}

/** Get a pointer to the map. Only use its const methods! */
const Map *MapController::map() {
	return _map;
}


/// @{
/** The undo/redo actions, for inclusion in a tool bar or menu. */
QAction *MapController::redoAction() {
	if (_redoAction == nullptr) {
		_redoAction = _undoStack.createRedoAction(this);
		_redoAction->setShortcut(QKeySequence::Redo);
		_redoAction->setIcon(QIcon(":/redo.svg"));
	}
	return _redoAction;
}


QAction *MapController::undoAction() {
	if (_undoAction == nullptr) {
		_undoAction = _undoStack.createUndoAction(this);
		_undoAction->setShortcut(QKeySequence::Undo);
		_undoAction->setIcon(QIcon(":/undo.svg"));
	}
	return _undoAction;
}
/// @}


/** Delete everything on the map, and clear the undo/redo history. */
void MapController::clear() {
	_map->clear();
	_undoStack.clear();
}


/// @{
/** Load/save the map from/to a file.
 * @param path the path to the map file
 * @return a null string on success, an error description otherwise
 */
QString MapController::load(const QString &path) {
	QString result = _map->load(path);
	if (result.isNull()) {
		_undoStack.clear();
	}
	return result;
}


QString MapController::save(const QString &path) {
	QString result = _map->save(path);
	if (result.isNull()) {
		_undoStack.setClean();
	}
	return result;
}
/// @}


/// @{
/** Delete the object in slot \a objectNo. */
void MapController::deleteObject(int objectNo) {
	Q_ASSERT(MapObject::IdMin <= objectNo and objectNo <= MapObject::IdMax);
	_undoStack.push(new MapCommands::DeleteObject(*_map, objectNo));
}


/** Move an object to a new position.
 * 
 * When calling this method several times in a row for the same object,
 * the moves are merged into a single undo action by default. To interrupt
 * the merging, call #incrementMergeCounter() after each group of moves.
 */
void MapController::moveObject(int objectNo, const QPoint &position) {
	_undoStack.push(new MapCommands::MoveObject(*_map, objectNo, position, _mergeCounter));
}


/**
 * Places the new \a object into the map.
 * 
 * If the object cannot be placed, #MapObject::IdNone is returned and \a error
 * is filled with an explanation, unless it is \c nullptr.
 * 
 * @param object the object to place
 * @param[out] error an error explanation will be stored here if not \c nullptr
 * @return the id of the created object, or #MapObject::IdNone on error
 */
int MapController::newObject(const MapObject &object, QString *error) {
	const MapObject::id_t id = _map->nextAvailableObjectId(object.group());
	if (id == MapObject::IdNone) {
		if (error) {
			const QString &groupS = MapObject::toString(object.group());
			*error = QString("the map already contains the maximum number of " + groupS);
			return MapObject::IdNone;
		}
	}
	
	_undoStack.push(new MapCommands::NewObject(*_map, id, object));
	
	return id;
}


/**
 * Put \a object into slot \a objectNo.
 * 
 * \a isNew only influences how this action is shown in the undo stack.
 */
void MapController::setObject(int objectNo, const MapObject &object, bool isNew) {
	if (isNew) {
		_undoStack.push(new MapCommands::NewObject(*_map, objectNo, object));
	} else {
		_undoStack.push(new MapCommands::ModifyObject(*_map, objectNo, object));
	}
}


/** Create an undo action merge barrier. @see #moveObject(). */
void MapController::incrementMergeCounter() {
	++_mergeCounter;
}
/// @}


/// @{
/** Call this before a group of related #setTile() calls. */
void MapController::beginUndoGroup() {
	_undoStack.beginMacro("Set Tiles");
}


/** Call this after a group of related #setTile() calls. */
void MapController::endUndoGroup() {
	_undoStack.endMacro();
}


/** Flood fill the map.
 * Flood fills the map with the tile \a tileNo, starting at the given
 * \a position. This works like in a painting program.
 */
void MapController::floodFill(const QPoint &position, uint8_t tileNo) {
	_undoStack.push(new MapCommands::FloodFill(*_map, position, tileNo));
}


/** Set a tile.
 * Change the map tile at the given \a position to \a tileNo.
 * 
 * To group several such calls into a single undo action, call
 * #beginUndoGroup() and #endUndoGroup before and after the #setTile()
 * calls that are to be grouped. Creating such a group is not required
 * however.
 */
void MapController::setTile(const QPoint &position, uint8_t tileNo) {
	_undoStack.push(new MapCommands::SetTile(*_map, position, tileNo));
}
/// @}
