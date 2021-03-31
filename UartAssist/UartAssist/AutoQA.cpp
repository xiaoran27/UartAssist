#include "AutoQA.h"

int AutoQA::append(QStringList qlist, QStringList alist, int sleepMs)
{
	//hex:
	QByteArrayList lstrsp;
	for each (QString e in alist)
	{
		QString val = e.mid(1);
		if (val.trimmed().startsWith("hex:", Qt::CaseInsensitive)) {
			val = val.trimmed().mid(4).replace(" ", "");
		}
		if (val.length() < 1) continue;
		if ( msgtype == MessageType::STRING) {
			lstrsp.append(e.length()-val.length()>1? QByteArray::fromHex(val.toUtf8()):val.toUtf8());
		}
		else {
			lstrsp.append(QByteArray::fromHex(val.toUtf8()));
		}
	}

	for each (QString e in qlist)
	{
		QString val = e.mid(1);
		if (val.trimmed().startsWith("hex:", Qt::CaseInsensitive)) {
			val = val.trimmed().mid(4).replace(" ", "");
		}
		if (msgtype == MessageType::STRING) {
			autoqaMap->insert((e.length() - val.length() > 1 ? QByteArray::fromHex(val.toUtf8()) : val.toUtf8()), lstrsp);
		}
		else {
			autoqaMap->insert(QByteArray::fromHex(val.toUtf8()), lstrsp);
		}
	}

	return autoqaMap->size();
}

int AutoQA::append(QStringList qalist, int sleepMs)
{
	return append(qalist, qalist, sleepMs);
}

int AutoQA::append(QString same, int sleepMs)
{
	return append({ same }, { same }, sleepMs);
}

//为保证qalist的有序，故分拆其到qlist和alist中, 所有忽略qalist的值.
int AutoQA::judge(QStringList qlist, QStringList alist, QStringList qalist, int sleepMs)
{
	if ( alist.isEmpty()) return 0;

	int count = 0;
	int flag = 0x000;  //q|a|qa
	switch (qlist.size())
	{
	case 0:
		flag = flag | 0x000;
		break;
	case 1:
		flag = flag | 0x100;
		break;
	default:
		flag = flag | 0xF00;
		break;
	}
	switch (alist.size())
	{
	case 0:
		flag = flag | 0x000;
		break;
	case 1:
		flag = flag | 0x010;
		break;
	default:
		flag = flag | 0x0F0;
		break;
	}
	switch (qalist.size())
	{
	case 0:
		flag = flag | 0x000;
		break;
	case 1:
		flag = flag | 0x001;
		break;
	default:
		flag = flag | 0x00F;
		break;
	}

	if (qlist.isEmpty()) {
		qlist << ">";
	}

	//[01f{3}
	switch (flag)
	{
	case 0x000: {
		//NA
		break;
	}case 0x001: {
		//count += append(qalist); =>
		count += append(qlist, alist);
		break;
	}case 0x00F: {
		//=>001
		break;
	}case 0x010: {
		count += append(qlist, alist);
		break;
	}case 0x011: {
		count += append(qlist, alist);
		//count += append(qalist);  //ignore
		break;
	}case 0x01F: {
		//=>011
		break;
	}case 0x0F0: {
		count += append(qlist, alist);
		break;
	}case 0x0F1: {
		count += append(qlist, alist);
		//count += append(qalist);  //ignore
		break;
	}case 0x0FF: {
		//=>0n1
		break;
	}
	case 0x100: {
		//NA
		break;
	}case 0x101: {
		count += append(qlist, alist);
		//count += append(qalist);  //ignore
		break;
	}case 0x10F: {
		//=>101
		break;
	}case 0x110: {
		count += append(qlist, alist);
		break;
	}case 0x111: {
		count += append(qlist, alist);
		//count += append(qalist);  //ignore
		break;
	}case 0x11F: {
		//=>111
		break;
	}case 0x1F0: {
		count += append(qlist, alist);
		break;
	}case 0x1F1: {
		count += append(qlist, alist);
		//count += append(qalist);  //ignore
		break;
	}case 0x1FF: {
		//=>1n1
		break;
	}
	case 0xF00: {
		//NA
		break;
	}case 0xF01: {
		count += append(qlist, alist);
		//count += append(qalist);  //ignore
		break;
	}case 0xF0F: {
		//=>n01
		break;
	}case 0xF10: {
		count += append(qlist, alist);
		break;
	}case 0xF11: {
		count += append(qlist, alist);
		//count += append(qalist); //ignore
		break;
	}case 0xF1F: {
		//=>n11
		break;
	}case 0xFF0: {
		count += append(qlist, alist);
		break;
	}case 0xFF1: {
		count += append(qlist, alist);
		//count += append(qalist);  //ignore
		break;
	}case 0xFFF: {
		//=>nn1
		break;
	}

	}

	return count;
}

