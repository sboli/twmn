#include "widget.h"
#include <exception>
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
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
#include <QScreen>
#include <QShortcut>
#include <QIcon>
#include <QWheelEvent>
#include <QCursor>
#include "settings.h"

Widget::Widget(const char* wname) : m_settings(wname)//, m_shortcutGrabber(this, m_settings)
{
    setWindowFlags(Qt::ToolTip);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowOpacity(m_settings.get("gui/opacity").toInt() / 100.0);
    QPropertyAnimation* anim = new QPropertyAnimation(this);
    anim->setTargetObject(this);
    m_animation.addAnimation(anim);
    anim->setEasingCurve(QEasingCurve::Type(m_settings.get("gui/in_animation").toInt()));
    connect(anim, SIGNAL(finished()), this, SLOT(reverseTrigger()));
    connectForPosition(m_settings.get("gui/position").toString());
    connect(&m_visible, SIGNAL(timeout()), this, SLOT(reverseStart()));
    m_visible.setSingleShot(true);
    QHBoxLayout* l = new QHBoxLayout;
    l->setSizeConstraint(QLayout::SetNoConstraint);
    l->setMargin(0);
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);
    l->addWidget(m_contentView["icon"] = new QLabel);
    l->addWidget(m_contentView["title"] = new QLabel);
    l->addWidget(m_contentView["text"] = new QLabel);
    m_contentView["title"]->setOpenExternalLinks(true);
    m_contentView["text"]->setOpenExternalLinks(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onHide()));
    // Let the event loop run
    QTimer::singleShot(30, this, SLOT(init()));
}

Widget::~Widget()
{
}

void Widget::connectToDBus(const DBusInterface& dbus)
{
    connect(&dbus, SIGNAL(messageReceived(Message)), this, SLOT(appendMessageToQueue(Message)));
}

void Widget::init()
{
    int port = m_settings.get("main/port").toInt();
    QHostAddress host = QHostAddress(m_settings.get("main/host").toString());
    if (!m_socket.bind(host, port)) {
        qCritical() << "Unable to listen port" << port;
        return;
    }
    connect(&m_socket, SIGNAL(readyRead()), this, SLOT(onDataReceived()));
   // m_shortcutGrabber.loadShortcuts();
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
            std::cout << it->first << " - " << it->second.get_value<std::string>() << std::endl;
            m.data[QString::fromStdString(it->first)] = boost::optional<QVariant>(it->second.get_value<std::string>().c_str());
        }
    }
    catch (const std::exception& e) {
        std::cout << "ERROR : " << e.what() << std::endl;
    }
    if (m.data.contains("remote") && m.data["remote"]) { // a remote control action
        processRemoteControl(qvariant_cast<QString>(m.data["remote"].get()));
    }
    else // A notification
        appendMessageToQueue(m);
}

void Widget::processRemoteControl(QString command)
{
    if (command == "activate")
        onActivate();
    else if (command == "hide")
        onHide();
    else if (command == "next")
        onNext();
    else if (command == "previous")
        onPrevious();
}

bool Widget::startMessageCommand(const Message& msg, const char* key) {
    if (!msg.data.contains(key))
      return false;
    QString command = msg.data[key]->toString();
    if (command.isEmpty())
      return false;
    QStringList args = QProcess::splitCommand(command);
    QString prog = args.takeFirst();
    QProcess::startDetached(prog, args);
    return true;
}

void Widget::appendMessageToQueue(const Message& msg)
{
    if (msg.data["id"] && !m_messageQueue.isEmpty()) {
        if (update(msg))
            return;
    }

    m_messageQueue.push_back(msg);
    QTimer::singleShot(30, this, SLOT(processMessageQueue()));
}

