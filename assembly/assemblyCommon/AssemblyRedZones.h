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

#ifndef ASSEMBLYREDZONES_H
#define ASSEMBLYREDZONES_H

/*  Description:
 *    - contains expressions defining (X,Y,Z,A) areas forbidden to the motion stage
 */

#include <string>
#include <map>

#include <QObject>
#include <QString>

class AssemblyRedZones : public QObject
{
 Q_OBJECT

 public:

  explicit AssemblyRedZones(const std::string&, QObject* parent=nullptr);
  virtual ~AssemblyRedZones();

  void issue_key_error(const std::string&) const;

  QString get(const std::string&) const;

  bool eval(const std::string&, const double, const double, const double, const double) const;

  bool eval_all(const double, const double, const double, const double) const;

  const std::map<std::string, QString>& valuemap() const { return valuemap_; }

  bool update();

 private:
  Q_DISABLE_COPY(AssemblyRedZones)

 protected:

  std::map<std::string, QString> valuemap_;

 public slots:

  void write_to_file(const QString&);
  void write_to_file(const std::string&);

  void read_from_file(const QString&);
  void read_from_file(const std::string&);

 signals:
};

#endif // ASSEMBLYREDZONES_H
