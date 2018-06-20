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

#include <AssemblyMainWindow.h>
#include <AssemblyParameters.h>
#include <AssemblyUtilities.h>

#include <string>

#include <QApplication>
#include <QString>

#include <opencv2/opencv.hpp>

AssemblyMainWindow::AssemblyMainWindow(const unsigned int camera_ID, QWidget* parent) :
  QMainWindow(parent),

  // Low-Level Controllers (Motion, Camera, Vacuum)
  conradModel_(nullptr),
  conradManager_(nullptr),

  motion_model_(nullptr),
  motion_manager_(nullptr),
  motion_manager_view_(nullptr),
  motion_thread_(nullptr),
  motionSettings_(nullptr),
  motionSettingsWidget_(nullptr),

  camera_model_(nullptr),
  camera_thread_(nullptr),
//  camera_widget_(nullptr),
  camera_(nullptr),
  camera_ID_(camera_ID),

  // High-Level Controllers
  image_ctr_(nullptr),
  zfocus_finder_(nullptr),
  thresholder_(nullptr),
  aligner_(nullptr),
  assembly_(nullptr),
  multipickup_tester_(nullptr),

  finder_(nullptr),
  finder_thread_(nullptr),

  params_(nullptr),

  redzones_(nullptr),

  // Views
  toolBar_(nullptr),
  tabWidget_(nullptr),

