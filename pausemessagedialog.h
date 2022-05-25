#ifndef  PAUSEMESSAGEDIALOG_H
#define  PAUSEMESSAGEDIALOG_H

#include <QComboBox>
#include <QDebug>
#include <QDialog>
#include <QString>
#include <QSerialPort>
#include <QSerialPortInfo>

#include "ui_pausemessagedialog.h"

class PauseMessageDialog : public QDialog
{
public:
   PauseMessageDialog(GCode *g_code, QString message, 
                      int auto_max_step, int auto_interval, 
                      QWidget *parent = 0) : 
      QDialog(parent),
      m_gCode(g_code),
      m_autoMaxStep(auto_max_step),
      m_autoIncrement(auto_interval)
   {
      m_ui.setupUi(this);
      connect(&m_timer,          SIGNAL(timeout()),         this,    SLOT(SlotMoveTimer()));
      connect(m_ui.zSlider,      SIGNAL(sliderPressed()),   this,    SLOT(SlotSliderStatus()));
      connect(m_ui.zSlider,      SIGNAL(sliderReleased()),  this,    SLOT(SlotSliderStatus()));
      connect(m_ui.upBtn,        SIGNAL(clicked()),         m_gCode, SLOT(JogUp()));
      connect(m_ui.dwnBtn,       SIGNAL(clicked()),         m_gCode, SLOT(JogDown()));
      connect(m_ui.stopRunBtn,   SIGNAL(clicked()),         this,    SLOT(reject()));

      m_stepSave = m_gCode->GetStep();
      m_gCode->SetStep(0.1);
      m_metricSave = m_gCode->SetMetric();
      m_ui.textEdit->setText(message);
   }

protected slots:
   void SlotSliderStatus()
   {
      if ( m_ui.zSlider->isSliderDown() ) 
      {
         m_gCode->SetMetric();
         m_timer.start(m_autoIncrement);
      }
      else 
      {
         m_ui.zSlider->setValue(0);
         m_timer.stop();
      }
   }

   void SlotMoveTimer()
   {
      if ( m_ui.zSlider->isSliderDown() && m_ui.zSlider->value() != 0 )
      {
         m_gCode->JogUp(m_ui.zSlider->value());
      }
      else 
      {
         m_ui.zSlider->setValue(0);
      }
   }

protected:
   void closeEvent(QCloseEvent *e) override
   {
      if (m_metricSave)
      {
         m_gCode->SetMetric();
      }
      else
      {
         m_gCode->SetInches();
      }
      m_gCode->SetStep(m_stepSave);
      QDialog::closeEvent(e);
   }

private:
   int                        m_autoMaxStep;
   int                        m_autoIncrement;
   bool                       m_metricSave;
   double                     m_stepSave;
   GCode*                     m_gCode;
   QTimer                     m_timer;
   Ui::PauseMessageDialog     m_ui;
};

#endif
