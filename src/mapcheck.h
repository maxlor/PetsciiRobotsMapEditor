#ifndef MAPCHECK_H
#define MAPCHECK_H

#include <QRect>
#include <QString>
#include <functional>
#include <vector>
#include "mapobject.h"

class MapController;
class Tileset;


class MapCheck {
public:
	enum class Severity {
		Silent, Info, Warning, Error
	};
	
	struct Problem {
		Severity severity;
		MapObject::id_t objectId;
		QString text;
		std::function<void()> fix;
	};
	
	MapCheck(MapController &mapController, const Tileset &tileset);
	
	void check();
	void fixAll();
	void fixSilent();
	
	const std::vector<Problem> &problems() const;
	const std::vector<Problem> &silentProblems() const;
	
private:
	enum Attribute { A, B, C, D, X, Y, HEALTH };
	void silent(MapObject::id_t id, const QString &text, std::function<void()> fix = nullptr);
	void info(MapObject::id_t id, const QString &text, std::function<void()> fix = nullptr);
	void warn(MapObject::id_t id, const QString &text, std::function<void()> fix = nullptr);
	void error(MapObject::id_t id, const QString &text, std::function<void()> fix = nullptr);
	void addProblem(Severity severity, MapObject::id_t id, const QString &text, std::function<void()> fix);
	
	void checkPlayerExists();
	void checkPlayerInBounds();
	void checkUnitTypes();
	void checkPlayerAndRobotsHealthABCD();
	void checkPlayerAndRobotsOnWalkable();
	void checkPositionBounds();
	void checkTransporterPads();
	void checkDoors();
	void checkTrashCompactors();
	void checkWaterRafts();
	void checkKeys();
	void checkWeapons();
	void checkSearchAreas();
	
	void checkUnused(MapObject::id_t id, bool a, bool b, bool c, bool d, bool health);
	
	QRect walkableRect() const;
	
	std::function<void()> setAttribute(MapObject::id_t id, Attribute attribute, uint8_t value);
	
	MapController &_mapController;
	const Tileset &_tileset;
	std::vector<Problem> _problems;
	std::vector<Problem> _silentProblems;
};

#endif // MAPCHECK_H
