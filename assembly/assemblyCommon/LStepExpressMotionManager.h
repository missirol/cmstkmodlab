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

#ifndef LSTEPEXPRESSMOTIONMANAGER_H
#define LSTEPEXPRESSMOTIONMANAGER_H

#include <LStepExpressModel.h>
#include <LStepExpressMotion.h>

#include <vector>

#include <QQueue>

class LStepExpressMotionManager : public QObject
{
 Q_OBJECT

  public:

    explicit LStepExpressMotionManager(LStepExpressModel*, QObject* parent=nullptr);
    virtual ~LStepExpressMotionManager();

    LStepExpressModel* model() const { return model_; }

    bool model_connected() const { return model_connected_; }

    double get_position_X() const { return model()->getPosition(0); }
    double get_position_Y() const { return model()->getPosition(1); }
    double get_position_Z() const { return model()->getPosition(2); }
    double get_position_A() const { return model()->getPosition(3); }

    void myMoveToThread(QThread*);

  protected:

    void run();

    bool AxisIsReady(const int) const;

    LStepExpressModel* model_;

    bool model_connected_;

    bool inMotion_;

    QQueue<LStepExpressMotion> motions_;

  public slots:

    void    connect_model();
    void disconnect_model();

    void appendMotion(const LStepExpressMotion& motion);
    void appendMotions(const QQueue<LStepExpressMotion>& motions);

    void moveRelative(const std::vector<double>& values);
    void moveRelative(const double x=0.0, const double y=0.0, const double z=0.0, const double a=0.0);
    void moveRelative(const unsigned int axis, const double value);

    void moveAbsolute(const std::vector<double>& values);
    void moveAbsolute(const double x=0.0, const double y=0.0, const double z=0.0, const double a=0.0);
    void moveAbsolute(const unsigned int axis, const double value);

    void read_position3D();
    void read_position4D();

    void clear_motion_queue();

    void emergency_stop();

  protected slots:

    void motionStarted();
    void finish_motion();

  signals:

    void motion_finished();

    void signalMoveRelative(double x, double y, double z, double a);
    void signalMoveAbsolute(double x, double y, double z, double a);

    void position3D(const double, const double, const double);
    void position4D(const double, const double, const double, const double);
};

#endif // LSTEPEXPRESSMOTIONMANAGER_H
