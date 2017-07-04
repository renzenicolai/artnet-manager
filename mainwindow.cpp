#include "mainwindow.h"

/*void MainWindow::createMenu()
{
  menuBar = new QMenuBar;
  fileMenu = new QMenu(tr("&File"), this);
  exitAction = fileMenu->addAction(tr("E&xit"));
  menuBar->addMenu(fileMenu);
  //connect(exitAction, SIGNAL(triggered()), this, SLOT(accept()));
}*/

void MainWindow::onDmxSliderChange()
{
  if (selectedDevice==-1) {
      qDebug()<<"ERROR: onDmxSliderChange() called while no device selected.";
      return;
  }
  QHostAddress ip = devices->at(selectedDevice).ip_address;

  uint16_t opcode = 0x5000; //OpDMX
  uint16_t version = 14;
  uint16_t length = 512;
  QString netStr = dmxNet->text();
  QString subnetStr = dmxSubNet->text();
  uint8_t net = netStr.toInt();
  uint8_t subnet = subnetStr.toInt();

  QByteArray datagram = "Art-Net"; //ID[] (0-7)
  datagram.append((char) 0);//ID[] (8)
  datagram.append(opcode&0x00FF);//OpCodeLo
  datagram.append((opcode&0xFF00)>>8);//OpCodeHi
  datagram.append((version&0xFF00)>>8);//ProtVerHi
  datagram.append(version&0x00FF);//ProtVerLo
  datagram.append((char) 0);//Sequence
  datagram.append((char) 0);//Physical
  datagram.append((char) 0);//SubUni
  datagram.append((char) 0);//Net
  datagram.append((length&0xFF00)>>8);//LengthHi
  datagram.append(length&0x00FF);//LengthLo

  for (uint16_t ch = 0; ch<512; ch++) {
      //qDebug()<<"Channel "+QString::number(ch+1)+": "+QString::number(dmxSliders->at(ch)->value());
      datagram.append((char) dmxSliders->at(ch)->value());
  }
  artnetSocket->writeDatagram(datagram.data(), datagram.size(), ip, ARTNET_PORT);
  qDebug()<<"Sent ArtDMX to: "+ip.toString()+" ["+QString::number(net,16)+":"+QString::number(subnet,16)+"]";
}

void MainWindow::openDmxDialog()
{
  if (!(devicePanel==nullptr)) {
    qDebug()<<"Panel does exist.";
    mainLayout->removeWidget(devicePanel);
    delete devicePanel;
  }
  devicePanel = new QGroupBox;
  if (selectedDevice==-1) {
      qDebug()<<"ERROR: openDmxDialog() called while no device selected.";
      return;
  }
  device dev = devices->at(selectedDevice);
  devicePanel->setTitle("artDMX (IP: "+dev.ip_address.toString()+")");
  QScrollArea *scrollArea = new QScrollArea();


  /* DMX SLIDERS */
  //Fixme: take first, delete later. (while size>0)
  dmxSliders->clear();
  QWidget *dmxSliderWidget = new QWidget;
  QHBoxLayout *slidersLayout = new QHBoxLayout(dmxSliderWidget);
  for (uint16_t ch = 1; ch<513; ch++) {
      QVBoxLayout *slider = new QVBoxLayout();
      QSlider *s = new QSlider(Qt::Vertical);
      s->setMinimum(0);
      s->setMaximum(255);
      s->setValue(0);
      connect(s, SIGNAL(valueChanged(int)),
                  this, SLOT(onDmxSliderChange()));
      dmxSliders->append(s);
      slider->addWidget(s);
      QLabel *l = new QLabel();
      l->setText(QString::number(ch));
      l->setMinimumWidth(30);
      slider->addWidget(l);
      slidersLayout->addLayout(slider);
  }
  slidersLayout->setSizeConstraint(QLayout::SetMinimumSize);

  /* END OF DMX SLIDERS */

  dmxNet = new QLineEdit();
  dmxSubNet  = new QLineEdit();
  dmxNet->setText("0");
  dmxSubNet->setText("0");


  scrollArea->setWidget(dmxSliderWidget);
  scrollArea->viewport()->setAutoFillBackground(true);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  scrollArea->setWidgetResizable(true);

  QVBoxLayout *panelLayout = new QVBoxLayout();
  panelLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

  QHBoxLayout *toolbox = new QHBoxLayout();

  QPushButton *backButton = new QPushButton;
  backButton->setText("< Back");
  connect(backButton, SIGNAL(clicked()),
          this, SLOT(updateDevicePanel()));
  toolbox->addWidget(backButton);

  QLabel *netLabel = new QLabel();
  QLabel *subnetLabel = new QLabel();
  netLabel->setText("Net: ");
  subnetLabel->setText("Subnet: ");

  toolbox->addWidget(netLabel);
  toolbox->addWidget(dmxNet);
  toolbox->addSpacing(4);
  toolbox->addWidget(subnetLabel);
  toolbox->addWidget(dmxSubNet);
  toolbox->addStretch(1);
  panelLayout->addLayout(toolbox);
  panelLayout->addWidget(scrollArea);
  devicePanel->setLayout(panelLayout);

  mainLayout->addWidget(devicePanel);
}

