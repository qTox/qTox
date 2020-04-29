To keep sounds consistent a sound file should follow the parameters listed
below.

# Format

Internally qTox needs PCM with signed 16Bit integers and a samplerate of
48kHz and one channel (mono).

You can use ffmpeg to create those files as follows:
```
ffmpeg -i notification.wav -f s16le -acodec pcm_s16le -ac 1 -ar 48000 notification.s16le.pcm
```

# Normalization

All sound files should have their maximum at -1.0 dB.
To normalize them correctly you can use Audacity with the "Normalize" plugin.
