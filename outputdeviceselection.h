#ifndef  OUTPUTDEVICESELECTION_H
#define  OUTPUTDEVICESELECTION_H

#include <QComboBox>
#include <QDebug>
#include <QDialog>
#include <QString>
#include <QSerialPort>
#include <QSerialPortInfo>

#include "ui_outputdeviceselection.h"

class OutputDeviceSelection : public QDialog
{
public:
   OutputDeviceSelection(QWidget *parent = 0) : QDialog(parent)
   {
      m_ui.setupUi(this);

      // Example for using QSerialPortInfo
      QStringList portNames;
#ifdef QT_SERIAL
      foreach (QSerialPortInfo info, QSerialPortInfo::availablePorts())
      {
          qDebug() << "Name        : " << info.portName();
          qDebug() << "Description : " << info.description();
          qDebug() << "Manufacturer: " << info.manufacturer();
          portNames << info.portName();
      }
//      m_ui.deviceCombo->addItems(portNames.filter("USB"));
      qDebug() << __FUNCTION__ << "Adding ports to combo box" << portNames;

      m_ui.deviceCombo->addItems(portNames);

      m_ui.baudrateCombo->clear();
      m_ui.baudrateCombo->addItem("115200",  SerialPort::Rate115200);
      m_ui.baudrateCombo->addItem("57600",   SerialPort::Rate57600);
      m_ui.baudrateCombo->addItem("38400",   SerialPort::Rate38400);
      m_ui.baudrateCombo->addItem("19200",   SerialPort::Rate19200);
      m_ui.baudrateCombo->addItem("9600",    SerialPort::Rate9600);
      m_ui.baudrateCombo->addItem("4800",    SerialPort::Rate4800);
      m_ui.baudrateCombo->addItem("2400",    SerialPort::Rate2400);
      m_ui.baudrateCombo->addItem("1200",    SerialPort::Rate1200);
//      m_ui.baudrateCombo->addItem("600",     SerialPort::Rate600);
//      m_ui.baudrateCombo->addItem("300",     SerialPort::Rate300);
//      m_ui.baudrateCombo->addItem("110",     SerialPort::Rate110);
#else
      m_ui.baudrateCombo->clear();
      m_ui.baudrateCombo->addItem("115200",  QSerialPort::Baud115200);
      m_ui.baudrateCombo->addItem("57600",   QSerialPort::Baud57600);
      m_ui.baudrateCombo->addItem("38400",   QSerialPort::Baud38400);
      m_ui.baudrateCombo->addItem("19200",   QSerialPort::Baud19200);
      m_ui.baudrateCombo->addItem("9600",    QSerialPort::Baud9600);
      m_ui.baudrateCombo->addItem("4800",    QSerialPort::Baud4800);
      m_ui.baudrateCombo->addItem("2400",    QSerialPort::Baud2400);
      m_ui.baudrateCombo->addItem("1200",    QSerialPort::Baud1200);

      QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
      foreach (QSerialPortInfo port, ports)
      {
         portNames << port.portName();
      }
      m_ui.deviceCombo->addItems(portNames.filter("USB"));
#endif
   }

   int baudrate()
   {
      return(m_ui.baudrateCombo->itemData(m_ui.baudrateCombo->currentIndex()).toInt());
   }

   //QSerialPortInfo device()
   QString device()
   {
//      return(QSerialPortInfo(m_ui.deviceCombo->currentText()));
      return(m_ui.deviceCombo->currentText());
   }

protected:

private:
   Ui::OutputDeviceSelection    m_ui;
};

#endif
