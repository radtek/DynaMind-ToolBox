/**
 * @file
 * @author  Chrisitan Urich <christian.urich@gmail.com>
 * @version 1.0
 * @section LICENSE
 *
 * DAnCE4Water (Dynamic Adaptation for eNabling City Evolution for Water) is a
 * strategic planning tool ti identify the transtion from an from a conventional
 * urban water system to an adaptable and sustainable system at a city scale
 *
 * Copyright (C) 2011  Christian Urich

 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @section DESCRIPTION
 *
 * The class contains the main window for the DynaMind Framework
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "dmcompilersettings.h"
#include <QMainWindow>
#include <qfuturewatcher.h>

//#include "plot.h"

#include <dmsimulationobserver.h>

#include <QMap>
#include <QVector>
#include <moduledescription.h>
#include <ui_dmmainwindow.h>
#include <fstream>

class QTreeWidgetItem;


class ProjectViewer;
class ModelNode;
class SimulationManagment;
class GroupNode;
class GuiLogSink;
class GUISimulation;
class GUIHelpViewer;
class QThread;
class GuiSimulationObserver;


namespace DM {
    class Group;
}
 namespace Ui {
    class MainWindow;
}


class SimulationTab;
/*
class GuiSimulationObserver: public DM::SimulationObserver
{
	QProgressBar* progressBar;
public:
	GuiSimulationObserver(QProgressBar* progressBar):progressBar(progressBar){}
	void update(float progress);
};*/

class DM_HELPER_DLL_EXPORT DMMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    DMMainWindow(QWidget *parent = 0);
    ~DMMainWindow();
    //QTreeWidgetItem * getRootItemModelTree(){return this->rootItemModelTree;}
    GuiLogSink *log_updater;
    GUISimulation * getSimulation() {return this->simulation;}
private:
    Ui::DMMainWindow *ui;
    GUISimulation * simulation;
    //SimulationManagment * simmanagment;
    //GUISimulationObserver * simobserver;
    //GUIHelpViewer * helpviewer;

    void createModuleListView();
    //QMap<QString, ModuleDescription> modules;
    //QMap<ProjectViewer *, QWidget *> tabmap;
    //QMap<int, ProjectViewer * >  groupscenes;
    //int counter;
    bool running;
    //QTreeWidgetItem * rootItemModelTree;
    //void appendGUIInformation(QString FileName);
    //void loadGUIModules(DM::Group * g, std::map<std::string, std::string> UUID_Translation,  QVector<LoadModule> posmodules);
    //void loadGUILinks(std::map<std::string, std::string> UUID_Translation);
	
	//QFutureWatcher<void> simulationWatcher;
	//QList<SimulationTab*> tabs;

	QThread *simulationThread;
	GuiSimulationObserver* simulationThreadWrapper;
	std::ofstream* outputFile;
public slots:
    void runSimulation();
    void sceneChanged();
    void preferences();
    void saveSimulation();
    void loadSimulation(int id=0);
    void clearSimulation();
    void setRunning();
    void saveAsSimulation();
    void importSimulation(QString fileName = "", QPointF = QPointF(0,0));
    void simulationFinished();
    void startEditor();
    void ReloadSimulation();
    void addNewGroupWindows(GroupNode *);
    void renameGroupWindow(GroupNode *);
    void removeGroupWindows(QString uuid);
    void updateSimulation();
    void resetSimulation();
    void cancelSimulation();
//    void showHelp(std::string classname);
	
	void newLogLine(QString line);


private slots:
    void on_actionZoomReset_triggered();
    void on_actionZoomOut_triggered();
    void on_actionZoomIn_triggered();
    void on_actionAbout_triggered();
    void on_actionShow_Help_triggered();
	
	void updateProgress(float progress);
};

#endif // MAINWINDOW_H
