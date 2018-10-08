/*
    Copyright © 2018 by The qTox Project Contributors

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef UPGRADABLESETTING_H
#define UPGRADABLESETTING_H

#include <QSettings>
#include <QString>
#include <QVariant>

#include <bitset>
#include <cassert>
#include <memory>

/**
 * Assists with settings value upgrades. Will ask the user if an upgrade is okay
 */
template <typename OldType, typename NewType, typename SettingsType = QSettings>
class UpgradableSetting
{
public:
    /**
     * Helps construct our UpgradableSetting without a lengthy constructor
     */
    class Builder
    {
    public:
        Builder() = default;

        /**
         * @param[in] name name of old settings field
         * @note Mandatory parameter
         */
        Builder& oldSettingsName(QString name);

        /**
         * @param[in] name name of new settings field
         * @note Mandatory parameter
         */
        Builder& newSettingsName(QString name);

        /**
         * @param[in] name name of setting indicating that we have already done this upgrade
         * @note Mandatory parameter
         */
        Builder& upgradeHasHappenedName(QString name);

        /**
         * @param[in] defaultVal default value for new field
         * @note Mandatory parameter
         */
        Builder& newDefault(NewType defaultVal);

        /**
         * @param[in] message Message to use when requesting an upgrade from the
         * user. Should be in the form of a question
         * @note Mandatory parameter
         */
        Builder& upgradeMessage(QString message);

        /**
         * @param[in] settings A reference to our saved settings storage
         * @note Mandatory parameter
         */
        Builder& settings(const SettingsType& settings);

        /**
         * @brief Indicates that if the saved value exists but is the same as a default constructed
         * value of OldType we can treat the item as if it's never been set
         * @note Optional parameter
         */
        Builder& defaultConstructedIsNull();

        /**
         * @brief Constructs our upgradable setting. Assumes all mandatory fields have been set
         * @return UpgradableSetting
         */
        UpgradableSetting build();

    private:
        enum class MandatoryFields
        {
            oldSettings,
            newSettings,
            upgradeHasHappened,
            newDefault,
            upgradeMessage,
            settings,
            numElems,
        };

        /**
         * @brief Helper function to cast the MandatoryFields enum back to something useable to index a bitset at compile time
         */
        static constexpr size_t mandatoryFieldsIdx(MandatoryFields field)
        {
            return static_cast<typename std::underlying_type<MandatoryFields>::type>(field);
        }

        UpgradableSetting setting;
        /// Tracking variable to ensure that the caller has set all mandatory fields before building
        /// the UpgradableSetting. Only use at time of writing is to assert on the build() function
        std::bitset<mandatoryFieldsIdx(MandatoryFields::numElems)> setMandatoryFields{0};
    };

    /**
     * @brief Requests upgrade of setting to the user. This will only be
     * prompted once per setting and only if the previous value has already
     * been set
     * @param[in] message Message to display
     */
    void requestUpgrade();

    /**
     * @return true if the user accepted the upgrade request
     */
    bool isUpgraded();

    /**
     * @return The old value loaded from the settings db
     */
    OldType oldValue();

    /**
     * @return The new value loaded from the settings db
     */
    NewType newValue();

private:
    UpgradableSetting() = default;

    QString oldSettingsName;
    QString newSettingsName;
    QString upgradeHasHappenedName;
    OldType newDefault;
    QString upgradeMessage;
    // Since settings is const we cannot change the value of newSettingsName. This becomes
    // problematic if newSettingsName == oldSettingsName so we just cache the value ourselves
    std::shared_ptr<NewType> _newValue;
    const SettingsType* settings;
    bool defaultConstructedIsNull = false;
    bool useNewValue = false;
};

class UpgradableSettingDetail
{
    template <typename OldType, typename NewType, typename SettingsType>
    friend class UpgradableSetting;

    /**
     * @brief Requests upgrade of setting to the user.
     * @param[in] message Message to display
     * @return true if the upgrade was successful
     */
    static bool doUpgradeRequest(QString const& message);
};

template <typename OldType, typename NewType, typename SettingsType>
typename UpgradableSetting<OldType, NewType, SettingsType>::Builder&
UpgradableSetting<OldType, NewType, SettingsType>::Builder::oldSettingsName(QString name)
{
    setting.oldSettingsName = std::move(name);
    setMandatoryFields[mandatoryFieldsIdx(MandatoryFields::oldSettings)] = true;
    return *this;
}

template <typename OldType, typename NewType, typename SettingsType>
typename UpgradableSetting<OldType, NewType, SettingsType>::Builder&
UpgradableSetting<OldType, NewType, SettingsType>::Builder::newSettingsName(QString name)
{
    setting.newSettingsName = std::move(name);
    setMandatoryFields[mandatoryFieldsIdx(MandatoryFields::newSettings)] = true;
    return *this;
}