void MainWindow::updateDevicePanel()
{
  if (!(devicePanel==nullptr)) {
    qDebug()<<"Panel does exist.";
    mainLayout->removeWidget(devicePanel);
    delete devicePanel;
  }
  devicePanel = new QGroupBox;

  QVBoxLayout *vbox = new QVBoxLayout;
  QLabel* label1 = new QLabel;
  if (selectedDevice==-1) {
      qDebug()<<"Panel: Welcome";
      devicePanel->setTitle("Welcome");
      label1->setText("No device selected.\nPlease choose a device from the list.");
  } else {
      qDebug()<<"Panel: "+QString::number(selectedDevice);
      device dev = devices->at(selectedDevice);
      devicePanel->setTitle(dev.shortname);
      QString t = "Unknown (0x"+QString::number(dev.style, 16)+")";

      QString macaddress = "";
      for (uint8_t i = 0; i<7; i++) {
        if (dev.mac_address[i]<0x10) macaddress = macaddress+"0";
        macaddress = macaddress+QString::number(dev.mac_address[i],16);
        if (i<6) macaddress = macaddress + ":";
      }

      QString portinfo = "";
      for (uint8_t i = 0; i<4; i++) {
          portinfo = portinfo + "Port "+QString::number(i)+": Input at "+QString::number(dev.swin[i])+". Output at "+QString::number(dev.swout[i])+"\n";
      }

      if (dev.style<0x07) t = styles->at(dev.style);
      label1->setText(QString("Device\n----------\n")+
                      "Name: "+dev.longname+"\n"+
                      "Style: "+t+"\n"+
                      "Manufacturer: "+QString((char)dev.estaman&0xFF)+QString((char)(dev.estaman>>8))+"\n"+
                      "Firmware version: 0x"+QString::number(dev.fw_version,16)+"\n"+
                      "\n"+
                      "Network\n----------\n"+
                      "MAC address: "+macaddress+"\n"+
                      "IP address: "+dev.ip_address.toString()+"\n"+
                      "\n"+
                      "Output\n----------\n"+
                      "Net and Sub-Net: "+QString::number(dev.netswitch,16)+":"+QString::number(dev.subswitch,16)+"\n"+
                      "Amount of ports: "+QString::number(dev.numports)+"\n"+
                      portinfo+
                      "\n"+
                      "Debug\n----------\n"+
                      "Nodereport: "+dev.nodereport);
      QHBoxLayout *toolbox = new QHBoxLayout();
      QPushButton *dmxTestButton = new QPushButton;
      dmxTestButton->setText("Test DMX output");
      connect(dmxTestButton, SIGNAL(clicked()),
              this, SLOT(openDmxDialog()));
      toolbox->addWidget(dmxTestButton);
      toolbox->addStretch(1);

      vbox->addLayout(toolbox);
  }

  //QPixmap pixmap(":/images/header.png");
  //label1->setPixmap(pixmap);
  //layout.addWidget((label1, 0, Qt::AlignLeft);
  //devicePanel->setLayout(&layout);

  vbox->addWidget(label1);
  vbox->addStretch(1);
  devicePanel->setLayout(vbox);

  mainLayout->addWidget(devicePanel);
}

