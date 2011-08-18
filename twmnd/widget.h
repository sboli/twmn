#ifndef WIDGET_H
#define WIDGET_H

#include <boost/optional.hpp>
#include <QtGui/QWidget>
#include <QSystemTrayIcon>
#include <QUdpSocket>
#include <QLabel>
#include <QQueue>
#include "settings.h"

#define BO(t,a) boost::optional<t> a;

struct Message
{
    QMap<QString, boost::optional<QVariant> > data;
};

class Widget : public QWidget
{
    Q_OBJECT
public:
    Widget();
    ~Widget();

private slots:
    void                    init();
    void                    onDataReceived();
    void                    processMessageQueue();
    void                    updateTopLeftAnimation(QVariant value);
    void                    updateTopRightAnimation(QVariant value);
    void                    updateBottomRightAnimation(QVariant value);
    void                    updateBottomLeftAnimation(QVariant value);
    void                    reverseTrigger();
    void                    reverseStart();

private:
    /*!
      * \brief Get the final width of the slide after everything is set.
      */
    int                     computeWidth();

    void                    setupFont();

    void                    setupColors();

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
private:
    Settings                m_settings;
    QUdpSocket              m_socket;
    QMap<QString, QLabel*>  m_contentView;
    QQueue<Message>         m_messageQueue;
    QParallelAnimationGroup m_animation;
};

#endif // WIDGET_H
