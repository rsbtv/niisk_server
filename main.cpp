#include <QTcpServer>
#include <QTcpSocket>
#include <QCoreApplication>
#include <QDebug>

struct Point {
    double x;
    double y;
};

class Server : public QObject {
    Q_OBJECT

public:
    Server(QObject* parent = nullptr) : QObject(parent) {
        connect(&m_server, &QTcpServer::newConnection, this, &Server::onNewConnection);
    }

    bool start(quint16 port) {
        if (!m_server.listen(QHostAddress::Any, port)) {
            qCritical() << "Failed to start server:" << m_server.errorString();
            return false;
        }
        qInfo() << "Server started, listening on port" << port;
        return true;
    }

private slots:
    void onNewConnection() {
        QTcpSocket* socket = m_server.nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
            processData(socket);
        });
        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
    }

    void processData(QTcpSocket* socket) {
        while (socket->bytesAvailable() >= sizeof(Point) * 2) {
            Point points[2];
            socket->read(reinterpret_cast<char*>(&points), sizeof(points));
            qInfo() << "Received points:" << points[0].x << "," << points[0].y << "and" << points[1].x << "," << points[1].y;

            // Умножаем координаты на 2
            points[0].x *= 2;
            points[0].y *= 2;
            points[1].x *= 2;
            points[1].y *= 2;

            socket->write(reinterpret_cast<const char*>(&points), sizeof(points));
        }
    }

private:
    QTcpServer m_server;
};

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    Server server;
    if (!server.start(1234)) {
        return 1;
    }
    return app.exec();
}

#include "main.moc"
