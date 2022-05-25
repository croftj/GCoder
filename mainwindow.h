#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <time.h>

#include <QDebug>
#include <QFile>
#include <QMainWindow>
#include <QSerialPort>
#include <QTime>
#include <QTimer>

#include "GCode.h"
#include "GCHttpServiceFactory.h"
#include "QcjData/QcjTcpService.h"
#include "outputdeviceselection.h"
#include "SweepDialog.h"

namespace Ui
{
   class MainWindow;
}

class MainWindow : public QMainWindow
{
   Q_OBJECT

public:
   explicit MainWindow(QWidget *parent = 0);
   ~MainWindow();

signals:
   void setStep(int step);
   void execScript(QFile*);

protected slots:
   void SlotExecuteFile();
   void SlotStopRun();
   void SlotManualCommandEntered();
   void SlotOpenOutputSelection();
   void SlotOpenFile();
   void SlotReadData();
   void SlotResetController();
   void SlotSliderStatus();
   void SlotMoveTimer();
   void SlotCommandSent(QString);
   void SlotExecutionTick();
   void SlotSetCurrPosition();
   void SlotPauseScript(QString);
   void SlotResumeScript();
   void SlotFontIncrease(bool);
   void SlotFontDecrease(bool);
   void SlotSweepTool();
   void updateAllWidgets();
   void SlotCommandPull(QString cmd, QVariant arg)
   {
      if (! arg.isNull())
         cmd = cmd.arg(arg.toString());
      qDebug() << __FUNCTION__ << "appending command to list: " << cmd;
      m_commandList.append(cmd);
      SlotReadData();
   }

//   void SlotSendCommand(const QString command);

private:
   void sendNextLine();

   QFile                m_script;
   bool                 m_scriptRunning;
   QString              m_deviceName;
   QString              m_pauseMessage;
   QStringList          m_commandList;
   time_t               m_executionElapsedTime;
   time_t               m_executionStartTime;
   QString              m_executionError;
   QString              m_executionState;
   QTimer               m_executionTimer;
   QTime                m_executionTime;
   QString              m_fileName;
   QString              m_lastDir;
   int                  m_httpPort;
   bool                 m_httpEnable;
   SweepDialog*         m_sweepDialog;
   QcjTcpServer*        m_tcpService;
   GCHttpServiceFactory m_httpFactory;
   bool                 m_countLines;
   bool                 m_checkCode;
   bool                 m_checkMode;
   QTimer               m_timer;
   QSerialPort*         m_serialDevice;
   QByteArray           m_serialBuffer;
   GCode*               m_gCode;
   int                  m_lineCount;
   int                  m_baudrate;
   bool                 m_atPrompt;
   bool                 m_wasReset;
   bool                 m_isPaused;
   QString              m_lastCommand;
   int                  m_totalCount;
   int                  m_currentCount;
   Ui::MainWindow*      m_ui;

   const static int     AUTOSTEP_INTERVAL;
   const static int     AUTOSTEP_MINMAX;
   const static QString EXECUTION_IDLE;
   const static QString EXECUTION_RUNNING;
   const static QString EXECUTION_PAUSED;
   const static QString EXECUTION_COMPLETE;
   const static QString EXECUTION_ERROR;
   const static QString EXECUTION_STOPPED;
};

#endif // MAINWINDOW_H
