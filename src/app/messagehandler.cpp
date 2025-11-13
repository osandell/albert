// Copyright (c) 2024 Manuel Schneider

#include "messagehandler.h"
#include <QMessageBox>
#include <QString>
#include <QTime>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QMutex>
#include <QMutexLocker>
#include <QCoreApplication>

static QMutex logFileMutex;
static QFile* logFile = nullptr;
static QTextStream* logStream = nullptr;

void initializeLogFile()
{
    QMutexLocker locker(&logFileMutex);
    if (logFile == nullptr) {
        // Try to write to the same directory as the executable
        QString logPath = QCoreApplication::applicationDirPath() + "/albert_debug.log";
        logFile = new QFile(logPath);
        if (logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            logStream = new QTextStream(logFile);
            *logStream << "\n=== Albert Debug Log Started ===\n";
            logStream->flush();
        } else {
            delete logFile;
            logFile = nullptr;
        }
    }
}

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    // Initialize log file if not already done
    if (logFile == nullptr && QCoreApplication::instance() != nullptr) {
        initializeLogFile();
    }

    QString logMessage;
    QString timestamp = QTime::currentTime().toString();
    
    // Todo use std::format as soon as apple gets it off the ground
    switch (type) {
    case QtDebugMsg:
        logMessage = QString("%1 [debg:%2] %3").arg(timestamp, context.category, message);
        fprintf(stdout, "%s \x1b[34m[debg:%s]\x1b[0m %s\x1b[0m\n",
                timestamp.toLocal8Bit().constData(),
                context.category,
                message.toLocal8Bit().constData());
        break;
    case QtInfoMsg:
        logMessage = QString("%1 [info:%2] %3").arg(timestamp, context.category, message);
        fprintf(stdout, "%s \x1b[32m[info:%s]\x1b[0m %s\n",
                timestamp.toLocal8Bit().constData(),
                context.category,
                message.toLocal8Bit().constData());
        break;
    case QtWarningMsg:
        logMessage = QString("%1 [warn:%2] %3").arg(timestamp, context.category, message);
        fprintf(stdout, "%s \x1b[33m[warn:%s]\x1b[0m %s\x1b[0m\n",
                timestamp.toLocal8Bit().constData(),
                context.category,
                message.toLocal8Bit().constData());
        break;
    case QtCriticalMsg:
        logMessage = QString("%1 [crit:%2] %3").arg(timestamp, context.category, message);
        fprintf(stdout, "%s \x1b[31m[crit:%s] %s\x1b[0m\n",
                timestamp.toLocal8Bit().constData(),
                context.category,
                message.toLocal8Bit().constData());
        break;
    case QtFatalMsg:
        logMessage = QString("%1 [fatal:%2] %3 -- [%4]").arg(timestamp, context.category, message, context.function);
        fprintf(stderr, "%s \x1b[41;30;4m[fatal:%s]\x1b[0;1m %s  --  [%s]\x1b[0m\n",
                timestamp.toLocal8Bit().constData(),
                context.category,
                message.toLocal8Bit().constData(),
                context.function);
        QMessageBox::critical(nullptr, "Fatal error", message);
        exit(1);
    }
    fflush(stdout);

    // Write to log file if available
    if (logStream != nullptr) {
        QMutexLocker locker(&logFileMutex);
        *logStream << logMessage << "\n";
        logStream->flush();
    }
}
