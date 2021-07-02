#include "util.h"
#include <QStringList>


QString capitalize(const QString &s) {
	QStringList words = s.split(' ');
	for (QString &word : words) {
		if (word.length() > 1) {
			word = word.at(0).toUpper() + word.mid(1);
		} else if (word.length() > 0) {
			word = word.toUpper();
		}
	}
	return words.join(' ');
}
