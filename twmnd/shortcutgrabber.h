#ifndef SHORTCUTGRABBER_H
#define SHORTCUTGRABBER_H

#include <QObject>
#include <QList>
#include <QPair>
#include <QtGlobal>
#include <boost/tuple/tuple.hpp>
#include <boost/optional.hpp>

class Widget;
class Settings;

class ShortcutGrabber : public QObject
{
    Q_OBJECT
    friend bool eventFilter(void *message);
    enum Action { PREVIOUS, NEXT, ACTIVATE, HIDE};
    typedef boost::tuples::tuple<quint32, quint32, Action> Shortcut;
public:
    explicit ShortcutGrabber(QObject *parent, Settings& settings_);
    static bool eventFilter(void* message);

    void        loadShortcuts();

    /*!
      * \return true if shortcuts is enabled from the config file.
      */
    bool        configAllowShortcut();

public slots:
    void        enableShortcuts();

    void        disableShortcuts();

private:
    void        propagate(const Shortcut& shortcut);

    /*!
      * \brief Find the shortcut in the shorcut list and return it's action
      * \return The action, if found
      */
    boost::optional<Action>         shortcutToAction(const Shortcut& shortcut);

private:
    Widget*                         m_widget;
    Settings&                       m_settings;
    QList<Shortcut>                 m_shortcuts;
};

#endif // SHORTCUTGRABBER_H
