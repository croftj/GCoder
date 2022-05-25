#include <QDateTime>
#include <QDebug>
#include <QFlags>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollBar>
#include <QThread>
#include <QTimeEdit>
#include <math.h>

#include "mainwindow.h"
#include "pausemessagedialog.h"
#include "xlineedit.h"
#include "XSettings.h"

#include <QSerialPortInfo>
#include <time.h>

#include "executionstate.h"
#include "ui_mainwindow.h"

const int MainWindow::AUTOSTEP_INTERVAL = 500;  // Time tnterval between auto-steps 
const int MainWindow::AUTOSTEP_MINMAX   = 26;   // Maximum +/- jog step for the sliders

static QRegularExpression GCODE_ERROR_RE("^error:(.*)$");

ExecutionState RunState;

MainWindow::MainWindow(QWidget *parent) :
   QMainWindow(parent),
   m_serialDevice(NULL),
   m_checkMode(false),
   m_isPaused(false),
   m_tcpService(0),
   m_ui(new Ui::MainWindow)
{
   qDebug() << __FUNCTION__ << ":Enter";
   m_serialDevice = 0;
   m_sweepDialog = new SweepDialog(this);
   m_ui->setupUi(this);
   m_ui->manualEntryBox->setLineEdit(new XLineEdit());
   m_ui->xSlider->setMinimum(-AUTOSTEP_MINMAX);
   m_ui->xSlider->setMaximum(AUTOSTEP_MINMAX);
   m_ui->ySlider->setMinimum(-AUTOSTEP_MINMAX);
   m_ui->ySlider->setMaximum(AUTOSTEP_MINMAX);
   m_ui->zSlider->setMinimum(-(AUTOSTEP_MINMAX / 3));
   m_ui->zSlider->setMaximum(AUTOSTEP_MINMAX / 3);
   m_timer.setSingleShot(false);
   m_timer.stop();
   connect(&m_executionTimer, SIGNAL(timeout()), this, SLOT(SlotExecutionTick()));
   m_gCode = 0;

   connect(m_ui->manualEntryBox->lineEdit(), SIGNAL(returnPressed()), this, SLOT(SlotManualCommandEntered()));
   connect(m_ui->actionDevice, SIGNAL(triggered()), this, SLOT(SlotOpenOutputSelection()));
   connect(m_ui->actionOpen, SIGNAL(triggered()), this, SLOT(SlotOpenFile()));
   connect(m_ui->executeBtn,     SIGNAL(clicked()),            this,    SLOT(SlotExecuteFile()));
   connect(m_ui->stopRunBtn,     SIGNAL(clicked()),            this,    SLOT(SlotStopRun()));
   connect(m_ui->actionIncreaseFont, SIGNAL(triggered(bool)), this, SLOT(SlotFontIncrease(bool)));
   connect(m_ui->actionDecreaseFont, SIGNAL(triggered(bool)), this, SLOT(SlotFontDecrease(bool)));
   connect(m_ui->actionSweep, SIGNAL(triggered()), this, SLOT(SlotSweepTool()));


   showMaximized();
}

MainWindow::~MainWindow()
{
   delete m_ui;
}

