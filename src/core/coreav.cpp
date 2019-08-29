/*
    Copyright © 2013 by Maxim Biro <nurupo.contributions@gmail.com>
    Copyright © 2014-2019 by The qTox Project Contributors

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

#include "coreav.h"
#include "core.h"
#include "src/model/friend.h"
#include "src/model/group.h"
#include "src/persistence/settings.h"
#include "src/video/corevideosource.h"
#include "src/video/videoframe.h"
#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QtConcurrent/QtConcurrentRun>
#include <cassert>

/**
 * @fn void CoreAV::avInvite(uint32_t friendId, bool video)
 * @brief Sent when a friend calls us.
 * @param friendId Id of friend in call list.
 * @param video False if chat is audio only, true audio and video.
 *
 * @fn void CoreAV::avStart(uint32_t friendId, bool video)
 * @brief Sent when a call we initiated has started.
 * @param friendId Id of friend in call list.
 * @param video False if chat is audio only, true audio and video.
 *
 * @fn void CoreAV::avEnd(uint32_t friendId)
 * @brief Sent when a call was ended by the peer.
 * @param friendId Id of friend in call list.
 *
 * @var CoreAV::VIDEO_DEFAULT_BITRATE
 * @brief Picked at random by fair dice roll.
 */

/**
 * @var std::atomic_flag CoreAV::threadSwitchLock
 * @brief This flag is to be acquired before switching in a blocking way between the UI and CoreAV
 * thread.
 *
 * The CoreAV thread must have priority for the flag, other threads should back off or release it
 * quickly.
 * CoreAV needs to interface with three threads, the toxcore/Core thread that fires non-payload
 * toxav callbacks, the toxav/CoreAV thread that fires AV payload callbacks and manages
 * most of CoreAV's members, and the UI thread, which calls our [start/answer/cancel]Call functions
 * and which we call via signals.
 * When the UI calls us, we switch from the UI thread to the CoreAV thread to do the processing,
 * when toxcore fires a non-payload av callback, we do the processing in the CoreAV thread and then
 * switch to the UI thread to send it a signal. Both switches block both threads, so this would
 * deadlock.
 */

/**
 * @brief Maps friend IDs to ToxFriendCall.
 * @note Need to use STL container here, because Qt containers need a copy constructor.
 */
std::map<uint32_t, CoreAV::ToxFriendCallPtr> CoreAV::calls;

/**
 * @brief Maps group IDs to ToxGroupCalls.
 * @note Need to use STL container here, because Qt containers need a copy constructor.
 */
std::map<int, CoreAV::ToxGroupCallPtr> CoreAV::groupCalls;

CoreAV::CoreAV(std::unique_ptr<ToxAV, ToxAVDeleter> toxav)
    : audio{nullptr}
    , toxav{std::move(toxav)}
    , coreavThread{new QThread{this}}
    , iterateTimer{new QTimer{this}}
    , threadSwitchLock{false}
{
    assert(coreavThread);
    assert(iterateTimer);

    coreavThread->setObjectName("qTox CoreAV");
    moveToThread(coreavThread.get());

    connectCallbacks(*this->toxav);

    iterateTimer->setSingleShot(true);

    connect(iterateTimer, &QTimer::timeout, this, &CoreAV::process);
    connect(coreavThread.get(), &QThread::finished, iterateTimer, &QTimer::stop);

    coreavThread->start();
}

void CoreAV::connectCallbacks(ToxAV& toxav)
{
    toxav_callback_call(&toxav, CoreAV::callCallback, this);
    toxav_callback_call_state(&toxav, CoreAV::stateCallback, this);
    toxav_callback_audio_bit_rate(&toxav, CoreAV::audioBitrateCallback, this);
    toxav_callback_video_bit_rate(&toxav, CoreAV::videoBitrateCallback, this);
    toxav_callback_audio_receive_frame(&toxav, CoreAV::audioFrameCallback, this);
    toxav_callback_video_receive_frame(&toxav, CoreAV::videoFrameCallback, this);
}

/**
 * @brief Factory method for CoreAV
 * @param core pointer to the Tox instance
 * @return CoreAV instance on success, {} on failure
 */
