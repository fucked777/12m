#include "CustomIPLineEdit.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPainter>
#include <QRegExpValidator>

CustomIPLineEdit::CustomIPLineEdit(QWidget* parent)
    : QLineEdit(parent)
{
    QRegExp rx("(2[0-5]{2}|2[0-4][0-9]|1?[0-9]{1,2})");
    QHBoxLayout* pHBox = new QHBoxLayout(this);
    pHBox->setSpacing(5);
    pHBox->setContentsMargins(1, 1, 1, 1);
    for (int i = 0; i < 4; ++i)
    {
        auto lineEdit = new QLineEdit(this);
        mLineEidtList << lineEdit;

        lineEdit->setFrame(false);
        lineEdit->setMaxLength(3);
        lineEdit->setAlignment(Qt::AlignCenter);
        lineEdit->installEventFilter(this);
        lineEdit->setValidator(new QRegExpValidator(rx, this));
        lineEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        // lineEdit->setStyleSheet("border: none;");

        connect(lineEdit, &QLineEdit::textChanged, this, &CustomIPLineEdit::onTextChanged);
        connect(lineEdit, &QLineEdit::editingFinished, this, &CustomIPLineEdit::onEditingFinished);

        pHBox->addWidget(lineEdit);
    }
    setContentsMargins(0, 0, 0, 0);

    this->setReadOnly(true);
    this->setStyleSheet(" background-color: white;");
}

CustomIPLineEdit::~CustomIPLineEdit() {}

void CustomIPLineEdit::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    QBrush brush;
    brush.setStyle(Qt::BrushStyle::SolidPattern);
    brush.setColor(Qt::black);
    painter.setBrush(brush);

    int width = 0;
    for (int i = 0; i < 3; i++)
    {
        width += mLineEidtList[i]->width() + (i == 0 ? 3 : 5);  //布局的间隔
        painter.drawEllipse(width, height() / 2, 1, 1);
    }
}

int CustomIPLineEdit::getIndex(QLineEdit* pEdit)
{
    int index = -1;
    for (int i = 0; i < 4; i++)
    {
        if (pEdit == mLineEidtList[i])
            index = i;
    }
    return index;
}

bool CustomIPLineEdit::eventFilter(QObject* obj, QEvent* ev)
{
    if (children().contains(obj) && QEvent::KeyPress == ev->type())
    {
        QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(ev);
        QLineEdit* pEdit = qobject_cast<QLineEdit*>(obj);
        switch (keyEvent->key())
        {
        case Qt::Key_0:
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9:
        {
            QString strText = pEdit->text();
            if (pEdit->selectedText().length())
            {
                pEdit->text().replace(pEdit->selectedText(), QChar(keyEvent->key()));
            }
            else if (strText.length() == 3 || ((strText.length() < 3) && (strText.toInt() * 10 > 255)))
            {
                int index = getIndex(pEdit);
                if (index != -1 && index != 3)
                {
                    mLineEidtList[index + 1]->setFocus();
                    mLineEidtList[index + 1]->selectAll();
                }
            }
            else if (strText.length() == 2 && strText.toInt() * 10 < 255)
            {
                if (Qt::Key_0 == keyEvent->key() && strText.toInt())
                {
                    pEdit->setText(strText.insert(pEdit->cursorPosition(), QChar(Qt::Key_0)));
                }
            }
            return QWidget::eventFilter(obj, ev);
        }
        break;
        case Qt::Key_Backspace:
        {
            QString strText = pEdit->text();
            if (!strText.length() || strText.length() && !pEdit->cursorPosition())
            {
                int index = getIndex(pEdit);
                if (index != -1 && index != 0)
                {
                    mLineEidtList[index - 1]->setFocus();
                    int length = mLineEidtList[index - 1]->text().length();
                    mLineEidtList[index - 1]->setCursorPosition(length ? length : 0);
                }
            }
            return QWidget::eventFilter(obj, ev);
        }
        case Qt::Key_Left:
        {
            if (!pEdit->cursorPosition())
            {
                int index = getIndex(pEdit);
                if (index != -1 && index != 0)
                {
                    mLineEidtList[index - 1]->setFocus();
                    int length = mLineEidtList[index - 1]->text().length();
                    mLineEidtList[index - 1]->setCursorPosition(length ? length : 0);
                }
            }
            return QWidget::eventFilter(obj, ev);
        }
        case Qt::Key_Right:
        {
            if (pEdit->cursorPosition() == pEdit->text().length())
            {
                int index = getIndex(pEdit);
                if (index != -1 && index != 3)
                {
                    mLineEidtList[index + 1]->setFocus();
                    mLineEidtList[index + 1]->setCursorPosition(0);
                }
            }
            return QWidget::eventFilter(obj, ev);
        }
        default: break;
        }
    }
    return false;
}

void CustomIPLineEdit::setText(const QString& strIP)
{
    int i = 0;
    QStringList ipList = strIP.split(".");
    for (QString ip : ipList)
    {
        mLineEidtList[i]->setText(ip);
        i++;
    }
}

bool CustomIPLineEdit::isValidIP() { return mIsValid; }

void CustomIPLineEdit::onEditingFinished()
{
    // 有焦点就表示未编辑完成
    for (int i = 0; i < 4; ++i)
    {
        if (mLineEidtList[i]->hasFocus())
        {
            return;
        }
    }

    emit editingFinished();
}

void CustomIPLineEdit::onTextChanged()
{
    QRegExp rx2("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
    if (rx2.exactMatch(text()))
        mIsValid = true;
    else
        mIsValid = false;

    auto currentLineEdit = qobject_cast<QLineEdit*>(sender());
    for (int i = 0; i < mLineEidtList.size(); ++i)
    {
        if (i >= 3)
        {
            break;
        }

        if (currentLineEdit == mLineEidtList[i])
        {
            if (currentLineEdit->text().length() == 3)
            {
                mLineEidtList[i + 1]->setFocus();
                mLineEidtList[i + 1]->selectAll();
                break;
            }
        }
    }

    emit textChanged(text());
}

QString CustomIPLineEdit::text() const
{
    QString strIP;
    for (int i = 0; i < 4; i++)
    {
        strIP.append(mLineEidtList[i]->text());
        strIP.append(".");
    }
    strIP.remove(strIP.length() - 1, 1);
    return strIP;
}
