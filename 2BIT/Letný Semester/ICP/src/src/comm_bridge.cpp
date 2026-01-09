/**
* @author Filip Jenis (xjenisf00)
*/

#include "../headers/comm_bridge.h"

/**
 * @brief Constructor
 * @param parent Parent QObject, used for memory management
 */
CommBridge::CommBridge(QObject *parent) : QObject(parent), sock(new QUdpSocket(this)){
    sock->bind(QHostAddress::LocalHost, localPort);
    connect(sock, &QUdpSocket::readyRead, this, &CommBridge::onReadyRead);
}

/**
 * @brief Destructor
 *
 * Cleans up class objects
 */
CommBridge::~CommBridge() {
    goodbye();
    sock->close();
}


/**
 * @brief Sends an UDP datagram to the generated executable automaton
 * @param message Contents of the datagram being sent
 */
void CommBridge::goodbye() {
    QUdpSocket* byeSocket = new QUdpSocket(nullptr);
    byeSocket->bind(QHostAddress::LocalHost, 56787);
    std::string endMsg;
    endMsg += {static_cast<char>(0x07)};
    endMsg += "\r\n";
    QByteArray dgram = QString::fromStdString(endMsg).toUtf8();
    byeSocket->writeDatagram(dgram, QHostAddress::LocalHost, 56789);
}

/**
 * @brief Send initial hello message to the generated executable automaton
 * @return True if an executable automaton replied within timeout, False otherwise
 */
void CommBridge::send(const QString& message) {
    QByteArray datagram = message.toUtf8();
    sock->writeDatagram(datagram, QHostAddress::LocalHost, runtimePort);
}

/**
 * @brief Correctly destroys connection between editor and generated executable automaton
 */
bool CommBridge::establishConnection() {
    std::string strMsg{static_cast<char>(0x00)};
    strMsg += "\r\n";
    send(QString::fromStdString(strMsg));
    QEventLoop loop;
    QTimer timeout;
    bool connected = false;

    timeout.setSingleShot(true);
    connect(&timeout, &QTimer::timeout, [&]() {
        loop.quit();
    });

    connect(this, &CommBridge::successfulConnection, [&](){
        connected = true;
        loop.quit();
    });

    timeout.start(500);
    loop.exec();

    return connected;
}

/**
 * @brief Slot for QUdpSocket, indicates that socket is ready to be read from
 */
void CommBridge::onReadyRead() {
    while (sock->hasPendingDatagrams()) {
        QByteArray buffer;
        buffer.resize(int(sock->pendingDatagramSize()));
        sock->readDatagram(buffer.data(), buffer.size());
        QString msg = QString::fromUtf8(buffer);
        if (msg.toStdString().at(0) == static_cast<char>(0x06)){
            emit successfulConnection();
        }
        emit eventReceived(msg);
    }
}
