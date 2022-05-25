#ifndef GCODE_CPP
#define GCODE_CPP

#include <QDebug>
#include <QFile>
#include <QRegExp>
#include <QString>
#include <QVariant>
#include <QSerialPort>

#include <unistd.h>

class GCode : public QObject
{
   Q_OBJECT

public:
   GCode(QSerialPort *device, QObject *parent = 0) :
      QObject(parent),
      m_device(device),
      m_step(1),
      m_xpos(0),
      m_ypos(0),
      m_zpos(0)
   {
      qDebug() << __FUNCTION__ << "step = " << m_step;
   }

public slots:
   bool SetMetric()
   {
      bool rv = m_metric;
      qDebug() << __FUNCTION__ << "Enter";
      m_metric = true;
      emit CommandPush(GCODE_G21, QVariant());
      return(rv);
   }

   bool SetInches()
   {
      bool rv = m_metric;
      qDebug() << __FUNCTION__ << "Enter";
      m_metric = false;
      emit CommandPush(GCODE_G20, QVariant());
      return(rv);
   }

   void SetRelative()
   {
      qDebug() << __FUNCTION__ << "Enter";
      emit CommandPush(GCODE_G91, QVariant());
   }

   void SetAbsolute()
   {
      qDebug() << __FUNCTION__ << "Enter";
      emit CommandPush(GCODE_G90, QVariant());
   }

   void SetStep(double step = 1)
   {
      m_step = step;
   }

   double GetStep()
   {
      return(m_step);
   }

   void CutToX(double x_pos = 0.0)
   {
      qDebug() << __FUNCTION__ << "x_pos = " << x_pos;
      emit CommandPush(GCODE_G1X, QVariant(x_pos));
   }

   void CutToY(double y_pos = 0.0)
   {
      qDebug() << __FUNCTION__ << "y_pos = " << y_pos;
      emit CommandPush(GCODE_G1Y, QVariant(y_pos));
   }

   void CutToZ(double z_step = 0.0)
   {
      qDebug() << __FUNCTION__ << "z_step = " << z_step;
      emit CommandPush(GCODE_G1Z, QVariant(z_step));
   }

   void CutToXY(double x_pos = 0.0, double y_pos = 0.0)
   {
      QString cmd(GCODE_G1XY);
      qDebug() << __FUNCTION__ << "x_pos = " << x_pos;
      cmd = cmd.arg(x_pos).arg(y_pos);
      emit CommandPush(cmd, QVariant());
   }

   void JogLeft(double step = 0.0)
   {
      if ( step == 0.0 ) 
      {
         step = -m_step;
      }
      qDebug() << __FUNCTION__ << "step = " << step;
      m_ypos -= m_step;
      SetRelative();
      emit CommandPush(GCODE_G0Y, QVariant(step));
      SetAbsolute();
   }

   void JogRight(double step = 0.0)
   {
      if ( step == 0.0 ) 
      {
         step = m_step;
      }
      qDebug() << __FUNCTION__ << "step = " << step;
      m_ypos += m_step;
      SetRelative();
      emit CommandPush(GCODE_G0Y, QVariant(step));
      SetAbsolute();
   }

   void JogForward(double step = 0.0)
   {
      if ( step == 0.0 ) 
      {
         step = -m_step;
      }
      qDebug() << __FUNCTION__ << "step = " << step;
      m_xpos -= m_step;
      SetRelative();
      emit CommandPush(GCODE_G0X, QVariant(step));
      SetAbsolute();
   }

   void JogBackward(double step = 0.0)
   {
      if ( step == 0.0 ) 
      {
         step = m_step;
      }
      qDebug() << __FUNCTION__ << "step = " << step;
      m_xpos += m_step;
      SetRelative();
      emit CommandPush(GCODE_G0X, QVariant(step));
      SetAbsolute();
   }

   void JogUp(double step = 0.0)
   {
      if ( step == 0.0 ) 
      {
         step = m_step;
      }
      qDebug() << __FUNCTION__ << "step = " << step;
      m_zpos += m_step;
      SetRelative();
      emit CommandPush(GCODE_G0Z, QVariant(step));
      SetAbsolute();
   }

   void JogDown(double step = 0.0)
   {
      if ( step == 0.0 ) 
      {
         step = -m_step;
      }
      qDebug() << __FUNCTION__ << "step = " << step;
      m_zpos -= m_step;
      SetRelative();
      emit CommandPush(GCODE_G0Z, QVariant(step));
      SetAbsolute();
   }

