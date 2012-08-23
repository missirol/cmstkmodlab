#ifndef DEFOKEITHLEYMODEL_H
#define DEFOKEITHLEYMODEL_H

#include <vector>
#include <limits>

#include <QObject>
#include <QString>
#include <QTimer>

#include "DefoState.h"

#ifdef USE_FAKEIO
#include "devices/Keithley/Keithley2700Fake.h"
typedef Keithley2700Fake Keithley2700_t;
#else
#include "devices/Keithley/Keithley2700.h"
typedef Keithley2700 Keithley2700_t;
#endif

class DefoKeithleyModel :
      public QObject
    , public DefoAbstractDeviceModel<Keithley2700_t>
{
    Q_OBJECT
public:
  static const unsigned int SENSOR_COUNT = 10;

  explicit DefoKeithleyModel(
      double updateInterval = 60
    , QObject *parent = 0
  );

  const State& getSensorState( unsigned int sensor ) const;
  double getTemperature( unsigned int sensor ) const;

public slots:
  void setDeviceEnabled( bool enabled );
  void setSensorEnabled( unsigned int sensor, bool enabled );

protected:
  // FIXME Find out which port this needs to be or autoconfigure
  static const QString KEITHLEY_PORT;

  void initialize();

  /// Time interval between cache refreshes; in seconds.
  const double updateInterval_;
  QTimer* timer_;

  // cached config information
  std::vector<State> sensorStates_;
  std::vector<double> temperatures_;

  void setDeviceState( State state );
  void setSensorState( unsigned int sensor, State state );

  static std::string constructString( unsigned int sensor );

protected slots:
  void scanTemperatures();

signals:
  void deviceStateChanged(State newState);
  void sensorStateChanged(unsigned int sensor, State newState);
  void temperatureChanged(unsigned int sensor, double temperature);

};

#endif // DEFOKEITHLEYMODEL_H
