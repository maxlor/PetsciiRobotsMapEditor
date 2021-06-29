#ifndef MULTISIGNALBLOCKER_H
#define MULTISIGNALBLOCKER_H

#include <forward_list>
#include <initializer_list>
class QObject;


class MultiSignalBlocker {
public:
	MultiSignalBlocker(std::initializer_list<QObject*> objects);
	~MultiSignalBlocker();
	
private:
	std::forward_list<QObject*> _objects;
};

#endif // MULTISIGNALBLOCKER_H