void MainWindow::SlotOpenOutputSelection()
{
   OutputDeviceSelection dlg(this);
   if ( dlg.exec() )
   {
      m_deviceName = dlg.device();
      m_ui->outputDeviceLineEdit->setText(m_deviceName);
      m_baudrate   = dlg.baudrate();

      if (m_serialDevice != NULL)
      {
         qDebug() << __FUNCTION__ << "Deleting existing device";
         delete m_serialDevice;
      }
      qDebug() << __FUNCTION__ << "device: " << m_deviceName << ", baud: " << m_baudrate;
      m_serialDevice = new QSerialPort(m_deviceName);
      m_serialDevice->setBaudRate(QSerialPort::Baud115200);
      m_serialDevice->setDataBits(QSerialPort::Data8);
      m_serialDevice->setFlowControl(QSerialPort::SoftwareControl);
      m_serialDevice->setParity(QSerialPort::NoParity);
      m_serialDevice->setStopBits(QSerialPort::OneStop);
      if (! m_serialDevice->open(QIODevice::ReadWrite))
      {
         QString errstr;
         switch(m_serialDevice->error())
         {
            case QSerialPort::DeviceNotFoundError:
             errstr = "Device Not Found Error";
             break;
            case QSerialPort::PermissionError:
             errstr = "Permission Error";
             break;
            case QSerialPort::OpenError:
             errstr = "Open Error";
             break;
            default:
             errstr = "Unknown Error";
             break;
         }
         qDebug() << __FUNCTION__ << "Error opening serial device: " << errstr;
         QMessageBox::warning(this, "Error opening device", errstr);
         m_ui->outputDeviceLineEdit->setEnabled(false);
      }
      else
      {
         qDebug() << __FUNCTION__ << "Opening device";
         m_ui->outputDeviceLineEdit->setEnabled(true);
         m_ui->outputDeviceLineEdit->setText(m_deviceName);
         if ( m_gCode != NULL )
            delete m_gCode;
         m_gCode = new GCode(m_serialDevice, this);
         connect(m_ui->inBtn,    SIGNAL(clicked()),   m_gCode, SLOT(SetInches()));
         connect(m_ui->mmBtn,    SIGNAL(clicked()),   m_gCode, SLOT(SetMetric()));

         connect(m_ui->xSlider,  SIGNAL(sliderPressed()),  this, SLOT(SlotSliderStatus()));
         connect(m_ui->ySlider,  SIGNAL(sliderPressed()),  this, SLOT(SlotSliderStatus()));
         connect(m_ui->zSlider,  SIGNAL(sliderPressed()),  this, SLOT(SlotSliderStatus()));
         connect(m_ui->xSlider,  SIGNAL(sliderReleased()), this, SLOT(SlotSliderStatus()));
         connect(m_ui->ySlider,  SIGNAL(sliderReleased()), this, SLOT(SlotSliderStatus()));
         connect(m_ui->zSlider,  SIGNAL(sliderReleased()), this, SLOT(SlotSliderStatus()));

         connect(&m_timer,       SIGNAL(timeout()),   this,    SLOT(SlotMoveTimer()));

         connect(m_ui->upBtn,    SIGNAL(clicked()),   m_gCode, SLOT(JogUp()));
         connect(m_ui->dwnBtn,   SIGNAL(clicked()),   m_gCode, SLOT(JogDown()));
         connect(m_ui->leftBtn,  SIGNAL(clicked()),   m_gCode, SLOT(JogLeft()));
         connect(m_ui->rightBtn, SIGNAL(clicked()),   m_gCode, SLOT(JogRight()));
         connect(m_ui->fwdBtn,   SIGNAL(clicked()),   m_gCode, SLOT(JogForward()));
         connect(m_ui->bkwBtn,   SIGNAL(clicked()),   m_gCode, SLOT(JogBackward()));
         connect(m_ui->gotoOriginBtn,  SIGNAL(clicked()),            m_gCode, SLOT(gotoOrigin()));
         connect(m_ui->stepSpinBox,    SIGNAL(valueChanged(double)), m_gCode, SLOT(SetStep(double)));
         connect(m_ui->executeBtn,     SIGNAL(clicked()),            this,    SLOT(SlotExecuteFile()));
         connect(m_ui->setMachineOriginBtn,  SIGNAL(clicked()),      m_gCode, SLOT(SetOrigin()));
         connect(m_ui->executeCommandBtn,    SIGNAL(clicked()),      this,    SLOT(SlotManualCommandEntered()));
         connect(m_ui->resetControlerBtn,    SIGNAL(clicked()),      this,    SLOT(SlotResetController()));
         connect(m_ui->setCurrPosBtn,        SIGNAL(clicked()),      this,    SLOT(SlotSetCurrPosition()));

         connect(m_gCode, SIGNAL(SentCommand(QString)), this,    SLOT(SlotCommandSent(QString)));
         connect(m_gCode, SIGNAL(PauseScript(QString)), this,    SLOT(SlotPauseScript(QString)));
         connect(m_gCode, SIGNAL(ResumeScript()), this,    SLOT(SlotResumeScript()));
         connect(m_gCode, SIGNAL(CommandPush(QString, QVariant)), this,    SLOT(SlotCommandPull(QString, QVariant)));
      }

      connect(m_serialDevice, SIGNAL(readyRead()), this, SLOT(SlotReadData()));
      sleep(1);
      m_gCode->reset();
      sleep(1);
      m_gCode->SetMetric();
      m_gCode->SetStep(0.1);
      m_ui->stepSpinBox->setValue(m_gCode->GetStep());
      m_ui->mmBtn->setDown(true);
   }
}

