#include "settings.h"

Settings::Settings()
{
    m_defaultSettings["main/port"] = 9797;
    m_defaultSettings["main/sound_command"] = "";
    m_defaultSettings["main/duration"] = "3000";
    m_defaultSettings["gui/position"] = "top_right";
    m_defaultSettings["gui/height"] = 18;
    m_defaultSettings["gui/foreground_color"] = "#000055";
    m_defaultSettings["gui/background_color"] = "#000000";
    m_defaultSettings["icons/critical_icon"] = "";
    m_defaultSettings["icons/warning_icon"] = "";
    m_defaultSettings["icons/info_icon"] = "";
    reload();
}

Settings::~Settings()
{
    save();
}

void Settings::reload()
{
    QSettings settings(QSettings::NativeFormat, QSettings::UserScope, "twmn", "twmn");
    settings.setIniCodec("UTF-8");
    QStringList keys = settings.allKeys();
    m_data.clear();
    foreach (QString i, keys) {
        m_data[i] = settings.value(i);
    }
    save();
}

void Settings::set(QString setting, const QVariant &value)
{
    m_data[setting] = value;
}

QVariant Settings::get(QString setting)
{
    if (!m_data.contains(setting)) {
        if (!m_defaultSettings.contains(setting)) {
            qCritical() << "Settings::get(QString setting): Attempt to get a non existing setting";
            qApp->quit();
        }
        return m_defaultSettings[setting];
    }
    return m_data[setting];
}

bool Settings::has(QString setting)
{
    return m_data.contains(setting) || m_defaultSettings.contains(setting);
}

void Settings::save()
{
    QSettings settings(QSettings::NativeFormat, QSettings::UserScope, "twmn", "twmn");
    settings.setIniCodec("UTF-8");
    settings.clear();
    for (QMap<QString, QVariant>::const_iterator it = m_data.begin(); it != m_data.end(); ++it) {
        m_defaultSettings[it.key()] = it.value();
    }
    for (QMap<QString, QVariant>::const_iterator it = m_defaultSettings.begin(); it != m_defaultSettings.end(); ++it) {
        settings.setValue(it.key(), it.value());
    }
}