   void SetOrigin()
   {
      qDebug() << __FUNCTION__ << "Setting origin";
      emit CommandPush(GCODE_G92, QVariant());
      m_xpos = 0;
      m_ypos = 0;
      m_zpos = 0;
   }

   void SetFeedRate(int rate)
   {
      emit CommandPush(GCODE_FEED, QVariant(rate));
   }

   void gotoOrigin()
   {
      emit CommandPush(GCODE_ORG, QVariant());
      emit CommandPush(GCODE_ZOG, QVariant());
      m_xpos = 0;
      m_ypos = 0;
      m_zpos = 0;
   }

   void gotoOriginSansZ()
   {
      emit CommandPush(GCODE_ORG, QVariant());
      m_xpos = 0;
      m_ypos = 0;
   }

   void setCurrentPosition(QVariant x, QVariant y, QVariant z)
   {
      QString cmd(GCODE_CURPOS);
      cmd = cmd.arg(x.toDouble(), 0, 'g', 8).arg(y.toDouble(), 0, 'g', 8).arg(z.toDouble(), 0, 'g', 8);
      emit CommandPush(cmd, QVariant());
   }

   void reset()
   {
      emit WriteCommand(GCODE_RST);
      m_xpos = 0;
      m_ypos = 0;
      m_zpos = 0;
   }

   void dwell(int time)
   {
      emit CommandPush(GCODE_DWELL, QVariant(time));
   }

   void pause()
   {
      emit CommandPush(GCODE_PAUSE, QVariant());
   }

   void toggleCheckCode()
   {
      emit CommandPush(GCODE_CHECK, QVariant());
   }

   void resume()
   {
      m_device->write("\r");
      m_device->flush();
   }

   void motor_on()
   {
      emit CommandPush(GCODE_M8, QVariant());
      emit CommandPush(GCODE_M3, QVariant());
   }


   void motor_off()
   {
      emit CommandPush(GCODE_RESTART, QVariant());
      emit CommandPush(GCODE_M9, QVariant());
      emit CommandPush(GCODE_M5, QVariant());
      emit CommandPush(GCODE_PAUSE, QVariant());
   }

   bool RunScript(QFile *script);

   void WriteCommand(QString cmd)
   {
      qDebug() << __FUNCTION__ << "cmd = " << cmd;
      cmd.replace("\n", "");
      cmd.replace("\r", "");
      if (cmd.contains(m_pauseRE))
      {
         QString message = m_pauseRE.cap(1).replace("(", "").replace(")", "");
         m_device->write("M9\r");
         m_device->flush();
         qDebug() << __FUNCTION__ << "Have pasuse command, message = " << message;
         emit PauseScript(message);
      }
      else if (cmd.contains(GCODE_MY_RESUME))
      {
         emit ResumeScript();
         m_device->write("\r");
         m_device->flush();
      }
      else
      {
         qDebug() << __FUNCTION__ << "Sending '" << cmd << "' to Device";
         m_device->write(cmd.toLocal8Bit());
         m_device->flush();
         emit SentCommand(cmd);
         m_device->write("\r");
         m_device->flush();
      }
   }

signals:
   void SentCommand(QString);
   void PauseScript(QString);
   void ResumeScript();
   void CommandPush(QString, QVariant arg);


private:
   const static QString GCODE_G0X;
   const static QString GCODE_G0Y;
   const static QString GCODE_G0Z;
   const static QString GCODE_G1X;
   const static QString GCODE_G1Y;
   const static QString GCODE_G1Z;
   const static QString GCODE_G1XY;
   const static QString GCODE_G20;
   const static QString GCODE_G21;
   const static QString GCODE_G90;
   const static QString GCODE_G91;
   const static QString GCODE_G92;
   const static QString GCODE_CURPOS;
   const static QString GCODE_DWELL;
   const static QString GCODE_FEED;
   const static QString GCODE_PAUSE;
   const static QString GCODE_RESTART;
   const static QString GCODE_RST;
   const static QString GCODE_ORG;
   const static QString GCODE_ZOG;
   const static QString GCODE_M3;
   const static QString GCODE_M5;
   const static QString GCODE_M8;
   const static QString GCODE_M9;
   const static QString GCODE_CHECK;
   const static QString GCODE_MY_RESUME;
         static QRegExp m_pauseRE;

   QSerialPort*   m_device;
   double         m_step;
   double         m_xpos;
   double         m_ypos;
   double         m_zpos;
   bool           m_metric;
};

#endif // GCODE_CPP