void MainWindow::SlotOpenFile()
{
   XSettings settings;
   m_lastDir      = settings.value(XSettings::RECENT_PATH, ".").toString();
   m_httpPort     = settings.value(XSettings::HTTP_PORT, "8080").toInt();
   m_httpEnable   = settings.value(XSettings::HTTP_ENABLE, true).toBool();
   m_countLines   = settings.value(XSettings::COUNT_LINES, true).toBool();
   m_checkCode    = settings.value(XSettings::CHECK_GCODE, false).toBool();

   m_fileName = QFileDialog::	getOpenFileName (this,
                                               "Open G Code File",
                                               m_lastDir,
                                               "GCode Files (*.ngc *.gcode);;Text files (*.txt);;JSON files (*.json)");
   if ( ! m_fileName.isEmpty() ) 
   {
      m_ui->scriptLineEdit->setText(m_fileName);
      QFileInfo fi(m_fileName);
      settings.setValue(XSettings::RECENT_PATH, fi.absolutePath());
      if ( QFileInfo::exists(m_fileName) ) 
      {
         QFile file(m_fileName);
         if (file.open(QIODevice::ReadOnly))
         {
            char buf[1024];
            RunState.set(ExecutionState::FILE_NAME, m_fileName);
            m_totalCount = 0;
            while (file.readLine(buf, sizeof(buf)) >= 0)
            {
               m_totalCount++;
               m_ui->deviceTextEdit->insertPlainText(QString::number(m_totalCount) + ": ");
               m_ui->deviceTextEdit->insertPlainText(buf);
            }
            m_ui->deviceTextEdit->verticalScrollBar()->setValue(m_ui->deviceTextEdit->verticalScrollBar()->maximum());
            file.close();
         }
         if (m_tcpService != 0)
         {
            delete m_tcpService;
         }
         RunState.setDefaults();
         RunState.set(ExecutionState::TOTAL_LINES, m_totalCount);

         qDebug() << "Creating tcp service on port" << (int)m_httpPort;
         m_tcpService = new QcjTcpServer(QHostAddress::AnyIPv4, m_httpPort, this);
         m_httpFactory.setRunState(&RunState);
         m_tcpService->setServiceFactory(&m_httpFactory);
      }
   }
}

void MainWindow::SlotStopRun()
{
   m_executionTimer.stop();
   m_script.close();
   m_commandList.clear();
   m_serialDevice->clear(QSerialPort::Output);
   m_gCode->pause();
   m_gCode->motor_off();
   RunState.set(ExecutionState::STATUS, ExecutionState::STOPPED);
   RunState.set(ExecutionState::FILE_NAME, "");
}