//  finderView_(nullptr),
//  edgeView_(nullptr),
//  rawView_(nullptr),
  image_view_(nullptr),
  thresholder_view_(nullptr),
  finder_view_(nullptr),
  aligner_view_(nullptr),
  assembly_view_(nullptr),
  toolbox_view_(nullptr),
  params_view_(nullptr),
  hwctr_view_(nullptr),

  autofocus_checkbox_(nullptr),

  // flags
  images_enabled_(false),
  aligner_connected_(false),

  // timing
  testTimerCount_(0.),
  liveTimer_(0)
{
    ApplicationConfig* config = ApplicationConfig::instance();
    if(config == nullptr)
    {
      NQLog("AssemblyMainWindow", NQLog::Fatal) << "-------------------------------------------------------------------------------------------------------";
      NQLog("AssemblyMainWindow", NQLog::Fatal) << "initialization error: ApplicationConfig::instance() not initialized (null pointer), exiting constructor";
      NQLog("AssemblyMainWindow", NQLog::Fatal) << "-------------------------------------------------------------------------------------------------------";

      return;
    }

    /// Parameters
    ///   * instance created up here, so controllers can access it
    params_ = AssemblyParameters::instance(config->getValue<std::string>("AssemblyParameters_file_path"));
    /// -------------------

    /// Motion
    motion_model_   = new LStepExpressModel(config->getValue<std::string>("LStepExpressDevice").c_str(), 1000, 1000);
    motion_manager_ = new LStepExpressMotionManager(motion_model_);

    motion_thread_  = new LStepExpressMotionThread(motion_manager_, this);
    motion_thread_->start();
    /// -------------------

    /// Camera
    camera_model_ = new AssemblyUEyeModel_t(10);
    camera_model_->updateInformation();

    camera_thread_ = new AssemblyUEyeCameraThread(camera_model_, this);
    camera_thread_->start();

    camera_ = camera_model_->getCameraByID(camera_ID_);
    if(camera_ == nullptr)
    {
      NQLog("AssemblyMainWindow", NQLog::Critical) << "---------------------------------------------------------------------------------";
      NQLog("AssemblyMainWindow", NQLog::Critical) << "initialization error: null pointer to AssemblyVUEyeCamera object (camera_ID=" << camera_ID_ << ")";
      NQLog("AssemblyMainWindow", NQLog::Critical) << "---------------------------------------------------------------------------------";
    }
    /// -------------------

    /// Vacuum Manager
    conradModel_   = new ConradModel;
    conradManager_ = new ConradManager(conradModel_);
    /// -------------------

    /// TAB WIDGET ----------------------------------------------
    tabWidget_ = new QTabWidget(this);
    tabWidget_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

//    finderView_ = new AssemblyUEyeSnapShooter(tabWidget_);
//    tabWidget_->addTab(finderView_, "finder");

//    edgeView_ = new AssemblyUEyeSnapShooter(tabWidget_);
//    tabWidget_->addTab(edgeView_, "edges");

//    rawView_ = new AssemblyUEyeSnapShooter(tabWidget_);
//    tabWidget_->addTab(rawView_, "raw");

    // IMAGE VIEW ----------------------------------------------
    const QString tabname_Image("Image Viewer");

    image_view_ = new AssemblyImageView(tabWidget_);
    tabWidget_->addTab(image_view_, tabname_Image);

    // Z-focus finder
    zfocus_finder_ = new AssemblyZFocusFinder(camera_, motion_manager_);

    connect(zfocus_finder_, SIGNAL(show_zscan(QString))          , image_view_   , SLOT(update_image_zscan(QString)));
    connect(zfocus_finder_, SIGNAL(text_update_request(double))  , image_view_   , SLOT(update_text(double)));

    connect(zfocus_finder_, SIGNAL(focus_config_request())       , image_view_   , SLOT(acquire_autofocus_config()));
    connect(image_view_   , SIGNAL(autofocus_config(double, int)), zfocus_finder_, SLOT(update_focus_config(double, int)));

    image_view_->update_autofocus_config(zfocus_finder_->zrange(), zfocus_finder_->points());

    NQLog("AssemblyMainWindow", NQLog::Message) << "added view " << tabname_Image;
    // ---------------------------------------------------------

    // IMAGE-THRESHOLDING VIEW ---------------------------------
    const QString tabname_ImageThresholding("Thresholding");

    thresholder_ = new AssemblyThresholder();

    thresholder_view_ = new AssemblyThresholderView(tabWidget_);
    tabWidget_->addTab(thresholder_view_, tabname_ImageThresholding);

    connect(thresholder_view_, SIGNAL(threshold_request        (int)), thresholder_, SLOT(update_image_binary_threshold        (int)));
    connect(thresholder_view_, SIGNAL(adaptiveThreshold_request(int)), thresholder_, SLOT(update_image_binary_adaptiveThreshold(int)));
    connect(thresholder_view_, SIGNAL(loaded_image_raw(cv::Mat))     , thresholder_, SLOT(update_image_raw(cv::Mat)));

    connect(thresholder_, SIGNAL(updated_image_raw   (cv::Mat)), thresholder_view_, SLOT(update_image_raw   (cv::Mat)));
    connect(thresholder_, SIGNAL(updated_image_binary(cv::Mat)), thresholder_view_, SLOT(update_image_binary(cv::Mat)));

    NQLog("AssemblyMainWindow", NQLog::Message) << "added view " << tabname_ImageThresholding;
    // ---------------------------------------------------------

    // PATTERN-RECOGNITION VIEW --------------------------------
    const QString tabname_PatRec("Pattern Recognition");

    finder_view_ = new AssemblyObjectFinderPatRecView(tabWidget_);
    tabWidget_->addTab(finder_view_, tabname_PatRec);

    // finder
    finder_ = new AssemblyObjectFinderPatRec(thresholder_, assembly::QtCacheDirectory()+"/AssemblyObjectFinderPatRec", "rotations");

    finder_view_->connect_to_finder(finder_);

    finder_thread_ = new AssemblyObjectFinderPatRecThread(finder_, this);
    finder_thread_->start();

    NQLog("AssemblyMainWindow", NQLog::Message) << "added view " << tabname_PatRec;
    // ---------------------------------------------------------

    // ALIGNMENT VIEW ------------------------------------------
    const QString tabname_Alignm("Alignment");

    aligner_view_ = new AssemblyObjectAlignerView(tabWidget_);
    tabWidget_->addTab(aligner_view_, tabname_Alignm);

    // aligner
    aligner_ = new AssemblyObjectAligner(motion_manager_);

    connect(aligner_view_, SIGNAL(configuration(AssemblyObjectAligner::Configuration)), this, SLOT(start_objectAligner(AssemblyObjectAligner::Configuration)));

    aligner_view_->PatRecOne_Image()->connectImageProducer(aligner_, SIGNAL(image_PatRecOne(cv::Mat)));
    aligner_view_->PatRecTwo_Image()->connectImageProducer(aligner_, SIGNAL(image_PatRecTwo(cv::Mat)));

    NQLog("AssemblyMainWindow", NQLog::Message) << "added view " << tabname_Alignm;
    // ---------------------------------------------------------

    // ASSEMBLY VIEW -------------------------------------------
    const QString tabname_Assembly("Assembly");

    assembly_ = new AssemblyAssembly(motion_manager_, conradManager_);

    assembly_view_ = new AssemblyAssemblyView(assembly_, tabWidget_);
    tabWidget_->addTab(assembly_view_, tabname_Assembly);

    NQLog("AssemblyMainWindow", NQLog::Message) << "added view " << tabname_Assembly;
    // ---------------------------------------------------------

//    // U-EYE VIEW ----------------------------------------------
//    const QString tabname_uEye("uEye");
//
//    camera_widget_ = new AssemblyUEyeWidget(camera_model_, this);
//    tabWidget_->addTab(camera_widget_, tabname_uEye);
//
//    NQLog("AssemblyMainWindow", NQLog::Message) << "added view " << tabname_uEye;
//    // ---------------------------------------------------------

    // TOOLBOX VIEW ----------------------------------------------
    const QString tabname_Toolbox("Toolbox");

    /// Red-Zones
    redzones_ = new AssemblyRedZones(config->getValue<std::string>("AssemblyRedZones_file_path"));
    /// -------------------

    toolbox_view_ = new AssemblyToolboxView(motion_manager_, redzones_, tabWidget_);
    tabWidget_->addTab(toolbox_view_, tabname_Toolbox);

    // multi-pickup tester
    multipickup_tester_ = new AssemblyMultiPickupTester(motion_manager_);

    connect(toolbox_view_->MultiPickupTester_Widget(), SIGNAL(multipickup_request(AssemblyMultiPickupTester::Configuration)), this, SLOT(start_multiPickupTest(AssemblyMultiPickupTester::Configuration)));

    NQLog("AssemblyMainWindow", NQLog::Message) << "added view " << tabname_Toolbox;
    // -----------------------------------------------------------

    // PARAMETERS VIEW -------------------------------------------
    const QString tabname_Parameters("Parameters");

    params_view_ = new AssemblyParametersView(tabWidget_);
    tabWidget_->addTab(params_view_, tabname_Parameters);

    params_->set_view(params_view_);

    params_view_->copy_values(params_->map_double());

    NQLog("AssemblyMainWindow", NQLog::Message) << "added view " << tabname_Parameters;
    // ---------------------------------------------------------

    // MOTION-SETTINGS VIEW ------------------------------------
    const QString tabname_MotionSettings("Motion Settings");

    motionSettings_ = new LStepExpressSettings(motion_model_, tabWidget_);

    motionSettingsWidget_ = new LStepExpressSettingsWidget(motionSettings_, tabWidget_);
    tabWidget_->addTab(motionSettingsWidget_, tabname_MotionSettings);

    NQLog("AssemblyMainWindow", NQLog::Message) << "added view " << tabname_MotionSettings;
    // ---------------------------------------------------------

    // HARDWARE CONTROLLERs VIEW (motion/vacuum) ---------------
    const QString tabname_HWCtrl("HW Controllers (Motion/Vacuum)");

    hwctr_view_ = new AssemblyHardwareControlView(motion_manager_, tabWidget_);
    tabWidget_->addTab(hwctr_view_, tabname_HWCtrl);

    connect(hwctr_view_->Vacuum_Widget(), SIGNAL(toggleVacuum(int)), conradManager_, SLOT(toggleVacuum(int)));
    connect(conradManager_, SIGNAL(updateVacuumChannelState(int, bool)), hwctr_view_->Vacuum_Widget(), SLOT(updateVacuumChannelState(int, bool)));

    conradManager_->updateVacuumChannelsStatus();

    NQLog("AssemblyMainWindow", NQLog::Message) << "added view " << tabname_HWCtrl;
    // ---------------------------------------------------------

    /// ---------------------------------------------------------

    /// Upper Toolbar -------------------------------------------
    toolBar_ = addToolBar("Tools");
    toolBar_ ->addAction("Camera ON" , this, SLOT( enable_images()));
    toolBar_ ->addAction("Camera OFF", this, SLOT(disable_images()));
    toolBar_ ->addAction("Snapshot"  , this, SLOT(    get_image ()));

    autofocus_checkbox_ = new QCheckBox("Auto-Focusing", this);
    toolBar_->addWidget(autofocus_checkbox_);

    connect(autofocus_checkbox_, SIGNAL(stateChanged(int)), this, SLOT(changeState_autofocus(int)));

    this->setCentralWidget(tabWidget_);

    this->updateGeometry();
    /// ---------------------------------------------------------

    liveTimer_ = new QTimer(this);

    connect(liveTimer_, SIGNAL(timeout()), this, SLOT(liveUpdate()));

    connect(QApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(quit()));

    NQLog("AssemblyMainWindow", NQLog::Message) << "///////////////////////////////////////////////////////";
    NQLog("AssemblyMainWindow", NQLog::Message) << "//                                                   //";
    NQLog("AssemblyMainWindow", NQLog::Message) << "//                     DESY-CMS                      //";
    NQLog("AssemblyMainWindow", NQLog::Message) << "//                                                   //";
    NQLog("AssemblyMainWindow", NQLog::Message) << "//       Automated Pixel-Strip Module Assembly       //";
    NQLog("AssemblyMainWindow", NQLog::Message) << "//                                                   //";
    NQLog("AssemblyMainWindow", NQLog::Message) << "//  - AssemblyMainWindow initialized successfully -  //";
    NQLog("AssemblyMainWindow", NQLog::Message) << "//                                                   //";
    NQLog("AssemblyMainWindow", NQLog::Message) << "///////////////////////////////////////////////////////";
}