void MainWindow::onDeviceListItemClicked(QListWidgetItem* item)
{
    for (int i = 0; i<devices->count(); i++) {
      if (devices->at(i).listitem==item) {
          qDebug()<<"Selected "+devices->at(i).ip_address.toString();
          selectedDevice = i;
      }
    }
    updateDevicePanel();
}

void MainWindow::parseArtPollReply(QHostAddress sender, quint16 senderPort, QByteArray datagram)
{
  device dev;
  dev.ip_address.setAddress(((uint8_t)datagram.at(10)<<24)+((uint8_t)datagram.at(11)<<16)+((uint8_t)datagram.at(12)<<8)+((uint8_t)datagram.at(13)));
  dev.port = ((uint8_t)datagram.at(14))+((uint8_t)datagram.at(15)<<8);
  dev.fw_version = ((uint8_t)datagram.at(17))+((uint8_t)datagram.at(16)<<8);
  dev.netswitch = (uint8_t)datagram.at(18);
  dev.subswitch = (uint8_t)datagram.at(19);
  dev.oem = ((uint8_t)datagram.at(21))+((uint8_t)datagram.at(20)<<8);
  dev.ubea_version = (uint8_t)datagram.at(22);
  dev.status1 = (uint8_t)datagram.at(23);
  dev.estaman = ((uint8_t)datagram.at(24))+((uint8_t)datagram.at(25)<<8);
  dev.shortname = QString::fromLatin1(datagram.mid(26,18));
  dev.longname = QString::fromLatin1(datagram.mid(44,64));
  dev.nodereport = QString::fromLatin1(datagram.mid(108,64));
  dev.numports = ((uint8_t)datagram.at(173))+((uint8_t)datagram.at(172)<<8);
  dev.porttypes[0] = (uint8_t)datagram.at(174);
  dev.porttypes[1] = (uint8_t)datagram.at(175);
  dev.porttypes[2] = (uint8_t)datagram.at(176);
  dev.porttypes[3] = (uint8_t)datagram.at(177);
  dev.goodinput[0] = (uint8_t)datagram.at(178);
  dev.goodinput[1] = (uint8_t)datagram.at(179);
  dev.goodinput[2] = (uint8_t)datagram.at(180);
  dev.goodinput[3] = (uint8_t)datagram.at(181);
  dev.goodoutput[0] = (uint8_t)datagram.at(182);
  dev.goodoutput[1] = (uint8_t)datagram.at(183);
  dev.goodoutput[2] = (uint8_t)datagram.at(184);
  dev.goodoutput[3] = (uint8_t)datagram.at(185);
  dev.swin[0] = (uint8_t)datagram.at(186);
  dev.swin[1] = (uint8_t)datagram.at(187);
  dev.swin[2] = (uint8_t)datagram.at(188);
  dev.swin[3] = (uint8_t)datagram.at(189);
  dev.swout[0] = (uint8_t)datagram.at(190);
  dev.swout[1] = (uint8_t)datagram.at(191);
  dev.swout[2] = (uint8_t)datagram.at(192);
  dev.swout[3] = (uint8_t)datagram.at(193);
  dev.swvideo = (uint8_t)datagram.at(194);
  dev.swmacro = (uint8_t)datagram.at(195);
  dev.swremote = (uint8_t)datagram.at(196);
  dev.style = (uint8_t)datagram.at(200);
  dev.mac_address[0] = (uint8_t)datagram.at(201);
  dev.mac_address[1] = (uint8_t)datagram.at(202);
  dev.mac_address[2] = (uint8_t)datagram.at(203);
  dev.mac_address[3] = (uint8_t)datagram.at(204);
  dev.mac_address[4] = (uint8_t)datagram.at(205);
  dev.mac_address[5] = (uint8_t)datagram.at(206);
  dev.bind_ip.setAddress(((uint8_t)datagram.at(207)<<24)+((uint8_t)datagram.at(208)<<16)+((uint8_t)datagram.at(209)<<8)+((uint8_t)datagram.at(210)));
  dev.bind_index = (uint8_t)datagram.at(211);
  dev.status2 = (uint8_t)datagram.at(212);

  int position = -1;
  for (int i = 0; i<devices->count(); i++) {
    if(devices->at(i).ip_address==dev.ip_address) {
        position = i;
        break;
    }
  }
  if (position==-1) {
    devices->append(dev);
  } else {
    devices->replace(position, dev);
  }
  qDebug()<<"Received artpollreply from "+dev.ip_address.toString()+" ["+dev.shortname+"]";
  updateDeviceList();
}

