/*

* Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.

*

* Author:     yangwu <sumdoll@163.com>

*

* Maintainer: yangwu <yangwu@uniontech.com>

*

* This program is free software: you can redistribute it and/or modify

* it under the terms of the GNU General Public License as published by

* the Free Software Foundation, either version 3 of the License, or

* any later version.

*

* This program is distributed in the hope that it will be useful,

* but WITHOUT ANY WARRANTY; without even the implied warranty of

* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the

* GNU General Public License for more details.

*

* You should have received a copy of the GNU General Public License

* along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
#include "tracesmoke.h"

#include <QTime>
#include <QDir>
#include <QDebug>
#include <QWaitCondition>
#include <QMutex>
#include <QMap>
#include <QLoggingCategory>

Q_GLOBAL_STATIC_WITH_ARGS(QLoggingCategory, normalLog, ("manager.normal"))

#define nDebug(...) qCDebug((*normalLog), __VA_ARGS__)
#define nInfo(...) qCInfo((*normalLog), __VA_ARGS__)
#define nWarning(...) qCWarning((*normalLog), __VA_ARGS__)

TraceSmoke::TraceSmoke(QObject *parent): QObject(parent)
{
}

QStringList TraceSmoke::logCategoryList()
{
    QStringList list;

    list << normalLog->categoryName();

    return list;
}

void TraceSmoke::onFileChanged(const QStringList &logs)
{
    for (QString log : logs) {
        QString lastpath = getFilePath(log);
        if (lastpath.startsWith("/home") || lastpath.startsWith("/data")
              || lastpath.startsWith("/media")) {
            QString name = getFileName(lastpath);
            if (!name.startsWith('.')) {
                // write log into footmark.log
                nDebug() << log;
            } else {
//                qInfo() << "ignore change to system hidden. log:" << log;
            }
        } else {
//            qInfo() << "ignore system file change:" << lastpath;
        }
    }
}

QMap<QString, QString> parseLog2Map(QString &log)
{
    QMap<QString, QString> logmap;
    QStringList sqelogs = log.split('|');
    for (QString sqe: sqelogs) {
        QStringList pairs = sqe.split(": ");
        if (pairs.count() == 2) {
            logmap.insert(pairs.at(0), pairs.at(1));
        } else {
            qWarning() << "incorrect format log:" << sqe;
        }
    }
}


QString TraceSmoke::getFileName(QString &path)
{
    QString name;
    int last_dir_split_pos = path.lastIndexOf('/');

    if (last_dir_split_pos >= 0)
        name = path.mid(last_dir_split_pos + 1);

    return name;
}

QString TraceSmoke::getFilePath(QString &log)
{
    QString path;
    QString keyw = "filepath:";
    int last_path_pos = log.lastIndexOf(keyw);

    if (last_path_pos >= 0) {
        QString fullpath = log.mid(last_path_pos + keyw.length());
        QString spe = "-> ";
        int has_spe = fullpath.lastIndexOf(spe);
        path = has_spe > 0? fullpath.mid(has_spe + spe.length()) : fullpath;
    } else {
        qInfo() << " error format log:" << log;
    }

    return path;
}