void AssemblyMainWindow::liveUpdate()
{
    NQLog("AssemblyMainWindow", NQLog::Debug) << "liveUpdate: emitting signal \"image_request\"";

    emit image_request();
}

void AssemblyMainWindow::enable_images()
{
  if(images_enabled_)
  {
    NQLog("AssemblyMainWindow", NQLog::Warning) << "enable_images"
       << ": images already enabled, no action taken";

    return;
  }

  images_enabled_ = true;

  if(image_ctr_ == nullptr)
  {
    image_ctr_ = new AssemblyImageController(camera_, zfocus_finder_);
  }

  connect(this      , SIGNAL(images_ON())      , image_ctr_, SLOT(enable()));
  connect(this      , SIGNAL(images_OFF())     , image_ctr_, SLOT(disable()));

  connect(image_ctr_, SIGNAL(camera_enabled()) , this      , SLOT(connect_images()));
  connect(image_ctr_, SIGNAL(camera_disabled()), this      , SLOT(disconnect_images()));

  connect(this      , SIGNAL(image_request())  , image_ctr_, SLOT(acquire_image()));
  connect(this      , SIGNAL(autofocus_ON ())  , image_ctr_, SLOT( enable_autofocus()));
  connect(this      , SIGNAL(autofocus_OFF())  , image_ctr_, SLOT(disable_autofocus()));

  NQLog("AssemblyMainWindow", NQLog::Message) << "enable_images"
     << ": ImageController connected";

  NQLog("AssemblyMainWindow", NQLog::Spam) << "enable_images"
     << ": emitting signal \"images_ON\"";

  emit images_ON();
}

