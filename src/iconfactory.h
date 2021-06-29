#ifndef ICONFACTORY_H
#define ICONFACTORY_H

#include <forward_list>
#include <unordered_map>
#include <QIcon>
#include <QString>
#include <string>

class QTemporaryFile;


class IconFactory {
public:
	IconFactory();
	~IconFactory();
	
	const QIcon &icon(const QString text);
	
private:
	QTemporaryFile *tempFile();
	
	std::forward_list<QTemporaryFile*> _tempFiles;
	std::unordered_map<std::string, QIcon> _cache;
};

#endif // ICONFACTORY_H