CoreAV::CoreAVPtr CoreAV::makeCoreAV(Tox* core)
{
    TOXAV_ERR_NEW err;
    std::unique_ptr<ToxAV, ToxAVDeleter> toxav{toxav_new(core, &err)};
    switch (err) {
    case TOXAV_ERR_NEW_OK:
        break;
    case TOXAV_ERR_NEW_MALLOC:
        qCritical() << "Failed to allocate ressources for ToxAV";
        return {};
    case TOXAV_ERR_NEW_MULTIPLE:
        qCritical() << "Attempted to create multiple ToxAV instances";
        return {};
    case TOXAV_ERR_NEW_NULL:
        qCritical() << "Unexpected NULL parameter";
        return {};
    }

    assert(toxav != nullptr);

    return CoreAVPtr{new CoreAV{std::move(toxav)}};
}

/**
 * @brief Set the audio backend
 * @param audio The audio backend to use
 * @note This must be called before starting CoreAV and audio must outlive CoreAV
 */
void CoreAV::setAudio(IAudioControl& newAudio)
{
    audio.exchange(&newAudio);
}

/**
 * @brief Get the audio backend used
 * @return Pointer to the audio backend
 * @note This is needed only for the case CoreAV needs to restart and the restarting class doesn't
 * have access to the audio backend and wants to keep it the same.
 */
IAudioControl* CoreAV::getAudio()
{
    return audio;
}

CoreAV::~CoreAV()
{
    for (const auto& call : calls) {
        cancelCall(call.first);
    }

    coreavThread->exit(0);
    coreavThread->wait();
}

/**
 * @brief Starts the CoreAV main loop that calls toxav's main loop
 */
void CoreAV::start()
{
    // Timers can only be touched from their own thread
    if (QThread::currentThread() != coreavThread.get())
        return (void)QMetaObject::invokeMethod(this, "start", Qt::BlockingQueuedConnection);
    iterateTimer->start();
}

void CoreAV::process()
{
    toxav_iterate(toxav.get());
    iterateTimer->start(toxav_iteration_interval(toxav.get()));
}

/**
 * @brief Checks the call status for a Tox friend.
 * @param f the friend to check
 * @return true, if call is started for the friend, false otherwise
 */
bool CoreAV::isCallStarted(const Friend* f) const
{
    return f && (calls.find(f->getId()) != calls.end());
}

/**
 * @brief Checks the call status for a Tox group.
 * @param g the group to check
 * @return true, if call is started for the group, false otherwise
 */
bool CoreAV::isCallStarted(const Group* g) const
{
    return g && (groupCalls.find(g->getId()) != groupCalls.end());
}

/**
 * @brief Checks the call status for a Tox friend.
 * @param f the friend to check
 * @return true, if call is active for the friend, false otherwise
 */
bool CoreAV::isCallActive(const Friend* f) const
{
    auto it = calls.find(f->getId());
    if (it == calls.end()) {
        return false;
    }
    return isCallStarted(f) && it->second->isActive();
}

/**
 * @brief Checks the call status for a Tox group.
 * @param g the group to check
 * @return true, if the call is active for the group, false otherwise
 */
bool CoreAV::isCallActive(const Group* g) const
{
    auto it = groupCalls.find(g->getId());
    if (it == groupCalls.end()) {
        return false;
    }
    return isCallStarted(g) && it->second->isActive();
}

bool CoreAV::isCallVideoEnabled(const Friend* f) const
{
    auto it = calls.find(f->getId());
    return isCallStarted(f) && it->second->getVideoEnabled();
}

