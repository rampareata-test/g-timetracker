#include <QThread>

#include "TimeLogHistory.h"
#include "TimeLogHistoryWorker.h"

TimeLogHistory::TimeLogHistory(QObject *parent) :
    QObject(parent),
    m_thread(new QThread(this)),
    m_worker(new TimeLogHistoryWorker(this)),
    m_size(0),
    m_undoCount(0)
{
    connect(m_worker, SIGNAL(error(QString)),
            this, SIGNAL(error(QString)));
    connect(m_worker, SIGNAL(dataOutdated()),
            this, SIGNAL(dataOutdated()));
    connect(m_worker, SIGNAL(historyRequestCompleted(QVector<TimeLogEntry>,qlonglong)),
            this, SIGNAL(historyRequestCompleted(QVector<TimeLogEntry>,qlonglong)));
    connect(m_worker, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)),
            this, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));
    connect(m_worker, SIGNAL(dataInserted(TimeLogEntry)),
            this, SIGNAL(dataInserted(TimeLogEntry)));
    connect(m_worker, SIGNAL(dataImported(QVector<TimeLogEntry>)),
            this, SIGNAL(dataImported(QVector<TimeLogEntry>)));
    connect(m_worker, SIGNAL(dataRemoved(TimeLogEntry)),
            this, SIGNAL(dataRemoved(TimeLogEntry)));
    connect(m_worker, SIGNAL(sizeChanged(qlonglong)),
            this, SLOT(workerSizeChanged(qlonglong)));
    connect(m_worker, SIGNAL(undoCountChanged(int)),
            this, SLOT(workerUndoCountChanged(int)));
    connect(m_worker, SIGNAL(categoriesChanged(QSet<QString>)),
            this, SLOT(workerCategoriesChanged(QSet<QString>)));
    connect(m_worker, SIGNAL(statsDataAvailable(QVector<TimeLogStats>,QDateTime)),
            this, SIGNAL(statsDataAvailable(QVector<TimeLogStats>,QDateTime)));
    connect(m_worker, SIGNAL(syncDataAvailable(QVector<TimeLogSyncData>,QDateTime)),
            this, SIGNAL(syncDataAvailable(QVector<TimeLogSyncData>,QDateTime)));
    connect(m_worker, SIGNAL(syncStatsAvailable(QVector<TimeLogSyncData>,QVector<TimeLogSyncData>,
                                                QVector<TimeLogSyncData>,QVector<TimeLogSyncData>,
                                                QVector<TimeLogSyncData>,QVector<TimeLogSyncData>)),
            this, SIGNAL(syncStatsAvailable(QVector<TimeLogSyncData>,QVector<TimeLogSyncData>,
                                            QVector<TimeLogSyncData>,QVector<TimeLogSyncData>,
                                            QVector<TimeLogSyncData>,QVector<TimeLogSyncData>)));
    connect(m_worker, SIGNAL(dataSynced(QVector<TimeLogSyncData>,QVector<TimeLogSyncData>)),
            this, SIGNAL(dataSynced(QVector<TimeLogSyncData>,QVector<TimeLogSyncData>)));
}

TimeLogHistory::~TimeLogHistory()
{
    if (m_thread->isRunning()) {
        m_thread->quit();
    }
}

bool TimeLogHistory::init(const QString &dataPath)
{
    if (m_worker->thread() != thread()) {
        return false;
    }

    if (!m_worker->init(dataPath)) {
        return false;
    }

    makeAsync();

    return true;
}

qlonglong TimeLogHistory::size() const
{
    return m_size;
}

QSet<QString> TimeLogHistory::categories() const
{
    return m_categories;
}

int TimeLogHistory::undoCount() const
{
    return m_undoCount;
}

void TimeLogHistory::insert(const TimeLogEntry &data)
{
    QMetaObject::invokeMethod(m_worker, "insert", Qt::AutoConnection, Q_ARG(TimeLogEntry, data));
}

void TimeLogHistory::import(const QVector<TimeLogEntry> &data)
{
    QMetaObject::invokeMethod(m_worker, "import", Qt::AutoConnection,
                              Q_ARG(QVector<TimeLogEntry>, data));
}

