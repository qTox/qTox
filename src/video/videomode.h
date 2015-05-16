#ifndef VIDEOMODE_H
#define VIDEOMODE_H

/// Describes a video mode supported by a device
struct VideoMode
{
    short width, height; ///< Displayed video resolution (NOT frame resolution)
    short FPS; ///< Max frames per second supported by the device at this resolution

    /// All zeros means a default/unspecified mode
    operator bool()
    {
        return width || height || FPS;
    }

    bool operator==(const VideoMode& other)
    {
        return width == other.width
                && height == other.height
                && FPS == other.FPS;
    }
};

#endif // VIDEOMODE_H

