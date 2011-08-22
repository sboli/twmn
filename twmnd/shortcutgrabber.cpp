#include "shortcutgrabber.h"
#include <QtCore>
#include <QtGui>
#include <QX11Info>
#include <X11/Xlib.h>
#include <QTimer>

#include <iomanip>
#include <iostream>

#include "widget.h"

static int (*defaultXErrhandler)(Display* display, XErrorEvent* event);

static int xErrhandler(Display* display, XErrorEvent *event)
{
    Q_UNUSED(display);
    switch (event->error_code)
    {
        case BadAccess:
        case BadValue:
        case BadWindow:
            if (event->request_code == 33 || //< Error from XGrabKey
                event->request_code == 34 )  //< Error from XUngrabKey
            {
                qDebug() << "XGrab|Ungrab error";
            }
        default:
            return 0;
    }
}

ShortcutGrabber* sgInstance = 0;

ShortcutGrabber::ShortcutGrabber(QObject *parent, Settings& settings_) : QObject(parent), m_settings(settings_)
{
    m_widget = qobject_cast<Widget*>(parent);
    Q_ASSERT(m_widget);
    Q_ASSERT(!sgInstance);
    sgInstance = this;
}

void ShortcutGrabber::enableShortcuts()
{
    if (!configAllowShortcut())
        return;
    Display* display = QX11Info::display();
    Window window = QX11Info::appRootWindow();
    foreach (Shortcut i, m_shortcuts) {
        quint32 nativeKey = i.get<0>();
        quint32 nativeMods = i.get<1>();

        Bool owner = True;
        int pointer = GrabModeAsync;
        int keyboard = GrabModeAsync;
        defaultXErrhandler = XSetErrorHandler(xErrhandler);
        XGrabKey(display, nativeKey, nativeMods | Mod2Mask, window, owner, pointer, keyboard);
        XGrabKey(display, nativeKey, nativeMods, window, owner, pointer, keyboard);
        XSync(display, False);
        XSetErrorHandler(defaultXErrhandler);
    }
}

void ShortcutGrabber::disableShortcuts()
{
    if (!configAllowShortcut())
        return;
    Display* display = QX11Info::display();
    Window window = QX11Info::appRootWindow();
    foreach (Shortcut i, m_shortcuts) {
        quint32 nativeKey = i.get<0>();
        quint32 nativeMods = i.get<1>();
        defaultXErrhandler = XSetErrorHandler(xErrhandler);
        XUngrabKey(display, nativeKey, nativeMods, window);
        XUngrabKey(display, nativeKey, nativeMods | Mod2Mask, window); // allow numlock
        XSync(display, False);
        XSetErrorHandler(defaultXErrhandler);
    }
}

bool ShortcutGrabber::eventFilter(void* message)
{
    Q_ASSERT(sgInstance);
    Q_ASSERT(message);
    if (!sgInstance->configAllowShortcut())
        return false;
    XEvent* event = static_cast<XEvent*>(message);
    if (event->type == KeyPress)
    {
        XKeyEvent* key = (XKeyEvent*) event;
        sgInstance->propagate(Shortcut(key->keycode, key->state & (ShiftMask | ControlMask | Mod1Mask | Mod4Mask)));
    }
    return false;
}

void ShortcutGrabber::loadShortcuts()
{
    if (!configAllowShortcut())
        return;
    Display* display = QX11Info::display();
    // reading the modifier QKeySequence as a string fix the + problem at the end in case of multiple modifiers
    QString tmp = m_settings.get("shortcuts/modifiers").toString();
    QKeySequence modSeq;
    if (!tmp.endsWith("+"))
        modSeq = QKeySequence(tmp + "+");
    else
        modSeq = QKeySequence(tmp);
    const Qt::KeyboardModifier modifier = Qt::KeyboardModifier(modSeq[0]);
    quint32 nativeMods = 0;
    if (modifier & Qt::ShiftModifier)
        nativeMods |= ShiftMask;
    if (modifier & Qt::ControlModifier)
        nativeMods |= ControlMask;
    if (modifier & Qt::AltModifier)
        nativeMods |= Mod1Mask;
    if (modifier & Qt::MetaModifier)
        nativeMods |= Mod4Mask;
    QList<QKeySequence> fromFile;
    fromFile    << qvariant_cast<QKeySequence>(m_settings.get("shortcuts/previous"))
                << qvariant_cast<QKeySequence>(m_settings.get("shortcuts/next"))
                << qvariant_cast<QKeySequence>(m_settings.get("shortcuts/activate"))
                << qvariant_cast<QKeySequence>(m_settings.get("shortcuts/hide"));
    Action act = PREVIOUS;
    foreach (const QKeySequence& i, fromFile) {
        quint32 nativeKey = XKeysymToKeycode(display, XStringToKeysym(i.toString().toLatin1().data()));
        m_shortcuts.append(Shortcut(nativeKey, nativeMods, act));
        qDebug() << modSeq.toString() << i.toString() << int(act);
        act = Action(int(act) + 1);
    }
}

bool ShortcutGrabber::configAllowShortcut()
{
    return m_settings.get("main/enable_shortcuts").toBool();
}

void ShortcutGrabber::propagate(const Shortcut &shortcut)
{
    switch (*shortcutToAction(shortcut)) {
    case PREVIOUS:
        m_widget->onPrevious();
        break;
    case NEXT:
        m_widget->onNext();
        break;
    case ACTIVATE:
        m_widget->onActivate();
        break;
    case HIDE:
        m_widget->onHide();
        break;
    }
}

boost::optional<ShortcutGrabber::Action> ShortcutGrabber::shortcutToAction(const Shortcut &shortcut)
{
    boost::optional<Action> ret;
    foreach(const Shortcut& i, m_shortcuts) {
        if (i.get<0>() == shortcut.get<0>() && i.get<1>() == shortcut.get<1>())
            ret = i.get<2>();
    }
    return ret;
}
