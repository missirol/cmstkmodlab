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

#ifndef ASSEMBLYREDZONESVIEW_H
#define ASSEMBLYREDZONESVIEW_H

#include <AssemblyRedZones.h>

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>

#include <string>
#include <map>

class AssemblyRedZoneWidget;

class AssemblyRedZonesView : public QWidget
{
 Q_OBJECT

 public:

  explicit AssemblyRedZonesView(const AssemblyRedZones* const, QWidget* parent=nullptr);
  virtual ~AssemblyRedZonesView() {}

  bool has(const std::string&) const;
  AssemblyRedZoneWidget* get(const std::string&) const;

 protected:

  std::map<std::string, AssemblyRedZoneWidget*> map_;
};
// ====================================================================================================

class AssemblyRedZoneWidget : public QWidget
{
 Q_OBJECT

 public:

  explicit AssemblyRedZoneWidget(QWidget* parent=nullptr);
  virtual ~AssemblyRedZoneWidget() {}

  QHBoxLayout* layout()   const { return layout_; }

  QLabel*      label1()   const { return label1_; }
  QLabel*      label2()   const { return label2_; }
  QLineEdit*   lineEdit() const { return lineEdit_; }

 protected:

  QHBoxLayout* layout_;

  QLabel*      label1_;
  QLabel*      label2_;
  QLineEdit*   lineEdit_;

 public slots:

 signals:
};
// ====================================================================================================

#endif // ASSEMBLYREDZONESVIEW_H