bool CoreAV::answerCall(uint32_t friendNum, bool video)
{
    if (QThread::currentThread() != coreavThread.get()) {
        if (threadSwitchLock.test_and_set(std::memory_order_acquire)) {
            qDebug() << "CoreAV::answerCall: Backed off of thread-switch lock";
            return false;
        }

        bool ret;
        QMetaObject::invokeMethod(this, "answerCall", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(bool, ret), Q_ARG(uint32_t, friendNum),
                                  Q_ARG(bool, video));

        threadSwitchLock.clear(std::memory_order_release);
        return ret;
    }

    qDebug() << QString("answering call %1").arg(friendNum);
    auto it = calls.find(friendNum);
    assert(it != calls.end());
    TOXAV_ERR_ANSWER err;

    const uint32_t videoBitrate = video ? VIDEO_DEFAULT_BITRATE : 0;
    if (toxav_answer(toxav.get(), friendNum, Settings::getInstance().getAudioBitrate(),
                     videoBitrate, &err)) {
        it->second->setActive(true);
        return true;
    } else {
        qWarning() << "Failed to answer call with error" << err;
        toxav_call_control(toxav.get(), friendNum, TOXAV_CALL_CONTROL_CANCEL, nullptr);
        calls.erase(it);
        return false;
    }
}

bool CoreAV::startCall(uint32_t friendNum, bool video)
{
    if (QThread::currentThread() != coreavThread.get()) {
        if (threadSwitchLock.test_and_set(std::memory_order_acquire)) {
            qDebug() << "CoreAV::startCall: Backed off of thread-switch lock";
            return false;
        }

        bool ret;
        QMetaObject::invokeMethod(this, "startCall", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(bool, ret), Q_ARG(uint32_t, friendNum),
                                  Q_ARG(bool, video));

        threadSwitchLock.clear(std::memory_order_release);
        return ret;
    }

    qDebug() << QString("Starting call with %1").arg(friendNum);
    auto it = calls.find(friendNum);
    if (it != calls.end()) {
        qWarning() << QString("Can't start call with %1, we're already in this call!").arg(friendNum);
        return false;
    }

    uint32_t videoBitrate = video ? VIDEO_DEFAULT_BITRATE : 0;
    if (!toxav_call(toxav.get(), friendNum, Settings::getInstance().getAudioBitrate(), videoBitrate,
                    nullptr))
        return false;

    // Audio backend must be set before making a call
    assert(audio != nullptr);
    ToxFriendCallPtr call = ToxFriendCallPtr(new ToxFriendCall(friendNum, video, *this, *audio));
    assert(call != nullptr);
    auto ret = calls.emplace(friendNum, std::move(call));
    ret.first->second->startTimeout(friendNum);
    return true;
}

bool CoreAV::cancelCall(uint32_t friendNum)
{
    if (QThread::currentThread() != coreavThread.get()) {
        if (threadSwitchLock.test_and_set(std::memory_order_acquire)) {
            qDebug() << "CoreAV::cancelCall: Backed off of thread-switch lock";
            return false;
        }

        bool ret;
        QMetaObject::invokeMethod(this, "cancelCall", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(bool, ret), Q_ARG(uint32_t, friendNum));

        threadSwitchLock.clear(std::memory_order_release);
        return ret;
    }

    qDebug() << QString("Cancelling call with %1").arg(friendNum);
    if (!toxav_call_control(toxav.get(), friendNum, TOXAV_CALL_CONTROL_CANCEL, nullptr)) {
        qWarning() << QString("Failed to cancel call with %1").arg(friendNum);
        return false;
    }

    calls.erase(friendNum);
    emit avEnd(friendNum);
    return true;
}

void CoreAV::timeoutCall(uint32_t friendNum)
{
    // Non-blocking switch to the CoreAV thread, we really don't want to be coming
    // blocking-queued from the UI thread while we emit blocking-queued to it
    if (QThread::currentThread() != coreavThread.get()) {
        QMetaObject::invokeMethod(this, "timeoutCall", Qt::QueuedConnection,
                                  Q_ARG(uint32_t, friendNum));
        return;
    }

    if (!cancelCall(friendNum)) {
        qWarning() << QString("Failed to timeout call with %1").arg(friendNum);
        return;
    }
    qDebug() << "Call with friend" << friendNum << "timed out";
}

/**
 * @brief Send audio frame to a friend
 * @param callId Id of friend in call list.
 * @param pcm An array of audio samples (Pulse-code modulation).
 * @param samples Number of samples in this frame.
 * @param chans Number of audio channels.
 * @param rate Audio sampling rate used in this frame.
 * @return False only on error, but not if there's nothing to send.
 */
