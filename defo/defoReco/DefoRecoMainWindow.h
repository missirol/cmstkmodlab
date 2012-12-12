#ifndef DEFORECOMAINWINDOW_H
#define DEFORECOMAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTableWidget>
#include <QBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QComboBox>
#include <QComboBox>
#include <QImage>
#include <QTimer>

#include "DefoMeasurementListModel.h"
#include "DefoMeasurementSelectionModel.h"
#include "DefoROIModel.h"
#include "DefoAlignmentModel.h"

#include "DefoMeasurementListTreeWidget.h"
#include "DefoMeasurementInfoListTreeWidget.h"
#include "DefoImageWidget.h"

#include "DefoPointRecognitionModel.h"
#include "DefoPointRecognitionWidget.h"

#include "DefoReconstructionModel.h"
#include "DefoReconstructionWidget.h"

class DefoRecoMainWindow : public QMainWindow
{
  Q_OBJECT
public:
  explicit DefoRecoMainWindow(QWidget *parent = 0);

public slots:

protected:

  QDir currentDir_;

  QTabWidget* tabWidget_;

  DefoMeasurementListModel *listModel_;
  DefoMeasurementSelectionModel *selectionModel_;

  DefoMeasurementSelectionModel *roiSelectionModel_;
  DefoROIModel *roiModel_;

  DefoMeasurementSelectionModel *alignmentSelectionModel_;
  DefoAlignmentModel *alignmentModel_;

  DefoMeasurementSelectionModel *refSelectionModel_;
  DefoMeasurementSelectionModel *defoSelectionModel_;

  DefoPointRecognitionModel* refPointModel_;
  DefoPointRecognitionModel* defoPointModel_;

  DefoReconstructionModel* reconstructionModel_;

protected slots:
  void loadMeasurementButtonClicked();
  void saveMeasurementButtonClicked();
};

#endif // DEFORECOMAINWINDOW_H