void AssemblyMainWindow::disable_images()
{
  if(images_enabled_ == false)
  {
    NQLog("AssemblyMainWindow", NQLog::Warning) << "disable_images"
       << ": images already disabled, no action taken";

    return;
  }

  images_enabled_ = false;

  disconnect(this      , SIGNAL(images_ON())      , image_ctr_, SLOT(enable()));
  disconnect(this      , SIGNAL(images_OFF())     , image_ctr_, SLOT(disable()));

  disconnect(image_ctr_, SIGNAL(camera_enabled()) , this      , SLOT(connect_images()));
  disconnect(image_ctr_, SIGNAL(camera_disabled()), this      , SLOT(disconnect_images()));

  disconnect(this      , SIGNAL(image_request())  , image_ctr_, SLOT(acquire_image()));
  disconnect(this      , SIGNAL(autofocus_ON())   , image_ctr_, SLOT( enable_autofocus()));
  disconnect(this      , SIGNAL(autofocus_OFF())  , image_ctr_, SLOT(disable_autofocus()));

  NQLog("AssemblyMainWindow", NQLog::Message) << "disable_images"
     << ": ImageController disconnected";

  NQLog("AssemblyMainWindow", NQLog::Spam) << "enable_images"
     << ": emitting signal \"images_OFF\"";

  emit images_OFF();
}