bool CoreAV::sendCallAudio(uint32_t callId, const int16_t* pcm, size_t samples, uint8_t chans,
                           uint32_t rate) const
{
    auto it = calls.find(callId);
    if (it == calls.end()) {
        return false;
    }

    ToxFriendCall const& call = *it->second;

    if (call.getMuteMic() || !call.isActive()
        || !(call.getState() & TOXAV_FRIEND_CALL_STATE_ACCEPTING_A)) {
        return true;
    }

    // TOXAV_ERR_SEND_FRAME_SYNC means toxav failed to lock, retry 5 times in this case
    TOXAV_ERR_SEND_FRAME err;
    int retries = 0;
    do {
        if (!toxav_audio_send_frame(toxav.get(), callId, pcm, samples, chans, rate, &err)) {
            if (err == TOXAV_ERR_SEND_FRAME_SYNC) {
                ++retries;
                QThread::usleep(500);
            } else {
                qDebug() << "toxav_audio_send_frame error: " << err;
            }
        }
    } while (err == TOXAV_ERR_SEND_FRAME_SYNC && retries < 5);
    if (err == TOXAV_ERR_SEND_FRAME_SYNC) {
        qDebug() << "toxav_audio_send_frame error: Lock busy, dropping frame";
    }

    return true;
}

void CoreAV::sendCallVideo(uint32_t callId, std::shared_ptr<VideoFrame> vframe)
{
    // We might be running in the FFmpeg thread and holding the CameraSource lock
    // So be careful not to deadlock with anything while toxav locks in toxav_video_send_frame
    auto it = calls.find(callId);
    if (it == calls.end()) {
        return;
    }

    ToxFriendCall& call = *it->second;

    if (!call.getVideoEnabled() || !call.isActive()
        || !(call.getState() & TOXAV_FRIEND_CALL_STATE_ACCEPTING_V)) {
        return;
    }

    if (call.getNullVideoBitrate()) {
        qDebug() << "Restarting video stream to friend" << callId;
        toxav_video_set_bit_rate(toxav.get(), callId, VIDEO_DEFAULT_BITRATE, nullptr);
        call.setNullVideoBitrate(false);
    }

    ToxYUVFrame frame = vframe->toToxYUVFrame();

    if (!frame) {
        return;
    }

    // TOXAV_ERR_SEND_FRAME_SYNC means toxav failed to lock, retry 5 times in this case
    // We don't want to be dropping iframes because of some lock held by toxav_iterate
    TOXAV_ERR_SEND_FRAME err;
    int retries = 0;
    do {
        if (!toxav_video_send_frame(toxav.get(), callId, frame.width, frame.height, frame.y,
                                    frame.u, frame.v, &err)) {
            if (err == TOXAV_ERR_SEND_FRAME_SYNC) {
                ++retries;
                QThread::usleep(500);
            } else {
                qDebug() << "toxav_video_send_frame error: " << err;
            }
        }
    } while (err == TOXAV_ERR_SEND_FRAME_SYNC && retries < 5);
    if (err == TOXAV_ERR_SEND_FRAME_SYNC) {
        qDebug() << "toxav_video_send_frame error: Lock busy, dropping frame";
    }
}

/**
 * @brief Toggles the mute state of the call's input (microphone).
 * @param f The friend assigned to the call
 */
void CoreAV::toggleMuteCallInput(const Friend* f)
{
    auto it = calls.find(f->getId());
    if (f && (it != calls.end())) {
        ToxCall& call = *it->second;
        call.setMuteMic(!call.getMuteMic());
    }
}

/**
 * @brief Toggles the mute state of the call's output (speaker).
 * @param f The friend assigned to the call
 */
void CoreAV::toggleMuteCallOutput(const Friend* f)
{
    auto it = calls.find(f->getId());
    if (f && (it != calls.end())) {
        ToxCall& call = *it->second;
        call.setMuteVol(!call.getMuteVol());
    }
}

