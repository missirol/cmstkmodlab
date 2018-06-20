/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//               Copyright (C) 2011-2017 - The DESY CMS Group                  //
//                           All rights reserved                               //
//                                                                             //
//      The CMStkModLab source code is licensed under the GNU GPL v3.0.        //
//      You have the right to modify and/or redistribute this source code      //
//      under the terms specified in the license, which may be found online    //
//      at http://www.gnu.org/licenses or at License.txt.                      //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

#include <nqlogger.h>
#include <ApplicationConfig.h>
#include <ApplicationConfigReader.h>

#include <AssemblyRedZones.h>
#include <AssemblyUtilities.h>

#include <sstream>

#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QScriptEngine>

AssemblyRedZones::AssemblyRedZones(const std::string& file_path, QObject* parent)
 : QObject(parent)
//!! , view_(nullptr)
{
  NQLog("AssemblyRedZones", NQLog::Debug) << "constructed";

  this->read_from_file(file_path);
}

AssemblyRedZones::~AssemblyRedZones()
{
  NQLog("AssemblyRedZones", NQLog::Debug) << "destructed";
}

void AssemblyRedZones::issue_key_error(const std::string& key) const
{
  NQLog("AssemblyRedZones", NQLog::Critical) << "issue_key_error"
     << " [issue_key_error::getValue] ** ERROR: failed to get value for key: " << key;

  QMessageBox::critical(0
   , tr("[AssemblyRedZones::issue_key_error]")
   , tr("Failed to find value for key: \"%1\"\n. Aborting.").arg(QString(key.c_str()))
   , QMessageBox::Abort
  );

  throw; // must abort
}

QString AssemblyRedZones::get(const std::string& key) const
{
  if(valuemap_.find(key) == valuemap_.end())
  {
    this->issue_key_error(key);
  }

  return valuemap_.at(key);
}

bool AssemblyRedZones::eval(const std::string& key, const double X0, const double Y0, const double Z0, const double A0) const
{
  QString expr = this->get(key);

  expr = expr.toLower();

  QScriptEngine e;

  e.globalObject().setProperty("x", X0);
  e.globalObject().setProperty("y", Y0);
  e.globalObject().setProperty("z", Z0);
  e.globalObject().setProperty("a", A0);

  return e.evaluate(expr).toBool();
}

bool AssemblyRedZones::eval_all(const double X0, const double Y0, const double Z0, const double A0) const
{
  for(const auto& i_val : valuemap_)
  {
    const bool i_res = this->eval(i_val.first, X0, Y0, Z0, A0);

    if(i_res == false){ return false; }
  }

  return true;
}

//!!void AssemblyRedZones::set_view(const AssemblyRedZonesView* const view)
//!!{
//!!  if(view == nullptr)
//!!  {
//!!    NQLog("AssemblyRedZones", NQLog::Critical) << "set_view"
//!!       << ": NULL pointer to AssemblyRedZonesView, no action taken";
//!!
//!!    return;
//!!  }
//!!
//!!  if(view_ != nullptr)
//!!  {
//!!    disconnect(view_, SIGNAL(read_from_file_request(QString)), this, SLOT(read_from_file(QString)));
//!!    disconnect(view_, SIGNAL( write_to_file_request(QString)), this, SLOT( write_to_file(QString)));
//!!  }
//!!
//!!  view_ = view;
//!!
//!!  connect(view_, SIGNAL(read_from_file_request(QString)), this, SLOT(read_from_file(QString)));
//!!  connect(view_, SIGNAL( write_to_file_request(QString)), this, SLOT( write_to_file(QString)));
//!!
//!!  return;
//!!}

void AssemblyRedZones::write_to_file(const QString& f_path)
{
  QFile data(f_path);

  if(data.open(QFile::WriteOnly | QFile::Truncate))
  {
    QTextStream out(&data);

    for(const auto& i_pair : valuemap_)
    {
      out << qSetFieldWidth(30) << left << QString::fromStdString(i_pair.first) << left << i_pair.second << "\n";
    }
  }

  return;
}

void AssemblyRedZones::write_to_file(const std::string& f_path)
{
  this->write_to_file(QString::fromStdString(f_path));
}

void AssemblyRedZones::read_from_file(const QString& f_path)
{
  this->read_from_file(std::string(f_path.toUtf8().constData()));
}

void AssemblyRedZones::read_from_file(const std::string& f_path)
{
  ApplicationConfigReader reader(f_path);

  std::multimap<std::string, std::string> multimap_str;
  reader.fill(multimap_str);

  valuemap_.clear();

  for(const auto& i_pair : multimap_str)
  {
    const std::string& i_key = i_pair.first;

    if(valuemap_.find(i_key) != valuemap_.end())
    {
      NQLog("AssemblyRedZones", NQLog::Warning) << "read_from_file"
         << ": duplicate AssemblyRedZone entry \"" << i_key << "\", will be overwritten";
    }

    valuemap_[i_key] = QString::fromStdString(i_pair.second);
  }

  return;
}

bool AssemblyRedZones::update()
{
//!!  if(view_ == nullptr)
//!!  {
//!!    NQLog("AssemblyRedZones", NQLog::Critical) << "update"
//!!       << ": NULL pointer to AssemblyRedZonesView, no action taken";
//!!
//!!    return false;
//!!  }
//!!
//!!  const auto& map_str = view_->entries_map();
//!!
//!!  valuemap_.clear();
//!!
//!!  for(const auto& i_pair : map_str)
//!!  {
//!!    const std::string& i_key = i_pair.first;
//!!
//!!    if(valuemap_.find(i_key) != valuemap_.end())
//!!    {
//!!      NQLog("AssemblyRedZones", NQLog::Warning) << "update"
//!!         << ": duplicate assembly parameter \"" << i_key << "\", will be overwritten";
//!!    }
//!!
//!!    valuemap_[i_key] = QString::fromStdString(i_pair.second);
//!!  }

  return true;
}
