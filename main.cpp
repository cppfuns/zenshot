/* Copyright (C) 2021 Nature Easy Soft Network Technology Co., LTD
 *
 * This file is part of Zenshot (https://github.com/easysoft/zenshot/).
 *
 * Zenshot is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Zenshot is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Zenshot. If not, see <https://www.gnu.org/licenses/>.
 */
#include <gtk/gtk.h>

#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QMetaEnum>
#include <QFile>
#include <QList>
#include <QFontDatabase>
#include <QWidget>

#include "spdlogwrapper.hpp"

#include "widget.h"
#include "starterui.h"
#include "starter.h"
#include "core/screeninfo.h"
#include "screen/helper/screengetter.h"
#include "core/gparams.h"

#include <QSystemSemaphore>

#ifdef Q_OS_WIN
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif // Q_OS_WIN

static const char mutex_name[] = "Zenshot@ZenTao";

int main(int argc, char *argv[])
{
#ifdef USE_SPDLOG_
#ifdef Q_OS_WIN
    mkdir("logs");
#else
    mkdir("logs", S_IRWXU);
#endif // Q_OS_WIN
    spdlog::spdlog_init("zenshot", "logs/log.log", 23, 57, 0, 0);
    L_TRACE("start");
#endif // USE_SPDLOG_

    QSystemSemaphore sema(mutex_name, 1, QSystemSemaphore::Open);
    L_INFO("++>>> {0} @ {1}", sema.error(), sema.errorString().toStdString().c_str());
    if (!sema.acquire())
    {
        L_ERROR("++>>> {0} @ {1}", sema.error(), sema.errorString().toStdString().c_str());
        return 0;
    }

    gdk_init(NULL, NULL);

    QCoreApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);

    QApplication a(argc, argv);

	if (!QSystemTrayIcon::isSystemTrayAvailable()) {
		L_ERROR("system tray disabled");
		return 1;
	}

    //加载并应用语言翻译界面
    QLocale locale;
    QLocale::Language langEnum = locale.language();
    QMetaEnum metaEnum = QMetaEnum::fromType<QLocale::Language>();
    const char* langChar = metaEnum.valueToKey(langEnum);
    QString langName(langChar);

    QString langFile = ":/translations/" + langName + ".qm";
    if(QFile(langFile).exists() == false)
    {
        langFile = ":/translations/English.qm";
    }

    QTranslator translator;
    translator.load(langFile);
    a.installTranslator(&translator);

    //加载图标字体文件
    int fontId = QFontDatabase::addApplicationFont(":/iconfont.ttf");
    QString fontNmae = QFontDatabase::applicationFontFamilies(fontId).at(0);

    //设置系统默认字体
    QFont defaultFont("微软雅黑",9);
    a.setFont(defaultFont);

    //解析命令行参数
    int result = GParams::instance()->fromArgs(argc,argv);

    if(result != 0)
        return result;

    a.setQuitOnLastWindowClosed(false);

    StarterUI ui;
	ui.show();

    int ret = a.exec();
    return ret;
}
