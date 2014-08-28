#include "core.h"
#include "widget/widget.h"

ToxCall Core::calls[TOXAV_MAX_CALLS];
const int Core::videobufsize{TOXAV_MAX_VIDEO_WIDTH * TOXAV_MAX_VIDEO_HEIGHT * 4};
uint8_t* Core::videobuf;
int Core::videoBusyness;

void Core::prepareCall(int friendId, int callId, ToxAv* toxav, bool videoEnabled)
{
    qDebug() << QString("Core: preparing call %1").arg(callId);
    calls[callId].callId = callId;
    calls[callId].friendId = friendId;
    // the following three lines are also now redundant from startCall, but are
    // necessary there for outbound and here for inbound
    calls[callId].codecSettings = av_DefaultSettings;
    calls[callId].codecSettings.max_video_width = TOXAV_MAX_VIDEO_WIDTH;
    calls[callId].codecSettings.max_video_height = TOXAV_MAX_VIDEO_HEIGHT;
    calls[callId].videoEnabled = videoEnabled;
    toxav_prepare_transmission(toxav, callId, av_jbufdc, av_VADd, videoEnabled);

    // Audio output
    calls[callId].alOutDev = alcOpenDevice(nullptr);
    if (!calls[callId].alOutDev)
    {
        qWarning() << "Coreav: Cannot open output audio device, hanging up call";
        toxav_hangup(toxav, callId);
        return;
    }
    calls[callId].alContext=alcCreateContext(calls[callId].alOutDev,nullptr);
    if (!alcMakeContextCurrent(calls[callId].alContext))
    {
        qWarning() << "Coreav: Cannot create output audio context, hanging up call";
        alcCloseDevice(calls[callId].alOutDev);
        toxav_hangup(toxav, callId);
        return;
    }
    alGenSources(1, &calls[callId].alSource);

    // Prepare output
    QAudioFormat format;
    format.setSampleRate(calls[callId].codecSettings.audio_sample_rate);
    format.setChannelCount(calls[callId].codecSettings.audio_channels);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);
    if (!QAudioDeviceInfo::defaultOutputDevice().isFormatSupported(format))
    {
        calls[callId].audioOutput = nullptr;
        qWarning() << "Core: Raw audio format not supported by output backend, cannot play audio.";
    }
    else if (calls[callId].audioOutput==nullptr)
    {
        calls[callId].audioOutput = new QAudioOutput(format);
        calls[callId].audioOutput->setBufferSize(1900*30); // Make this bigger to get less underflows, but more latency
        calls[callId].audioOutput->start(&calls[callId].audioBuffer);
        int error = calls[callId].audioOutput->error();
        if (error != QAudio::NoError)
        {
            qWarning() << QString("Core: Error %1 when starting audio output").arg(error);
        }
    }

    // Start input
    if (!QAudioDeviceInfo::defaultInputDevice().isFormatSupported(format))
    {
        calls[callId].audioInput = nullptr;
        qWarning() << "Core: Default input format not supported, cannot record audio";
    }
    else if (calls[callId].audioInput==nullptr)
    {
        qDebug() << "Core: Starting new audio input";
        calls[callId].audioInput = new QAudioInput(format);
        calls[callId].audioInputDevice = calls[callId].audioInput->start();
    }
    else if (calls[callId].audioInput->state() == QAudio::StoppedState)
    {
        calls[callId].audioInputDevice = calls[callId].audioInput->start();
    }

    // Go
    calls[callId].active = true;

    if (calls[callId].audioInput != nullptr)
    {
        calls[callId].sendAudioTimer->setInterval(2);
        calls[callId].sendAudioTimer->setSingleShot(true);
        connect(calls[callId].sendAudioTimer, &QTimer::timeout, [=](){sendCallAudio(callId,toxav);});
        calls[callId].sendAudioTimer->start();
    }

    if (calls[callId].videoEnabled)
    {
        calls[callId].sendVideoTimer->setInterval(50);
        calls[callId].sendVideoTimer->setSingleShot(true);
        calls[callId].sendVideoTimer->start();

        Widget::getInstance()->getCamera()->suscribe();
    }
    else if (calls[callId].audioInput == nullptr && calls[callId].audioOutput == nullptr)
    {
        qWarning() << "Audio only call can neither play nor record audio, killing call";
        toxav_hangup(toxav, callId);
    }
}

void Core::onAvMediaChange(void*, int32_t, void*)
{
    // HALP, PLS COMPLETE MEH
    qWarning() << "If you see this, please complain on GitHub about seeing me! (Don't forget to say what caused me!)";
}

