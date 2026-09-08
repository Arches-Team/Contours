// System

#include <QtCore/QProcessEnvironment>
#include <QtCore/QFileInfo>

#include "cpu.h"

bool System::avx = true;

/*!
Get the Advanced Vector Extensions flag.
\param a Boolean, set to true to run AVX accelerated algorithms.
*/
bool System::Avx()
{
  return avx;
}

/*!
\brief Get the environment variable with Qt.
\param name Name.
*/
QString System::GetEnv(const QString& name)
{
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  return env.value(name);
}

QString System::GetResource(const QString& env, const QString& name)
{
  QString envvalue = System::GetEnv(env);
  if (envvalue.isEmpty())
  {
    // The env variable is not defined, we will try to find the file in the current directory
    envvalue = ".";
    if (name.isEmpty())
      return envvalue; // a precise filename has not been asked, we just fall back to the current directory

    QString fullPath = envvalue + name;
    QFileInfo check_file(fullPath);
    if (check_file.exists())
    {
      return fullPath;
    }
    else {  // if file is still not found, remove the path and search only the filename in the current directory
      QString justName = QFileInfo(name).fileName();
      QFileInfo check_file2(justName);
      if (check_file2.exists())
      {
        return justName;
      }

    }
  }
  else
  {
    return envvalue + name;
  }
  return QString();
}

/*
\brief Set s string according to date and time.

This function is useful for labeling screenshots.
\param n Integer.
*/
QString System::DateTime()
{
  // Date and time
  QDate date = QDate::currentDate();
  QTime time = QTime::currentTime();

  QString s = QString("%1%2%3-%4%5%6")
    .arg(date.year(), 4)
    .arg(date.month(), 2, 10, QChar('0'))
    .arg(date.day(), 2, 10, QChar('0'))
    .arg(time.hour(), 2, 10, QChar('0'))
    .arg(time.minute(), 2, 10, QChar('0'))
    .arg(time.second(), 2, 10, QChar('0'));
  return s;
}