void Widget::processMessageQueue()
{
    if (m_messageQueue.empty()) {
        return;
    }

    if (m_animation.state() == QAbstractAnimation::Running ||
            m_visible.isActive()) {
       //(m_animation.totalDuration() - m_animation.currentTime()) < 50) {
        return;
    }

    QFont boldFont = font();
    boldFont.setBold(true);
    Message& m = m_messageQueue.front();
    loadDefaults();
    if (m.data["aot"]->toBool()) {
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint | Qt::BypassWindowManagerHint);
        raise();
    }
    setupFont();
    setupColors();
    setupIcon();
    setupTitle();
    setupContent();
    connectForPosition(m.data["pos"]->toString());
    m_animation.setDirection(QAnimationGroup::Forward);
    int width = computeWidth();
    qobject_cast<QPropertyAnimation*>(m_animation.animationAt(0))->setEasingCurve(QEasingCurve::Type(m_settings.get("gui/in_animation").toInt()));
    qobject_cast<QPropertyAnimation*>(m_animation.animationAt(0))->setStartValue(0);
    qobject_cast<QPropertyAnimation*>(m_animation.animationAt(0))->setEndValue(width);
    m_animation.start();
    startMessageCommand(m, "sc");
   // m_shortcutGrabber.enableShortcuts();
}

QRect Widget::getScreenRect()
{
    return QGuiApplication::screens().at(m_settings.get("gui/screen").toInt())->geometry();
}

void Widget::updateTopLeftAnimation(QVariant value)
{
    const int finalHeight = getHeight();
    QPoint p(0, 0);
    if (m_settings.has("gui/screen") && !m_settings.get("gui/screen").toString().isEmpty()) {
        p = getScreenRect().topLeft();
    } else if (m_settings.has("gui/absolute_position") && !m_settings.get("gui/absolute_position").toString().isEmpty()) {
        QPoint tmp = stringToPos(m_settings.get("gui/absolute_position").toString());
        if (!tmp.isNull())
            p = tmp;
    }
    //setGeometry(p.x(), p.y(), value.toInt(), finalHeight);
    int width = computeWidth();
    if (width != -1)
        m_computedWidth = width;
    int offset_x = m_settings.get("gui/offset_x").toInt();
    int offset_y = m_settings.get("gui/offset_y").toInt();
    setGeometry(value.toInt()-m_computedWidth+offset_x, p.y()+offset_y, m_computedWidth, finalHeight);
    layout()->setSpacing(0);
    show();
}

void Widget::updateTopRightAnimation(QVariant value)
{
    const int end = QDesktopWidget().screenGeometry(this).width();
    const int val = value.toInt();
    const int finalHeight = getHeight();
    QPoint p(end, 0);
    if (m_settings.has("gui/screen") && !m_settings.get("gui/screen").toString().isEmpty()) {
        p = getScreenRect().topRight();
        ++p.rx();
    } else if (m_settings.has("gui/absolute_position") && !m_settings.get("gui/absolute_position").toString().isEmpty()) {
        QPoint tmp = stringToPos(m_settings.get("gui/absolute_position").toString());
        if (!tmp.isNull()) {
          p = tmp;
        }
    }
    int offset_x = m_settings.get("gui/offset_x").toInt();
    int offset_y = m_settings.get("gui/offset_y").toInt();
    setGeometry(p.x()-val+offset_x, p.y()+offset_y, val, finalHeight);
    layout()->setSpacing(0);
    show();
}

void Widget::updateBottomRightAnimation(QVariant value)
{
    const int wend = QDesktopWidget().screenGeometry(this).width();
    const int hend = QDesktopWidget().screenGeometry(this).height();
    const int finalHeight = getHeight();
    const int val = value.toInt();
    QPoint p(wend, hend);
    if (m_settings.has("gui/screen") && !m_settings.get("gui/screen").toString().isEmpty()) {
        p = getScreenRect().bottomRight();
        ++p.rx();
        ++p.ry();
    } else if (m_settings.has("gui/absolute_position") && !m_settings.get("gui/absolute_position").toString().isEmpty()) {
        QPoint tmp = stringToPos(m_settings.get("gui/absolute_position").toString());
        if (!tmp.isNull())
            p = tmp;
    }
    int offset_x = m_settings.get("gui/offset_x").toInt();
    int offset_y = m_settings.get("gui/offset_y").toInt();
    setGeometry(p.x()-val+offset_x, p.y()-height()-offset_y, val, finalHeight);	// Fixed inverted offset_y.
    layout()->setSpacing(0);
    show();
}