void MainWindow::SlotReadData()
{
   QByteArray buf;
   int bytesAvailable;

   while ( (bytesAvailable = m_serialDevice->bytesAvailable()) > 0 )
   {
      qDebug() << __FUNCTION__ << "Have " << bytesAvailable << " bytes ready for reading...";
      buf.resize(bytesAvailable);
      m_serialDevice->read(buf.data(), buf.size());
      while(buf.size() > 0)
      {
         char ch = buf.at(0);
         m_serialBuffer.append(ch);
         buf.remove(0, 1);
      }
      //        buf = m_serialDevice.read(bytesAvailable);
      //        QString string(buf);
      qDebug() << __FUNCTION__ << "Have data: " << buf;
   }
//   qDebug() << __FUNCTION__ << "No more data, buf:" << m_ui->deviceTextEdit->toPlainText();
   qDebug() << __FUNCTION__ << "No more data";

   /******************************************/
   /* See if we received the end of the line */
   /******************************************/
   while (m_serialBuffer.contains('\n'))
   {
      while (m_serialBuffer.size() > 0 && ! buf.contains('\n'))
      {
         char ch = m_serialBuffer.at(0);
         buf.append(ch);
         m_serialBuffer.remove(0, 1);
//         qDebug() << __FUNCTION__ << "buf: " << buf;
      }
      qDebug() << __FUNCTION__ << "Reached the end of the line! m_serialBuffer.size() = " << m_serialBuffer.size() << ", serialBuffer = " << m_serialBuffer;
      m_ui->deviceTextEdit->insertPlainText(buf);
      m_ui->deviceTextEdit->verticalScrollBar()->setValue(m_ui->deviceTextEdit->verticalScrollBar()->maximum());
      if ( m_ui->deviceTextEdit->toPlainText().endsWith("ok\n") // ||
   //        m_ui->deviceTextEdit->toPlainText().endsWith("Unsupported command\n")
         )
      {
         m_atPrompt = true;
         if ( ! m_wasReset ) 
         {
            if (m_script.isOpen())
            {
               sendNextLine();
            }
         }
         m_wasReset = false;
      }
      else 
      {
         /*****************************/
         /* Check if we have an error */
         /*****************************/
         QString line(buf.data());
         QRegularExpressionMatch match = GCODE_ERROR_RE.match(line);
         if (match.hasMatch())
         {
            RunState.set(ExecutionState::ERROR_STRING, match.captured(1));
            RunState.set(ExecutionState::STATUS, ExecutionState::ERROR);
            QMessageBox::warning(this, "Runtime Error", QString("Runtime error encountered: %1").arg(match.captured(1)));
            m_atPrompt = true;
         }
         else
         {
            qDebug() << __FUNCTION__ << "NOT at prompt";
            m_atPrompt = false;
         }
      }
      if (m_atPrompt)
      {
         qDebug() << __FUNCTION__ << "1: At prompt";
         if (m_commandList.count() > 0)
         {
            QString cmd = m_commandList.takeFirst();
            m_gCode->WriteCommand(cmd);
            m_atPrompt = false;
//            if ( cmd.size() > 0)
//            {
//               m_ui->deviceTextEdit->insertPlainText(cmd + "\r");
//            }
//            else
//            {
//               m_ui->deviceTextEdit->insertPlainText("\r");
//            }
            m_ui->deviceTextEdit->verticalScrollBar()->setValue(m_ui->deviceTextEdit->verticalScrollBar()->maximum());
         }
      }
      buf.clear();
      qDebug() << __FUNCTION__ << "Fetching more data from serialBuf: " << m_serialBuffer;
   }
   if (m_atPrompt)
   {
      qDebug() << __FUNCTION__ << "2: At prompt";
      if (m_commandList.count() > 0)
      {
         QString cmd = m_commandList.takeFirst();
         m_gCode->WriteCommand(cmd);
         m_atPrompt = false;
//         if ( cmd.size() > 0)
//         {
//            m_ui->deviceTextEdit->insertPlainText(cmd + "\r");
//         }
//         else
//         {
//            m_ui->deviceTextEdit->insertPlainText("\r");
//         }
//         m_ui->deviceTextEdit->verticalScrollBar()->setValue(m_ui->deviceTextEdit->verticalScrollBar()->maximum());
      }
   }
   qDebug() << __FUNCTION__ << "exit";
}

void MainWindow::SlotResetController()
{
   m_executionTimer.stop();
   m_script.close();
   m_wasReset = true;
   m_serialDevice->clear(QSerialPort::Output);
   m_gCode->reset();
}

void MainWindow::SlotSetCurrPosition()
{
   QVariant x(m_ui->xPosSpinBox->value());
   QVariant y(m_ui->yPosSpinBox->value());
   QVariant z(m_ui->zPosSpinBox->value());
   m_gCode->setCurrentPosition(x, y, z);
}

void MainWindow::SlotManualCommandEntered()
{
   if (m_serialDevice != 0)
   {
      QString cmd = m_ui->manualEntryBox->lineEdit()->text();
      m_ui->manualEntryBox->lineEdit()->setText("");
      qDebug() << __FUNCTION__ << ":Sending out " << cmd;
//      m_serialDevice->write(qPrintable(cmd));
//      m_serialDevice->write("\r", 1);
      m_commandList.append(cmd);
      SlotReadData();
   }
}