/**
 * @brief Called from Tox API when group call receives audio data.
 *
 * @param[in] tox          the Tox object
 * @param[in] group        the group number
 * @param[in] peer         the peer number
 * @param[in] data         the audio data to playback
 * @param[in] samples      the audio samples
 * @param[in] channels     the audio channels
 * @param[in] sample_rate  the audio sample rate
 * @param[in] core         the qTox Core class
 */
void CoreAV::groupCallCallback(void* tox, uint32_t group, uint32_t peer, const int16_t* data,
                               unsigned samples, uint8_t channels, uint32_t sample_rate, void* core)
{
    Q_UNUSED(tox);

    Core* c = static_cast<Core*>(core);
    const ToxPk peerPk = c->getGroupPeerPk(group, peer);
    const Settings& s = Settings::getInstance();
    // don't play the audio if it comes from a muted peer
    if (s.getBlackList().contains(peerPk.toString())) {
        return;
    }

    emit c->groupPeerAudioPlaying(group, peerPk);

    CoreAV* cav = c->getAv();

    auto it = cav->groupCalls.find(group);
    if (it == cav->groupCalls.end()) {
        return;
    }

    ToxGroupCall& call = *it->second;

    if (call.getMuteVol() || !call.isActive()) {
        return;
    }

    call.playAudioBuffer(peerPk, data, samples, channels, sample_rate);
}

/**
 * @brief Called from core to make sure the source for that peer is invalidated when they leave.
 * @param group Group Index
 * @param peer Peer Index
 */
void CoreAV::invalidateGroupCallPeerSource(int group, ToxPk peerPk)
{
    auto it = groupCalls.find(group);
    if (it == groupCalls.end()) {
        return;
    }
    it->second->removePeer(peerPk);
}

/**
 * @brief Get a call's video source.
 * @param friendNum Id of friend in call list.
 * @return Video surface to show
 */
VideoSource* CoreAV::getVideoSourceFromCall(int friendNum) const
{
    auto it = calls.find(friendNum);
    if (it == calls.end()) {
        qWarning() << "CoreAV::getVideoSourceFromCall: No such call, did it die before we finished "
                      "answering?";
        return nullptr;
    }

    return it->second->getVideoSource();
}

/**
 * @brief Starts a call in an existing AV groupchat.
 * @note Call from the GUI thread.
 * @param groupId Id of group to join
 */
void CoreAV::joinGroupCall(int groupId)
{
    qDebug() << QString("Joining group call %1").arg(groupId);

    // Audio backend must be set before starting a call
    assert(audio != nullptr);
    ToxGroupCallPtr groupcall = ToxGroupCallPtr(new ToxGroupCall{groupId, *this, *audio});
    assert(groupcall != nullptr);
    auto ret = groupCalls.emplace(groupId, std::move(groupcall));
    if (ret.second == false) {
        qWarning() << "This group call already exists, not joining!";
        return;
    }
    ret.first->second->setActive(true);
}

/**
 * @brief Will not leave the group, just stop the call.
 * @note Call from the GUI thread.
 * @param groupId Id of group to leave
 */
void CoreAV::leaveGroupCall(int groupId)
{
    qDebug() << QString("Leaving group call %1").arg(groupId);

    groupCalls.erase(groupId);
}

bool CoreAV::sendGroupCallAudio(int groupId, const int16_t* pcm, size_t samples, uint8_t chans,
                                uint32_t rate) const
{
    std::map<int, ToxGroupCallPtr>::const_iterator it = groupCalls.find(groupId);
    if (it == groupCalls.end()) {
        return false;
    }

    if (!it->second->isActive() || it->second->getMuteMic()) {
        return true;
    }

    if (toxav_group_send_audio(toxav_get_tox(toxav.get()), groupId, pcm, samples, chans, rate) != 0)
        qDebug() << "toxav_group_send_audio error";

    return true;
}

/**
 * @brief Mutes or unmutes the group call's input (microphone).
 * @param g The group
 * @param mute True to mute, false to unmute
 */
void CoreAV::muteCallInput(const Group* g, bool mute)
{
    auto it = groupCalls.find(g->getId());
    if (g && (it != groupCalls.end())) {
        it->second->setMuteMic(mute);
    }
}

