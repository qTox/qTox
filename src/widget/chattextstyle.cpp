#include "chattextstyle.h"
#include <QCryptographicHash>
#include "src/widget/style.h"

ChatTextStyle::ChatTextStyle()
{
    defFont = std::make_shared<QFont>();
    busyFont = std::make_shared<QFont>();

    fonts[key(User, Nickname)] = std::make_shared<QFont>();
    fonts[key(User, Msg)] = std::make_shared<QFont>();
    fonts[key(Friend, Nickname)] = std::make_shared<QFont>();
    fonts[key(Friend, Msg)] = std::make_shared<QFont>();

    colors[key(User, Nickname)] = std::make_shared<QColor>();
    colors[key(User, Msg)] = std::make_shared<QColor>();
    colors[key(Friend, Nickname)] = std::make_shared<QColor>();
    colors[key(Friend, Msg)] = std::make_shared<QColor>();

    actionColor = std::make_shared<QColor>(Style::getColor(Style::Action));

    nicknameUserColor = std::make_shared<QColor>(Style::getColor(Style::MainText));
    msgUserColor = std::make_shared<QColor>(Style::getColor(Style::MainText));
}

void ChatTextStyle::setDefaultData()
{
    setColor(User, Nickname, Style::getColor(Style::MainText));
    setColor(User, Msg, Style::getColor(Style::MainText));
    setColor(Friend, Nickname, Style::getColor(Style::MainText));
    setColor(Friend, Msg, Style::getColor(Style::MainText));
}

void ChatTextStyle::setFonts(QFont font)
{
    *defFont = font;
    *busyFont = font;
    busyFont->setBold(true);
    busyFont->setPixelSize(busyFont->pixelSize() + 2);

    *fonts[key(User, Nickname)] = font;
    *fonts[key(User, Msg)] = font;
    *fonts[key(Friend, Nickname)] = font;
    *fonts[key(Friend, Msg)] = font;
}

void ChatTextStyle::setColor(const Owner owner, const Type type, const QColor& c)
{
    if (c.isValid()) {
        *colors[key(owner, type)] = c;
    } else {
        *colors[key(owner, type)] = Style::getColor(Style::MainText);
    }
}

void ChatTextStyle::setFontBold(const Owner owner, const Type type, const bool bold)
{
    fonts[key(owner, type)]->setBold(bold);
}

void ChatTextStyle::setFontItalic(const Owner owner, const Type type, const bool italic)
{
    fonts[key(owner, type)]->setItalic(italic);
}

void ChatTextStyle::setSettingsForGroup(bool isNickname, bool isMsg, bool isUser)
{
    if (isGroupNicknameColors != isNickname) {
        isGroupNicknameColors = isNickname;
        auto keys = groupsNicknameColors.keys();
        for (auto k : keys) {
            if (isGroupNicknameColors) {
                *groupsNicknameColors[k] = genColor(k);
            } else {
                *groupsNicknameColors[k] = Style::getColor(Style::MainText);
            }
        }
    }

    if (isGroupMsgColors != isMsg) {
        isGroupMsgColors = isMsg;
        auto keys = groupsMsgColors.keys();
        for (auto k : keys) {
            if (isGroupMsgColors) {
                *groupsMsgColors[k] = genColorForMsg(k);
            } else {
                *groupsMsgColors[k] = Style::getColor(Style::MainText);
            }
        }
    }

    if (isGroupColorsForUser != isUser) {
        isGroupColorsForUser = isUser;
        if (isGroupColorsForUser) {
            *nicknameUserColor = *colors[key(User, Nickname)];
            *msgUserColor = *colors[key(User, Msg)];
        } else {
            *nicknameUserColor = Style::getColor(Style::MainText);
            *msgUserColor = Style::getColor(Style::MainText);
        }
    }
}

std::shared_ptr<QColor> ChatTextStyle::getColor(const Owner owner, const Type type, const QString& sender, bool isGroup)
{
    if (isGroup) {
        if (owner == Friend) {
            if (type == Nickname) {
                return getGroupFriendNicknameColor(sender);
            } else {
                return getGroupFriendMsgColor(sender);
            }
        } else {
            if (type == Nickname) {
                return nicknameUserColor;
            } else {
                return msgUserColor;
            }
        }
    }

    return colors[key(owner, type)];
}

std::shared_ptr<QColor> ChatTextStyle::getActionColor() const
{
    return actionColor;
}

std::shared_ptr<QFont> ChatTextStyle::getFont(const Owner owner, const Type type) const
{
    return fonts[key(owner, type)];
}

std::shared_ptr<QFont> ChatTextStyle::getDefFont() const
{
    return defFont;
}

std::shared_ptr<QFont> ChatTextStyle::getBusyFont() const
{
    return busyFont;
}

std::shared_ptr<QColor> ChatTextStyle::getGroupFriendNicknameColor(const QString &key)
{
    if (!groupsNicknameColors.contains(key)) {
        if (isGroupNicknameColors) {
            groupsNicknameColors[key] = std::make_shared<QColor>(genColor(key));
        } else {
            groupsNicknameColors[key] = std::make_shared<QColor>(Style::getColor(Style::MainText));
        }
    }

    return groupsNicknameColors[key];
}

std::shared_ptr<QColor> ChatTextStyle::getGroupFriendMsgColor(const QString &key)
{
    if (!groupsMsgColors.contains(key)) {
        if (isGroupMsgColors) {
            groupsMsgColors[key] = std::make_shared<QColor>(genColorForMsg(key));
        } else {
            groupsMsgColors[key] = std::make_shared<QColor>(Style::getColor(Style::MainText));
        }
    }

    return groupsMsgColors[key];
}

QColor ChatTextStyle::genColor(const QString &key) const
{
    // Note: Eliding cannot be enabled for RichText items. (QTBUG-17207)
    QByteArray hash = QCryptographicHash::hash((key.toUtf8()), QCryptographicHash::Sha256);
    const auto* data = hash.data();
    return QColor(data[0], 255, 196);
}

QColor ChatTextStyle::genColorForMsg(const QString &key) const
{
    if (Style::Dark == Style::typeTheme()) {
        return genColor(key).lighter(130);
    }

    return genColor(key).darker(130);
}

int ChatTextStyle::key(const Owner owner, const Type type) const
{
    return owner + type;
}
