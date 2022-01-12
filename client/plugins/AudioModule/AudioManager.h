#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QObject>
#include <QThread>
#include <QWidget>
namespace cppmicroservices
{
    class BundleContext;
}

class AudioManagerImpl;
class AudioManager : public QWidget
{
    Q_OBJECT
public:
    explicit AudioManager(cppmicroservices::BundleContext context, QWidget* parent = nullptr);
    ~AudioManager();
    void init();

private:
    void reload();
    void saveCurPage(qint32 defaultCoiledPlayInterval = -1);

private:
    AudioManagerImpl* m_impl;
};

#endif  // AUDIOMANAGER_H
