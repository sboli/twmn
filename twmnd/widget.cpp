#include "widget.h"
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QHBoxLayout>
#include <QTimer>
#include <QLabel>
#include <QDesktopWidget>
#include <QPixmap>
#include <QPainter>
#include "settings.h"

Widget::Widget()
{
    //setWindowFlags(Qt::SplashScreen);
    setWindowFlags(Qt::ToolTip);
    // Let the event loop run
    QTimer::singleShot(30, this, SLOT(init()));
    QPropertyAnimation* anim = new QPropertyAnimation;
    anim->setTargetObject(this);
    m_animation.addAnimation(anim);
    anim->setEasingCurve(QEasingCurve::OutBounce);
    anim->setDuration(1000);
    connect(anim, SIGNAL(finished()), this, SLOT(reverseTrigger()));
    if (m_settings.get("position") == "top_left") {
        connect(anim, SIGNAL(valueChanged(QVariant)), this, SLOT(updateTopLeftAnimation(QVariant)));
    }
    else if (m_settings.get("position") == "top_right") {
        connect(anim, SIGNAL(valueChanged(QVariant)), this, SLOT(updateTopRightAnimation(QVariant)));
    }
    else if (m_settings.get("position") == "bottom_right") {
        connect(anim, SIGNAL(valueChanged(QVariant)), this, SLOT(updateBottomRightAnimation(QVariant)));
    }
    else if (m_settings.get("position") == "bottom_left") {
        connect(anim, SIGNAL(valueChanged(QVariant)), this, SLOT(updateBottomLeftAnimation(QVariant)));
    }
    setFixedHeight(m_settings.get("height").toInt());
}

Widget::~Widget()
{
}


void Widget::init()
{
    int port = m_settings.get("port").toInt();
    if (!m_socket.bind(QHostAddress::Any, port)) {
        qCritical() << "Unable to listen port" << port;
        close();
        return;
    }
    connect(&m_socket, SIGNAL(readyRead()), this, SLOT(onDataReceived()));
    QHBoxLayout* l = new QHBoxLayout;
    l->setSizeConstraint(QLayout::SetNoConstraint);
    l->setMargin(0);
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);
    l->addWidget(m_contentView["icon"] = new QLabel);
    l->addWidget(m_contentView["title"] = new QLabel);
    l->addWidget(m_contentView["text"] = new QLabel);
}

void Widget::onDataReceived()
{
    quint64 size = m_socket.pendingDatagramSize();
    QByteArray data(size, '\0');
    m_socket.readDatagram(data.data(), size);
    QStringList out = QString(data).split("|");
    Message m;
    QIcon icon(out[0]);
    if (icon.pixmap(50, 50).isNull()) {
        if (m_settings.has(out[0]))
            icon = QIcon(m_settings.get(out[0]).toString());
        else {
            QImage img(1, 1, QImage::Format_ARGB32);
            QPainter p;
            p.begin(&img);
            p.fillRect(0, 0, 1, 1, QBrush(QColor::fromRgb(255, 255, 255, 0)));
            p.end();
            icon = QIcon(QPixmap::fromImage(img));
        }
    }
    m.icon = icon;
    m.title = out.size() >= 2 ? out[1] : QString();
    m.text = out.size() >= 3 ? out[2] : QString();
    m_messageQueue.push_back(m);
    // get out of here
    QTimer::singleShot(30, this, SLOT(processMessageQueue()));
}

void Widget::processMessageQueue()
{
    if (m_messageQueue.empty())
        return;
    if (m_animation.state() == QAbstractAnimation::Running || (m_animation.totalDuration()-m_animation.currentTime()) < 50)
        return;
    Message m = m_messageQueue.front();
    m_messageQueue.pop_front();
    m_contentView["icon"]->setPixmap(m.icon.pixmap(25, 25));
    m_contentView["title"]->setText("<strong>  " + m.title + "</strong>");
    m_contentView["text"]->setText("  " + m.text + "  ");
    QFont tmp = m_contentView["title"]->font();
    tmp.setBold(true);
    int width = QFontMetrics(m_contentView["text"]->font()).width(m_contentView["text"]->text())
                + QFontMetrics(tmp).width("  " + m.title)
                + m_contentView["icon"]->pixmap()->width();
    m_animation.setDirection(QAnimationGroup::Forward);
    qobject_cast<QPropertyAnimation*>(m_animation.animationAt(0))->setEasingCurve(QEasingCurve::OutBounce);
    qobject_cast<QPropertyAnimation*>(m_animation.animationAt(0))->setStartValue(0);
    qobject_cast<QPropertyAnimation*>(m_animation.animationAt(0))->setEndValue(width);
    m_animation.start();
}

void Widget::updateTopLeftAnimation(QVariant value)
{
    show();
    setFixedWidth(value.toInt());
    layout()->setSpacing(0);
}

void Widget::updateTopRightAnimation(QVariant value)
{
    show();
    int end = QDesktopWidget().availableGeometry(this).width();
    int val = value.toInt();
    setGeometry(end-val, 0, val, height());
    layout()->setSpacing(0);
}

void Widget::updateBottomRightAnimation(QVariant value)
{
    show();
    int wend = QDesktopWidget().availableGeometry(this).width();
    int hend = QDesktopWidget().availableGeometry(this).height();
    int val = value.toInt();
    setGeometry(wend-val, hend-height(), val, height());
    layout()->setSpacing(0);
}

void Widget::updateBottomLeftAnimation(QVariant value)
{
    show();
    int hend = QDesktopWidget().availableGeometry(this).height();
    int val = value.toInt();
    setGeometry(0, hend-height(), val, height());
    layout()->setSpacing(0);
}

void Widget::reverseTrigger()
{
    if (m_animation.direction() == QAnimationGroup::Backward) {
        QTimer::singleShot(30, this, SLOT(processMessageQueue()));
        return;
    }
    QTimer::singleShot(m_settings.get("duration").toInt(), this, SLOT(reverseStart()));
}

void Widget::reverseStart()
{
    m_animation.setDirection(QAnimationGroup::Backward);
    qobject_cast<QPropertyAnimation*>(m_animation.animationAt(0))->setEasingCurve(QEasingCurve::InCubic);
    m_animation.start();
}
