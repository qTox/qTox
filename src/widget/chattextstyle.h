#ifndef CHATTEXTSTYLE_H
#define CHATTEXTSTYLE_H


#include <QColor>
#include <QFont>
#include <QMap>

class ChatTextStyle {
public:
    enum Owner {
        User = 0,
        Friend = 1
    };

    enum Type {
        Nickname = 10,
        Msg = 20
    };

    ChatTextStyle();

    void setDefaultData();

    void setFonts(QFont font);
    void setColor(const Owner owner, const Type type, const QColor& c);
    void setFontBold(const Owner owner, const Type type, const bool bold);
    void setFontItalic(const Owner owner, const Type type, const bool italic);
    void setSettingsForGroup(bool isNickname, bool isMsg, bool isUser);

    std::shared_ptr<QColor> getColor(const Owner owner, const Type type, const QString& sender = "", bool isGroup = false);
    std::shared_ptr<QColor> getActionColor() const;
    std::shared_ptr<QFont> getFont(const Owner owner, const Type type) const;
    std::shared_ptr<QFont> getDefFont() const;
    std::shared_ptr<QFont> getBusyFont() const;

private:
    std::shared_ptr<QColor> getGroupFriendNicknameColor(const QString& key);
    std::shared_ptr<QColor> getGroupFriendMsgColor(const QString& key);
    QColor genColor(const QString& key) const;
    QColor genColorForMsg(const QString& key) const;
    int key(const Owner owner, const Type type) const;

    QMap<int, std::shared_ptr<QColor>> colors;
    QMap<int, std::shared_ptr<QFont>> fonts;
    QMap<QString, std::shared_ptr<QColor>> groupsNicknameColors;
    QMap<QString, std::shared_ptr<QColor>> groupsMsgColors;
    std::shared_ptr<QColor> actionColor;
    std::shared_ptr<QColor> nicknameUserColor;
    std::shared_ptr<QColor> msgUserColor;
    std::shared_ptr<QFont> defFont;
    std::shared_ptr<QFont> busyFont;

    bool isGroupNicknameColors{false};
    bool isGroupMsgColors{false};
    bool isGroupColorsForUser{false};
};

#endif // CHATTEXTSTYLE_H