void Core::answerCall(int callId)
{
    int friendId = toxav_get_peer_id(toxav, callId, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV answer peer ID";
        return;
    }

    ToxAvCSettings* transSettings = new ToxAvCSettings;
    int err = toxav_get_peer_csettings(toxav, callId, 0, transSettings);
    if (err != ErrorNone)
    {
         qWarning() << "Core::answerCall: error getting call settings";
         delete transSettings;
         return;
    }

    if (transSettings->call_type == TypeVideo)
    {
        qDebug() << QString("Core: answering call %1 with video").arg(callId);
        toxav_answer(toxav, callId, transSettings);
    }
    else
    {
        qDebug() << QString("Core: answering call %1 without video").arg(callId);
        toxav_answer(toxav, callId, transSettings);
    }

    delete transSettings;
}

void Core::hangupCall(int callId)
{
    qDebug() << QString("Core: hanging up call %1").arg(callId);
    calls[callId].active = false;
    toxav_hangup(toxav, callId);
}

void Core::startCall(int friendId, bool video)
{
    int callId;
    ToxAvCSettings cSettings = av_DefaultSettings;
    cSettings.max_video_width = TOXAV_MAX_VIDEO_WIDTH;
    cSettings.max_video_height = TOXAV_MAX_VIDEO_HEIGHT;
    if (video)
    {
        qDebug() << QString("Core: Starting new call with %1 with video").arg(friendId);
        cSettings.call_type = TypeVideo;
        toxav_call(toxav, &callId, friendId, &cSettings, TOXAV_RINGING_TIME);
        calls[callId].videoEnabled=true;
    }
    else
    {
        qDebug() << QString("Core: Starting new call with %1 without video").arg(friendId);
        cSettings.call_type = TypeAudio;
        toxav_call(toxav, &callId, friendId, &cSettings, TOXAV_RINGING_TIME);
        calls[callId].videoEnabled=false;
    }
}

void Core::cancelCall(int callId, int friendId)
{
    qDebug() << QString("Core: Cancelling call with %1").arg(friendId);
    calls[callId].active = false;
    toxav_cancel(toxav, callId, friendId, 0);
}

void Core::cleanupCall(int callId)
{
    qDebug() << QString("Core: cleaning up call %1").arg(callId);
    calls[callId].active = false;
    disconnect(calls[callId].sendAudioTimer,0,0,0);
    calls[callId].sendAudioTimer->stop();
    calls[callId].sendVideoTimer->stop();
    if (calls[callId].audioOutput != nullptr)
    {
        calls[callId].audioOutput->stop();
    }
    if (calls[callId].audioInput != nullptr)
    {
        calls[callId].audioInput->stop();
    }
    if (calls[callId].videoEnabled)
        Widget::getInstance()->getCamera()->unsuscribe();
    calls[callId].audioBuffer.clear();
}

void Core::playCallAudio(ToxAv*, int32_t callId, int16_t *data, int samples, void *user_data)
{
    Q_UNUSED(user_data);

    if (!calls[callId].active || calls[callId].audioOutput == nullptr)
        return;

    playAudioBuffer(callId, data, samples);
}

void Core::sendCallAudio(int callId, ToxAv* toxav)
{
    if (!calls[callId].active || calls[callId].audioInput == nullptr)
        return;
    int framesize = (calls[callId].codecSettings.audio_frame_duration * calls[callId].codecSettings.audio_sample_rate) / 1000;
    uint8_t buf[framesize*2], dest[framesize*2];
    int bytesReady = calls[callId].audioInput->bytesReady();
    if (bytesReady >= framesize*2)
    {
        calls[callId].audioInputDevice->read((char*)buf, framesize*2);
        int result = toxav_prepare_audio_frame(toxav, callId, dest, framesize*2, (int16_t*)buf, framesize);
        if (result < 0)
        {
            qWarning() << QString("Core: Unable to prepare audio frame, error %1").arg(result);
            calls[callId].sendAudioTimer->start();
            return;
        }
        result = toxav_send_audio(toxav, callId, dest, result);
        if (result < 0)
        {
            qWarning() << QString("Core: Unable to send audio frame, error %1").arg(result);
            calls[callId].sendAudioTimer->start();
            return;
        }
        calls[callId].sendAudioTimer->start();
    }
    else
        calls[callId].sendAudioTimer->start();
}