void MainWindow::readArtnetPackets()
{
  while (artnetSocket->hasPendingDatagrams()) {
    QByteArray datagram;
    datagram.resize(artnetSocket->pendingDatagramSize());
    QHostAddress sender;
    quint16 senderPort;
    artnetSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

    QString packetid = QString::fromLatin1(datagram.mid(0,6));

    if (packetid.compare("Art-Net")) {
      //qDebug()<<"Received artnet packet.";
      uint16_t opcode = (datagram.at(9)<<8)+(datagram.at(8));
      if (opcode==0x2100) { //Artpollreply
          parseArtPollReply(sender, senderPort, datagram);
      } else if (opcode==0x2000) { //Artpoll
          //replyToArtpoll(sender, senderPort, datagram);
      } else {
          qDebug()<<"Unknown opcode: "+QString::number(opcode);
      }
    }
  }
}

void MainWindow::replyToArtpoll(QHostAddress sender, quint16 senderPort, QByteArray datagram)
{
  uint16_t opcode = 0x2100; //OpPollReply

  //qDebug()<<"Received artpoll. Received on IP address "+FindLocalIPAddressOfIncomingPacket(sender).toString();

  /*QByteArray datagram = "Art-Net"; //ID[] (0-7)
  datagram.append((char) 0);//ID[] (8)
  datagram.append(opcode&0x00FF);//OpCodeLo
  datagram.append((opcode&0xFF00)>>8);//OpCodeHi*/
  //datagram.append();
  //artnetSocket->writeDatagram(datagram.data(), datagram.size(), addr, ARTNET_PORT);
}

/*QHostAddress MainWindow::FindLocalIPAddressOfIncomingPacket( QHostAddress senderAddr )
{
    foreach (QNetworkInterface iface, QNetworkInterface::allInterfaces())
    {
        foreach (QNetworkAddressEntry ifaceaddr, iface.addressEntries()) {
            int nm =ifaceaddr.netmask().toIPv4Address();
            if( QHostAddress::isInSubnet(senderAddr, nm) )
            {
                return ifaceaddr.ip();
            }
        }
    }
}*/

void MainWindow::sendArtpoll(QHostAddress addr)
{
  qDebug()<<"Sending artpoll packet to "+addr.toString();

  uint16_t opcode = 0x2000; //OpPoll
  uint16_t version = 14;
  uint8_t talktome = 0b00000110;
  uint8_t priority = 0x10; //DpLow

  QByteArray datagram = "Art-Net"; //ID[] (0-7)
  datagram.append((char) 0);//ID[] (8)
  datagram.append(opcode&0x00FF);//OpCodeLo
  datagram.append((opcode&0xFF00)>>8);//OpCodeHi
  datagram.append((version&0xFF00)>>8);//ProtVerHi
  datagram.append(version&0x00FF);//ProtVerLo
  datagram.append(talktome);//TalkToMe
  datagram.append(priority);//Priority

  artnetSocket->writeDatagram(datagram.data(), datagram.size(), addr, ARTNET_PORT);
}

void MainWindow::scanForDevices()
{
   selectedDevice = -1;
   updateDevicePanel();
   devices->clear();
   sendArtpoll(QHostAddress::Broadcast);
   updateDeviceList();
}