/**
 * @brief Mutes or unmutes the group call's output (speaker).
 * @param g The group
 * @param mute True to mute, false to unmute
 */
void CoreAV::muteCallOutput(const Group* g, bool mute)
{
    auto it = groupCalls.find(g->getId());
    if (g && (it != groupCalls.end())) {
        it->second->setMuteVol(mute);
    }
}

/**
 * @brief Returns the group calls input (microphone) state.
 * @param groupId The group id to check
 * @return true when muted, false otherwise
 */
bool CoreAV::isGroupCallInputMuted(const Group* g) const
{
    if (!g) {
        return false;
    }

    const uint32_t groupId = g->getId();
    auto it = groupCalls.find(groupId);
    return (it != groupCalls.end()) && it->second->getMuteMic();
}

/**
 * @brief Returns the group calls output (speaker) state.
 * @param groupId The group id to check
 * @return true when muted, false otherwise
 */
bool CoreAV::isGroupCallOutputMuted(const Group* g) const
{
    if (!g) {
        return false;
    }

    const uint32_t groupId = g->getId();
    auto it = groupCalls.find(groupId);
    return (it != groupCalls.end()) && it->second->getMuteVol();
}

/**
 * @brief Returns the calls input (microphone) mute state.
 * @param f The friend to check
 * @return true when muted, false otherwise
 */
bool CoreAV::isCallInputMuted(const Friend* f) const
{
    if (!f) {
        return false;
    }
    const uint32_t friendId = f->getId();
    auto it = calls.find(friendId);
    return (it != calls.end()) && it->second->getMuteMic();
}

/**
 * @brief Returns the calls output (speaker) mute state.
 * @param friendId The friend to check
 * @return true when muted, false otherwise
 */
bool CoreAV::isCallOutputMuted(const Friend* f) const
{
    if (!f) {
        return false;
    }
    const uint32_t friendId = f->getId();
    auto it = calls.find(friendId);
    return (it != calls.end()) && it->second->getMuteVol();
}

/**
 * @brief Signal to all peers that we're not sending video anymore.
 * @note The next frame sent cancels this.
 */
void CoreAV::sendNoVideo()
{
    // We don't change the audio bitrate, but we signal that we're not sending video anymore
    qDebug() << "CoreAV: Signaling end of video sending";
    for (auto& kv : calls) {
        ToxFriendCall& call = *kv.second;
        toxav_video_set_bit_rate(toxav.get(), kv.first, 0, nullptr);
        call.setNullVideoBitrate(true);
    }
}

void CoreAV::callCallback(ToxAV* toxav, uint32_t friendNum, bool audio, bool video, void* vSelf)
{
    CoreAV* self = static_cast<CoreAV*>(vSelf);

    // Run this slow callback asynchronously on the AV thread to avoid deadlocks with what our
    // caller (toxcore) holds
    // Also run the code to switch to the CoreAV thread in yet another thread, in case CoreAV
    // has threadSwitchLock and wants a toxcore lock that our call stack is holding...
    if (QThread::currentThread() != self->coreavThread.get()) {
        QtConcurrent::run([=]() {
            // We assume the original caller doesn't come from the CoreAV thread here
            while (self->threadSwitchLock.test_and_set(std::memory_order_acquire))
                QThread::yieldCurrentThread(); // Shouldn't spin for long, we have priority

            QMetaObject::invokeMethod(self, "callCallback", Qt::QueuedConnection,
                                      Q_ARG(ToxAV*, toxav), Q_ARG(uint32_t, friendNum),
                                      Q_ARG(bool, audio), Q_ARG(bool, video), Q_ARG(void*, vSelf));
        });
        return;
    }

    // Audio backend must be set before receiving a call
    assert(self->audio != nullptr);
    ToxFriendCallPtr call = ToxFriendCallPtr(new ToxFriendCall{friendNum, video, *self, *self->audio});
    assert(call != nullptr);

    auto it = self->calls.emplace(friendNum, std::move(call));
    if (it.second == false) {
        /// Hanging up from a callback is supposed to be UB,
        /// but since currently the toxav callbacks are fired from the toxcore thread,
        /// we'll always reach this point through a non-blocking queud connection, so not in the
        /// callback.
        qWarning() << QString("Rejecting call invite from %1, we're already in that call!").arg(friendNum);
        toxav_call_control(toxav, friendNum, TOXAV_CALL_CONTROL_CANCEL, nullptr);
        return;
    }
    qDebug() << QString("Received call invite from %1").arg(friendNum);

    // We don't get a state callback when answering, so fill the state ourselves in advance
    int state = 0;
    if (audio)
        state |= TOXAV_FRIEND_CALL_STATE_SENDING_A | TOXAV_FRIEND_CALL_STATE_ACCEPTING_A;
    if (video)
        state |= TOXAV_FRIEND_CALL_STATE_SENDING_V | TOXAV_FRIEND_CALL_STATE_ACCEPTING_V;
    it.first->second->setState(static_cast<TOXAV_FRIEND_CALL_STATE>(state));
    // note: changed to self->

    emit reinterpret_cast<CoreAV*>(self)->avInvite(friendNum, video);
    self->threadSwitchLock.clear(std::memory_order_release);
}