void AutoQA::load(QString filename)
{
	if (filename.isEmpty()) return;

	QRegularExpression tsreg("^[tTsS]([0-9]+)");
	QRegularExpressionMatch tsmatch;
	QFile qafile(filename);
	bool ok = qafile.open(QFile::ReadOnly | QFile::Text);
	if (!ok) return;

	QTextStream qa(&qafile);
	QString line;
	QStringList qlist, alist;
	int count = 0;
	autoqaMap->clear();
	while (!qa.atEnd())//逐行读取文本，并去除每行的回车
	{
		line = qa.readLine().trimmed();
		qDebug() << tr("line=%1").arg(line);

		//line.remove('\n');
		//line.remove('\r');
		if (line.length() <= 1 || '#' == line.at(0).toLatin1() ) {
			qWarning() << tr("IGNORE: '%1'!").arg(line);
			continue;
		}

		switch (line.at(0).toLatin1()) {
		case '>':
		{
			if (!alist.isEmpty()) { //有<或=应答

				if (qlist.isEmpty()) {
					qlist << ">";
				}
				count = append(qlist, alist, sleepMs);
				qlist.clear();
				alist.clear();
			}

			qlist << line;
			break;
		}
		case '<':
		{
			alist << line;
			break;
		}
		case '=':
		{
			if (!alist.isEmpty()) { //有<或=应答
				if (qlist.isEmpty()) {
					qlist << ">";
				}
				count = append(qlist, alist, sleepMs);
				qlist.clear();
				alist.clear();
			}

			//为了保证顺序，qa分拆到q和a中
			qlist << line;
			alist << line;
			break;
		}
		case 's':
		case 'S':
		{
			tsmatch = tsreg.match(line);
			if (!tsmatch.capturedTexts().isEmpty()) {
				sleepMs = tsmatch.captured(1).toInt();
			}
			else {
				qWarning() << tr("IGNORE: '%1' not match '%2'!").arg(line).arg(tsreg.pattern()) ;
			}
			break;
		}
		case 't':
		case 'T':
		{
			tsmatch = tsreg.match(line);
			if (!tsmatch.capturedTexts().isEmpty()) {
				if (tsmatch.captured(1).toInt() == 0) {
					msgtype = MessageType::STRING;
				}
				else {
					msgtype = MessageType::HEX;
				}
			}
			else {
				qWarning() << tr("IGNORE: '%1' not match '%2'!").arg(line).arg(tsreg.pattern());
			}
			break;
		}
		default:
		{
			qWarning() << tr("IGNORE: '%1'!").arg(line);
			break;
		}
		}

	}  //end while
	qafile.close();

	if (!alist.isEmpty()) { //有<或=应答

		if (qlist.isEmpty()) { 
			qlist << ">";
		}
		count = append(qlist, alist);
		qlist.clear();
		alist.clear();
	}
	
}

void AutoQA::loadX(QString filename)
{
	if (filename.isEmpty()) return;

	QRegularExpression tsreg("^[tTsS]([0-9]+)");
	QRegularExpressionMatch tsmatch;
	QFile qafile(filename);
	bool ok = qafile.open(QFile::ReadOnly | QFile::Text);
	if (!ok) return;

	QTextStream qaStream(&qafile);
	QString line, latestSleepLine="sleep 100";
	QStringList qlist, alist,qalist;
	int count = 0;
	autoqaMap->clear();
	while (!qaStream.atEnd())//逐行读取文本，并去除每行的回车
	{
		line = qaStream.readLine().trimmed();
		qDebug() << tr("line=%1").arg(line);

		//line.remove('\n');
		//line.remove('\r');
		if (line.length() < 1) {
			continue;
		}

		//TODO: [01F]{3}
		switch (line.at(0).toLatin1()) {
		case '>':
		{
			if (!alist.isEmpty() || !qalist.isEmpty()) { //有<或=应答
				count = judge(qlist, alist, qalist, sleepMs);
				qlist.clear();
				alist.clear();
				qalist.clear();
			}

			qlist << line;
			break;
		}
		case '<':
		{
			alist << line;
			break;
		}
		case '=':
		{			
			if (!qalist.isEmpty() ) { //有=应答
				count = judge(qlist, alist, qalist, sleepMs);
				qlist.clear();
				alist.clear();
				qalist.clear();
			}

			//为了保证顺序，qa分拆到q和a中
			qlist << line;
			alist << line;
			qalist << line;
			break;
		}
		case 's':
		case 'S':
		{
			tsmatch = tsreg.match(line);
			if (!tsmatch.capturedTexts().isEmpty()) {
				sleepMs = tsmatch.captured(1).toInt();
			}
			else {
				qWarning() << tr("IGNORE: '%1' not match '%2'!").arg(line).arg(tsreg.pattern());
			}
			break;
		}
		case 't':
		case 'T':
		{
			tsmatch = tsreg.match(line);
			if (!tsmatch.capturedTexts().isEmpty()) {
				if (tsmatch.captured(1).toInt() == 0) {
					msgtype = MessageType::STRING;
				}
				else {
					msgtype = MessageType::HEX;
				}
			}
			else {
				qWarning() << tr("IGNORE: '%1' not match '%2'!").arg(line).arg(tsreg.pattern());
			}
			break;
		}
		default:
		{
			// # 
			break;
		}
		} // end switch

		//judge
		count = judge(qlist, alist, qalist, sleepMs);

	}  //end while
	qafile.close();

	count = judge(qlist, alist, qalist, sleepMs);

}


void AutoQA::reset()
{
	autoqaMap->clear();
}


QByteArrayList AutoQA::getAnwser(QByteArray q) {
	if (q.isEmpty()) return autoqaMap->value(QString("").toUtf8(), {});

	QByteArray key = QString("").toUtf8();
	QByteArrayList keys = autoqaMap->keys();
	for each (QByteArray e in keys)
	{
		int idx = q.indexOf(e);
		if ( idx >= 0 && e.size() > key.size()) {
			key = e;  //最大左匹配
		}
	}

	if (key.size() < 1) {
		return {};
	}
	else {
		return autoqaMap->value(key, {});
	}
}

int AutoQA::getSleep() {
	return sleepMs;
}

AutoQA::MessageType AutoQA::getMessageType() {
	return msgtype;
}
