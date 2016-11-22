#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QApplication>
#include <QtCore>

/*!
  * \brief Gère les paramètres du programme
  */
class Settings
{
public:
    Settings(QString file = QApplication::applicationName());

    /*!
      * \brief Sauvegarde les réglages
      */
     ~Settings();

    /*!
      * \brief Recharge les paramètres à partir du fichier.
      */
    void                reload();

    /*!
      * \brief Enregistre le paramètre setting
      */
    void                set(QString setting, const QVariant& value);

    /*!
      * \brief Récupère le paramètre setting
      */
    QVariant		    get(QString setting);

    /*!
      * \return True si le paramètre setting existe.
      */
    bool                has(QString setting);

    /*!
      * \brief Create default settings
      */
    void                createDefaults();

    /*!
      * \brief Complète cette instance avec les donnés de s, sans écraser les données de cette instance.
      */
    void                fillWith(const Settings& s);

private:
    QMap<QString, QVariant> m_data;
    QMap<QString, QVariant> m_defaultSettings;
    QString                 m_file;
};

#endif // SETTINGS_H
