#ifndef XSETTINGS_H
#define XSETTINGS_H

# include <QDebug>
# include <QSettings>

class XSettings : public QSettings
{
   Q_OBJECT

public:
   XSettings() : QSettings() 
   {
      qDebug() << __FUNCTION__ << ": Creating object for manf: " << organizationName() << ", app: " << applicationName();
      qDebug() << __FUNCTION__ << ":             Recent Path: " << RECENT_PATH;
   }

   static const QString RECENT_PATH;
   static const QString CHECK_GCODE;
   static const QString COUNT_LINES;
   static const QString HTTP_ENABLE;
   static const QString HTTP_PORT;

private:
};

#endif
