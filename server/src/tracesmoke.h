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
#ifndef TRACESMOKE_H
#define TRACESMOKE_H

#include <QObject>
#include <QFile>

class TraceSmoke : public QObject
{
    Q_OBJECT
public:
    explicit TraceSmoke(QObject *parent = nullptr);

    static QStringList logCategoryList();
signals:

public slots:
    void onFileChanged(const QStringList &logs);

private:
    QString getFileName(QString &path);
    QString getFilePath(QString &log);
    bool checkAction(QString &log);
};

#endif // TRACESMOKE_H
