#ifndef STATIONNETCENTERCMDTEST_H
#define STATIONNETCENTERCMDTEST_H
#include <QWidget>

namespace Ui
{
    class StationNetCenterCmdTest;
}

class StationNetCenterCmdTestImpl;
class StationNetCenterCmdTest : public QWidget
{
    Q_OBJECT
public:
    StationNetCenterCmdTest(QWidget* parent = nullptr);
    ~StationNetCenterCmdTest();

private:
    void send();
    void log(const QString&);
    bool parseData(const QByteArray&);

    void resetConnect();
    void onReadyRead();

private:
    Ui::StationNetCenterCmdTest* ui;
    StationNetCenterCmdTestImpl* m_impl;
};

#endif  // STATIONNETCENTERCMDTEST_H