void Core::playCallVideo(ToxAv*, int32_t callId, vpx_image_t* img, void *user_data)
{
    Q_UNUSED(user_data);

    if (!calls[callId].active || !calls[callId].videoEnabled)
        return;

    if (videoBusyness >= 1)
        qWarning() << "Core: playCallVideo: Busy, dropping current frame";
    else
        emit Widget::getInstance()->getCore()->videoFrameReceived(img);
    vpx_img_free(img);
}

void Core::sendCallVideo(int callId)
{
    if (!calls[callId].active || !calls[callId].videoEnabled)
        return;

    vpx_image frame = camera->getLastVPXImage();
    if (frame.w && frame.h)
    {
        int result;
        if((result = toxav_prepare_video_frame(toxav, callId, videobuf, videobufsize, &frame)) < 0)
        {
            qDebug() << QString("Core: toxav_prepare_video_frame: error %1").arg(result);
            vpx_img_free(&frame);
            calls[callId].sendVideoTimer->start();
            return;
        }

        if((result = toxav_send_video(toxav, callId, (uint8_t*)videobuf, result)) < 0)
            qDebug() << QString("Core: toxav_send_video error: %1").arg(result);

        vpx_img_free(&frame);
    }
    else
        qDebug("Core::sendCallVideo: Invalid frame (bad camera ?)");

    calls[callId].sendVideoTimer->start();
}


void Core::increaseVideoBusyness()
{
    videoBusyness++;
}

void Core::decreaseVideoBusyness()
{
  videoBusyness--;
}

void Core::micMuteToggle(int callId)
{
  if (calls[callId].audioInput->state() == QAudio::ActiveState)
    calls[callId].audioInput->suspend();
  else
    calls[callId].audioInput->start();
}

void Core::onAvCancel(void* _toxav, int32_t call_index, void* core)
{
    ToxAv* toxav = static_cast<ToxAv*>(_toxav);

    int friendId = toxav_get_peer_id(toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV cancel";
        return;
    }
    qDebug() << QString("Core: AV cancel from %1").arg(friendId);

    emit static_cast<Core*>(core)->avCancel(friendId, call_index);
}

void Core::onAvReject(void*, int32_t, void*)
{
    qDebug() << "Core: AV reject";
}

void Core::onAvEnd(void* _toxav, int32_t call_index, void* core)
{
    ToxAv* toxav = static_cast<ToxAv*>(_toxav);

    int friendId = toxav_get_peer_id(toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV end";
        return;
    }
    qDebug() << QString("Core: AV end from %1").arg(friendId);

    cleanupCall(call_index);

    emit static_cast<Core*>(core)->avEnd(friendId, call_index);
}

void Core::onAvRinging(void* _toxav, int32_t call_index, void* core)
{
    ToxAv* toxav = static_cast<ToxAv*>(_toxav);

    int friendId = toxav_get_peer_id(toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV ringing";
        return;
    }

    if (calls[call_index].videoEnabled)
    {
        qDebug() << QString("Core: AV ringing with %1 with video").arg(friendId);
        emit static_cast<Core*>(core)->avRinging(friendId, call_index, true);
    }
    else
    {
        qDebug() << QString("Core: AV ringing with %1 without video").arg(friendId);
        emit static_cast<Core*>(core)->avRinging(friendId, call_index, false);
    }
}

void Core::onAvStarting(void* _toxav, int32_t call_index, void* core)
{
    ToxAv* toxav = static_cast<ToxAv*>(_toxav);

    int friendId = toxav_get_peer_id(toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV starting";
        return;
    }

    ToxAvCSettings* transSettings = new ToxAvCSettings;
    int err = toxav_get_peer_csettings(toxav, call_index, 0, transSettings);
    if (err != ErrorNone)
    {
        qWarning() << "Core::onAvStarting: error getting call type";
        delete transSettings;
        return;
    }

    if (transSettings->call_type == TypeVideo)
    {
        qDebug() << QString("Core: AV starting from %1 with video").arg(friendId);
        prepareCall(friendId, call_index, toxav, true);
        emit static_cast<Core*>(core)->avStarting(friendId, call_index, true);
    }
    else
    {
        qDebug() << QString("Core: AV starting from %1 without video").arg(friendId);
        prepareCall(friendId, call_index, toxav, false);
        emit static_cast<Core*>(core)->avStarting(friendId, call_index, false);
    }

    delete transSettings;
}

void Core::onAvEnding(void* _toxav, int32_t call_index, void* core)
{
    ToxAv* toxav = static_cast<ToxAv*>(_toxav);

    int friendId = toxav_get_peer_id(toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV ending";
        return;
    }
    qDebug() << QString("Core: AV ending from %1").arg(friendId);

    cleanupCall(call_index);

    emit static_cast<Core*>(core)->avEnding(friendId, call_index);
}

