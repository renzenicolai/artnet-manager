#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>
#include <QBoxLayout>
#include <QDebug>
#include <QMenuBar>
#include <QApplication>
#include <QDesktopWidget>
#include <QNetworkInterface>
#include <QListWidget>
#include <QList>
#include <QStandardItem>
#include <QHeaderView>
#include <QUdpSocket>
#include <QList>
#include <QListWidgetItem>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QLineEdit>

#define ARTNET_PORT 6454

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void readArtnetPackets(void);
    void onDeviceListItemClicked(QListWidgetItem* item);
    void scanForDevices(void);
    void openDmxDialog(void);
    void onDmxSliderChange(void);
    void updateDevicePanel(void);

private:
    void createMenu(void);
    void sendArtpoll(QHostAddress addr);
    void parseArtPollReply(QHostAddress sender, quint16 senderPort, QByteArray datagram);
    void updateDeviceList(void);
    QHostAddress FindLocalIPAddressOfIncomingPacket(QHostAddress senderAddr);
    void replyToArtpoll(QHostAddress sender, quint16 senderPort, QByteArray datagram);
    QWidget *centralWidget;
    QBoxLayout* formLayout;
    QBoxLayout* headerLayout;
    QBoxLayout* mainLayout;
    QMenuBar *menuBar;
    QMenu *fileMenu;
    QAction *exitAction;
    QPushButton *scanButton;

    QListWidget *deviceList;
    QGroupBox *devicePanel;

    QUdpSocket *artnetSocket;

    QList<QString> *styles;

    QList<QSlider*> *dmxSliders;

    QLineEdit *dmxNet;
    QLineEdit *dmxSubNet;

    struct device {
        QHostAddress ip_address;
        uint16_t port;
        uint16_t fw_version;
        uint8_t netswitch;
        uint8_t subswitch;
        uint16_t oem;
        uint8_t ubea_version;
        uint8_t status1;
        uint16_t estaman;
        QString shortname;
        QString longname;
        QString nodereport;
        uint16_t numports;
        uint8_t porttypes[4];
        uint8_t goodinput[4];
        uint8_t goodoutput[4];
        uint8_t swin[4];
        uint8_t swout[4];
        uint8_t swvideo;
        uint8_t swmacro;
        uint8_t swremote;
        uint8_t style;
        uint8_t mac_address[6];
        QHostAddress bind_ip;
        uint8_t bind_index;
        uint8_t status2;
        QListWidgetItem* listitem; //For linking qlistview with actual device list
    };

    QList<device> *devices;

    int selectedDevice;
};

#endif // MAINWINDOW_H
