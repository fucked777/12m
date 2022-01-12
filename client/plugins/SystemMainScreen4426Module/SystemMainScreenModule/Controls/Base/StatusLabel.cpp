#include "StatusLabel.h"

#include <QHBoxLayout>

StatusLabel::StatusLabel(const QDomElement& domEle, QWidget* parent)
    : BaseWidget(domEle, parent)
    , StatusController()
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    initLayout();

    parseNode(domEle);

    // 更新样式
    updateNormalStyleSheet();
    updateAbnormalStyleSheet();
    updateUnknowStyleSheet();

    // 设置默认为未知状态样式
    setText(mDefaultText);
    setStyleSheet(mUnknowStyleSheet);
}

void StatusLabel::setText(const QString& text) { mLabel->setText(text); }

QString StatusLabel::getText() const { return mLabel->text(); }

void StatusLabel::setDefaultText(const QString& text) { mDefaultText = text; }

QString StatusLabel::getDefaultText() const { return mDefaultText; }

void StatusLabel::setMinWidth(const int width)
{
    mMinWidth = width;
    mLabel->setMinimumWidth(width);
}

int StatusLabel::getMinWidth() const { return mMinWidth; }

void StatusLabel::statusChanged(Status status)
{
    switch (status)
    {
    case Unknown:
    {
        setStyleSheet(mUnknowStyleSheet);
        setText(getDefaultText());
    }
    break;
    case Normal: setStyleSheet(mNormalStyleSheet); break;
    case Abnormal: setStyleSheet(mAbnormalStyleSheet); break;
    default: setStyleSheet(mUnknowStyleSheet); break;
    }
}

void StatusLabel::initLayout()
{
    mLabel = new QLabel(this);

    auto hLayout = new QHBoxLayout;
    hLayout->setSpacing(0);
    hLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(hLayout);

    hLayout->addWidget(mLabel);
}

void StatusLabel::parseNode(const QDomElement& domEle)
{
    if (domEle.isNull())
    {
        return;
    }

    // 正常字体色
    if (domEle.hasAttribute("color"))
    {
        mNormalTextColor = domEle.attribute("color");
    }
    // 正常背景色
    if (domEle.hasAttribute("backgroundColor"))
    {
        mNormalBackgroundColor = domEle.attribute("backgroundColor");
    }
    // 字体大小
    if (domEle.hasAttribute("fontSize"))
    {
        mFontSize = domEle.attribute("fontSize").toInt();
    }
    // 控件宽度
    if (domEle.hasAttribute("minWidth"))
    {
        mMinWidth = domEle.attribute("minWidth").toInt();
    }
    // 默认文本
    if (domEle.hasAttribute("defaultText"))
    {
        mDefaultText = domEle.attribute("defaultText");
    }
    // 文本对齐方式
    if (domEle.hasAttribute("Align"))
    {
        auto align = domEle.attribute("Align");
        if (align == "Left")
        {
            mLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        }
        else if (align == "Right")
        {
            mLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        }
        else if (align == "Center")
        {
            mLabel->setAlignment(Qt::AlignCenter);
        }
    }
    else
    {
        mLabel->setAlignment(Qt::AlignCenter);
    }
}

void StatusLabel::updateUnknowStyleSheet()
{
    mUnknowStyleSheet = QString("background-color: %1;"
                                "color: %2;"
                                "border-radius: %3;"
                                "font-size:%4px;"
                                "min-width:%5;"
                                "min-height:%6;")
                            .arg(mUnknowBackgroundColor.name())
                            .arg(mUnknowTextColor.name())
                            .arg(mRadius)
                            .arg(mFontSize)
                            .arg(mMinWidth)
                            .arg(mMinHeight);
}

void StatusLabel::updateNormalStyleSheet()
{
    mNormalStyleSheet = QString("background-color: %1;"
                                "color: %2;"
                                "border-radius: %3;"
                                "font-size:%4px;"
                                "min-width:%5;"
                                "min-height:%6;")
                            .arg(mNormalBackgroundColor.name())
                            .arg(mNormalTextColor.name())
                            .arg(mRadius)
                            .arg(mFontSize)
                            .arg(mMinWidth)
                            .arg(mMinHeight);
}

void StatusLabel::updateAbnormalStyleSheet()
{
    mAbnormalStyleSheet = QString("background-color: %1;"
                                  "color: %2;"
                                  "border-radius: %3;"
                                  "font-size:%4px;"
                                  "min-width:%5;"
                                  "min-height:%6;")
                              .arg(mAbnormalBackgroundColor.name())
                              .arg(mAbnormalTextColor.name())
                              .arg(mRadius)
                              .arg(mFontSize)
                              .arg(mMinWidth)
                              .arg(mMinHeight);
}
