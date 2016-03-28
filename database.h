#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QMap>
QT_BEGIN_NAMESPACE
class QSqlTableModel;
class QItemSelection;
class QSettings;
QT_END_NAMESPACE

class DBModel;

class Database : public QObject //singleton
{	Q_OBJECT
//	Q_DECLARE_TR_FUNCTIONS(Database)	//for translation context "tr()"
public:
	static Database& getInstance() {
		static Database instance;
		return instance;
	}
//	static Database* getInstance() {
//		if(!instance) instance=new Database();
//		return instance;
//	}
//	QSqlDatabase database;
	DBModel *model;
	virtual ~Database();	//should work, I think

	void finalize(void);

	void setupDatabase(const QString &fileName);	//open connection, create tables

	class Settings {
	public:
		Settings();
		void write() const;

		void refreshDatabase(const DBModel *sourceModel);	//can't know model from outside (in constructor)

		QMap<QString,QList<QVariant> > database;
		QMap<QString,QString> db_create;
	private:
		QSettings *myini;
	} settings;

	QColor getMarkColor(const QModelIndex &idx) const;
	void setMarkColor(const QItemSelection &rangesSelected, const QColor &color);
public slots:
	void handleRowsRemoved(const QSet<int> &rowIds);
	void handleColumnsRemoved(const QList<int> &columns);
	void handleColumnsRearranged(const QList<int> &newOrder);
private:
	explicit Database(QObject *parent = 0);
	QSqlDatabase database;
	QSqlTableModel *marks;

	//doesn't submit, writes to marks - make sure marks have no pending changes (relies on it)
	void insertMarkRecord(const QSqlRecord &recordWrite, int markRowCount);

	//writes to marks directly, changes marks->rowCount; doesn't submit
	void updateMarkRecord(const QList<int> &interval, int index);

	//merge marks; can operate with pending changes
	void sanitizeMarks();
public:
	Database(Database const&)=delete;
	void operator=(Database const&)=delete;
//via Scott Meyers: deleted functions should generally
//	be public as it results in better error messages
//	due to the compilers behavior to check accessibility
//	before deleted status
};

#endif // DATABASE_H