void CoreAV::stateCallback(ToxAV* toxav, uint32_t friendNum, uint32_t state, void* vSelf)
{
    CoreAV* self = static_cast<CoreAV*>(vSelf);

    // Run this slow callback asynchronously on the AV thread to avoid deadlocks with what our
    // caller (toxcore) holds
    // Also run the code to switch to the CoreAV thread in yet another thread, in case CoreAV
    // has threadSwitchLock and wants a toxcore lock that our call stack is holding...
    if (QThread::currentThread() != self->coreavThread.get()) {
        QtConcurrent::run([=]() {
            // We assume the original caller doesn't come from the CoreAV thread here
            while (self->threadSwitchLock.test_and_set(std::memory_order_acquire))
                QThread::yieldCurrentThread(); // Shouldn't spin for long, we have priority

            QMetaObject::invokeMethod(self, "stateCallback", Qt::QueuedConnection,
                                      Q_ARG(ToxAV*, toxav), Q_ARG(uint32_t, friendNum),
                                      Q_ARG(uint32_t, state), Q_ARG(void*, vSelf));
        });
        return;
    }

    auto it = self->calls.find(friendNum);
    if (it == self->calls.end()) {
        qWarning() << QString("stateCallback called, but call %1 is already dead").arg(friendNum);
        self->threadSwitchLock.clear(std::memory_order_release);
        return;
    }

    ToxFriendCall& call = *it->second;

    if (state & TOXAV_FRIEND_CALL_STATE_ERROR) {
        qWarning() << "Call with friend" << friendNum << "died of unnatural causes!";
        calls.erase(friendNum);
        // why not self->
        emit self->avEnd(friendNum, true);
    } else if (state & TOXAV_FRIEND_CALL_STATE_FINISHED) {
        qDebug() << "Call with friend" << friendNum << "finished quietly";
        calls.erase(friendNum);
        // why not self->
        emit self->avEnd(friendNum);
    } else {
        // If our state was null, we started the call and were still ringing
        if (!call.getState() && state) {
            call.stopTimeout();
            call.setActive(true);
            emit self->avStart(friendNum, call.getVideoEnabled());
        } else if ((call.getState() & TOXAV_FRIEND_CALL_STATE_SENDING_V)
                   && !(state & TOXAV_FRIEND_CALL_STATE_SENDING_V)) {
            qDebug() << "Friend" << friendNum << "stopped sending video";
            if (call.getVideoSource()) {
                call.getVideoSource()->stopSource();
            }
        } else if (!(call.getState() & TOXAV_FRIEND_CALL_STATE_SENDING_V)
                   && (state & TOXAV_FRIEND_CALL_STATE_SENDING_V)) {
            // Workaround toxav sometimes firing callbacks for "send last frame" -> "stop sending
            // video"
            // out of orders (even though they were sent in order by the other end).
            // We simply stop the videoSource from emitting anything while the other end says it's
            // not sending
            if (call.getVideoSource()) {
                call.getVideoSource()->restartSource();
            }
        }

        call.setState(static_cast<TOXAV_FRIEND_CALL_STATE>(state));
    }
    self->threadSwitchLock.clear(std::memory_order_release);
}