void AssemblyMainWindow::changeState_autofocus(const int state)
{
    if(image_ctr_ == nullptr)
    {
      NQLog("AssemblyMainWindow", NQLog::Warning) << "changeState_autofocus"
         << ": ImageController not initialized, no action taken (hint: click \"Camera ON\")";

      return;
    }

    if(motion_model_ == nullptr)
    {
      NQLog("AssemblyMainWindow", NQLog::Warning) << "changeState_autofocus"
         << ": LStepExpressModel not initialized, no action taken (hint: plug-in motion stage cable)";

      return;
    }

    if(state == 2)
    {
      NQLog("AssemblyMainWindow", NQLog::Spam) << "changeState_autofocus"
         << ": emitting signal \"autofocus_ON\"";

      emit autofocus_ON();
    }
    else if(state == 0)
    {
      NQLog("AssemblyMainWindow", NQLog::Spam) << "changeState_autofocus"
         << ": emitting signal \"autofocus_OFF\"";

      emit autofocus_OFF();
    }

    return;
}

void AssemblyMainWindow::get_image()
{
    if(image_ctr_ == nullptr)
    {
      NQLog("AssemblyMainWindow", NQLog::Warning) << "get_image"
         << ": ImageController not initialized, no action taken (hint: click \"Camera ON\")";

      return;
    }

    if(image_ctr_->is_enabled() == false)
    {
      NQLog("AssemblyMainWindow", NQLog::Warning) << "get_image"
         << ": ImageController not enabled, no action taken (hint: click \"Camera ON\")";

      return;
    }

    NQLog("AssemblyMainWindow", NQLog::Spam) << "get_image"
       << ": emitting signal \"image_request\"";

    emit image_request();
}

void AssemblyMainWindow::connect_images()
{
//  finderView_->connectImageProducer(finder_, SIGNAL(markerFound  (const cv::Mat&)));
//  edgeView_  ->connectImageProducer(finder_, SIGNAL(edgesDetected(const cv::Mat&)));
//  rawView_   ->connectImageProducer(camera_, SIGNAL(imageAcquired(const cv::Mat&)));

  connect(image_ctr_, SIGNAL(image_acquired(cv::Mat)), image_view_ , SLOT(update_image(cv::Mat)));
  connect(image_ctr_, SIGNAL(image_acquired(cv::Mat)), thresholder_, SLOT(update_image_raw   (cv::Mat)));
  connect(image_ctr_, SIGNAL(image_acquired(cv::Mat)), finder_     , SLOT(update_image_master(cv::Mat)));

  connect(image_view_, SIGNAL(image_loaded(cv::Mat)), image_ctr_, SLOT(retrieve_image(cv::Mat)));

  connect(image_view_->autofocus_button(), SIGNAL(clicked()), image_ctr_, SLOT(acquire_autofocused_image()));

  NQLog("AssemblyMainWindow", NQLog::Message) << "connect_images"
     << ": enabled images in application view(s)";

//  liveTimer_->start(2000);
}

void AssemblyMainWindow::disconnect_images()
{
//  finderView_->disconnectImageProducer(finder_, SIGNAL(markerFound  (const cv::Mat&)));
//  edgeView_  ->disconnectImageProducer(finder_, SIGNAL(edgesDetected(const cv::Mat&)));
//  rawView_   ->disconnectImageProducer(camera_, SIGNAL(imagef       (const cv::Mat&)));

  disconnect(image_ctr_, SIGNAL(image_acquired(cv::Mat)), image_view_ , SLOT(update_image(cv::Mat)));
  disconnect(image_ctr_, SIGNAL(image_acquired(cv::Mat)), thresholder_, SLOT(update_image_raw   (cv::Mat)));
  disconnect(image_ctr_, SIGNAL(image_acquired(cv::Mat)), finder_     , SLOT(update_image_master(cv::Mat)));

  disconnect(image_view_, SIGNAL(image_loaded(cv::Mat)), image_ctr_, SLOT(retrieve_image(cv::Mat)));

  disconnect(image_view_->autofocus_button(), SIGNAL(clicked()), image_ctr_, SLOT(acquire_autofocused_image()));

  NQLog("AssemblyMainWindow", NQLog::Message) << "disconnect_images"
     << ": disabled images in application view(s)";

  liveTimer_->stop();
}

