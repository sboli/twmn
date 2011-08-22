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
    enum Action { PREVIOUS, NEXT, ACTIVATE, HIDE};      ///< An action \see Settings
    typedef boost::tuples::tuple<quint32, quint32, Action> Shortcut;    ///< A shortcut (X11 key, X11 mod, action) \see Action

public:
    explicit ShortcutGrabber(QObject *parent, Settings& settings_);

    /*!
      * \brief Workaround to grab X11 input.
      */
    static bool eventFilter(void* message);

    /*!
      * \brief Load the shortcuts from the configuration file
      */
    void        loadShortcuts();

    /*!
      * \brief It this return false then \see loadShortcuts \see enableShortcuts \see disableShortcuts won't work.
      * \return true if shortcuts is enabled from the config file.
      */
    bool        configAllowShortcut();

public slots:
    /*!
      * \brief Enable the shortcuts if any is found
      */
    void        enableShortcuts();

    /*!
      * \brief Disable shortcuts
      */
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