template <typename OldType, typename NewType, typename SettingsType>
typename UpgradableSetting<OldType, NewType, SettingsType>::Builder&
UpgradableSetting<OldType, NewType, SettingsType>::Builder::upgradeHasHappenedName(QString name)
{
    setting.upgradeHasHappenedName = std::move(name);
    setMandatoryFields[mandatoryFieldsIdx(MandatoryFields::upgradeHasHappened)] = true;
    return *this;
}

template <typename OldType, typename NewType, typename SettingsType>
typename UpgradableSetting<OldType, NewType, SettingsType>::Builder&
UpgradableSetting<OldType, NewType, SettingsType>::Builder::newDefault(NewType defaultVal)
{
    setting.newDefault = std::move(defaultVal);
    setMandatoryFields[mandatoryFieldsIdx(MandatoryFields::newDefault)] = true;
    return *this;
}

template <typename OldType, typename NewType, typename SettingsType>
typename UpgradableSetting<OldType, NewType, SettingsType>::Builder&
UpgradableSetting<OldType, NewType, SettingsType>::Builder::upgradeMessage(QString message)
{
    setting.upgradeMessage = std::move(message);
    setMandatoryFields[mandatoryFieldsIdx(MandatoryFields::upgradeMessage)] = true;
    return *this;
}

template <typename OldType, typename NewType, typename SettingsType>
typename UpgradableSetting<OldType, NewType, SettingsType>::Builder&
UpgradableSetting<OldType, NewType, SettingsType>::Builder::settings(const SettingsType& settings)
{
    setting.settings = &settings;
    setMandatoryFields[mandatoryFieldsIdx(MandatoryFields::settings)] = true;
    return *this;
}

template <typename OldType, typename NewType, typename SettingsType>
typename UpgradableSetting<OldType, NewType, SettingsType>::Builder&
UpgradableSetting<OldType, NewType, SettingsType>::Builder::defaultConstructedIsNull()
{
    setting.defaultConstructedIsNull = true;
    return *this;
}

template <typename OldType, typename NewType, typename SettingsType>
UpgradableSetting<OldType, NewType, SettingsType>
UpgradableSetting<OldType, NewType, SettingsType>::Builder::build()
{
    assert(setMandatoryFields.all());
    return setting;
}

template <typename OldType, typename NewType, typename SettingsType>
void UpgradableSetting<OldType, NewType, SettingsType>::requestUpgrade()
{
    auto upgradeHasHappened = settings->value(upgradeHasHappenedName);
    QVariant oldValue = settings->value(oldSettingsName);

    // If upgradeHasHappened is either true or false we've already asked the user and they said no
    // so we're stuck never upgrading
    bool needsUpgrade = !upgradeHasHappened.isValid();

    // If the user never set the old default we don't have to ask them
    bool oldValueIsNull =
        !oldValue.isValid() || (defaultConstructedIsNull && oldValue.value<OldType>() == OldType());

    bool needsSettingsRequest = false;

    // We should only request a settings upgrade if we have never requested one
    // before *and* the old settings have ever been set
    if (needsUpgrade && !oldValueIsNull) {
        needsSettingsRequest = true;
    }

    if (needsSettingsRequest) {
        useNewValue = UpgradableSettingDetail::doUpgradeRequest(upgradeMessage);
    } else {
        // If we don't have to ask the user we only don't use the new value if we previously asked
        // and they said no
        useNewValue =
            !upgradeHasHappened.isValid() || settings->value(upgradeHasHappenedName).toBool();
    }

    if (needsUpgrade && oldSettingsName == newSettingsName) {
        // We cannot load the newSettings value if newSettings == oldSettings or we'll just get the
        // new one again. If we need an upgrade we can just unconditionally use the new default in
        // this case
        _newValue.reset(new NewType(newDefault));
    } else if (useNewValue) {
        // If we didn't upgrade this time around but we still want to use the new value we load from
        // storage
        QVariant val = settings->value(newSettingsName, newDefault);
        _newValue.reset(new NewType(val.value<NewType>()));
    } else {
        // If we don't want to use the new value just reset the pointer to avoid wasting memory
        _newValue.reset();
    }
}

template <typename OldType, typename NewType, typename SettingsType>
bool UpgradableSetting<OldType, NewType, SettingsType>::isUpgraded()
{
    return useNewValue;
}

template <typename OldType, typename NewType, typename SettingsType>
OldType UpgradableSetting<OldType, NewType, SettingsType>::oldValue()
{
    assert(!useNewValue);
    QVariant val = settings->value(oldSettingsName);
    return val.value<OldType>();
}

template <typename OldType, typename NewType, typename SettingsType>
NewType UpgradableSetting<OldType, NewType, SettingsType>::newValue()
{
    assert(_newValue && useNewValue);
    return *_newValue;
}


#endif // UPGRADABLESETTING_H
