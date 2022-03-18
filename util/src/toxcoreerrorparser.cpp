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
    }
    qCritical() << line << ": Unknown Tox_Err_Conference_Title error code:" << error;
    return false;
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
    }
    qCritical() << line << "Unknown friend send message error:" << static_cast<int>(error);
    return false;
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
    }
    qCritical() << line << "Unknown Tox_Err_Conference_Send_Message  error:" << static_cast<int>(error);
    return false;
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
    }
    qCritical() << line << "Unknown Tox_Err_Conference_Peer_Query error code:" << error;
    return false;
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
    }
    qCritical() << line << "Unknown Tox_Err_Conference_Join error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Conference_Get_Type error, int line)
{
    switch (error) {
    case TOX_ERR_CONFERENCE_GET_TYPE_OK:
        return true;

    case TOX_ERR_CONFERENCE_GET_TYPE_CONFERENCE_NOT_FOUND:
        qCritical() << line << "Conference not found";
        return false;
    }
    qCritical() << line << "Unknown Tox_Err_Conference_Get_Type error code:" << error;
    return false;
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
    }
    qWarning() << "Unknown Tox_Err_Conference_Invite error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Conference_New error, int line)
{
    switch (error) {
    case TOX_ERR_CONFERENCE_NEW_OK:
        return true;

    case TOX_ERR_CONFERENCE_NEW_INIT:
        qCritical() << line << "The conference instance failed to initialize";
        return false;
    }
    qCritical() << line << "Unknown Tox_Err_Conference_New error code:" << error;
    return false;
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
    }
    qCritical() << line << "Unknown Tox_Err_Friend_By_Public_Key error code:" << error;
    return false;
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
    }
    qCritical() << line << "Unknown Tox_Err_bootstrap error code:" << error;
    return false;
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
    }
    qCritical() << line << "Unknown Tox_Err_Friend_Add error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Friend_Delete error, int line)
{
    switch(error) {
    case TOX_ERR_FRIEND_DELETE_OK:
        return true;

    case TOX_ERR_FRIEND_DELETE_FRIEND_NOT_FOUND:
        qCritical() << line << "There is no friend with the given friend number";
        return false;
    }
    qCritical() << line << "Unknown Tox_Err_Friend_Delete error code:" << error;
    return false;
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
    }
    qCritical() << line << "Unknown Tox_Err_Set_Info error code:" << error;
    return false;
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
    }
    qCritical() << line << "Unknown Tox_Err_Friend_Query error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Friend_Get_Public_Key error, int line)
{
    switch (error) {
    case TOX_ERR_FRIEND_GET_PUBLIC_KEY_OK:
        return true;

    case TOX_ERR_FRIEND_GET_PUBLIC_KEY_FRIEND_NOT_FOUND:
        qCritical() << line << "There is no friend with the given friend number";
        return false;
    }
    qCritical() << line << "Unknown Tox_Err_Friend_Get_Public_Key error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Friend_Get_Last_Online error, int line)
{
    switch (error) {
    case TOX_ERR_FRIEND_GET_LAST_ONLINE_OK:
        return true;

    case TOX_ERR_FRIEND_GET_LAST_ONLINE_FRIEND_NOT_FOUND:
        qCritical() << line << "There is no friend with the given friend number";
        return false;
    }
    qCritical() << line << "Unknown Tox_Err_Friend_Get_Last_Online error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Set_Typing error, int line)
{
    switch (error) {
    case TOX_ERR_SET_TYPING_OK:
        return true;

    case TOX_ERR_SET_TYPING_FRIEND_NOT_FOUND:
        qCritical() << line << "There is no friend with the given friend number";
        return false;
    }
    qCritical() << line << "Unknown Tox_Err_Set_Typing error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Conference_Delete error, int line)
{
    switch (error) {
    case TOX_ERR_CONFERENCE_DELETE_OK:
        return true;

    case TOX_ERR_CONFERENCE_DELETE_CONFERENCE_NOT_FOUND:
        qCritical() << line << "The conference number passed did not designate a valid conference.";
        return false;
    }
    qCritical() << line << "Unknown Tox_Err_Conference_Delete error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Get_Port error, int line)
{
    switch (error) {
    case TOX_ERR_GET_PORT_OK:
        return true;

    case TOX_ERR_GET_PORT_NOT_BOUND:
        qCritical() << line << "Tox instance was not bound to any port.";
        return false;
    }
    qCritical() << line << "Unknown Tox_Err_Get_Port error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_File_Control error, int line)
{
    switch (error) {
    case TOX_ERR_FILE_CONTROL_OK:
        return true;

    case TOX_ERR_FILE_CONTROL_FRIEND_NOT_FOUND:
        qCritical() << line << ": The friend_number passed did not designate a valid friend.";
        return false;

    case TOX_ERR_FILE_CONTROL_FRIEND_NOT_CONNECTED:
        qCritical() << line << ": This client is currently not connected to the friend.";
        return false;

    case TOX_ERR_FILE_CONTROL_NOT_FOUND:
         qCritical() << line << ": No file transfer with the given file number was found for the given friend.";
         return false;

    case TOX_ERR_FILE_CONTROL_NOT_PAUSED:
        qCritical() << line << ": A RESUME control was sent, but the file transfer is running normally.";
        return false;

    case TOX_ERR_FILE_CONTROL_DENIED:
        qCritical() << line << ": A RESUME control was sent, but the file transfer was paused by the other party.";
        return false;

    case TOX_ERR_FILE_CONTROL_ALREADY_PAUSED:
        qCritical() << line << ": A PAUSE control was sent, but the file transfer was already paused.";
        return false;

    case TOX_ERR_FILE_CONTROL_SENDQ:
        qCritical() << line << ": Packet queue is full.";
    }
    qCritical() << line << "Unknown Tox_Err_File_Control error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_File_Get error, int line)
{
    switch (error) {
    case TOX_ERR_FILE_GET_OK:
        return true;

    case TOX_ERR_FILE_GET_NULL:
        qCritical() << line << ": One of the arguments to the function was NULL when it was not expected.";
        return false;

    case TOX_ERR_FILE_GET_FRIEND_NOT_FOUND:
        qCritical() << line << ": The friend_number passed did not designate a valid friend.";
        return false;

    case TOX_ERR_FILE_GET_NOT_FOUND:
        qCritical() << line << ": The friend_number passed did not designate a valid friend.";
        return false;
    }
    qCritical() << line << ": Unknown Tox_Err_Conference_Title error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_File_Send error, int line)
{
    switch (error) {
    case TOX_ERR_FILE_SEND_OK:
        return true;

    case TOX_ERR_FILE_SEND_NULL:
        qCritical() << line << ": One of the arguments to the function was NULL when it was not expected.";
        return false;

    case TOX_ERR_FILE_SEND_FRIEND_NOT_FOUND:
        qCritical() << line << ": The friend_number passed did not designate a valid friend.";
        return false;

    case TOX_ERR_FILE_SEND_FRIEND_NOT_CONNECTED:
        qCritical() << line << ": This client is currently not connected to the friend.";
        return false;

    case TOX_ERR_FILE_SEND_NAME_TOO_LONG:
        qCritical() << line << ": Filename length exceeded TOX_MAX_FILENAME_LENGTH bytes.";
        return false;

    case TOX_ERR_FILE_SEND_TOO_MANY:
        qCritical() << line << ": Too many ongoing transfers.";
        return false;
    }
    qCritical() << line << "Unknown Tox_Err_File_Send error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_File_Send_Chunk error, int line)
{
    switch (error) {
    case TOX_ERR_FILE_SEND_CHUNK_OK:
        return true;

    case TOX_ERR_FILE_SEND_CHUNK_NULL:
        qCritical() << line << ": The length parameter was non-zero, but data was NULL.";
        return false;

    case TOX_ERR_FILE_SEND_CHUNK_FRIEND_NOT_FOUND:
        qCritical() << line << ": The friend_number passed did not designate a valid friend.";
        return false;

    case TOX_ERR_FILE_SEND_CHUNK_FRIEND_NOT_CONNECTED:
        qCritical() << line << ": This client is currently not connected to the friend.";
        return false;

    case TOX_ERR_FILE_SEND_CHUNK_NOT_FOUND:
        qCritical() << line << ": No file transfer with the given file number was found for the given friend.";
        return false;

    case TOX_ERR_FILE_SEND_CHUNK_NOT_TRANSFERRING:
        qCritical() << line << ": File transfer was found but isn't in a transferring state.";
        return false;

    case TOX_ERR_FILE_SEND_CHUNK_INVALID_LENGTH:
        qCritical() << line << ": Attempted to send more or less data than requested.";
        return false;

    case TOX_ERR_FILE_SEND_CHUNK_SENDQ:
        qCritical() << line << ": Packet queue is full.";
        return false;

    case TOX_ERR_FILE_SEND_CHUNK_WRONG_POSITION:
        qCritical() << line << ": Position parameter was wrong.";
        return false;
    }
    qCritical() << line << "Unknown Tox_Err_File_Send_Chunk error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Toxav_Err_Bit_Rate_Set error, int line)
{
    switch (error) {
    case TOXAV_ERR_BIT_RATE_SET_OK:
        return true;

    case TOXAV_ERR_BIT_RATE_SET_SYNC:
        qCritical() << line << ": Synchronization error occurred.";
        return false;

    case TOXAV_ERR_BIT_RATE_SET_INVALID_BIT_RATE:
        qCritical() << line << ": The bit rate passed was not one of the supported values.";
        return false;

    case TOXAV_ERR_BIT_RATE_SET_FRIEND_NOT_FOUND:
        qCritical() << line << ": The friend_number passed did not designate a valid friend.";
        return false;

    case TOXAV_ERR_BIT_RATE_SET_FRIEND_NOT_IN_CALL:
        qCritical() << line << ": This client is currently not in a call with the friend.";
        return false;
    }
    qCritical() << line << "Unknown Toxav_Err_Bit_Rate_Set error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Toxav_Err_Call_Control error, int line)
{
    switch (error) {
    case TOXAV_ERR_CALL_CONTROL_OK:
        return true;

    case TOXAV_ERR_CALL_CONTROL_SYNC:
        qCritical() << line << ": Synchronization error occurred.";
        return false;

    case TOXAV_ERR_CALL_CONTROL_FRIEND_NOT_FOUND:
        qCritical() << line << ": The friend_number passed did not designate a valid friend.";
        return false;

    case TOXAV_ERR_CALL_CONTROL_FRIEND_NOT_IN_CALL:
        qCritical() << line << ": This client is currently not in a call with the friend.";
        return false;

    case TOXAV_ERR_CALL_CONTROL_INVALID_TRANSITION:
        qCritical() << line << ": Call already paused or resumed.";
        return false;
    }
    qCritical() << line << "Unknown Toxav_Err_Call_Control error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Toxav_Err_Call error, int line)
{
    switch (error) {
    case TOXAV_ERR_CALL_OK:
        return true;

    case TOXAV_ERR_CALL_MALLOC:
        qCritical() << line << ": A resource allocation error occurred.";
        return false;

    case TOXAV_ERR_CALL_SYNC:
        qCritical() << line << ": Synchronization error occurred.";
        return false;

    case TOXAV_ERR_CALL_FRIEND_NOT_FOUND:
        qCritical() << line << ": The friend number did not designate a valid friend.";
        return false;

    case TOXAV_ERR_CALL_FRIEND_NOT_CONNECTED:
        qCritical() << line << ": The friend was valid, but not currently connected.";
        return false;

    case TOXAV_ERR_CALL_FRIEND_ALREADY_IN_CALL:
        qCritical() << line << ": Attempted to call a friend while already in a call with them.";
        return false;

    case TOXAV_ERR_CALL_INVALID_BIT_RATE:
        qCritical() << line << ": Audio or video bit rate is invalid.";
        return false;
    }
    qCritical() << line << "Unknown Toxav_Err_Call error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Options_New error, int line)
{
    switch (error) {
    case TOX_ERR_OPTIONS_NEW_OK:
        return true;

    case TOX_ERR_OPTIONS_NEW_MALLOC:
        qCritical() << line << ": Failed to allocate memory.";
        return false;
    }
    qCritical() << line << "Unknown Tox_Err_Options_New error code:" << error;
    return false;
}
