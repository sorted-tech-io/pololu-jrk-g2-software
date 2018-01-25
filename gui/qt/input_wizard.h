#pragma once

#include <QWizard>
#include <stdint.h>

class QLabel;
class QProgressBar;

class input_wizard : public QWizard
{
  Q_OBJECT

  // 20 samples, one every 50 ms.  Total time is 1000 ms.
  static const uint32_t SAMPLE_COUNT = 20;

  enum page_number { INTRO, LEARN, CONCLUSION };
  enum learn_step { NEUTRAL, MAX, MIN };

public:
  input_wizard(QWidget * parent = 0);

  struct result
  {
    bool invert = false;
    uint16_t absolute_minimum = 0;
    uint16_t absolute_maximum = 4095;
    uint16_t minimum = 0;
    uint16_t maximum = 4095;
    uint16_t neutral_minimum = 2048;
    uint16_t neutral_maximum = 2048;
  };

  result result;

public slots:
  void set_input(uint16_t);

private:
  void set_progress_visible(bool visible);

  QWizardPage * setup_intro_page();
  QWizardPage * setup_learn_page();
  QLayout * setup_input_layout();

  // Controls on the 'Learn' page
  QLabel * instruction_label;
  QLabel * sampling_label;
  QProgressBar * sampling_progress;
  QLabel * input_value;

  uint16_t input;
};
