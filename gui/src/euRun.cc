#include <QApplication>
#include <QDateTime>
#include <fstream>
#include "euRun.hh"
#include "Colours.hh"
#include "config.h" // for version symbols


RunControlGUI::RunControlGUI()
  : QMainWindow(0, 0),m_state(eudaq::Status::STATE_UNINIT){
  m_map_label_str = {
    {"RUN", "Run Number"}
    // {"FULLRATE","Rate"}, {"TRIG", "Triggers"},
    // {"FILEBYTES", "File Bytes"},{"PARTICLES", "Particles"},
    // {"TLUSTAT", "TLU Status"}, {"SCALERS", "Scalers"}
  };
  
  qRegisterMetaType<QModelIndex>("QModelIndex");
  setupUi(this);

  if (!grpStatus->layout())
    grpStatus->setLayout(new QGridLayout(grpStatus));
  lblCurrent->setText(m_map_state_str.at(m_state));
  QGridLayout *layout = dynamic_cast<QGridLayout *>(grpStatus->layout());
  int row = 0, col = 0;
  for(auto &label_str: m_map_label_str) {
    QLabel *lblname = new QLabel(grpStatus);
    lblname->setObjectName("lbl_st_" + label_str.first);
    lblname->setText(label_str.second + ": ");
    QLabel *lblvalue = new QLabel(grpStatus);
    lblvalue->setObjectName("txt_st_" + label_str.first);
    layout->addWidget(lblname, row, col * 2);
    layout->addWidget(lblvalue, row, col * 2 + 1);
    m_status[label_str.first] = lblvalue;
    if (++col > 1){
      ++row;
      col = 0;
    }
  }
  
  viewConn->setModel(&m_model_conns);
  viewConn->setItemDelegate(&m_delegate);
  
  QRect geom(-1,-1, 150, 200);
  QRect geom_from_last_program_run;
  QSettings settings("EUDAQ collaboration", "EUDAQ");
  settings.beginGroup("euRun2");
  geom_from_last_program_run.setSize(settings.value("size", geom.size()).toSize());
  geom_from_last_program_run.moveTo(settings.value("pos", geom.topLeft()).toPoint());
  txtConfigFileName
    ->setText(settings.value("lastConfigFile", "config file not set").toString());
  txtInitFileName
    ->setText(settings.value("lastInitFile", "init file not set").toString());
  settings.endGroup();
  
  QSize fsize = frameGeometry().size();
  if((geom.x() == -1)||(geom.y() == -1)||(geom.width() == -1)||(geom.height() == -1)) {
    if((geom_from_last_program_run.x() == -1)||(geom_from_last_program_run.y() == -1)||(geom_from_last_program_run.width() == -1)||(geom_from_last_program_run.height() == -1)) {
      geom.setX(x()); 
      geom.setY(y());
      geom.setWidth(fsize.width());
      geom.setHeight(fsize.height());
      move(geom.topLeft());
      resize(geom.size());
    } else {
      move(geom_from_last_program_run.topLeft());
      resize(geom_from_last_program_run.size());
    }
  }

  setWindowTitle("eudaq Run Control " PACKAGE_VERSION);
  connect(&m_timer_display, SIGNAL(timeout()), this, SLOT(DisplayTimer()));
  m_timer_display.start(500);
}

void RunControlGUI::SetInstance(eudaq::RunControlUP rc){
  m_rc = std::move(rc);
  m_rc->StartRunControl();
}

// void RunControlGUI::ChangeStatus(const QString &name, const QString &value){
//   auto i = m_status.find(name.toStdString());
//   if (i != m_status.end()) {
//     i->second->setText(value);
//   }
// }

void RunControlGUI::on_btnInit_clicked(){
  std::string settings = txtInitFileName->text().toStdString();
  std::cout<<settings<<std::endl;
  if(m_rc){
    m_rc->ReadInitilizeFile(settings);
    m_rc->Initialise();
  }
}

void RunControlGUI::on_btnTerminate_clicked(){
  close();
}

void RunControlGUI::on_btnConfig_clicked(){
  std::string settings = txtConfigFileName->text().toStdString();
  if(m_rc){
    m_rc->ReadConfigureFile(settings);
    m_rc->Configure();
  }
}

void RunControlGUI::on_btnStart_clicked(){
  if(m_rc)
    m_rc->StartRun();
}

void RunControlGUI::on_btnStop_clicked() {
  if(m_rc)
    m_rc->StopRun();
}

void RunControlGUI::on_btnReset_clicked() {
  if(m_rc)
    m_rc->Reset();
}

void RunControlGUI::on_btnLog_clicked() {
  std::string msg = txtLogmsg->displayText().toStdString();
  EUDAQ_USER(msg);
}

void RunControlGUI::on_btnLoadInit_clicked() {
  QString usedpath =QFileInfo(txtInitFileName->text()).path();
  QString filename =QFileDialog::getOpenFileName(this, tr("Open File"),
						 usedpath,
						 tr("*.init.conf (*.init.conf)"));
  if (!filename.isNull()){
    txtInitFileName->setText(filename);
  }
}

