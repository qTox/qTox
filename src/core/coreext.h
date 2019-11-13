#pragma once

#include "src/model/status.h"
#include "icoreextpacket.h"

#include <QObject>
#include <QMap>

#include <bitset>
#include <memory>

struct Tox;
struct ToxExt;
struct ToxExtensionMessages;
struct ToxExtPacketList;

/**
 * Bridge between the toxext library and the rest of qTox.
 */
class CoreExt : public QObject, public ICoreExtPacketAllocator
{
    Q_OBJECT
private:
    // PassKey idiom to prevent others from making PacketBuilders
    struct PacketPassKey {};
public:

    /**
     * @brief Creates a CoreExt instance. Using a pointer here makes our
     *  registrations with extensions significantly easier to manage
     *
     * @param[in] pointer to core tox instance
     * @return CoreExt on success, nullptr on failure
     */
    static std::unique_ptr<CoreExt> makeCoreExt(Tox* core);

    // We do registration with our own pointer, need to ensure we're in a stable location
    CoreExt(CoreExt const& other) = delete;
    CoreExt(CoreExt&& other) = delete;
    CoreExt& operator=(CoreExt const& other) = delete;
    CoreExt& operator=(CoreExt&& other) = delete;

    /**
     * @brief Periodic service function
     */
    void process();

    /**
     * @brief Handles extension related lossless packets
     * @param[in] friendId Core id of friend
     * @param[in] data Packet data
     * @param[in] length Length of packet data
     */
    void onLosslessPacket(uint32_t friendId, const uint8_t* data, size_t length);

    /**
     * See documentatino of ICoreExtPacket
     */
    class Packet : public ICoreExtPacket
    {
    public:
        /**
         * @brief Internal constructor for a packet.
         */
        Packet(
            ToxExtPacketList* packetList,
            ToxExtensionMessages* toxExtMessages,
            PacketPassKey);

        // Delete copy constructor, we shouldn't be able to copy
        Packet(Packet const& other) = delete;

        Packet(Packet&& other)
        {
            toxExtMessages = other.toxExtMessages;
            packetList = other.packetList;
            other.toxExtMessages = nullptr;
            other.packetList = nullptr;
        }

        uint64_t addExtendedMessage(QString message) override;

        bool send() override;
    private:
        bool hasBeenSent = false;
        ToxExtensionMessages* toxExtMessages;
        ToxExtPacketList* packetList;
    };

    std::unique_ptr<ICoreExtPacket> getPacket(uint32_t friendId) override;

signals:
    void extendedMessageReceived(uint32_t friendId, const QString& message);
    void extendedReceiptReceived(uint32_t friendId, uint64_t receiptId);
    void extendedMessageSupport(uint32_t friendId, bool supported);

public slots:
    void onFriendStatusChanged(uint32_t friendId, Status::Status status);

private:

    static void onExtendedMessageReceived(uint32_t friendId, const uint8_t* data, size_t size, void* userData);
    static void onExtendedMessageReceipt(uint32_t friendId, uint64_t receiptId, void* userData);
    static void onExtendedMessageNegotiation(uint32_t friendId, bool compatible, void* userData);

    // A little extra cost to hide the deleters, but this lets us fwd declare
    // and prevent any extension headers from leaking out to the rest of the
    // system
    template <class T>
    using ExtensionPtr = std::unique_ptr<T, void(*)(T*)>;

    CoreExt(ExtensionPtr<ToxExt> toxExt);

    ExtensionPtr<ToxExt> toxExt;
    ExtensionPtr<ToxExtensionMessages> toxExtMessages;
};