void AssemblyMainWindow::start_objectAligner(const AssemblyObjectAligner::Configuration& conf)
{
  if(image_ctr_ == nullptr)
  {
    NQLog("AssemblyMainWindow", NQLog::Warning) << "start_objectAligner"
       << ": ImageController not initialized, no action taken (hint: click \"Camera ON\")";

    return;
  }

  if(aligner_connected_)
  {
    NQLog("AssemblyMainWindow", NQLog::Warning) << "start_objectAligner"
       << ": AssemblyObjectAligner already connected, no action taken";

    return;
  }

  if(params_ != nullptr)
  {
    const bool valid_params = params_->update();

    if(valid_params == false)
    {
      NQLog("AssemblyMainWindow", NQLog::Warning) << "start_objectAligner"
         << ": failed to update AssemblyParameters, no action taken";

      return;
    }
  }

  // acquire image
  connect(aligner_, SIGNAL(image_request()), image_ctr_, SLOT(acquire_image()));

  // master-image updated, go to next step (PatRec)
  connect(finder_, SIGNAL(updated_image_master()), aligner_, SLOT(launch_next_alignment_step()));

  // launch PatRec
  connect(aligner_, SIGNAL(PatRec_request(AssemblyObjectFinderPatRec::Configuration)), finder_, SLOT(launch_PatRec(AssemblyObjectFinderPatRec::Configuration)));

  // show PatRec-edited image in Aligner widget
  connect(finder_, SIGNAL(PatRec_res_image_master_edited(cv::Mat)), aligner_, SLOT(redirect_image(cv::Mat)));

  // use PatRec results for next alignment step
  connect(finder_, SIGNAL(PatRec_results(double, double, double)), aligner_, SLOT(run_alignment(double, double, double)));

  // show measured angle
  connect(aligner_, SIGNAL(measured_angle(double)), aligner_view_, SLOT(show_measured_angle(double)));

  // once completed, disable connections between controllers used for alignment
  connect(aligner_, SIGNAL(execution_completed()), this, SLOT(disconnect_objectAligner()));

  aligner_view_->Configuration_Widget()->setEnabled(false);

  aligner_connected_ = true;

  // if successful, emits signal "configuration_updated()"
  aligner_->update_configuration(conf);

  return;
}

void AssemblyMainWindow::disconnect_objectAligner()
{
  if(image_ctr_ == nullptr)
  {
    NQLog("AssemblyMainWindow", NQLog::Warning) << "disconnect_objectAligner"
       << ": ImageController not initialized, no action taken (hint: click \"Camera ON\")";

    return;
  }

  if(aligner_connected_ == false)
  {
    NQLog("AssemblyMainWindow", NQLog::Warning) << "disconnect_objectAligner"
       << ": AssemblyObjectAligner already disconnected, no action taken";

    return;
  }

  // acquire image
  disconnect(aligner_, SIGNAL(image_request()), image_ctr_, SLOT(acquire_image()));

  // master-image updated, go to next step (PatRec)
  disconnect(finder_, SIGNAL(updated_image_master()), aligner_, SLOT(launch_next_alignment_step()));

  // launch PatRec
  disconnect(aligner_, SIGNAL(PatRec_request(AssemblyObjectFinderPatRec::Configuration)), finder_, SLOT(launch_PatRec(AssemblyObjectFinderPatRec::Configuration)));

  // show PatRec-edited image in Aligner widget
  disconnect(finder_, SIGNAL(PatRec_res_image_master_edited(cv::Mat)), aligner_, SLOT(redirect_image(cv::Mat)));

  // use PatRec results for next alignment step
  disconnect(finder_, SIGNAL(PatRec_results(double, double, double)), aligner_, SLOT(run_alignment(double, double, double)));

  // show measured angle
  disconnect(aligner_, SIGNAL(measured_angle(double)), aligner_view_, SLOT(show_measured_angle(double)));

  // once completed, disable connections between controllers used for alignment
  disconnect(aligner_, SIGNAL(execution_completed()), this, SLOT(disconnect_objectAligner()));

  aligner_view_->Configuration_Widget()->setEnabled(true);

  aligner_connected_ = false;

  return;
}