void Widget::updateBottomLeftAnimation(QVariant value)
{
    const int hend = QDesktopWidget().screenGeometry(this).height();
    const int finalHeight = getHeight();
    QPoint p(0, hend);
    if (m_settings.has("gui/screen") && !m_settings.get("gui/screen").toString().isEmpty()) {
        p = getScreenRect().bottomLeft();
        ++p.ry();
    } else if (m_settings.has("gui/absolute_position") && !m_settings.get("gui/absolute_position").toString().isEmpty()) {
        QPoint tmp = stringToPos(m_settings.get("gui/absolute_position").toString());
        if (!tmp.isNull())
            p = tmp;
    }
    //setGeometry(p.x(), p.y()-height(), val, finalHeight);
    int width = computeWidth();
    if (width != -1)
        m_computedWidth = width;
    int offset_x = m_settings.get("gui/offset_x").toInt();
    int offset_y = m_settings.get("gui/offset_y").toInt();
    //setGeometry(value.toInt()-m_computedWidth, p.y()-height()+offset_y, m_computedWidth+offset_x, finalHeight);	// Fixed inverted offsets.
    setGeometry(value.toInt()-m_computedWidth, p.y()-height()-offset_y, m_computedWidth+offset_x, finalHeight);	// Fixed inverted offsets and offset_y.
	/* Implementing offsets like that means the widget will be constantly exposed. Consider applying offsets to the calculated screen corner. p.rx()? */
    layout()->setSpacing(0);
    show();
}

void Widget::updateTopCenterAnimation(QVariant value)
{
    const int finalWidth = qobject_cast<QPropertyAnimation*>(m_animation.animationAt(0))->endValue().toInt();
    const int finalHeight = getHeight();
    const int h = value.toInt() * finalHeight / finalWidth;
    const int wend = QDesktopWidget().screenGeometry(this).width();

    QPoint p1(wend, 0);
    QPoint p2(0, 0);
    if (m_settings.has("gui/screen") && !m_settings.get("gui/screen").toString().isEmpty()) {
        p1 = getScreenRect().topRight();
        ++p1.rx();
        p2 = getScreenRect().topLeft();
    } else if (m_settings.has("gui/absolute_position") && !m_settings.get("gui/absolute_position").toString().isEmpty()) {
        QPoint tmp = stringToPos(m_settings.get("gui/absolute_position").toString());
        if (!tmp.isNull())
            p1 = tmp;
    }

    int offset_x = m_settings.get("gui/offset_x").toInt();
    int offset_y = m_settings.get("gui/offset_y").toInt();
    setGeometry(((p2.x() - p1.x())/2 - finalWidth/2 + p1.x())+offset_x, p1.y()+offset_y, finalWidth, h);
    layout()->setSpacing(0);
    show();
}

void Widget::updateBottomCenterAnimation(QVariant value)
{
    const int finalWidth = qobject_cast<QPropertyAnimation*>(m_animation.animationAt(0))->endValue().toInt();
    const int finalHeight = getHeight();
    const int h = value.toInt() * finalHeight / finalWidth;
    const int wend = QDesktopWidget().screenGeometry(this).width();
    const int hend = QDesktopWidget().screenGeometry(this).height();
    QPoint p1(wend, hend);
    QPoint p2(0, 0);
    if (m_settings.has("gui/screen") && !m_settings.get("gui/screen").toString().isEmpty()) {
        p1 = getScreenRect().bottomRight();
        ++p1.rx();
        ++p1.ry();
        p2 = getScreenRect().topLeft();
    } else if (m_settings.has("gui/absolute_position") && !m_settings.get("gui/absolute_position").toString().isEmpty()) {
        QPoint tmp = stringToPos(m_settings.get("gui/absolute_position").toString());
        if (!tmp.isNull())
            p1 = tmp;
    }
    int offset_x = m_settings.get("gui/offset_x").toInt();
    int offset_y = m_settings.get("gui/offset_y").toInt();
    setGeometry(((p2.x() - p1.x())/2 - finalWidth/2 + p1.x())+offset_x, p1.y()-h+offset_y, finalWidth, h);
    layout()->setSpacing(0);
    show();
}

