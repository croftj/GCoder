#include <QString>
#include <QFile>

#include "GCode.h"

const QString GCode::GCODE_G0X("G0X%1 \r");           // Goto X
const QString GCode::GCODE_G0Y("G0Y%1 \r");           // Goto Y
const QString GCode::GCODE_G0Z("G0Z%1 \r");           // Goto Z
const QString GCode::GCODE_G1X("G1X%1 \r");           // Move X
const QString GCode::GCODE_G1Y("G1Y%1 \r");           // Move X;
const QString GCode::GCODE_G1Z("G1Z%1 \r");           // Move X;
const QString GCode::GCODE_G1XY("G1X%1Y%2 \r");       // Move X, Y;
const QString GCode::GCODE_G92("G92X0Y0Z0 \r");     // Set Origin
const QString GCode::GCODE_G20("G20\r");              // Set Inches
const QString GCode::GCODE_G21("G21\r");              // Set mm
const QString GCode::GCODE_G90("G90 \r");              // Set Absolute
const QString GCode::GCODE_G91("G91 \r");              // Set Relative
const QString GCode::GCODE_DWELL("G04 P%1 \r");        // Dwell seconds
const QString GCode::GCODE_FEED("F%1 \r");            // Feed Rate units/minute
const QString GCode::GCODE_PAUSE("!");                // Pause
const QString GCode::GCODE_RESTART("~");              // Restart from Pause
const QString GCode::GCODE_RST("\x18");               // Reset
const QString GCode::GCODE_ORG("G0 X0 Y0\r");         // Goto Origin
const QString GCode::GCODE_ZOG("G0 Z0\r");            // Goto Z Origin
const QString GCode::GCODE_CURPOS("G92 X%1 Y%2 Z%3\r");  // Set Current Position
const QString GCode::GCODE_M3("M3\r");                // Laser on  
const QString GCode::GCODE_M5("M5\r");                // Laser off
const QString GCode::GCODE_M8("M8\r");                // Motor on
const QString GCode::GCODE_M9("M9\r");                // Motor off
const QString GCode::GCODE_CHECK("$C\r");               // Check Code toggle
const QString GCode::GCODE_MY_RESUME(GCODE_M8);         // Resume Script
      QRegExp GCode::m_pauseRE("^P[0-9]+ *(?:\\((.+)\\))*");      // Pause Command w/ Message

bool GCode::RunScript(QFile *script)
{
   char buf[1024];
   while ( ! script->atEnd())
   {
      int bcnt = script->readLine(buf, sizeof(buf));
   }
   return(true);
}
