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

#include <algorithm>
#include <bitset>
#include <cassert>
#include <memory>

/**
 * Respose handle for caller after upgrade request has completed
 */
template <typename OldType, typename NewType>
class UpgradableSettingResponse
{
private:
    const bool upgraded;
    void* value;

public:
    UpgradableSettingResponse(NewType* newType, bool upgraded)
        : upgraded(upgraded)
        , value(newType)
    {}

    // we can only instantiate this if OldType != NewType or we'll have a duplicate declaration
    template <bool OldIsNewType = std::is_same<OldType, NewType>::value>
    UpgradableSettingResponse(OldType* oldType, bool upgraded,
                              typename std::enable_if<OldIsNewType, int>::type = 0)
        : upgraded(upgraded)
        , value(oldType)
    {}

    /**
     * @brief Indicates if inner data is upgraded to NewType
     * @return true if upgraded
     */
    bool isUpgraded() const
    {
        return upgraded;
    }

    OldType& asOldType() const
    {
        assert(!upgraded);
        return *static_cast<OldType*>(value);
    }

    OldType& asNewType() const
    {
        assert(upgraded);
        return *static_cast<NewType*>(value);
    }
};

/**
 * Assists with settings value upgrades. Will ask the user if an upgrade is okay
 */
template <typename OldType, typename NewType>
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
         * @param[in] message Message to use when requesting an upgrade from the
         * user. Should be in the form of a question
         * @note Mandatory parameter
         */
        Builder& upgradeMessage(QString message);

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
            upgradeMessage,
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
     * @brief Adds an upgrade request. We re-use the names assuming that all fields are related,
     * but it's expected that you may have a different default for each contact. On completion
     * of the request we callback to the above layer to decide what to do with the upgrade of this item.
     * Note that fields from the settings struct are resolved on call of addItem, so if they change between
     * the call of addItem and requestUpgrade you may end up with out of date state. This is done to help
     * with the parsing of arrays of data.
     * @param name Name to associate with this item
     * @param settings Settings struct we get old/new values from
     * @param newDefault New default value in the case that we're okay to upgrade and the new one isn't set
     * @param defaultStr In some cases the default value isn't something sensable to expose to a user.
     * In this case we allow the caller to pass in a string they'd prefer to show instead
     * @param onRequestFinsihed Callback for handling user response
     */
    template <typename SettingsType>
    void addItem(SettingsType& settings, QString name, NewType newDefault, QString defaultStr,
                 std::function<void(UpgradableSettingResponse<OldType, NewType> const&)> onRequestFinished)
    {
        QVariant upgradeHasHappened = settings.value(upgradeHasHappenedName);
        QVariant oldValue = settings.value(oldSettingsName);
        // If upgradeHasHappened is either true or false we've already asked the user and they
        // said no so we're stuck never upgrading
        bool needsUpgrade = !upgradeHasHappened.isValid();

        // If the user never set the old default we don't have to ask them
        bool oldValueIsNull = !oldValue.isValid()
                              || (defaultConstructedIsNull && oldValue.value<OldType>() == OldType());

        bool needsSettingsRequest = false;

        // We should only request a settings upgrade if we have never requested one
        // before *and* the old settings have ever been set
        if (needsUpgrade && !oldValueIsNull) {
            needsSettingsRequest = true;
        }

        if (needsSettingsRequest) {
            auto oldItem = std::shared_ptr<OldType>(new OldType(oldValue.value<OldType>()));
            requestUpgradeItems.push_back({name, std::shared_ptr<NewType>(new NewType(newDefault)),
                                           defaultStr, oldItem, onRequestFinished});
        } else {
            if (upgradeHasHappened.isValid() && upgradeHasHappened.value<bool>() == false) {
                auto oldItem = std::shared_ptr<OldType>(new OldType(oldValue.value<OldType>()));
                upgradeItems.push_back({name, nullptr, "", std::move(oldItem), onRequestFinished});
            } else {
                // We already know we don't need a settings request which means we're good to go for using the new type
                QVariant newValue = settings.value(newSettingsName, newDefault);
                auto newItem = std::shared_ptr<NewType>(new NewType(newValue.value<NewType>()));
                upgradeItems.push_back({name, std::move(newItem), "", nullptr, onRequestFinished});
            }
        }
    }