void Widget::updateCenterAnimation(QVariant value)
{
    const int finalWidth = qobject_cast<QPropertyAnimation*>(m_animation.animationAt(0))->endValue().toInt();
    const int finalHeight = getHeight();
    const int h = value.toInt() * finalHeight / finalWidth;
    const int wend = QDesktopWidget().screenGeometry(this).width();
    const int hend = QDesktopWidget().screenGeometry(this).height();
    QPoint p1(wend, hend);
    QPoint p2(0, 0);
    if (m_settings.has("gui/screen") && !m_settings.get("gui/screen").toString().isEmpty()) {
        p1 = getScreenRect().bottomRight();
        ++p1.rx();
        ++p1.ry();
        p2 = getScreenRect().topLeft();
    } else if (m_settings.has("gui/absolute_position") && !m_settings.get("gui/absolute_position").toString().isEmpty()) {
        QPoint tmp = stringToPos(m_settings.get("gui/absolute_position").toString());
        if (!tmp.isNull())
            p1 = tmp;
    }
    int offset_x = m_settings.get("gui/offset_x").toInt();
    int offset_y = m_settings.get("gui/offset_y").toInt();
    setGeometry(((p2.x() - p1.x())/2 - value.toInt()/2 + p1.x())+offset_x, (p1.y()/2 - h/2)+offset_y, value.toInt(), h);
    layout()->setSpacing(0);
    show();
}

void Widget::startBounce()
{
    if (!m_settings.get("gui/bounce").toBool()) {
        return;
    }

    doneBounce();

    QPropertyAnimation* anim = new QPropertyAnimation(this);
    anim->setTargetObject(this);
    m_animation.addAnimation(anim);

    anim->setEasingCurve(QEasingCurve::OutQuad);
    anim->setDuration(m_settings.get("gui/bounce_duration").toInt() * 0.25f);
    anim->setStartValue(0);

    QString position = m_messageQueue.front().data["pos"]->toString();
    if (position == "top_center" || position == "tc" ||
        position == "bottom_center" || position == "bc" ||
        position == "center" || position == "c")
        anim->setEndValue(height());
    else
        anim->setEndValue(40);

    tmpBouncePos = pos();

    anim->start();

    connect(anim, SIGNAL(valueChanged(QVariant)), this, SLOT(updateBounceAnimation(QVariant)));
    connect(anim, SIGNAL(finished()), this, SLOT(unbounce()));
}

void Widget::unbounce()
{
    QPropertyAnimation* anim = qobject_cast<QPropertyAnimation*>(m_animation.animationAt(1));
    if (!anim)
        return;
    disconnect(anim, SIGNAL(finished()), this, SLOT(unbounce()));
    connect(anim, SIGNAL(finished()), this, SLOT(doneBounce()));
    anim->setDirection(QAnimationGroup::Backward);
    anim->setEasingCurve(QEasingCurve::InBounce);
    anim->setDuration(m_settings.get("gui/bounce_duration").toInt() * 0.75f);
    anim->start();
}

void Widget::doneBounce()
{
    QPropertyAnimation* anim =
        qobject_cast<QPropertyAnimation*>(m_animation.animationAt(1));

    if(!anim) {
        return;
    }

    disconnect(anim, SIGNAL(finished()), this, SLOT(unbounce()));
    disconnect(anim, SIGNAL(finished()), this, SLOT(doneBounce()));
    disconnect(anim, SIGNAL(valueChanged(QVariant)), this, SLOT(updateBounceAnimation(QVariant)));

    m_animation.removeAnimation(anim);
}

