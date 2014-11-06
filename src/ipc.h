#ifndef IPC_H
#define IPC_H

#include <QSharedMemory>
#include <QTimer>
#include <QObject>
#include <QThread>
#include <QVector>
#include <functional>
#include <ctime>

/// Handles an IPC event, must filter out and ignore events it doesn't recognize
using IPCEventHandler = std::function<void (const QByteArray&)>;

/// Class used for inter-process communication with other qTox instances
/// IPC event handlers will be called from the GUI thread after its event loop starts
class IPC : public QThread
{
    Q_OBJECT

public:
    IPC();
    ~IPC();
    /// Posts an event to the global shared memory, returns the time at wich it was posted
    time_t postEvent(const QByteArray& data);
    bool isCurrentOwner(); ///< Returns whether we're responsible for event processing of the global shared memory
    void registerEventHandler(IPCEventHandler handler); ///< Registers a function to be called whenever an event is received
    bool isEventProcessed(time_t postTime); ///< Returns wether a previously posted event was already processed
    void waitUntilProcessed(time_t postTime); ///< Blocks until a previously posted event is processed

protected slots:
    void processEvents();

private:
    /// Runs an IPC event handler from the main (GUI) thread, will block until the handler returns
    Q_INVOKABLE void runEventHandler(IPCEventHandler handler, const QByteArray& arg);
    /// Assumes that the memory IS LOCKED
    /// Returns a pointer to the first free chunk of shared memory or a nullptr on error
    char* getFirstFreeChunk();
    /// Assumes that the memory IS LOCKED
    /// Returns a pointer to the last used chunk of shared memory or a nullptr on error
    char* getLastUsedChunk();
    /// Assumes that the memory IS LOCKED
    /// Removes the last event from the shared memory and returns its data, or an empty object on error
    QByteArray fetchEvent();
    /// Assumes that the memory IS LOCKED
    /// Updates the global shared timestamp
    void updateGlobalTimestamp();
    /// Assumes that the memory IS LOCKED
    /// Returns the global shared timestamp
    time_t getGlobalTimestamp();

private:
    QSharedMemory globalMemory;
    uint64_t globalId;
    QTimer ownerTimer;
    QVector<IPCEventHandler> eventHandlers;
    time_t lastSeenTimestamp;

    // Constants
    static const int MEMORY_SIZE = 4096;
    static const int MEMORY_HEADER_SIZE = sizeof(globalId) + sizeof(time_t);
    static const int EVENT_TIMER_MS = 1000;
    static const int OWNERSHIP_TIMEOUT_S = 5;
};

#endif // IPC_H
