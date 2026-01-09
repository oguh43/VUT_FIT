/**
 * @author Filip Jenis (xjenisf00)
 */

#ifndef COMM_BRIDGE_H
#define COMM_BRIDGE_H

#include <QObject>
#include <QUdpSocket>
#include <QEventLoop>
#include <QTimer>

/**
 * @class CommBridge
 * @brief Bridge class to connect the FSM backend to the running generated executable automaton
 */
class CommBridge : public QObject{
    Q_OBJECT
public:
    /**
     * @brief Constructor
     * @param parent Parent QObject, used for memory management
     */
    explicit CommBridge(QObject *parent = nullptr);

    /**
     * @brief Destructor
     *
     * Cleans up class objects
     */
    ~CommBridge();

    /**
     * @brief Sends an UDP datagram to the generated executable automaton
     * @param message Contents of the datagram being sent
     */
    void send(const QString& message);

    /**
     * @brief Send initial hello message to the generated executable automaton
     * @return True if an executable automaton replied within timeout, False otherwise
     */
    bool establishConnection();

    /**
     * @brief Correctly destroys connection between editor and generated executable automaton
     */
    static void goodbye();

signals:
    /**
     * @brief Signal indicating received message from UDP communication
     */
    void eventReceived(const QString& message);
    /**
     * @brief Signal indicating successful establishment of connection with generated executable automaton
     */
    void successfulConnection();

private slots:
    /**
     * @brief Slot for QUdpSocket, indicates that socket is ready to be read from
     */
    void onReadyRead();

private:
    QUdpSocket* sock; /**< An UDP socket */
    quint16 localPort = 56788; /**< Local port number */
    quint16 runtimePort = 56789; /**< Generated automaton port number */
};

#endif //COMM_BRIDGE_H