void Widget::updateBounceAnimation(QVariant value)
{
    if(m_messageQueue.empty()) {
        doneBounce();
        return;
    }

    QString position = m_messageQueue.front().data["pos"]->toString();
    if (position == "top_left" || position == "tl" ||
        position == "bottom_left" || position == "bl")
        move(tmpBouncePos.x() + value.toInt(), tmpBouncePos.y());
    else if (position == "top_right" || position == "tr" ||
             position == "bottom_right" || position == "br")
        move(tmpBouncePos.x() - value.toInt(), tmpBouncePos.y());
    else if (position == "top_center" || position == "tc")
        move(tmpBouncePos.x(), tmpBouncePos.y() + value.toInt());
    else if (position == "bottom_center" || position == "bc")
        move(tmpBouncePos.x(), tmpBouncePos.y() - value.toInt());
    else if (position == "center" || position == "c")
        move(tmpBouncePos.x(), tmpBouncePos.y() - value.toInt());
    layout()->setSpacing(0);
    show();
}

void Widget::reverseTrigger()
{
    if (m_animation.direction() == QAnimationGroup::Backward ||
            m_messageQueue.isEmpty()) {
        QTimer::singleShot(30, this, SLOT(processMessageQueue()));
        return;
    }

    const bool bounce  = m_settings.get("gui/bounce").toBool();
    const int duration = m_messageQueue.front().data["duration"]->toInt();

    const unsigned int minDuration = m_settings.get("gui/bounce_duration").toInt() + 10;

    if (duration == -1) {
        m_visible.setInterval(minDuration);
    } else { // ensure its visible long enough to bounce
        if (bounce) {
            m_visible.setInterval((unsigned)duration < minDuration ? minDuration : duration);
        } else {
            m_visible.setInterval(duration);
        }
    }

    m_visible.start();
}

void Widget::reverseStart()
{
    //If last message, play hide animation.
    if (m_messageQueue.size() <= 1) {
        QPropertyAnimation* bounceAnim = qobject_cast<QPropertyAnimation*>(m_animation.animationAt(1));
        if(bounceAnim) {
            if(bounceAnim->state() == QAbstractAnimation::Running){
                return;
            }
        }

        if (!m_messageQueue.isEmpty()){
            if(m_animation.animationAt(1)){
                doneBounce();
            }
            m_messageQueue.pop_front();
        }

        unsigned int duration = m_settings.get("gui/out_animation_duration").toInt();
        if (duration <= 30)
            duration = 30;

        QPropertyAnimation* anim = qobject_cast<QPropertyAnimation*>(m_animation.animationAt(0));
        if (!anim) {
            return;
        }

        disconnect(anim, SIGNAL(valueChanged(QVariant)), this, m_activePositionSlot.c_str());

        anim->setDirection(QAnimationGroup::Backward);
        anim->setEasingCurve(QEasingCurve::Type(m_settings.get("gui/out_animation").toInt()));
        anim->setDuration(duration);
        //anim->setCurrentTime(duration);

        connect(anim, SIGNAL(valueChanged(QVariant)), this, m_activePositionSlot.c_str());

        anim->start();
        //m_shortcutGrabber.disableShortcuts();
    } else {
        autoNext();
    }
}

int Widget::computeWidth()
{
    if (m_messageQueue.isEmpty())
        return -1;
    Message& m = m_messageQueue.front();
    QFont boldFont = font();
    boldFont.setBold(true);
    int width = 0;
    QString text = m_contentView["text"]->text();
    width += QFontMetrics(boldFont).boundingRect(m_contentView["title"]->text()).width();
    if (Qt::mightBeRichText(text)) {
        QTextDocument doc;
        doc.setUseDesignMetrics(true);
        doc.setHtml(text);
        doc.setDefaultFont(font());
        width += doc.idealWidth();
    }
    else
        width += QFontMetrics(font()).boundingRect(text).width();
    if (m.data["icon"])
        width += m_contentView["icon"]->pixmap(Qt::ReturnByValue).width();
    return width;
}