void Core::onAvRequestTimeout(void* _toxav, int32_t call_index, void* core)
{
    ToxAv* toxav = static_cast<ToxAv*>(_toxav);

    int friendId = toxav_get_peer_id(toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV request timeout";
        return;
    }
    qDebug() << QString("Core: AV request timeout with %1").arg(friendId);

    cleanupCall(call_index);

    emit static_cast<Core*>(core)->avRequestTimeout(friendId, call_index);
}

void Core::onAvPeerTimeout(void* _toxav, int32_t call_index, void* core)
{
    ToxAv* toxav = static_cast<ToxAv*>(_toxav);

    int friendId = toxav_get_peer_id(toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV peer timeout";
        return;
    }
    qDebug() << QString("Core: AV peer timeout with %1").arg(friendId);

    cleanupCall(call_index);

    emit static_cast<Core*>(core)->avPeerTimeout(friendId, call_index);
}


void Core::onAvInvite(void* _toxav, int32_t call_index, void* core)
{
    ToxAv* toxav = static_cast<ToxAv*>(_toxav);

    int friendId = toxav_get_peer_id(toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV invite";
        return;
    }

    ToxAvCSettings* transSettings = new ToxAvCSettings;
    int err = toxav_get_peer_csettings(toxav, call_index, 0, transSettings);
    if (err != ErrorNone)
    {
        qWarning() << "Core::onAvInvite: error getting call type";
        delete transSettings;
        return;
    }

    if (transSettings->call_type == TypeVideo)
    {
        qDebug() << QString("Core: AV invite from %1 with video").arg(friendId);
        emit static_cast<Core*>(core)->avInvite(friendId, call_index, true);
    }
    else
    {
        qDebug() << QString("Core: AV invite from %1 without video").arg(friendId);
        emit static_cast<Core*>(core)->avInvite(friendId, call_index, false);
    }

    delete transSettings;
}

void Core::onAvStart(void* _toxav, int32_t call_index, void* core)
{
    ToxAv* toxav = static_cast<ToxAv*>(_toxav);

    int friendId = toxav_get_peer_id(toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV start";
        return;
    }

    ToxAvCSettings* transSettings = new ToxAvCSettings;
    int err = toxav_get_peer_csettings(toxav, call_index, 0, transSettings);
    if (err != ErrorNone)
    {
        qWarning() << "Core::onAvStart: error getting call type";
        delete transSettings;
        return;
    }

    if (transSettings->call_type == TypeVideo)
    {
        qDebug() << QString("Core: AV start from %1 with video").arg(friendId);
        prepareCall(friendId, call_index, toxav, true);
        emit static_cast<Core*>(core)->avStart(friendId, call_index, true);
    }
    else
    {
        qDebug() << QString("Core: AV start from %1 without video").arg(friendId);
        prepareCall(friendId, call_index, toxav, false);
        emit static_cast<Core*>(core)->avStart(friendId, call_index, false);
    }

    delete transSettings;
}

void Core::playAudioBuffer(int callId, int16_t *data, int samples)
{
    unsigned channels = calls[callId].codecSettings.audio_channels;
    if(!channels || channels > 2) {
        qWarning() << "Core::playAudioBuffer: trying to play on "<<channels<<" channels! Giving up.";
        return;
    }

    ALuint bufid;
    ALint processed, queued;
    alGetSourcei(calls[callId].alSource, AL_BUFFERS_PROCESSED, &processed);
    alGetSourcei(calls[callId].alSource, AL_BUFFERS_QUEUED, &queued);
    alSourcei(calls[callId].alSource, AL_LOOPING, AL_FALSE);

    if(processed)
    {
        ALuint bufids[processed];
        alSourceUnqueueBuffers(calls[callId].alSource, processed, bufids);
        alDeleteBuffers(processed - 1, bufids + 1);
        bufid = bufids[0];
    }
    else if(queued < 16)
    {
        alGenBuffers(1, &bufid);
    }
    else
    {
        qDebug() << "Core: Dropped audio frame";
        return;
    }

    alBufferData(bufid, (channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, data,
                    samples * 2 * channels, calls[callId].codecSettings.audio_sample_rate);
    alSourceQueueBuffers(calls[callId].alSource, 1, &bufid);

    ALint state;
    alGetSourcei(calls[callId].alSource, AL_SOURCE_STATE, &state);
    if(state != AL_PLAYING)
    {
        alSourcePlay(calls[callId].alSource);
        qDebug() << "Core: Starting audio source of call " << callId;
    }
}