void MainWindow::SlotExecuteFile()
{
   if ( QFileInfo::exists(m_fileName) ) 
   {
      if ( ! m_isPaused)
      {
         if ( ! m_script.isOpen() ) 
         {
            m_script.setFileName(m_fileName);
            if ( ! m_script.open(QIODevice::ReadOnly) )
            {
            }
            else 
            {
               qDebug() << __FUNCTION__ << "Starting timer";
               m_ui->executionTimeEdit->clear();
               m_executionTimer.start(1000);
               m_executionElapsedTime = 0;
               m_executionTime.restart();
               m_currentCount = 0; 
               RunState.set(ExecutionState::FILE_NAME, m_fileName);
               if ( m_atPrompt ) 
               {
                  RunState.set(ExecutionState::STATUS, ExecutionState::RUNNING);
                  m_currentCount++;
                  qDebug() << __FUNCTION__ << "CURRENT_LINE = " << m_currentCount;
                  RunState.set(ExecutionState::CURRENT_LINE, m_currentCount);
                  sendNextLine();
               }
               else 
               {
                  QThread::sleep(2);
               }
            }
         }
      }
      else
      {
         m_gCode->resume();
      }
   }
}

void MainWindow::sendNextLine()
{
   if ( m_script.isOpen() ) 
   {
      if ( ! m_isPaused)
      {
         char buf[2048];
         if ( m_script.readLine(buf, sizeof(buf) - 2) >= 0 ) 
         {
            RunState.set(ExecutionState::CURRENT_COMMAND, QString(buf));
            m_ui->deviceTextEdit->insertPlainText(buf);
            m_lastCommand = QString(buf);
            m_currentCount++;
            RunState.set(ExecutionState::STATUS, ExecutionState::RUNNING);
            RunState.set(ExecutionState::CURRENT_LINE, m_currentCount);
            QString cmd(buf);
            m_gCode->WriteCommand(cmd);

//            cmd.replace("\n", "");
//            cmd.replace("\r", "");
//            qDebug() << __FUNCTION__ << "Sending: " << cmd;
//            m_serialDevice->write(qPrintable(cmd));
//            m_serialDevice->write("\r", 1);
         }
         else 
         {
            m_script.close();
            m_executionTimer.stop();
            RunState.set(ExecutionState::STATUS, ExecutionState::COMPLETE);
         }
      }
      else
      {
         m_executionTimer.stop();
         m_ui->deviceTextEdit->insertPlainText(m_pauseMessage);
         PauseMessageDialog dlg(m_gCode, m_pauseMessage, m_gCode->GetStep(), AUTOSTEP_INTERVAL, this);
         if (dlg.exec() == QDialog::Accepted)
         {
            m_isPaused = false;
            m_executionTimer.start(1000);
            m_gCode->resume();
            qDebug() << "Script resumed!!!";
        }
        else
        {
           SlotStopRun();
        }
      }
   }
}

void MainWindow::SlotCommandSent(QString command)
{
   qDebug() << __FUNCTION__ << "command: " << command;
   m_ui->deviceTextEdit->insertPlainText(command);
}

void MainWindow::SlotSliderStatus()
{
   if ( m_ui->xSlider->isSliderDown() || m_ui->ySlider->isSliderDown() || m_ui->zSlider->isSliderDown() ) 
   {
      m_gCode->SetMetric();
      m_timer.start(AUTOSTEP_INTERVAL);
   }
   else 
   {
      if ( m_ui->inBtn->isChecked() ) 
      {
         m_gCode->SetInches();
      }
      m_ui->xSlider->setValue(0);
      m_ui->ySlider->setValue(0);
      m_ui->zSlider->setValue(0);
      m_timer.stop();
   }
}

void MainWindow::SlotMoveTimer()
{
   if ( m_ui->xSlider->isSliderDown() && m_ui->xSlider->value() != 0 )
   {
      m_gCode->JogForward(m_ui->xSlider->value());
   }
   else 
   {
      m_ui->xSlider->setValue(0);
   }

   if ( m_ui->ySlider->isSliderDown() && m_ui->ySlider->value() != 0 )
   {
      m_gCode->JogRight(m_ui->ySlider->value());
   }
   else 
   {
      m_ui->ySlider->setValue(0);
   }

   if ( m_ui->zSlider->isSliderDown() && m_ui->zSlider->value() != 0 )
   {
      m_gCode->JogUp(m_ui->zSlider->value());
   }
   else 
   {
      m_ui->zSlider->setValue(0);
   }
}

