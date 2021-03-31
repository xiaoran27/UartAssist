#ifndef AUTOQA_H
#define AUTOQA_H

#include <qobject.h>
#include <qmap.h>
#include <qfile.h>
#include <qregularexpression.h>
#include <qtextstream.h>
#include <qloggingcategory.h>

class AutoQA :
    public QObject
{


public:
	enum MessageType { STRING, HEX };
	void load( QString filename = "autoqa.txt" );
	void reset();
	void loadX(QString filename = "autoqa.txt");
	QByteArrayList getAnwser(QByteArray q);
	int getSleep();
	MessageType getMessageType();

private:
	int sleepMs = 100;
	MessageType msgtype = MessageType::HEX;
	QMap<QByteArray, QByteArrayList>* autoqaMap = new QMap<QByteArray, QByteArrayList>();
	int append(QStringList qlist, QStringList alist, int sleepMs = 100);
	int append(QStringList qalist, int sleepMs = 100);
	int append(QString same, int sleepMs = 100);
	int judge(QStringList qlist, QStringList alist, QStringList qalist, int sleepMs = 100);


};

#endif
