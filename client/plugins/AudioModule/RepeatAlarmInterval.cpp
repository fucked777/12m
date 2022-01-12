#include "RepeatAlarmInterval.h"
#include "ui_RepeatAlarmInterval.h"

RepeatAlarmInterval::RepeatAlarmInterval(qint32 curInterval, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::RepeatAlarmInterval)
{
    ui->setupUi(this);
    ui->interval->setValue(curInterval);
    connect(ui->ok, &QPushButton::clicked, this, &RepeatAlarmInterval::accept);
}

RepeatAlarmInterval::~RepeatAlarmInterval() { delete ui; }
qint32 RepeatAlarmInterval::interval() { return ui->interval->value(); }
