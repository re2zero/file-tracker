/*
 * Copyright (C) 2021 UOS Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
 *             yangwu <yangwu@uniontech.com>
 *             wangrong <wangrong@uniontech.com>
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

#include <sys/utsname.h>
#include <signal.h>

#include <QCoreApplication>
#include <QThread>
#include <QLoggingCategory>
#include <QFile>
#include <QDebug>

#include <DLog>

#include <unistd.h>
#include <sys/time.h>

#include "server.h"
#include "tracesmoke.h"

enum WriteMountInfoError
{
    Success = 0,
    UnameFail,
    UnrecognizedVersion,
    OpenSrcFileFail,
    OpenDstFileFail,
    WriteDstFileFail
};

void handleSIGTERM(int sig)
{
    qDebug() << sig;

    if (qApp) {
        qApp->quit();
    }
}

DCORE_USE_NAMESPACE

// write mountinfo for vfs_monitor when kernel version >= 5.10
WriteMountInfoError writeMountInfo()
{
    struct utsname uts;
    if (uname(&uts) != 0) {
        qWarning() << "uname fail";
        return WriteMountInfoError::UnameFail;
    }
    qDebug() << "the kernel version: " << uts.release;

    QStringList ver_list = QString(uts.release).split(".");
    if (ver_list.size() < 3) {
        qWarning() << "unrecognized version format, expect x.y.z";
        return WriteMountInfoError::UnrecognizedVersion;
    }
    int ver_x = ver_list[0].toInt();
    int ver_y = ver_list[1].toInt();

    // write when version >= 5.10
    if (ver_x >= 6 || (5 == ver_x && ver_y >= 10)) {
        QString file_mountinfo_path("/proc/self/mountinfo");
        QFile file_mountinfo(file_mountinfo_path);
        if (!file_mountinfo.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "open file " << file_mountinfo_path << " failed";
            return WriteMountInfoError::OpenSrcFileFail;
        }
        QByteArray mount_info;
        mount_info = file_mountinfo.readAll();
        file_mountinfo.close();

        // driver_set_info is created by vfs_monitor and be used to receive mount information
        QString file_drv_path("/dev/driver_set_info");
        QFile file_drv(file_drv_path);
        if (!file_drv.open(QIODevice::ExistingOnly | QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "open file " << file_drv_path << " failed";
            return WriteMountInfoError::OpenDstFileFail;
        }
        if (file_drv.write(mount_info.data(), mount_info.size()) != mount_info.size()) {
            qWarning() << "write file " << file_drv_path << " failed";
            return WriteMountInfoError::WriteDstFileFail;
        }
        file_drv.close();

        qDebug() << "write mountinfo success";
    }

    return WriteMountInfoError::Success;
}

QString getLogFile()
{
    QString cachePath = QString("/var/cache/%1/file-tracker").arg(qApp->organizationName());

    if (getuid() != 0 && !QFileInfo(cachePath).isWritable()) {
        cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);

        if (cachePath.isEmpty() || cachePath == "/") {
            cachePath = QString("/tmp/%1/file-tracker").arg(qApp->organizationName());
        }
    }

    qInfo() << "Cache Dir:" << cachePath;

    if (!QDir::home().mkpath(cachePath)) {
        qWarning() << "Failed on create chache path";
    }

    return cachePath;
}

static QString logFormat = "%{message}\n";

int main(int argc, char *argv[])
{
    writeMountInfo();

    QCoreApplication app(argc, argv);
    app.setOrganizationName("re2zero");

#ifdef QT_NO_DEBUG
    QLoggingCategory::setFilterRules("vfs.info=false");
#endif

    // init log
    ConsoleAppender *consoleAppender = new ConsoleAppender;
    consoleAppender->setFormat(logFormat);

    QString logPath = getLogFile();
    RollingFileAppender *rollingFileAppender = new RollingFileAppender(logPath + "/footmark.log");
    rollingFileAppender->setFormat(logFormat);
    rollingFileAppender->setLogFilesLimit(5);
    rollingFileAppender->setDatePattern(RollingFileAppender::DailyRollover);

    logger->registerAppender(consoleAppender);
    logger->registerAppender(rollingFileAppender);

    for (const QString &c : TraceSmoke::logCategoryList()) {
        logger->registerCategoryAppender(c, consoleAppender);
        logger->registerCategoryAppender(c, rollingFileAppender);
    }

    Server *server = new Server();
    TraceSmoke *smoker = new TraceSmoke();

    QObject::connect(server, &Server::fileChanged, smoker, &TraceSmoke::onFileChanged);

    server->start();

    signal(SIGTERM, handleSIGTERM);

    return app.exec();
}