void AssemblyMainWindow::start_multiPickupTest(const AssemblyMultiPickupTester::Configuration& conf)
{
  if(image_ctr_ == nullptr)
  {
    NQLog("AssemblyMainWindow", NQLog::Warning) << "start_multiPickupTest"
       << ": ImageController not initialized, no action taken (hint: click \"Camera ON\")";

    return;
  }

  if(params_ != nullptr)
  {
    const bool valid_params = params_->update();

    if(valid_params == false)
    {
      NQLog("AssemblyMainWindow", NQLog::Warning) << "start_multiPickupTest"
         << ": failed to update AssemblyParameters, no action taken";

      return;
    }
  }

  toolbox_view_->MultiPickupTester_Widget()->enable(false);

  connect(multipickup_tester_, SIGNAL(measurement_finished()), multipickup_tester_, SLOT(start_pickup()));
  connect(multipickup_tester_, SIGNAL(pickup_finished())     , multipickup_tester_, SLOT(start_measurement()));
  connect(multipickup_tester_, SIGNAL(test_finished())       , this               , SLOT(disconnect_multiPickupTest()));

  // measurement
  connect(multipickup_tester_, SIGNAL(measurement_request()), image_ctr_, SLOT(acquire_image()));

  connect(finder_, SIGNAL(updated_image_master()), finder_view_->PatRec_exe_button(), SLOT(click()));
  connect(finder_, SIGNAL(PatRec_exitcode(int)), multipickup_tester_, SLOT(finish_measurement(int)));
  // ---

  // pickup (vacuum)
  connect(multipickup_tester_, SIGNAL(vacuum_toggle(int)), conradManager_, SLOT(toggleVacuum(int)));

  connect(conradManager_, SIGNAL(enableVacuumButton()), multipickup_tester_, SLOT(setup_next_step()));
  // ---

  multipickup_tester_->set_configuration(conf);

  multipickup_tester_->start_measurement();

  return;
}

void AssemblyMainWindow::disconnect_multiPickupTest()
{
  disconnect(multipickup_tester_, SIGNAL(measurement_finished()), multipickup_tester_, SLOT(start_pickup()));
  disconnect(multipickup_tester_, SIGNAL(pickup_finished())     , multipickup_tester_, SLOT(start_measurement()));
  disconnect(multipickup_tester_, SIGNAL(test_finished())       , this               , SLOT(disconnect_multiPickupTest()));

  // measurement
  disconnect(multipickup_tester_, SIGNAL(measurement_request()), image_ctr_, SLOT(acquire_image()));

  disconnect(finder_, SIGNAL(updated_image_master()), finder_view_->PatRec_exe_button(), SLOT(click()));
  disconnect(finder_, SIGNAL(PatRec_exitcode(int)), multipickup_tester_, SLOT(finish_measurement(int)));
  // ---

  // pickup (vacuum)
  disconnect(multipickup_tester_, SIGNAL(vacuum_toggle(int)), conradManager_, SLOT(toggleVacuum(int)));

  disconnect(conradManager_, SIGNAL(enableVacuumButton()), multipickup_tester_, SLOT(setup_next_step()));
  // ---

  toolbox_view_->MultiPickupTester_Widget()->enable(true);

  NQLog("AssemblyMainWindow", NQLog::Spam) << "disconnect_multiPickupTest"
     << ": emitting signal \"multiPickupTest_disconnected\"";

  emit multiPickupTest_disconnected();

  NQLog("AssemblyMainWindow", NQLog::Message) << "disconnect_multiPickupTest"
     << ": multi-pickup test completed";
}

void AssemblyMainWindow::testTimer()
{
    NQLog("AssemblyMainWindow", NQLog::Spam) << "testTimer"
       << ": timeOut=" << testTimerCount_;

    testTimerCount_ += 0.1;

    return;
}

void AssemblyMainWindow::quit_thread(QThread* thread, const QString& msg) const
{
    if(thread != nullptr)
    {
      thread->quit();

      if(thread->wait(2000) == false)
      {
         thread->terminate();
         thread->wait();
      }

      NQLog("AssemblyMainWindow", NQLog::Spam) << "quit_thread: "+msg;
    }
}

void AssemblyMainWindow::quit()
{
    if(camera_ != nullptr)
    {
      NQLog("AssemblyMainWindow", NQLog::Spam) << "quit"
         << ": emitting signal \"images_OFF\"";

      emit images_OFF();

      camera_ = 0;
    }

    this->quit_thread(motion_thread_, "terminated LStepExpressMotionThread");
    this->quit_thread(camera_thread_, "terminated AssemblyUEyeCameraThread");
    this->quit_thread(finder_thread_, "terminated AssemblyObjectFinderPatRecThread");

    NQLog("AssemblyMainWindow", NQLog::Message) << "quit: application closed";

    return;
}