void Widget::setupFont()
{
    Message& m = m_messageQueue.front();
    QFont font;
    QString name = m.data["fn"]->toString();
    font.setPixelSize(m.data["fs"]->toInt());
    font.setFamily(name);
    QString ss( m.data["fv"]->toString() );
    if (ss == "oblique")
		font.setStyle( QFont::StyleOblique );
	else if (ss == "italic")
		font.setStyle( QFont::StyleItalic );
	else if (ss == "ultra-light")
		font.setWeight( 13 );
	else if (ss == "light")
		font.setWeight( QFont::Light );
	else if (ss == "medium")
		font.setWeight( 50 );
	else if (ss == "semi-bold")
		font.setWeight( QFont::DemiBold );
	else if (ss == "bold")
		font.setWeight( QFont::Bold );
	else if (ss == "ultra-bold")
		font.setWeight( QFont::Black );
	else if (ss == "heavy")
		font.setWeight( 99 );
	else if (ss == "ultra-condensed")
		font.setStretch( QFont::UltraCondensed );
	else if (ss == "extra-condensed")
		font.setStretch( QFont::ExtraCondensed );
	else if (ss == "condensed")
		font.setStretch( QFont::Condensed );
	else if (ss == "semi-condensed")
		font.setStretch( QFont::SemiCondensed );
	else if (ss == "semi-expanded")
		font.setStretch( QFont::SemiExpanded );
	else if (ss == "expanded")
		font.setStretch( QFont::Expanded );
	else if (ss == "extra-expanded")
		font.setStretch( QFont::ExtraExpanded );
	else if (ss == "ultra-expanded")
		font.setStretch( QFont::UltraExpanded );
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
    int duration = m_settings.get("gui/in_animation_duration").toInt();
    if (duration <= 30)
        duration = 30;
    if (anim->duration() != duration)
        anim->setDuration(duration);
    disconnect(anim, SIGNAL(valueChanged(QVariant)), this, SLOT(updateTopLeftAnimation(QVariant)));
    disconnect(anim, SIGNAL(valueChanged(QVariant)), this, SLOT(updateTopRightAnimation(QVariant)));
    disconnect(anim, SIGNAL(valueChanged(QVariant)), this, SLOT(updateBottomRightAnimation(QVariant)));
    disconnect(anim, SIGNAL(valueChanged(QVariant)), this, SLOT(updateBottomLeftAnimation(QVariant)));
    disconnect(anim, SIGNAL(valueChanged(QVariant)), this, SLOT(updateTopCenterAnimation(QVariant)));
    disconnect(anim, SIGNAL(valueChanged(QVariant)), this, SLOT(updateBottomCenterAnimation(QVariant)));
    disconnect(anim, SIGNAL(valueChanged(QVariant)), this, SLOT(updateCenterAnimation(QVariant)));

    if (position == "top_left" || position == "tl") {
        m_activePositionSlot = SLOT(updateTopLeftAnimation(QVariant));
    }
    else if (position == "top_right" || position == "tr") {
        m_activePositionSlot = SLOT(updateTopRightAnimation(QVariant));
    }
    else if (position == "bottom_right" || position == "br") {
        m_activePositionSlot = SLOT(updateBottomRightAnimation(QVariant));
    }
    else if (position == "bottom_left" || position == "bl") {
        m_activePositionSlot = SLOT(updateBottomLeftAnimation(QVariant));
    }
    else if (position == "top_center" || position == "tc") {
        m_activePositionSlot = SLOT(updateTopCenterAnimation(QVariant));
    }
    else if (position == "bottom_center" || position == "bc") {
        m_activePositionSlot = SLOT(updateBottomCenterAnimation(QVariant));
    }
    else if (position == "center" || position == "c") {
        m_activePositionSlot = SLOT(updateCenterAnimation(QVariant));
    }
    else if (position == "below_cursor" || position == "bcur") {
        m_activePositionSlot = SLOT(updateBelowCursorAnimation(QVariant));
    }
    else {
        // top_right seems to be the classic case so fallback to it.
        m_activePositionSlot = SLOT(updateTopRightAnimation(QVariant));
    }

    connect(anim, SIGNAL(valueChanged(QVariant)), this, m_activePositionSlot.c_str());
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
            pix = pix.scaled(m.data["size"]->toInt()-2, m.data["size"]->toInt()-2, Qt::KeepAspectRatio);
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
        QString text = (m.data["icon"] ? " " : "") + m.data["title"]->toString() + " ";
        foreach (QString i, QStringList() << "\n" << "\r" << "<br/>" << "<br />")
            text.replace(i, " ");

        m_contentView["title"]->setText(text + "| ");
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
        QString text = (m.data["icon"] && !m.data["title"] ? " " : "") + m.data["content"]->toString() + " ";
        foreach (QString i, QStringList() << "\n" << "\r" << "<br/>" << "<br />")
            text.replace(i, " ");
        int max_length = m_settings.get("gui/max_length").toInt();
        if (max_length != -1 && text.size() >= max_length) {
            text.resize(max_length);
            text.append("...");
        }
        m_contentView["text"]->setText(text);
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
    if (!m.data["fv"])
        m.data["fv"] = boost::optional<QVariant>(s->get("gui/font_variant"));
    if (!m.data["pos"])
        m.data["pos"] = boost::optional<QVariant>(s->get("gui/position"));
    if (!m.data["size"])
        m.data["size"] = boost::optional<QVariant>(s->get("gui/height"));
    if (!m.data["icon"])
        m.data["icon"] = loadPixmap(s->has("gui/icon") ? s->get("gui/icon").toString() : "");
    if (!m.data["aot"])
        m.data["aot"] = boost::optional<QVariant>(s->get("gui/always_on_top"));
    if (!m.data["ac"])
        m.data["ac"] = boost::optional<QVariant>(s->get("main/activate_command"));
    if (!m.data["bounce"])
        m.data["bounce"] = boost::optional<QVariant>(s->get("gui/bounce"));
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
            ///TODO: Load standard icons. Surprisingly this doesn't work
            //icon = QIcon::fromTheme(pattern).pixmap(999, 999);
            //if (icon.isNull()) {
                QImage img(1, 1, QImage::Format_ARGB32);
                QPainter p;
                p.begin(&img);
                p.fillRect(0, 0, 1, 1, QBrush(QColor::fromRgb(255, 255, 255, 0)));
                p.end();
                icon = QPixmap::fromImage(img);
            //}
        }
    }
    return icon;
}

