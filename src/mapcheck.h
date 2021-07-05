#ifndef MAPCHECK_H
#define MAPCHECK_H

#include <QRect>
#include <QString>
#include <functional>
#include <list>
#include "mapobject.h"

class MapController;


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
	
	MapCheck(MapController &mapController);
	
	const std::list<Problem> problems() const;
	
private:
	enum Attribute { A, B, C, D, HEALTH };
	void silent(MapObject::id_t id, const QString &text, std::function<void()> fix = nullptr);
	void info(MapObject::id_t id, const QString &text, std::function<void()> fix = nullptr);
	void warn(MapObject::id_t id, const QString &text, std::function<void()> fix = nullptr);
	void error(MapObject::id_t id, const QString &text, std::function<void()> fix = nullptr);
	void addProblem(Severity severity, MapObject::id_t id, const QString &text, std::function<void()> fix);
	
	void checkPlayerExists();
	void checkPlayerInBounds();
	void checkUnitTypes();
	void checkHealth();
	void checkDoors();
	
	QRect walkableRect() const;
	
	std::function<void()> setAttribute(MapObject::id_t id, Attribute attribute, uint8_t value);
	
	MapController &_mapController;
	std::list<Problem> _problems;
};

#endif // MAPCHECK_H
