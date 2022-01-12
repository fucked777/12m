#ifndef STATUSLABEL_H
#define STATUSLABEL_H

#include <QDomElement>
#include <QLabel>

#include "BaseWidget.h"
#include "StatusController.h"

// 状态标签控件 该控件显示上报的状态，如果一段时间内没有状态上报，控件会显示默认值
class StatusLabel : public BaseWidget, public StatusController
{
    Q_OBJECT
    struct StatusLabelInfo
    {
        QString name;
    };

public:
    StatusLabel(const QDomElement& domEle, QWidget* parent = nullptr);

    void setText(const QString& text);
    QString getText() const;

    void setDefaultText(const QString& text);
    QString getDefaultText() const;

    void setMinWidth(const int width);
    int getMinWidth() const;

protected:
    virtual void statusChanged(Status status) override;

private:
    void initLayout();
    // 解析节点
    void parseNode(const QDomElement& domEle);

    // 更新样式字符串
    void updateUnknowStyleSheet();
    void updateNormalStyleSheet();
    void updateAbnormalStyleSheet();

private:
    QLabel* mLabel = nullptr;

    StatusLabelInfo mStatusLableInfo;

    QString mDefaultText = "--";  // 默认文本

    // 未知
    QColor mUnknowBackgroundColor = QColor(Qt::black);
    QColor mUnknowTextColor = QColor(Qt::green);

    // 正常
    QColor mNormalBackgroundColor = QColor(Qt::black);
    QColor mNormalTextColor = QColor(Qt::green);

    // 异常
    QColor mAbnormalBackgroundColor = QColor(Qt::black);
    QColor mAbnormalTextColor = QColor(Qt::red);

    int mRadius = 5;
    int mFontSize = 14;
    int mMinWidth = 60;
    int mMinHeight = 30;

    QString mUnknowStyleSheet;
    QString mNormalStyleSheet;
    QString mAbnormalStyleSheet;
};

#endif  // STATUSLABEL_H