void TimeLogHistory::remove(const TimeLogEntry &data)
{
    QMetaObject::invokeMethod(m_worker, "remove", Qt::AutoConnection, Q_ARG(TimeLogEntry, data));
}

void TimeLogHistory::edit(const TimeLogEntry &data, TimeLogHistory::Fields fields)
{
    QMetaObject::invokeMethod(m_worker, "edit", Qt::AutoConnection, Q_ARG(TimeLogEntry, data),
                              Q_ARG(TimeLogHistory::Fields, fields));
}

void TimeLogHistory::editCategory(QString oldName, QString newName)
{
    QMetaObject::invokeMethod(m_worker, "editCategory", Qt::AutoConnection,
                              Q_ARG(QString, oldName), Q_ARG(QString, newName));
}

void TimeLogHistory::sync(const QVector<TimeLogSyncData> &updatedData, const QVector<TimeLogSyncData> &removedData)
{
    QMetaObject::invokeMethod(m_worker, "sync", Qt::AutoConnection,
                              Q_ARG(QVector<TimeLogSyncData>, updatedData),
                              Q_ARG(QVector<TimeLogSyncData>, removedData));
}

void TimeLogHistory::undo()
{
    QMetaObject::invokeMethod(m_worker, "undo", Qt::AutoConnection);
}

void TimeLogHistory::getHistoryBetween(qlonglong id, const QDateTime &begin, const QDateTime &end, const QString &category) const
{
    QMetaObject::invokeMethod(m_worker, "getHistoryBetween", Qt::AutoConnection, Q_ARG(qlonglong, id),
                              Q_ARG(QDateTime, begin), Q_ARG(QDateTime, end), Q_ARG(QString, category));
}

void TimeLogHistory::getHistoryAfter(qlonglong id, const uint limit, const QDateTime &from) const
{
    QMetaObject::invokeMethod(m_worker, "getHistoryAfter", Qt::AutoConnection, Q_ARG(qlonglong, id),
                              Q_ARG(uint, limit), Q_ARG(QDateTime, from));
}

void TimeLogHistory::getHistoryBefore(qlonglong id, const uint limit, const QDateTime &until) const
{
    QMetaObject::invokeMethod(m_worker, "getHistoryBefore", Qt::AutoConnection, Q_ARG(qlonglong, id),
                              Q_ARG(uint, limit), Q_ARG(QDateTime, until));
}

void TimeLogHistory::getStats(const QDateTime &begin, const QDateTime &end, const QString &category, const QString &separator) const
{
    QMetaObject::invokeMethod(m_worker, "getStats", Qt::AutoConnection,
                              Q_ARG(QDateTime, begin), Q_ARG(QDateTime, end),
                              Q_ARG(QString, category), Q_ARG(QString, separator));
}

void TimeLogHistory::getSyncData(const QDateTime &mBegin, const QDateTime &mEnd) const
{
    QMetaObject::invokeMethod(m_worker, "getSyncData", Qt::AutoConnection,
                              Q_ARG(QDateTime, mBegin), Q_ARG(QDateTime, mEnd));
}

void TimeLogHistory::workerSizeChanged(qlonglong size)
{
    m_size = size;
}

void TimeLogHistory::workerCategoriesChanged(QSet<QString> categories)
{
    m_categories.swap(categories);

    emit categoriesChanged(m_categories);
}

void TimeLogHistory::workerUndoCountChanged(int undoCount)
{
    if (m_undoCount == undoCount) {
        return;
    }

    m_undoCount = undoCount;

    emit undoCountChanged(m_undoCount);
}

void TimeLogHistory::makeAsync()
{
    if (m_worker->thread() != thread()) {
        return;
    }

    m_thread->setParent(0);
    m_worker->setParent(0);

    connect(m_thread, SIGNAL(finished()), m_worker, SLOT(deleteLater()));
    connect(m_worker, SIGNAL(destroyed()), m_thread, SLOT(deleteLater()));

    m_worker->moveToThread(m_thread);

    m_thread->start();
}