void MainWindow::SlotExecutionTick()
{

   if ( m_script.isOpen() ) 
   {
      m_scriptRunning = true;
      m_executionElapsedTime++;
      RunState.set(ExecutionState::EXECUTION_TIME, QVariant((int)m_executionElapsedTime));
      m_executionTime = QTime::fromMSecsSinceStartOfDay(m_executionElapsedTime * 1000);
      qDebug() << "ExecutionTime(): elapsed_time = " << m_executionTime.msecsSinceStartOfDay();
      m_ui->executionTimeEdit->setTime(m_executionTime);
   }
   else if (m_scriptRunning) 
   {
      m_scriptRunning = false;
      m_executionTimer.stop();
   }
}

void MainWindow::SlotPauseScript(QString message)
{
   m_pauseMessage = message;
   qDebug() << "Script paused!!!";
   m_isPaused = true;
   RunState.set(ExecutionState::STATUS, ExecutionState::PAUSED);
}

void MainWindow::SlotResumeScript()
{
   qDebug() << __FUNCTION__ << "Changing button text";
   m_isPaused = false;
   RunState.set(ExecutionState::STATUS, ExecutionState::RUNNING);
}

void MainWindow::SlotFontIncrease(bool)
{
   QFont f = QApplication::font();
   int ps = f.pointSize();
   ps += 2;
   qDebug() << __FUNCTION__ << ": New Pointsize: " << ps;
   f.setPointSize(ps);
   QApplication::setFont(f);
   updateAllWidgets();
}

void MainWindow::SlotFontDecrease(bool)
{
   QFont f = QApplication::font();
   int ps = f.pointSize();
   ps -= 2;
   qDebug() << __FUNCTION__ << ": New Pointsize: " << ps;
   f.setPointSize(ps);
   QApplication::setFont(f);
   updateAllWidgets();
}

void MainWindow::SlotSweepTool()
{
   double x_pos = 0.0;
   double y_pos = 0.0;
   double z_pos = 0.0;
   SweepDialog *dlg = m_sweepDialog;
//   OutputDeviceSelection dlg(this);
   if (dlg->exec() == QDialog::Accepted)
   {
      qDebug() << __FUNCTION__ << ": accepted!";
      m_gCode->SetOrigin();
      if (dlg->isMetric())
      {
         m_gCode->SetMetric();
      }
      else
      {
         m_gCode->SetInches();
      }
      m_gCode->SetFeedRate(dlg->feedRate());
      m_gCode->CutToZ(dlg->safeZ());
      m_gCode->motor_on();
      m_gCode->dwell(3);

      while (z_pos > dlg->targetDepth())
      {
         x_pos = 0.0;
         y_pos = 0.0;
         z_pos += dlg->swipeDepth();
         if (z_pos < dlg->targetDepth())
         {
            z_pos = dlg->targetDepth();
         }
         m_gCode->CutToZ(z_pos);
         if (dlg->xAxis())
         {
            int direction = (dlg->targetY() < 0) ? -1 : 1;
            while (fabs(y_pos) < fabs(dlg->targetY()))
            {
               m_gCode->CutToX(dlg->targetX());
               y_pos += dlg->increment() * direction;
               if (fabs(y_pos) > fabs(dlg->targetY()))
               {
                  y_pos += dlg->increment() * direction;
               }
               m_gCode->CutToXY(0.0, y_pos);
            }
            m_gCode->CutToX(dlg->targetX());
         }
         else
         {
            int direction = (dlg->targetX() < 0) ? -1 : 1;
            while (fabs(x_pos) < fabs(dlg->targetX()))
            {
               m_gCode->CutToY(dlg->targetY());
               x_pos += dlg->increment() * direction;
               if (fabs(x_pos) > fabs(dlg->targetX()))
               {
                  x_pos += dlg->increment() * direction;
               }
               m_gCode->CutToXY(x_pos, 0.0);
            }
            m_gCode->CutToY(dlg->targetY());
         }
         m_gCode->CutToZ(dlg->safeZ());
         m_gCode->gotoOriginSansZ();
      }
   }
   m_gCode->motor_off();
   m_gCode->dwell(3);
   qDebug() << __FUNCTION__ << ": done!";
}

void MainWindow::updateAllWidgets()
{
    foreach (QWidget *widget, QApplication::allWidgets())
        widget->update();
}