void MainWindow::updateDeviceList()
{
  deviceList->clear();
  if (devices->count()>0) {
    for (int i = 0; i<devices->count(); i++) {
      device dev = devices->at(i);
      QListWidgetItem* item = new QListWidgetItem;
      item->setText(dev.shortname);
      dev.listitem = item;
      devices->replace(i, dev);
      deviceList->addItem(item);
    }
    deviceList->setEnabled(true);
  } else {
    QListWidgetItem* item = new QListWidgetItem;
    item->setText("No Art-Net devices found.");
    deviceList->addItem(item);
    deviceList->setEnabled(false);
  }
}

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent)
{
  qDebug()<<"Starting...";

  //Fill styles list
  styles = new QList<QString>;
  styles->append("Node - A DMX to / from Art-Net device");
  styles->append("Controller - A lighting console.");
  styles->append("Media - A Media Server.");
  styles->append("Route - A network routing device.");
  styles->append("Backup - A backup device.");
  styles->append("Config - A configuration or diagnostic tool.");
  styles->append("Visual - A visualiser.");

  //Create window
  setWindowTitle(tr("RN+ Art-Net device manager"));
  this->setMinimumSize(QSize(800,600));
  this->move(QApplication::desktop()->screen()->rect().center() - this->rect().center());
  centralWidget = new QWidget(this);
  this->setCentralWidget( centralWidget );

  //Set background color
  QPalette Pal(palette());
  Pal.setColor(QPalette::Background, Qt::white);
  centralWidget->setAutoFillBackground(true);
  centralWidget->setPalette(Pal);

  //Create main layout
  formLayout = new QBoxLayout(QBoxLayout::TopToBottom, centralWidget );

  //Create header layout
  headerLayout = new QBoxLayout(QBoxLayout::LeftToRight, centralWidget );
  formLayout->addLayout(headerLayout, 0);

  //Create header contents
  QLabel* logo = new QLabel;
  QPixmap pixmap(":/images/header.png");
  logo->setPixmap(pixmap);
  headerLayout->addWidget(logo);
  QLabel* title = new QLabel;
  title->setText("Art-Net device manager");
  QFont titlefont("sans", 20);
  title->setFont(titlefont);
  QPalette titlePal(palette());
  titlePal.setColor(QPalette::Foreground, Qt::black);
  title->setPalette(titlePal);
  headerLayout->addSpacing(2);
  headerLayout->addWidget(title);
  headerLayout->addSpacing(20);

  scanButton = new QPushButton;
  scanButton->setText("Refresh device list");
  connect(scanButton, SIGNAL(clicked()),
          this, SLOT(scanForDevices()));


  headerLayout->addWidget(scanButton);
  headerLayout->addStretch(1);

  //Create line between header and main layout
  QWidget *horizontalLineWidget = new QWidget;
  horizontalLineWidget->setFixedHeight(2);
  horizontalLineWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  horizontalLineWidget->setStyleSheet(QString("background-color: #00AEEF;"));
  formLayout->addWidget(horizontalLineWidget);

  //Create main layout
  mainLayout = new QBoxLayout(QBoxLayout::LeftToRight, centralWidget );
  formLayout->addLayout(mainLayout, 0);

  deviceList = new QListWidget;
  deviceList->clear();
  deviceList->setMaximumWidth(300);
  //deviceList->setMinimumWidth(300);
  mainLayout->addWidget(deviceList);

  connect(deviceList, SIGNAL(itemClicked(QListWidgetItem*)),
          this, SLOT(onDeviceListItemClicked(QListWidgetItem*)));

  artnetSocket = new QUdpSocket(this);
  artnetSocket->bind(QHostAddress::Any, ARTNET_PORT);
  connect(artnetSocket, SIGNAL(readyRead()),
          this, SLOT(readArtnetPackets()));

  devices = new QList<device>;

  //createMenu();
  //gridLayout->setMenuBar(menuBar);

  //Create DMX stuff
  dmxSliders = new QList<QSlider*>;

  //Magic...
  //MainWindow::show();
  selectedDevice = -1;
  devicePanel = nullptr;
  //updateDevicePanel();
  scanForDevices();
}

MainWindow::~MainWindow()
{

}
