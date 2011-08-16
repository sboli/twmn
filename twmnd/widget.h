#ifndef WIDGET_H
#define WIDGET_H

#include <QtGui/QWidget>
#include <QSystemTrayIcon>
#include <QUdpSocket>
#include <QLabel>
#include <QQueue>
#include "settings.h"

struct Message
{
    QIcon icon;
    QString title;
    QString text;
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
    Settings                m_settings;
    QUdpSocket              m_socket;
    QMap<QString, QLabel*>  m_contentView;
    QQueue<Message>         m_messageQueue;
    QParallelAnimationGroup m_animation;
};

#endif // WIDGET_H