void RunControlGUI::on_btnLoadConf_clicked() {
  QString usedpath =QFileInfo(txtConfigFileName->text()).path();
  QString filename =QFileDialog::getOpenFileName(this, tr("Open File"),
						 usedpath,
						 tr("*.conf (*.conf)"));
  if (!filename.isNull()) {
    txtConfigFileName->setText(filename);
  }
}

void RunControlGUI::DisplayTimer(){
  std::map<eudaq::ConnectionSPC, eudaq::StatusSPC> map_conn_status;
  if(m_rc)
    map_conn_status= m_rc->GetActiveConnectionStatusMap();
  
  for(auto &conn_status_last: m_map_conn_status_last){
    if(!map_conn_status.count(conn_status_last.first)){
      m_model_conns.disconnected(conn_status_last.first);
    }
  }
  for(auto &conn_status: map_conn_status){
    if(!m_map_conn_status_last.count(conn_status.first)){
      m_model_conns.newconnection(conn_status.first);
    }
  }

  if(map_conn_status.empty()){
    m_state = eudaq::Status::STATE_UNINIT;
  }
  else{
    m_state = eudaq::Status::STATE_RUNNING;
    for(auto &conn_status: map_conn_status){
      if(!conn_status.second)
	continue;
      auto state_conn = conn_status.second->GetState();
      switch(state_conn){
      case eudaq::Status::STATE_ERROR:{
  	m_state = eudaq::Status::STATE_ERROR;
  	break;
      }
      case eudaq::Status::STATE_UNINIT:{
  	if(m_state != eudaq::Status::STATE_ERROR){
  	  m_state = eudaq::Status::STATE_UNINIT;
  	}
  	break;
      }
      case eudaq::Status::STATE_UNCONF:{
  	if(m_state != eudaq::Status::STATE_ERROR &&
  	   m_state != eudaq::Status::STATE_UNINIT){
  	  m_state = eudaq::Status::STATE_UNCONF;
  	}
  	break;
      }
      case eudaq::Status::STATE_CONF:{
  	if(m_state != eudaq::Status::STATE_ERROR &&
  	   m_state != eudaq::Status::STATE_UNINIT &&
  	   m_state != eudaq::Status::STATE_UNCONF){
  	  m_state = eudaq::Status::STATE_CONF;
  	}
  	break;
      }
      }
      m_model_conns.SetStatus(conn_status.first, conn_status.second);
    }
  }
  
  QRegExp rx_init(".+(\\.init\\.conf$)");
  QRegExp rx_conf(".+(\\.conf$)");
  bool confLoaded = rx_conf.exactMatch(txtConfigFileName->text());
  bool initLoaded = rx_init.exactMatch(txtInitFileName->text());
  std::cout<<"state "<<m_state<<"  confLoaded "<<confLoaded<<std::endl;
  
  btnInit->setEnabled(m_state == eudaq::Status::STATE_UNINIT && initLoaded);
  btnConfig->setEnabled((m_state == eudaq::Status::STATE_UNCONF ||
			 m_state == eudaq::Status::STATE_CONF )&& confLoaded);
  btnLoadInit->setEnabled(m_state != eudaq::Status::STATE_RUNNING);
  btnLoadConf->setEnabled(m_state != eudaq::Status::STATE_RUNNING);
  btnStart->setEnabled(m_state == eudaq::Status::STATE_CONF);
  btnStop->setEnabled(m_state == eudaq::Status::STATE_RUNNING);
  btnReset->setEnabled(m_state != eudaq::Status::STATE_RUNNING);
  btnTerminate->setEnabled(m_state != eudaq::Status::STATE_RUNNING);
  
  lblCurrent->setText(m_map_state_str.at(m_state));

  m_map_conn_status_last = map_conn_status;
}

void RunControlGUI::closeEvent(QCloseEvent *event) {
  if (QMessageBox::question(this, "Quitting",
			    "Terminate all connections and quit?",
			    QMessageBox::Ok | QMessageBox::Cancel)
      == QMessageBox::Cancel){
    event->ignore();
  } else {
    QSettings settings("EUDAQ collaboration", "EUDAQ");  
    settings.beginGroup("euRun2");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.setValue("lastConfigFile", txtConfigFileName->text());
    settings.setValue("lastInitFile", txtInitFileName->text());
    settings.endGroup();
    if(m_rc)
      m_rc->Terminate();
    event->accept();
  }
}


void RunControlGUI::Exec(){
  std::cout<<">>>>"<<std::endl;
  show();
  std::cout<<">>>>"<<std::endl;
  if(QApplication::instance())
    QApplication::instance()->exec(); 
  else
    std::cerr<<"ERROR: RUNContrlGUI::EXEC\n";   
}


std::map<int, QString> RunControlGUI::m_map_state_str ={
    {eudaq::Status::STATE_UNINIT,
     "<font size=12 color='red'><b>Current State: Uninitialised </b></font>"},
    {eudaq::Status::STATE_UNCONF,
     "<font size=12 color='red'><b>Current State: Unconfigured </b></font>"},
    {eudaq::Status::STATE_CONF,
     "<font size=12 color='orange'><b>Current State: Configured </b></font>"},
    {eudaq::Status::STATE_RUNNING,
     "<font size=12 color='green'><b>Current State: Running </b></font>"},
    {eudaq::Status::STATE_ERROR,
     "<font size=12 color='darkred'><b>Current State: Error </b></font>"}
};
