#include <QCoreApplication>
#include <QModbusTcpServer>
#include <QModbusDataUnit>
#include <QDebug>
#include <QHostAddress>

class ModbusServer : public QObject
{
    Q_OBJECT

public:
    ModbusServer(QObject *parent = nullptr) : QObject(parent)
    {
        server = new QModbusTcpServer(this);

        // Настройка карты регистров
        QModbusDataUnitMap reg;
        reg.insert(QModbusDataUnit::HoldingRegisters, {QModbusDataUnit::HoldingRegisters, 0, 10});
        server->setMap(reg);

        connect(server, &QModbusServer::dataWritten, this, &ModbusServer::onDataWritten);

        server->setConnectionParameter(QModbusDevice::NetworkAddressParameter, QHostAddress(QHostAddress::LocalHost).toString());
        server->setConnectionParameter(QModbusDevice::NetworkPortParameter, 502);

        if (!server->connectDevice()) {
            qDebug() << "Server failed to start:" << server->errorString();
        } else {
            qDebug() << "Server is listening on port 502";
        }
    }

private slots:
    void onDataWritten(QModbusDataUnit::RegisterType type, int address, int size)
    {
        if (type == QModbusDataUnit::HoldingRegisters && address == 0 && size == 2) {
            QModbusDataUnit unit(QModbusDataUnit::HoldingRegisters, 0, 2);
            if (server->data(&unit)) {
                double x = static_cast<double>(unit.value(0)) / 1000.0;
                double y = static_cast<double>(unit.value(1)) / 1000.0;

                qDebug() << "Received data:" << x << y;

                // Умножаем на 2
                x *= 2;
                y *= 2;

                // Обновляем данные в регистрах
                unit.setValue(0, static_cast<quint16>(x * 1000));
                unit.setValue(1, static_cast<quint16>(y * 1000));
                server->setData(unit);

                qDebug() << "Sent back data:" << x << y;
            }
        }
    }

private:
    QModbusTcpServer *server;
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    ModbusServer server;
    return a.exec();
}

#include "main.moc"
