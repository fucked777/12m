#ifndef FEPMANAGERSHELL_H
#define FEPMANAGERSHELL_H

#include "INetMsg.h"
#include <QObject>
#include <QThread>
namespace cppmicroservices
{
    class BundleContext;
}

class FepServer;
class FepSendUnit;
class QDomElement;
class FepManagerShell : public INetMsg
{
    Q_OBJECT
public:
    explicit FepManagerShell(cppmicroservices::BundleContext context);
    ~FepManagerShell();
    void init();
    void sendInfoRecv(const QString& filename);
    void getPlan(QString workDir);
public slots:
    void slotRecvData(const QByteArray& bArrayData);
    void slot_oneFileRecvSuccess(const QVariantMap&);

private:
    void initRecvFile();
    QString initGetPlanFile();
    QMap<QString, QVariantMap> getCurIPCheckData(const QString& filePath);

    int number = 1;

private:
    FepServer* m_fepServer;
    FepSendUnit* m_fepSend;
};

#endif  // FEPMANAGERSHELL_H
