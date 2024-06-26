#include "settingsmanager.h"
SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent),
    settings(new QSettings(QSettings::IniFormat, QSettings::UserScope, "SonegGX", "VSoneGX")),
    saveTimer(new QTimer(this))
{
    connect(saveTimer, &QTimer::timeout, this, &SettingsManager::saveSettings);
    saveTimer->setSingleShot(true);
}

int SettingsManager::getControlVolume(int controlIndex) const
{
    return settings->value(QString("controlVolume/c_%1").arg(controlIndex), 0).toInt();
}

void SettingsManager::setControlVolume(int controlIndex, int value)
{
    scheduleSettingSave(QString("controlVolume/c_%1").arg(controlIndex), value);
}

int SettingsManager::getControlReverb(int controlIndex) const
{
    return settings->value(QString("controlReverb/c_%1").arg(controlIndex), 0).toInt();
}

void SettingsManager::setControlReverb(int controlIndex, int value)
{
    scheduleSettingSave(QString("controlReverb/c_%1").arg(controlIndex), value);
}


void SettingsManager::saveSelectedInput(const QString &name)
{
    scheduleSettingSave("MidiInput/Input", name);
}

void SettingsManager::saveSelectedOutput(const QString &name)
{
    scheduleSettingSave("MidiOutput/Output", name);
}

QString SettingsManager::loadSelectedInput()
{
    return settings->value("MidiInput/Input", "").toString();
}

QString SettingsManager::loadSelectedOutput()
{
    return settings->value("MidiOutput/Output", "").toString();
}

void  SettingsManager::saveLayerEnabled(int layerSet, int layerNumber, bool enabled){
    QString key = QString("LayerSettings/Set%1_Layer%2").arg(layerSet).arg(layerNumber);
    scheduleSettingSave(key, enabled);
}

bool SettingsManager::getLayerEnabled(int layerSet, int layerNumber) const{
    QString key = QString("LayerSettings/Set%1_Layer%2").arg(layerSet).arg(layerNumber);
    return settings->value(key, false).toBool();
}

QStringList SettingsManager::getCategories() const
{
    return settings->value("Categories").toStringList();
}

// int SettingsManager::saveCategory(const QString &name, int mode, const QString &oldName)
// {
//     // mode 0: add
//     // mode 1: edit
//     QStringList categories = getCategories();

//     if (mode == 0) {
//         // Add mode
//         if (!categories.contains(name)) {
//             categories.append(name);
//             scheduleSettingSave("Categories", categories);
//             return 0; // Success
//         } else {
//             return 1; // Category already exists
//         }
//     } else if (mode == 1) {
//         // Edit mode
//         int index = categories.indexOf(oldName);
//         if (index != -1) {
//             if (name != oldName && categories.contains(name)) {
//                 return 2; // New name already exists
//             }
//             categories[index] = name;
//             scheduleSettingSave("Categories", categories);
//             return 0; // Success
//         } else {
//             return 3; // Old category not found
//         }
//     }

//     return 4; // Invalid mode
// }




int SettingsManager::saveCategory(const QString &name, int mode, const QString &oldName)
{
    QStringList categories = getCategories();

    if (mode == 0) {
        // Add mode
        if (!categories.contains(name)) {
            categories.append(name);
            scheduleSettingSave("Categories", categories);
            return 0; // Success
        } else {
            return 1; // Category already exists
        }
    } else if (mode == 1) {
        // Edit mode
        int index = categories.indexOf(oldName);
        if (index != -1) {
            if (name != oldName && categories.contains(name)) {
                return 2; // New name already exists
            }

            // Rename sounds associated with the old category
            QStringList sounds = getSoundsForCategory(oldName);
            for (const QString &sound : sounds) {
                QVariantMap soundDetails = getSoundDetails(oldName, sound);
                settings->remove("Sounds/" + oldName + "/" + sound);
                settings->setValue("Sounds/" + name + "/" + sound, soundDetails);
            }

            // Remove old category key
            settings->remove("Sounds/" + oldName);

            // Update category name in the list
            categories[index] = name;
            scheduleSettingSave("Categories", categories);

            // Rename the category section
            settings->beginGroup("Sounds");
            settings->remove(oldName);
            settings->endGroup();
            settings->beginGroup("Sounds");
            settings->setValue(name, sounds);
            settings->endGroup();

            return 0; // Success
        } else {
            return 3; // Old category not found
        }
    }

    return 4; // Invalid mode
}


void SettingsManager::deleteCategory(const QString &name)
{
    QStringList categories = getCategories();
    if (categories.removeOne(name)) {
        scheduleSettingSave("Categories", categories);
        settings->remove("Sounds/" + name);
    }
}





QStringList SettingsManager::getSoundsForCategory(const QString &category) const {
    return settings->value("Sounds/" + category).toStringList();
}

int SettingsManager::saveSound(const QString &category, const QString &name, int msb, int lsb, int pc) {
    QStringList sounds = getSoundsForCategory(category);
    QVariantMap soundDetails;
    soundDetails["msb"] = msb;
    soundDetails["lsb"] = lsb;
    soundDetails["pc"] = pc;

    if (!sounds.contains(name)) {
        sounds.append(name);
        settings->setValue("Sounds/" + category, sounds);
    }
    settings->setValue("Sounds/" + category + "/" + name, soundDetails);
    return 0; // Success
}

bool SettingsManager::deleteSound(const QString &category, const QString &name) {
    QStringList sounds = getSoundsForCategory(category);
    if (sounds.removeOne(name)) {
        settings->setValue("Sounds/" + category, sounds);
        settings->remove("Sounds/" + category + "/" + name);
        return true;
    }
    return false;
}

QVariantMap SettingsManager::getSoundDetails(const QString &category, const QString &name) const {
    QVariantMap soundDetails = settings->value("Sounds/" + category + "/" + name).toMap();
    if (soundDetails.isEmpty()) {
        qWarning() << "Sound details not found for category" << category << "and name" << name;
    }
    if (!soundDetails.contains("name")) {
        soundDetails["name"] = name;
    }
    return soundDetails;
}

void SettingsManager::scheduleSettingSave(const QString &key, const QVariant &value)
{
    pendingSettings[key] = value;
    if (!saveTimer->isActive()) {
        saveTimer->start(saveDelay);
    }
}

void SettingsManager::saveSettings()
{
    QMap<QString, QVariant>::iterator it;
    for (it = pendingSettings.begin(); it != pendingSettings.end(); ++it) {
        settings->setValue(it.key(), it.value());
    }
    emit categoriesLoaded();
    pendingSettings.clear();
}