bool Widget::update(const Message &m)
{
    bool found = false;
    for (QQueue<Message>::iterator it = m_messageQueue.begin(); it != m_messageQueue.end(); ++it) {
        if (it->data["id"] && it->data["id"]->toInt() == m.data["id"]->toInt()) {
            it->data = m.data;
            found = true;
            break;
        }
    }
    if (found && !m_messageQueue.isEmpty() && m_messageQueue.front().data["id"]
        && m_messageQueue.front().data["id"]->toInt() == m.data["id"]->toInt()) {
        loadDefaults();
        setupFont();
        setupColors();
        setupIcon();
        setupTitle();
        setupContent();
        updateFinalWidth();
        connectForPosition(m_messageQueue.front().data["pos"]->toString());
        m_visible.start();
    }
    return found;
}

QPoint Widget::stringToPos(QString string)
{
    string.replace("X", "x");
    string.replace("*", "x");
    const QStringList splitted = string.split("x");
    if (string.isEmpty() || !string.contains('x') || splitted.size() < 2)
        return QPoint();
    QPoint ret;
    ret.setX(QString(splitted[0]).toInt());
    ret.setY(QString(splitted[1]).toInt());
    if (ret.x() < 0)
      ret.setX(QDesktopWidget().screenGeometry(this).width() + ret.x());
    if (ret.y() < 0)
      ret.setY(QDesktopWidget().screenGeometry(this).height() + ret.y());

    return ret;
}

