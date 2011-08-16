#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QtCore>

/*!
  * \brief Gère les paramètres du programme
  */
class Settings
{
public:
    Settings();

    /*!
      * \brief Sauvegarde les réglages
      */
     ~Settings();

    /*!
      * \brief Recharge les paramètres à partir du fichier.
      */
    void		    reload();

    /*!
      * \brief Enregistre le paramètre setting
      */
    void		    set(QString setting, const QVariant& value);

    /*!
      * \brief Récupère le paramètre setting
      */
    QVariant		    get(QString setting);

    /*!
      * \return True si le paramètre setting existe.
      */
    bool                has(QString setting);

    /*!
      * \brief Sauvegarde les paramètres.
      */
    void		    save();

private:
    QMap<QString, QVariant> m_data;
    QMap<QString, QVariant> m_defaultSettings;
};

#endif // SETTINGS_H
