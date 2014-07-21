#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H

#include <QDialog>
#include <QTcpSocket>

class QComboBox;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTcpSocket;
class QNetworkSession;

class NetworkClient : public QDialog
{
    Q_OBJECT

public:
    NetworkClient(QWidget *parent = 0);

private slots:
    void requestNewPolyworldMessage();
    void readPolyworldMessage();
    void displayError(QAbstractSocket::SocketError socketError);
    void enableGetPolyworldMessageButton();
    void sessionOpened();

private:
    QLabel *hostLabel;
    QLabel *portLabel;
    QComboBox *hostCombo;
    QLineEdit *portLineEdit;
    QLabel *statusLabel;
    QPushButton *getPolyworldMessageButton;
    QPushButton *quitButton;
    QDialogButtonBox *buttonBox;

    QTcpSocket *tcpSocket;
    QString currentPolyworldMessage;
    quint16 blockSize;

    QNetworkSession *networkSession;
};


#endif // NETWORK_CLIENT_H
