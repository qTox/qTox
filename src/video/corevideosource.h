#ifndef COREVIDEOSOURCE_H
#define COREVIDEOSOURCE_H

#include <vpx/vpx_image.h>
#include <atomic>
#include "videosource.h"

/// A VideoSource that emits frames received by Core
class CoreVideoSource : public VideoSource
{
    Q_OBJECT
public:
    // VideoSource interface
    virtual bool subscribe() override;
    virtual void unsubscribe() override;

private:
    // Only Core should create a CoreVideoSource since
    // only Core can push images to it
    CoreVideoSource();

    /// Makes a copy of the vpx_image_t and emits it as a new VideoFrame
    void pushFrame(const vpx_image_t *frame);
    /// If true, self-delete after the last suscriber is gone
    void setDeleteOnClose(bool newstate);

private:
    std::atomic_int subscribers; ///< Number of suscribers
    std::atomic_bool deleteOnClose; ///< If true, self-delete after the last suscriber is gone
    std::atomic_bool biglock; ///< Fast lock

friend class Core;
};

#endif // COREVIDEOSOURCE_H
