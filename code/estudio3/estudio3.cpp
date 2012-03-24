/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   This file is part of
 *       _______   ______________  ______     _____
 *      / ____/ | / /  _/ ____/  |/  /   |   |__  /
 *     / __/ /  |/ // // / __/ /|_/ / /| |    /_ <
 *    / /___/ /|  // // /_/ / /  / / ___ |  ___/ /
 *   /_____/_/ |_/___/\____/_/  /_/_/  |_| /____/.
 *
 *   Copyright © 2003-2010 Brain Control, all rights reserved.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <QtGui/QApplication>
#include <QtCore/QFile>

#include "gui/mainwnd.hpp"
#include "../configinfo.hpp"
#include "../eshared/eshared.hpp"

// Sets some application settings.
static void initApplication()
{
    QApplication::setEffectEnabled(Qt::UI_FadeMenu, false);
    QApplication::setEffectEnabled(Qt::UI_AnimateMenu, false);

    QApplication::setApplicationName("Enigma Studio 3");
    QApplication::setApplicationVersion(eENIGMA3_VERSION);
    QApplication::setOrganizationName("Brain Control");
    QApplication::setOrganizationDomain("http://www.braincontrol.org");

    QApplication::setWheelScrollLines(1);

#ifdef eRELEASE
    QFile cssFile(QApplication::applicationDirPath()+"/estudio3.css");
#else
	QFile cssFile(QApplication::applicationDirPath()+"/estudio3_debug.css");
#endif

	if (cssFile.open(QFile::ReadOnly))
	{
		qApp->setStyleSheet(QString(cssFile.readAll()));
	}
}

// Qt application's entry point.
eInt main(eInt argc, eChar **argv)
{
    eMemTrackerStart();
    
    if (!eVerifyInstructionSets())
    {
        return -1;
    }

    QApplication app(argc, argv);
    initApplication();

    eMainWnd mainWnd;
    app.setActiveWindow(&mainWnd);
    mainWnd.show();

    return app.exec();
}