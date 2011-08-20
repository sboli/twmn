#include "widget.h"
#include <exception>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/shared_ptr.hpp>
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
#include <QTextDocument>
#include "settings.h"

Widget::Widget()
{
    setWindowFlags(Qt::ToolTip);
    // Let the event loop run
    QTimer::singleShot(30, this, SLOT(init()));
    QPropertyAnimation* anim = new QPropertyAnimation;
    anim->setTargetObject(this);
    m_animation.addAnimation(anim);
    anim->setEasingCurve(QEasingCurve::OutBounce);
    anim->setDuration(1000);
    connect(anim, SIGNAL(finished()), this, SLOT(reverseTrigger()));
    connectForPosition(m_settings.get("gui/position").toString());
    setFixedHeight(m_settings.get("gui/height").toInt());
    connect(&m_dbus, SIGNAL(messageReceived(Message)), this, SLOT(appendMessageToQueue(Message)));
}

Widget::~Widget()
{
}

void Widget::init()
{
    int port = m_settings.get("main/port").toInt();
    if (!m_socket.bind(QHostAddress::Any, port)) {
        qCritical() << "Unable to listen port" << port;
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
    boost::property_tree::ptree tree;
    Message m;
    try {
        quint64 size = m_socket.pendingDatagramSize();
        QByteArray data(size, '\0');
        m_socket.readDatagram(data.data(), size);
        std::istringstream iss (data.data());
        boost::property_tree::xml_parser::read_xml(iss, tree);
        boost::property_tree::ptree& root = tree.get_child("root");
        boost::property_tree::ptree::iterator it;
        for (it = root.begin(); it != root.end(); ++it) {
            //std::cout << it->first << " - " << it->second.get_value<std::string>() << std::endl;
            m.data[QString::fromStdString(it->first)] = boost::optional<QVariant>(it->second.get_value<std::string>().c_str());
        }
    }
    catch (const std::exception& e) {
        std::cout << "ERROR : " << e.what() << std::endl;
    }
    appendMessageToQueue(m);
}

void Widget::appendMessageToQueue(const Message& msg)
{
    m_messageQueue.push_back(msg);
    QTimer::singleShot(30, this, SLOT(processMessageQueue()));
}

void Widget::processMessageQueue()
{
    if (m_messageQueue.empty())
        return;
    if (m_animation.state() == QAbstractAnimation::Running || (m_animation.totalDuration()-m_animation.currentTime()) < 50)
        return;
    QFont boldFont = font();
    boldFont.setBold(true);
    Message& m = m_messageQueue.front();
    loadDefaults();
    setFixedHeight(m.data["size"]->toInt());
    setupFont();
    setupColors();
    setupIcon();
    setupTitle();
    setupContent();
    connectForPosition(m.data["pos"]->toString());
    m_animation.setDirection(QAnimationGroup::Forward);
    int width = computeWidth();
    qobject_cast<QPropertyAnimation*>(m_animation.animationAt(0))->setEasingCurve(QEasingCurve::OutBounce);
    qobject_cast<QPropertyAnimation*>(m_animation.animationAt(0))->setStartValue(0);
    qobject_cast<QPropertyAnimation*>(m_animation.animationAt(0))->setEndValue(width);
    m_animation.start();
    QString soundCommand = m.data["sc"]->toString();
    if (!soundCommand.isEmpty())
        QProcess::startDetached(soundCommand);
}

void Widget::updateTopLeftAnimation(QVariant value)
{
    show();
    setGeometry(0, 0, value.toInt(), height());
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
    QTimer::singleShot(m_messageQueue.front().data["duration"]->toInt(), this, SLOT(reverseStart()));
    m_messageQueue.pop_front();
}

void Widget::reverseStart()
{
    m_animation.setDirection(QAnimationGroup::Backward);
    qobject_cast<QPropertyAnimation*>(m_animation.animationAt(0))->setEasingCurve(QEasingCurve::InCubic);
    m_animation.start();
}

int Widget::computeWidth()
{
    Message& m = m_messageQueue.front();
    QFont boldFont = font();
    boldFont.setBold(true);
    int width = 0;
    QString text = m_contentView["text"]->text();
    width += QFontMetrics(boldFont).width(m_contentView["title"]->text());
    if (Qt::mightBeRichText(text)) {
        QTextDocument doc;
        doc.setUseDesignMetrics(true);
        doc.setHtml(text);
        doc.setDefaultFont(font());
        width += doc.idealWidth();
        qDebug() << doc.toPlainText();
    }
    else
        width += QFontMetrics(font()).width(text);
    if (m.data["icon"])
        width += m_contentView["icon"]->pixmap()->width();
    return width;
    /*
    Message& m = m_messageQueue.front();
    QFont boldFont = font();
    boldFont.setBold(true);
    int width = 0;
    width += QFontMetrics(boldFont).width(m_contentView["title"]->text())
            + QFontMetrics(font()).width(m_contentView["text"]->text());
    if (m.data["icon"])
        width += m_contentView["icon"]->pixmap()->width();
    return width;
      */
}

void Widget::setupFont()
{
    Message& m = m_messageQueue.front();
    QFont font(m.data["fn"]->toString());
    font.setPixelSize(m.data["fs"]->toInt());
    QApplication::setFont(font);
}

void Widget::setupColors()
{
    Message& m = m_messageQueue.front();
    QString bg = m.data["bg"]->toString();
    QString fg = m.data["fg"]->toString();
    QString sheet;
    if (!bg.isEmpty())
        sheet += QString("background-color: %1;").arg(bg);
    if (!fg.isEmpty())
        sheet += QString("color: %1;").arg(fg);
    setStyleSheet(sheet);
}

void Widget::connectForPosition(QString position)
{
    QPropertyAnimation* anim = qobject_cast<QPropertyAnimation*>(m_animation.animationAt(0));
    if (!anim)
        return;
    disconnect(anim, SIGNAL(valueChanged(QVariant)), this, SLOT(updateTopLeftAnimation(QVariant)));
    disconnect(anim, SIGNAL(valueChanged(QVariant)), this, SLOT(updateTopLeftAnimation(QVariant)));
    disconnect(anim, SIGNAL(valueChanged(QVariant)), this, SLOT(updateBottomRightAnimation(QVariant)));
    disconnect(anim, SIGNAL(valueChanged(QVariant)), this, SLOT(updateBottomLeftAnimation(QVariant)));
    if (position == "top_left") {
        connect(anim, SIGNAL(valueChanged(QVariant)), this, SLOT(updateTopLeftAnimation(QVariant)));
    }
    else if (position == "top_right") {
        connect(anim, SIGNAL(valueChanged(QVariant)), this, SLOT(updateTopRightAnimation(QVariant)));
    }
    else if (position == "bottom_right") {
        connect(anim, SIGNAL(valueChanged(QVariant)), this, SLOT(updateBottomRightAnimation(QVariant)));
    }
    else if (position == "bottom_left") {
        connect(anim, SIGNAL(valueChanged(QVariant)), this, SLOT(updateBottomLeftAnimation(QVariant)));
    }
}

void Widget::setupIcon()
{
    Message& m = m_messageQueue.front();
    bool done = true;
    if (m.data["icon"]) {
        QPixmap pix = qvariant_cast<QPixmap>(*m.data["icon"]);
        if (pix.isNull())
            pix = loadPixmap(m.data["icon"]->toString());
        if (!pix.isNull())
            m.data["icon"].reset(pix);
        else if (pix.isNull())
            done = false;
        if (pix.height() > m.data["size"]->toInt())
            pix = pix.scaled(m.data["size"]->toInt(), m.data["size"]->toInt());
        m.data["icon"].reset(pix);
        m_contentView["icon"]->setPixmap(pix);
        m_contentView["icon"]->setMaximumWidth(9999);
    }
    if (!done) {
        m_contentView["icon"]->setPixmap(QPixmap());
        m_contentView["icon"]->setFixedWidth(2);
    }
}

void Widget::setupTitle()
{
    QFont boldFont = font();
    boldFont.setBold(true);
    Message& m = m_messageQueue.front();
    if (m.data["title"]) {              // avoid ugly space if no icon is set
        m_contentView["title"]->setText((m.data["icon"] ? " " : "") + m.data["title"]->toString() + " ");
        m_contentView["title"]->setFont(boldFont);
        m_contentView["title"]->setMaximumWidth(9999);
    }
    else {
        m_contentView["title"]->setText("");
        m_contentView["title"]->setFixedWidth(0);
    }
}

void Widget::setupContent()
{
    Message& m = m_messageQueue.front();
    if (m.data["content"]) {
        m_contentView["text"]->setText((m.data["icon"] && !m.data["title"] ? " " : "") + m.data["content"]->toString() + " ");
        m_contentView["text"]->setMaximumWidth(9999);
    }
    else {
        m_contentView["text"]->setText("");
        m_contentView["text"]->setFixedWidth(0);
    }
}

void Widget::loadDefaults()
{
    // "content" << "icon" << "title" << "layout" << "size" << "pos" << "fn" << "fs" << "duration" < "sc" << "bg" << "fg";
    Message& m = m_messageQueue.front();
    Settings* s = &m_settings;
    if (m.data["layout"]) {
        QString name = m.data["layout"]->toString();
        name.remove(".conf");
        s = new Settings(name);
        s->fillWith(m_settings);
        qDebug() << "Layout loaded : " << name;
        qDebug() << s->get("gui/foreground_color");
    }
    if (!m.data["bg"])
        m.data["bg"] = boost::optional<QVariant>(s->get("gui/background_color"));
    if (!m.data["fg"])
        m.data["fg"] = boost::optional<QVariant>(s->get("gui/foreground_color"));
    if (!m.data["sc"])
        m.data["sc"] = boost::optional<QVariant>(s->get("main/sound_command"));
    if (!m.data["duration"])
        m.data["duration"] = boost::optional<QVariant>(s->get("main/duration"));
    if (!m.data["fs"])
        m.data["fs"] = boost::optional<QVariant>(s->get("gui/font_size"));
    if (!m.data["fn"])
        m.data["fn"] = boost::optional<QVariant>(s->get("gui/font"));
    if (!m.data["pos"])
        m.data["pos"] = boost::optional<QVariant>(s->get("gui/position"));
    if (!m.data["size"])
        m.data["size"] = boost::optional<QVariant>(s->get("gui/height"));
    if (!m.data["icon"])
        m.data["icon"] = loadPixmap(s->has("gui/icon") ? s->get("gui/icon").toString() : "");
    if (s != &m_settings)
        delete s;
}

QPixmap Widget::loadPixmap(QString pattern)
{
    QPixmap icon(pattern);
    if (icon.isNull()) {
        if (m_settings.has("icons/" + pattern))
            icon = QPixmap(m_settings.get("icons/" + pattern).toString());
        else {
            QImage img(1, 1, QImage::Format_ARGB32);
            QPainter p;
            p.begin(&img);
            p.fillRect(0, 0, 1, 1, QBrush(QColor::fromRgb(255, 255, 255, 0)));
            p.end();
            icon = QPixmap::fromImage(img);
        }
    }
    return icon;
}


