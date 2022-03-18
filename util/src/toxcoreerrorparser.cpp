/*
    Copyright © 2022 by The qTox Project Contributors

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

#include "util/toxcoreerrorparser.h"

#include <QDebug>

#include <tox/tox.h>

bool ToxcoreErrorParser::parseErr(Tox_Err_Conference_Title error, int line)
{
    switch (error) {
    case TOX_ERR_CONFERENCE_TITLE_OK:
        return true;

    case TOX_ERR_CONFERENCE_TITLE_CONFERENCE_NOT_FOUND:
        qCritical() << line << ": Conference title not found";
        return false;

    case TOX_ERR_CONFERENCE_TITLE_INVALID_LENGTH:
        qCritical() << line << ": Invalid conference title length";
        return false;

    case TOX_ERR_CONFERENCE_TITLE_FAIL_SEND:
        qCritical() << line << ": Failed to send title packet";
        return false;

    default:
        qCritical() << line << ": Unknown Tox_Err_Conference_Title error code:" << error;
        return false;
    }
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Friend_Send_Message error, int line)
{
    switch (error) {
    case TOX_ERR_FRIEND_SEND_MESSAGE_OK:
        return true;

    case TOX_ERR_FRIEND_SEND_MESSAGE_NULL:
        qCritical() << line << "Send friend message passed an unexpected null argument";
        return false;

    case TOX_ERR_FRIEND_SEND_MESSAGE_FRIEND_NOT_FOUND:
        qCritical() << line << "Send friend message could not find friend";
        return false;

    case TOX_ERR_FRIEND_SEND_MESSAGE_FRIEND_NOT_CONNECTED:
        qCritical() << line << "Send friend message: friend is offline";
        return false;

    case TOX_ERR_FRIEND_SEND_MESSAGE_SENDQ:
        qCritical() << line << "Failed to allocate more message queue";
        return false;

    case TOX_ERR_FRIEND_SEND_MESSAGE_TOO_LONG:
        qCritical() << line << "Attemped to send message that's too long";
        return false;

    case TOX_ERR_FRIEND_SEND_MESSAGE_EMPTY:
        qCritical() << line << "Attempted to send an empty message";
        return false;

    default:
        qCritical() << line << "Unknown friend send message error:" << static_cast<int>(error);
        return false;
    }
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Conference_Send_Message error, int line)
{
    switch (error) {
    case TOX_ERR_CONFERENCE_SEND_MESSAGE_OK:
        return true;

    case TOX_ERR_CONFERENCE_SEND_MESSAGE_CONFERENCE_NOT_FOUND:
        qCritical() << line << "Conference not found";
        return false;

    case TOX_ERR_CONFERENCE_SEND_MESSAGE_FAIL_SEND:
        qCritical() << line << "Conference message failed to send";
        return false;

    case TOX_ERR_CONFERENCE_SEND_MESSAGE_NO_CONNECTION:
        qCritical() << line << "No connection";
        return false;

    case TOX_ERR_CONFERENCE_SEND_MESSAGE_TOO_LONG:
        qCritical() << line << "Message too long";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Conference_Send_Message  error:" << static_cast<int>(error);
        return false;
    }
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Conference_Peer_Query error, int line)
{
    switch (error) {
    case TOX_ERR_CONFERENCE_PEER_QUERY_OK:
        return true;

    case TOX_ERR_CONFERENCE_PEER_QUERY_CONFERENCE_NOT_FOUND:
        qCritical() << line << "Conference not found";
        return false;

    case TOX_ERR_CONFERENCE_PEER_QUERY_NO_CONNECTION:
        qCritical() << line << "No connection";
        return false;

    case TOX_ERR_CONFERENCE_PEER_QUERY_PEER_NOT_FOUND:
        qCritical() << line << "Peer not found";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Conference_Peer_Query error code:" << error;
        return false;
    }
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Conference_Join error, int line)
{
    switch (error) {
    case TOX_ERR_CONFERENCE_JOIN_OK:
        return true;

    case TOX_ERR_CONFERENCE_JOIN_DUPLICATE:
        qCritical() << line << "Conference duplicate";
        return false;

    case TOX_ERR_CONFERENCE_JOIN_FAIL_SEND:
        qCritical() << line << "Conference join failed to send";
        return false;

    case TOX_ERR_CONFERENCE_JOIN_FRIEND_NOT_FOUND:
        qCritical() << line << "Friend not found";
        return false;

    case TOX_ERR_CONFERENCE_JOIN_INIT_FAIL:
        qCritical() << line << "Init fail";
        return false;

    case TOX_ERR_CONFERENCE_JOIN_INVALID_LENGTH:
        qCritical() << line << "Invalid length";
        return false;

    case TOX_ERR_CONFERENCE_JOIN_WRONG_TYPE:
        qCritical() << line << "Wrong conference type";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Conference_Join error code:" << error;
        return false;
    }
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Conference_Get_Type error, int line)
{
    switch (error) {
    case TOX_ERR_CONFERENCE_GET_TYPE_OK:
        return true;

    case TOX_ERR_CONFERENCE_GET_TYPE_CONFERENCE_NOT_FOUND:
        qCritical() << line << "Conference not found";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Conference_Get_Type error code:" << error;
        return false;
    }
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Conference_Invite error, int line)
{
    switch (error) {
    case TOX_ERR_CONFERENCE_INVITE_OK:
        return true;

    case TOX_ERR_CONFERENCE_INVITE_CONFERENCE_NOT_FOUND:
        qCritical() << line << "Conference not found";
        return false;

    case TOX_ERR_CONFERENCE_INVITE_FAIL_SEND:
        qCritical() << line << "Conference invite failed to send";
        return false;

    case TOX_ERR_CONFERENCE_INVITE_NO_CONNECTION:
        qCritical() << line << "Cannot invite to conference that we're not connected to";
        return false;

    default:
        qWarning() << "Unknown Tox_Err_Conference_Invite error code:" << error;
        return false;
    }
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Conference_New error, int line)
{
    switch (error) {
    case TOX_ERR_CONFERENCE_NEW_OK:
        return true;

    case TOX_ERR_CONFERENCE_NEW_INIT:
        qCritical() << line << "The conference instance failed to initialize";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Conference_New error code:" << error;
        return false;
    }
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Friend_By_Public_Key error, int line)
{
    switch (error) {
    case TOX_ERR_FRIEND_BY_PUBLIC_KEY_OK:
        return true;

    case TOX_ERR_FRIEND_BY_PUBLIC_KEY_NULL:
        qCritical() << line << "null argument when not expected";
        return false;

    case TOX_ERR_FRIEND_BY_PUBLIC_KEY_NOT_FOUND:
        // we use this as a check for friendship, so this can be an expected result
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Friend_By_Public_Key error code:" << error;
        return false;
    }
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Bootstrap error, int line)
{
    switch(error) {
    case TOX_ERR_BOOTSTRAP_OK:
        return true;

    case TOX_ERR_BOOTSTRAP_NULL:
        qCritical() << line << "null argument when not expected";
        return false;

    case TOX_ERR_BOOTSTRAP_BAD_HOST:
        qCritical() << line << "Could not resolve hostname, or invalid IP address";
        return false;

    case TOX_ERR_BOOTSTRAP_BAD_PORT:
        qCritical() << line << "out of range port";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_bootstrap error code:" << error;
        return false;
    }
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Friend_Add error, int line)
{
    switch(error) {
    case TOX_ERR_FRIEND_ADD_OK:
        return true;

    case TOX_ERR_FRIEND_ADD_NULL:
        qCritical() << line << "null argument when not expected";
        return false;

    case TOX_ERR_FRIEND_ADD_TOO_LONG:
        qCritical() << line << "The length of the friend request message exceeded";
        return false;

    case TOX_ERR_FRIEND_ADD_NO_MESSAGE:
        qCritical() << line << "The friend request message was empty.";
        return false;

    case TOX_ERR_FRIEND_ADD_OWN_KEY:
        qCritical() << line << "The friend address belongs to the sending client.";
        return false;

    case TOX_ERR_FRIEND_ADD_ALREADY_SENT:
        qCritical() << line << "The address belongs to a friend that is already on the friend list.";
        return false;

    case TOX_ERR_FRIEND_ADD_BAD_CHECKSUM:
        qCritical() << line << "The friend address checksum failed.";
        return false;

    case TOX_ERR_FRIEND_ADD_SET_NEW_NOSPAM:
        qCritical() << line << "The address belongs to a friend that is already on the friend list.";
        return false;

    case TOX_ERR_FRIEND_ADD_MALLOC:
        qCritical() << line << "A memory allocation failed when trying to increase the friend list size.";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Friend_Add error code:" << error;
        return false;

    }
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Friend_Delete error, int line)
{
    switch(error) {
    case TOX_ERR_FRIEND_DELETE_OK:
        return true;

    case TOX_ERR_FRIEND_DELETE_FRIEND_NOT_FOUND:
        qCritical() << line << "There is no friend with the given friend number";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Friend_Delete error code:" << error;
        return false;
    }
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Set_Info error, int line)
{
    switch (error) {
    case TOX_ERR_SET_INFO_OK:
        return true;

    case TOX_ERR_SET_INFO_NULL:
        qCritical() << line << "null argument when not expected";
        return false;

    case TOX_ERR_SET_INFO_TOO_LONG:
        qCritical() << line << "Information length exceeded maximum permissible size.";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Set_Info error code:" << error;
        return false;
    }
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Friend_Query error, int line)
{
    switch (error) {
    case TOX_ERR_FRIEND_QUERY_OK:
        return true;

    case TOX_ERR_FRIEND_QUERY_NULL:
        qCritical() << line << "null argument when not expected";
        return false;

    case TOX_ERR_FRIEND_QUERY_FRIEND_NOT_FOUND:
        qCritical() << line << "The friend_number did not designate a valid friend.";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Friend_Query error code:" << error;
        return false;
    }
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Friend_Get_Public_Key error, int line)
{
    switch (error) {
    case TOX_ERR_FRIEND_GET_PUBLIC_KEY_OK:
        return true;

    case TOX_ERR_FRIEND_GET_PUBLIC_KEY_FRIEND_NOT_FOUND:
        qCritical() << line << "There is no friend with the given friend number";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Friend_Get_Public_Key error code:" << error;
        return false;
    }
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Friend_Get_Last_Online error, int line)
{
    switch (error) {
    case TOX_ERR_FRIEND_GET_LAST_ONLINE_OK:
        return true;

    case TOX_ERR_FRIEND_GET_LAST_ONLINE_FRIEND_NOT_FOUND:
        qCritical() << line << "There is no friend with the given friend number";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Friend_Get_Last_Online error code:" << error;
        return false;
    }
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Set_Typing error, int line)
{
    switch (error) {
    case TOX_ERR_SET_TYPING_OK:
        return true;

    case TOX_ERR_SET_TYPING_FRIEND_NOT_FOUND:
        qCritical() << line << "There is no friend with the given friend number";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Set_Typing error code:" << error;
        return false;
    }
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Conference_Delete error, int line)
{
    switch (error) {
    case TOX_ERR_CONFERENCE_DELETE_OK:
        return true;

    case TOX_ERR_CONFERENCE_DELETE_CONFERENCE_NOT_FOUND:
        qCritical() << line << "The conference number passed did not designate a valid conference.";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Conference_Delete error code:" << error;
        return false;
    }
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Get_Port error, int line)
{
    switch (error) {
    case TOX_ERR_GET_PORT_OK:
        return true;

    case TOX_ERR_GET_PORT_NOT_BOUND:
        qCritical() << line << "Tox instance was not bound to any port.";
        return false;

    default:
        qCritical() << line << "Unknown Tox_Err_Get_Port error code:" << error;
        return false;
    }
}
