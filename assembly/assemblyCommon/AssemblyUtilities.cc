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

#include <AssemblyUtilities.h>

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <QDesktopServices>
#else
#include <QStandardPaths>
#endif
#include <QDir>
#include <QFileInfo>

QString assembly::QtCacheDirectory()
{
 #if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  return QDesktopServices::storageLocation(QDesktopServices::CacheLocation);
 #else
  return QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
 #endif
}

bool assembly::QDir_mkpath(const QString& path)
{
  const QDir dir(path);
  if(dir.exists() == false){ return dir.mkpath("."); }

  return true;
}

bool assembly::QDir_mkpath(const std::string& path)
{
  return assembly::QDir_mkpath(QString(path.c_str()));
}

bool assembly::DirectoryExists(const QString& path)
{
  const QDir dir(path);

  return dir.exists();
}

bool assembly::DirectoryExists(const std::string& path)
{
  return assembly::DirectoryExists(QString(path.c_str()));
}

bool assembly::IsFile(const QString& path)
{
  const QFileInfo qfileinfo(path);

  return (qfileinfo.exists() && qfileinfo.isFile());
}

bool assembly::IsFile(const std::string& path)
{
  return assembly::IsFile(QString(path.c_str()));
}

cv::Mat assembly::cv_imread_png(const std::string& path, const int imread_flag)
{
  const cv::Mat img_inp = cv::imread(path, imread_flag);

  cv::Mat img_out;

  if(img_inp.channels() > 1)
  {
    cv::cvtColor(img_inp, img_out, CV_RGB2BGR);
  }
  else
  {
    img_out = img_inp.clone();
  }

  return img_out;
}

void assembly::cv_imwrite_png(const std::string& path, const cv::Mat& img)
{
  cv::Mat img_conv;

  if(img.channels() > 1)
  {
    cv::cvtColor(img, img_conv, CV_BGR2RGB);
  }
  else
  {
    img_conv = img.clone();
  }

  cv::imwrite(path, img_conv);

  return;
}