void CoreAV::bitrateCallback(ToxAV* toxav, uint32_t friendNum, uint32_t arate, uint32_t vrate,
                             void* vSelf)
{
    CoreAV* self = static_cast<CoreAV*>(vSelf);

    // Run this slow path callback asynchronously on the AV thread to avoid deadlocks
    if (QThread::currentThread() != self->coreavThread.get()) {
        return (void)QMetaObject::invokeMethod(self, "bitrateCallback", Qt::QueuedConnection,
                                               Q_ARG(ToxAV*, toxav), Q_ARG(uint32_t, friendNum),
                                               Q_ARG(uint32_t, arate), Q_ARG(uint32_t, vrate),
                                               Q_ARG(void*, vSelf));
    }

    qDebug() << "Recommended bitrate with" << friendNum << " is now " << arate << "/" << vrate
             << ", ignoring it";
}

void CoreAV::audioBitrateCallback(ToxAV* toxav, uint32_t friendNum, uint32_t rate, void* vSelf)
{
    CoreAV* self = static_cast<CoreAV*>(vSelf);

    // Run this slow path callback asynchronously on the AV thread to avoid deadlocks
    if (QThread::currentThread() != self->coreavThread.get()) {
        return (void)QMetaObject::invokeMethod(self, "audioBitrateCallback", Qt::QueuedConnection,
                                               Q_ARG(ToxAV*, toxav), Q_ARG(uint32_t, friendNum),
                                               Q_ARG(uint32_t, rate), Q_ARG(void*, vSelf));
    }

    qDebug() << "Recommended audio bitrate with" << friendNum << " is now " << rate << ", ignoring it";
}

void CoreAV::videoBitrateCallback(ToxAV* toxav, uint32_t friendNum, uint32_t rate, void* vSelf)
{
    CoreAV* self = static_cast<CoreAV*>(vSelf);

    // Run this slow path callback asynchronously on the AV thread to avoid deadlocks
    if (QThread::currentThread() != self->coreavThread.get()) {
        return (void)QMetaObject::invokeMethod(self, "videoBitrateCallback", Qt::QueuedConnection,
                                               Q_ARG(ToxAV*, toxav), Q_ARG(uint32_t, friendNum),
                                               Q_ARG(uint32_t, rate), Q_ARG(void*, vSelf));
    }

    qDebug() << "Recommended video bitrate with" << friendNum << " is now " << rate << ", ignoring it";
}

void CoreAV::audioFrameCallback(ToxAV*, uint32_t friendNum, const int16_t* pcm, size_t sampleCount,
                                uint8_t channels, uint32_t samplingRate, void* vSelf)
{
    CoreAV* self = static_cast<CoreAV*>(vSelf);
    auto it = calls.find(friendNum);
    if (it == self->calls.end()) {
        return;
    }

    ToxFriendCall& call = *it->second;

    if (call.getMuteVol()) {
        return;
    }

    call.playAudioBuffer(pcm, sampleCount, channels, samplingRate);
}

void CoreAV::videoFrameCallback(ToxAV*, uint32_t friendNum, uint16_t w, uint16_t h,
                                const uint8_t* y, const uint8_t* u, const uint8_t* v,
                                int32_t ystride, int32_t ustride, int32_t vstride, void*)
{
    auto it = calls.find(friendNum);
    if (it == calls.end()) {
        return;
    }

    CoreVideoSource* videoSource = it->second->getVideoSource();
    if (!videoSource) {
        return;
    }

    vpx_image frame;
    frame.d_h = h;
    frame.d_w = w;
    frame.planes[0] = const_cast<uint8_t*>(y);
    frame.planes[1] = const_cast<uint8_t*>(u);
    frame.planes[2] = const_cast<uint8_t*>(v);
    frame.stride[0] = ystride;
    frame.stride[1] = ustride;
    frame.stride[2] = vstride;

    videoSource->pushFrame(&frame);
}
