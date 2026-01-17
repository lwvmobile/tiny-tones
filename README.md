
# Tiny Tones

## Information

Tiny Tones is an open sourced vocoder socket library for encoding and decoding tones in M17 stream mode or other Codec2 projects. This method is written to be bit compatible with Codec2 1600 and 3200 Voice by way of employing Codec2 silence frames to carry tone information.

## General Tone Structure

Each tiny tone frame is a modified Codec2 1600 `0x010004002575DDF2` or 3200 `0x010009439CE42108` silence frame such that the last 3 bytes, or 24 bits become encoded tiny tone frame information. If these frames are passed to standard Codec2 decoder, they will result in silence, or indistinct near silence.

Each tiny tone frame can occupy any value from `0x010009439CF00000` to `0x010009439CFFFFFF` for 3200, and similarly `0x0100040025F00000` to `0x0100040025FFFFFF` for 1600, although many values will be unused. This same concept can also potentially be used to stealth embed other information as a n-diary embedded data method.

A tiny tone frame is laid out such that is uses this basic structure for 3200 and 1600.

`QQ QQ QQ QQ QQ PG FX CS`

Where as:

QQ - Original Silence Frame Bits

P  - Payload Indicator Value (0xF)

G  - Gain Step Value

FX - Frame Index ID

CS - Checksum

## Payload Indicator

A 4-bit value that is always 0xF to help identify tone frames from silence frames.

## Gain Step Value

The 4-bit gain stepping is calculated such that each value of 0-15 is a multiple of 6.25%, whereas 0 = 6.25% and 15 = 100%.

## Frame Index ID

Frame Index ID values ranging from 0x00 to 0x0F correspond to DTMF tones 1-9, A, B, C, D, *, #.

Frame Index ID values ranging from 0x10 to 0x1F correspond to KNOX tones 1-9, A, B, C, D, *, #.

Frame Index ID values ranging from 0x20 to 0x49 correspond to musical notes G3 through C7.

see `tt.h` for a comprehensive breakdown of values.

## Checksum

An 8-bit checksum value is calculated across all bytes of the 64-bit payload via summing all byte values mod 256, and inverting value & 0xFF; This provides an extra layer of positive identification on tone frames.

```
//calculate and check vs embedded checksum
uint8_t checksum = 0;
for (int i = 0; i < 7; i++)
  checksum += input[i];

//invert checksum
checksum = ~checksum;
checksum &= 0xFF;
```

## Silence Frames

What is a silence frame? A silence frame from the perspective of this project is any frame that is encoded via Codec2 where-as all the input audio samples are absolute zero. Codec2 encodes S16LE audio sampled at 8k, which is typically sourced from a microphone, or other source, like a wav file, etc. If the input of the S16LE is an array of all zero values, the encoded value becomes a silence frame. Decoding a silence frame results in "silence" or inaudible sound to the human ear compared to human speech. 

## Why

Tone information could be encoded directly as audio, but fidelity of tones could also be impacted by encoder and decoder, depending on frequencies used. Using tone frames and generating audio locally helps to keep the audio fidelity, and also allows this to be a purely optional item. Many users may enjoy or find utility in the ability to send tones, or listen to tones, and others may not care for them. Rendering them into silence frames allows for both options. People who want to use them can use them, and those who do not want them only hear silence.