private:
    UpgradableSetting() = default;

    // Note: shared_ptrs are used since UpgradeItem is intended to be stored in a vector. Thanks to
    // vector semantics we cannot do that if we use unique_ptrs instead
    struct UpgradeItem
    {
        QString name;
        // Stores default or new depending on if we have already upgraded
        std::shared_ptr<NewType> newItem;
        // Sometimes the default string will be NULL to indicate an automatic value. In this case
        // the caller passes in a string indicating what the value will actually be
        QString defaultStr;
        // Stores old if we are upgrading or we have decided we do not want to upgrade
        std::shared_ptr<OldType> oldItem;
        std::function<void(UpgradableSettingResponse<OldType, NewType> const&)> onRequestFinished;
    };

    QString oldSettingsName;
    QString newSettingsName;
    QString upgradeHasHappenedName;
    QString upgradeMessage;
    // Items we can upgrade for free
    std::vector<UpgradeItem> upgradeItems;
    // Items that require a prompt to upgrade
    std::vector<UpgradeItem> requestUpgradeItems;
    bool defaultConstructedIsNull = false;
};

class UpgradableSettingDetail
{
    template <typename OldType, typename NewType>
    friend class UpgradableSetting;

    struct MessageItem
    {
        QString name;
        QString oldValue;
        QString newDefault;
    };

    /**
     * @brief Requests upgrade of setting to the user.
     * @param[in] message Message to display
     * @return true if the upgrade was successful for each item in items
     */
    static std::vector<bool> doUpgradeRequest(QString const& message, std::vector<MessageItem> items);
};

template <typename OldType, typename NewType>
typename UpgradableSetting<OldType, NewType>::Builder&
UpgradableSetting<OldType, NewType>::Builder::oldSettingsName(QString name)
{
    setting.oldSettingsName = std::move(name);
    setMandatoryFields[mandatoryFieldsIdx(MandatoryFields::oldSettings)] = true;
    return *this;
}

template <typename OldType, typename NewType>
typename UpgradableSetting<OldType, NewType>::Builder&
UpgradableSetting<OldType, NewType>::Builder::newSettingsName(QString name)
{
    setting.newSettingsName = std::move(name);
    setMandatoryFields[mandatoryFieldsIdx(MandatoryFields::newSettings)] = true;
    return *this;
}

template <typename OldType, typename NewType>
typename UpgradableSetting<OldType, NewType>::Builder&
UpgradableSetting<OldType, NewType>::Builder::upgradeHasHappenedName(QString name)
{
    setting.upgradeHasHappenedName = std::move(name);
    setMandatoryFields[mandatoryFieldsIdx(MandatoryFields::upgradeHasHappened)] = true;
    return *this;
}

template <typename OldType, typename NewType>
typename UpgradableSetting<OldType, NewType>::Builder&
UpgradableSetting<OldType, NewType>::Builder::upgradeMessage(QString message)
{
    setting.upgradeMessage = std::move(message);
    setMandatoryFields[mandatoryFieldsIdx(MandatoryFields::upgradeMessage)] = true;
    return *this;
}

template <typename OldType, typename NewType>
typename UpgradableSetting<OldType, NewType>::Builder&
UpgradableSetting<OldType, NewType>::Builder::defaultConstructedIsNull()
{
    setting.defaultConstructedIsNull = true;
    return *this;
}

template <typename OldType, typename NewType>
UpgradableSetting<OldType, NewType> UpgradableSetting<OldType, NewType>::Builder::build()
{
    assert(setMandatoryFields.all());
    return setting;
}

template <typename OldType, typename NewType>
void UpgradableSetting<OldType, NewType>::requestUpgrade()
{
    std::vector<UpgradableSettingDetail::MessageItem> messageItems;
    messageItems.reserve(requestUpgradeItems.size());
    std::transform(requestUpgradeItems.begin(), requestUpgradeItems.end(),
                   std::back_inserter(messageItems), [&](UpgradeItem& upgradeItem) {
                       QString name = upgradeItem.name;
                       QString oldValue = static_cast<QString>(*upgradeItem.oldItem);
                       QString newDefault = !upgradeItem.defaultStr.isEmpty()
                                                ? upgradeItem.defaultStr
                                                : static_cast<QString>(*upgradeItem.newItem);
                       return UpgradableSettingDetail::MessageItem{name, oldValue, newDefault};
                   });

    std::vector<bool> requestResults =
        UpgradableSettingDetail::doUpgradeRequest(upgradeMessage, messageItems);

    assert(requestResults.size() == requestUpgradeItems.size());

    for (size_t i = 0; i < requestResults.size(); ++i) {
        auto& upgradeItem = requestUpgradeItems[i];
        if (requestResults[i]) {
            UpgradableSettingResponse<OldType, NewType> response(upgradeItem.newItem.get(), true);
            upgradeItem.onRequestFinished(response);

        } else {
            UpgradableSettingResponse<OldType, NewType> response(upgradeItem.oldItem.get(), false);
            upgradeItem.onRequestFinished(response);
        }
    }

    for (auto& upgradeItem : upgradeItems) {
        UpgradableSettingResponse<OldType, NewType> response(upgradeItem.newItem.get(), true);
        upgradeItem.onRequestFinished(response);
    }
}

#endif // UPGRADABLESETTING_H
