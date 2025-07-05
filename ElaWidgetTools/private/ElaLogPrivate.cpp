#include "ElaLogPrivate.h"

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QMutex>
#ifndef QT_NO_DEBUG
#include <iostream>
#endif
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextStream>
#endif
#include "ElaLog.h"
Q_GLOBAL_STATIC(QMutex, messageLogMutex)
Q_GLOBAL_STATIC(QString, logFileNameTime)

#ifdef QT_MESSAGELOGCONTEXT
  #define LOG_DEBUG "[信息-%1](函数: %2 , 行数: %3) -> %4"
  #define LOG_WARING "[警告-%1](函数: %2 , 行数: %3) -> %4"
  #define LOG_CRITICAL "[错误-%1](函数: %2 , 行数: %3) -> %4"
#define DEBUG logInfo = QString(LOG_DEBUG).arg(logTime, ctx.function, QString::number(ctx.line), msg);
#define WARING logInfo = QString(LOG_DEBUG).arg(logTime, ctx.function, QString::number(ctx.line), msg);
#define  CRITICAL logInfo = QString(LOG_DEBUG).arg(logTime, ctx.function, QString::number(ctx.line), msg);

#else
#define LOG_DEBUG "[信息]-> %1"
#define LOG_WARING "[警告]-> %1"
#define LOG_CRITICAL "[错误]-> %1"
#define DEBUG logInfo = QString(LOG_DEBUG).arg( msg);
#define WARING logInfo = QString(LOG_DEBUG).arg( msg);
#define  CRITICAL logInfo = QString(LOG_DEBUG).arg( msg);
#endif
ElaLogPrivate::ElaLogPrivate(QObject *parent)
    : QObject{parent} {
}

ElaLogPrivate::~ElaLogPrivate() {
}

void ElaLogPrivate::_messageLogHander(QtMsgType type, const QMessageLogContext &ctx, const QString &msg) {
    if (type > QtCriticalMsg) {
        return;
    }
    QString logInfo;
    QString logTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    switch (type) {
        case QtDebugMsg: {
            DEBUG
            break;
        }
        case QtWarningMsg: {
            WARING
            break;
        }
        case QtCriticalMsg: {
            CRITICAL
            break;
        }
        default: {
            qCritical("发生致命错误！");
            break;
        }
    }
    qDebug() << logInfo;
    ElaLog *log = ElaLog::getInstance();
    Q_EMIT log->logMessage(logInfo);
    messageLogMutex->lock();
    QFile logfile;
    if (log->getIsLogFileNameWithTime()) {
        logfile.setFileName(log->getLogSavePath() + "\\" + log->getLogFileName() + *logFileNameTime + ".txt");
    } else {
        logfile.setFileName(log->getLogSavePath() + "\\" + log->getLogFileName() + ".txt");
    }
    if (logfile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream logFileStream(&logfile);
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        logFileStream << logInfo << Qt::endl;
#else
        logFileStream << logInfo << endl;
#endif
        logfile.close();
    }
    messageLogMutex->unlock();
}

void ElaLogPrivate::_clearLogFile() {
    if (_pIsLogFileNameWithTime) {
        QString logTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss");
        logTime.prepend("_");
        logTime.replace(" ", "_");
        logFileNameTime->clear();
        logFileNameTime->append(logTime);
    } else {
        QFile file(_pLogSavePath + "\\" + _pLogFileName + ".txt");
        if (file.exists()) {
            if (file.open(QIODevice::WriteOnly | QIODevice::Text | QFile::Truncate)) {
                file.close();
            }
        }
    }
}