void Widget::updateFinalWidth()
{
    if (m_messageQueue.empty()) {
        return;
    }

    QString position = m_messageQueue.front().data["pos"]->toString();
    int width = computeWidth();

    qobject_cast<QPropertyAnimation*>(m_animation.animationAt(0))->setEndValue(width);
    if (position == "top_left" || position == "tl")
        updateTopLeftAnimation(width);
    else if (position == "top_right" || position == "tr")
        updateTopRightAnimation(width);
    else if (position == "bottom_right" || position == "br")
        updateBottomRightAnimation(width);
    else if (position == "bottom_left" || position == "bl")
        updateBottomLeftAnimation(width);
    else if (position == "top_center" || position == "tc")
        updateTopCenterAnimation(width);
    else if (position == "bottom_center" || position == "bc")
        updateBottomCenterAnimation(width);
    else if (position == "center" || position == "c")
        updateCenterAnimation(width);
}

void Widget::onPrevious()
{
    if (m_previousStack.size() < 1)
        return;
    m_visible.start();	// Don't run this if returning.
    Message m = m_previousStack.pop();
    m_messageQueue.push_front(m);
    loadDefaults();
    setupFont();
    setupColors();
    setupIcon();
    setupTitle();
    setupContent();
    connectForPosition(m_messageQueue.front().data["pos"]->toString());
    updateFinalWidth();
}

void Widget::onNext()
{
    if (m_messageQueue.size() < 2)
        return;
    m_visible.start();	// Don't run this if returning.
    Message m = m_messageQueue.front();
    boost::optional<QVariant> tmpManual = m.data["manually_shown"];
    m.data["manually_shown"] = boost::optional<QVariant>(true);
    m_previousStack.push(m);
    m_messageQueue.pop_front();
    loadDefaults();
    setupFont();
    setupColors();
    setupIcon();
    setupTitle();
    setupContent();
    connectForPosition(m_messageQueue.front().data["pos"]->toString());
    updateFinalWidth();
    if (m_messageQueue.front().data["bounce"] && !tmpManual)
        startBounce();
}

void Widget::onActivate()
{
	/* I did not implement sticking protection here because it might be desirable. */

    if (!m_messageQueue.isEmpty()) {
        if (startMessageCommand(m_messageQueue.front(), "ac")) {
            m_messageQueue.front().data["ac"] = "";
        }
    }

    if(m_messageQueue.size() > 1) {
        onNext();
    } else {
        onHide();
    }
}

void Widget::onHide()
{
    m_animation.stop();
    m_messageQueue.clear();
    if (m_visible.isActive()) {
        m_visible.setInterval(2);
        //reverseStart();
    }
}

void Widget::autoNext()
{
    Q_ASSERT (m_messageQueue.size() >= 2);
    Message&m = m_messageQueue.front();
    // The user already saw it manually.
    if (m.data["manually_shown"]) {
        m_messageQueue.pop_front();
        reverseStart();
    }
    else {
        startMessageCommand(*(m_messageQueue.begin()+1), "sc");
    }
    onNext();
}

void Widget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
        onActivate();
    QWidget::mousePressEvent(e);
}

void Widget::wheelEvent(QWheelEvent *e)
{
    QPoint angleDelta = e->angleDelta();
    int delta = angleDelta.x() != 0 ? angleDelta.x() : angleDelta.y();
    if (delta > 0)
        onPrevious();
    else if (delta < 0)
        onNext();
    QWidget::wheelEvent(e);
}

std::size_t Widget::getHeight()
{
    QPropertyAnimation* anim = qobject_cast<QPropertyAnimation*>(m_animation.animationAt(0));
    if(anim->direction() == QAbstractAnimation::Forward
            && !m_messageQueue.empty()) {
        return m_messageQueue.front().data["size"]->toInt();
    } else {
        return height();
    }
}
