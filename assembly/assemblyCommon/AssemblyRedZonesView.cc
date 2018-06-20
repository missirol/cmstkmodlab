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

#include <AssemblyRedZonesView.h>
#include <AssemblyUtilities.h>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBox>
#include <QLabel>

AssemblyRedZonesView::AssemblyRedZonesView(const AssemblyRedZones* const redzones, QWidget* parent)
 : QWidget(parent)
{
  if(redzones == nullptr)
  {
    NQLog("AssemblyRedZonesView", NQLog::Fatal) << "initialization error"
       << ": null pointer to AssemblyRedZones object, exiting constructor";

    return;
  }

  QVBoxLayout* layout = new QVBoxLayout;
  this->setLayout(layout);

  uint count(0);

  for(const auto& i_redzone : redzones->valuemap())
  {
    ++count;

    const auto& key = i_redzone.first;

    map_[key] = new AssemblyRedZoneWidget;

    this->get(key)->label1()  ->setText(QString::number(count));
    this->get(key)->label2()  ->setText(QString::fromStdString(key));
    this->get(key)->lineEdit()->setText(i_redzone.second);

    layout->addWidget(this->get(key));
  }

  layout->addStretch(1);
}

bool AssemblyRedZonesView::has(const std::string& key) const
{
  return bool(map_.find(key) != map_.end());
}

AssemblyRedZoneWidget* AssemblyRedZonesView::get(const std::string& key) const
{
  AssemblyRedZoneWidget* ptr(nullptr);

  if(this->has(key) == false)
  {
    NQLog("AssemblyRedZonesView", NQLog::Fatal) << "get"
       << ": no AssemblyRedZoneWidget object associated to parameter key \"" << key << "\", closing application";

    QMessageBox::critical(0
     , tr("[AssemblyRedZonesView::get]")
     , tr("Failed to find AssemblyRedZoneWidget for key: \"%1\"\n.").arg(QString(key.c_str()))
     , QMessageBox::Abort
    );

    throw; // must abort
  }
  else
  {
    ptr = map_.at(key);
  }

  if(ptr == nullptr)
  {
    NQLog("AssemblyRedZonesView", NQLog::Fatal) << "get"
       << ": null pointer to AssemblyRedZoneWidget associated to key \"" << key << "\", closing application";

    QMessageBox::critical(0
     , tr("[AssemblyRedZonesView::get]")
     , tr("Null pointer to AssemblyRedZoneWidget for key: \"%1\"\n.").arg(QString(key.c_str()))
     , QMessageBox::Abort
    );

    throw; // must abort
  }

  return ptr;
}
// ====================================================================================================

AssemblyRedZoneWidget::AssemblyRedZoneWidget(QWidget* parent)
 : QWidget(parent)
 , layout_  (nullptr)
 , label1_  (nullptr)
 , label2_  (nullptr)
 , lineEdit_(nullptr)
{
  // layout
  layout_ = new QHBoxLayout;
  this->setLayout(layout_);

  label1_ = new QLabel;
  label1_->setStyleSheet(
    "QLabel { font-weight : bold; }"
  );

  label2_ = new QLabel;

  lineEdit_ = new QLineEdit;

  layout_->addWidget(label1_   ,  2, Qt::AlignRight);
  layout_->addWidget(label2_   , 40);
  layout_->addWidget(lineEdit_ , 58);
  // --------------
}
// ====================================================================================================
