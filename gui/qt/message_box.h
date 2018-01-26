#pragma once

#include <QMessageBox>

namespace
{
  void show_error_message(const std::string & message, QWidget * parent)
  {
    QMessageBox mbox(QMessageBox::Critical, parent->windowTitle(),
      QString::fromStdString(message), QMessageBox::NoButton, parent);
    mbox.exec();
  }

  void show_info_message(const std::string & message, QWidget * parent)
  {
    QMessageBox mbox(QMessageBox::Information, parent->windowTitle(),
      QString::fromStdString(message), QMessageBox::NoButton, parent);
    mbox.exec();
  }

  void show_warning_message(const std::string & message, QWidget * parent)
  {
    QMessageBox mbox(QMessageBox::Warning, parent->windowTitle(),
      QString::fromStdString(message), QMessageBox::NoButton, parent);
    mbox.exec();
  }

  bool confirm(const std::string & question, QWidget * parent)
  {
    QMessageBox mbox(QMessageBox::Question, parent->windowTitle(),
      QString::fromStdString(question),
      QMessageBox::Ok | QMessageBox::Cancel, parent);
    int button = mbox.exec();
    return button == QMessageBox::Ok;
  }
}