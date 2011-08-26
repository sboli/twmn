#ifndef WIDGET_H
#define WIDGET_H

#include <QtGui/QWidget>
#include <QUdpSocket>
#include <QLabel>
#include <QQueue>
#include "settings.h"
#include "message.h"
#include "dbusinterface.h"
#include "shortcutgrabber.h"

class Widget : public QWidget
{
    Q_OBJECT
public:
    Widget();
    ~Widget();

private slots:
    void                    init();
    void                    onDataReceived();
    void                    appendMessageToQueue(const Message& msg);
    void                    processMessageQueue();

    void                    updateTopLeftAnimation(QVariant value);
    void                    updateTopRightAnimation(QVariant value);
    void                    updateBottomRightAnimation(QVariant value);
    void                    updateBottomLeftAnimation(QVariant value);
    void                    updateTopCenterAnimation(QVariant value);
    void                    updateBottomCenterAnimation(QVariant value);
    void                    updateCenterAnimation(QVariant value);
    void                    reverseTrigger();
    void                    reverseStart();

    void                    updateFinalWidth();


public slots:
    // Called from the ShortcutGrabber
    void                    onPrevious();

    void                    onNext();

    /*!
      * \brief Run a command when the user activate the notification
      */
    void                    onActivate();

    /*!
      * \brief Hide the notification
      */
    void                    onHide();

    /*!
      * \brief Display the next notification as if the user invoqued onNext()
      */
    void                    autoNext();

    void                    mousePressEvent(QMouseEvent *);

    void                    wheelEvent(QWheelEvent *e);

private:
    /*!
      * \brief Get the final width of the slide after everything is set.
      */
    int                     computeWidth();

    void                    setupFont();

    void                    setupColors();

    /*!
      * \brief Create an appropriate connection according to the position parameter from Settings
      */
    void                    connectForPosition(QString position);

    /*!
      * \brief Set the icon.
      */
    void                    setupIcon();

    /*!
      * \brief Set the "title" widget content according to the front() Message.
      */
    void                    setupTitle();

    /*!
      * \brief Set the "text" widget content according to the front() Message.
      */
    void                    setupContent();

    /*!
      * \brief Load default settings for the front() Message according to the specified profile and configuration files.
      */
    void                    loadDefaults();

    /*!
      * \brief Tries to load a Pixmap from pattern : from a file, from a setting value.
      */
    QPixmap                 loadPixmap(QString pattern);

    /*!
      * \brief Update the message m if it's already in the queue
      * \return true if a message has been updated
      */
    bool                    update(const Message& m);

    QPoint                  stringToPos(QString string);

private:
    Settings                m_settings;
    QUdpSocket              m_socket;
    QMap<QString, QLabel*>  m_contentView;
    QQueue<Message>         m_messageQueue;
    QParallelAnimationGroup m_animation;
    DBusInterface           m_dbus;
    QTimer                  m_visible;
    ShortcutGrabber         m_shortcutGrabber;
    QStack<Message>         m_previousStack;
};

#endif // WIDGET_H
