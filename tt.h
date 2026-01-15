/*-------------------------------------------------------------------------------
 * tt.h
 * Tiny Tones Encoder and Decoder for Codec2 1600/3200 Voice Frames
 *
 * LWVMOBILE
 * 2026-01 Tiny Tones
 *-----------------------------------------------------------------------------*/

#ifndef TT_H
#define TT_H

#define TT_VER 0.1

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#define SILENCE_3200 0x010009439CE42108
#define SILENCE_1600 0x010004002575DDF2
#define LEN_3200 160
#define LEN_1600 320

#define FTOSGAIN 25

//TODO: Make this a struct?
extern int tone_pitch;
extern int tone_n;
extern int tone_frames_to_send;
extern uint8_t tone_idx;
extern uint8_t tone_gain;

#ifdef __cplusplus
extern "C" {
#endif

//function prototypes
void init_tt_static(void);
int tiny_tone_decoder (uint64_t silence_frame, uint8_t * input, int n, int len, short * audio);
int tiny_tone_encoder(uint64_t silence_frame, uint8_t idx, uint8_t gain_step, uint8_t * output);

#ifdef __cplusplus
}
#endif

/*
Tone Values 0x00 - 0x0F are DTMF Tones
Tone Values 0x10 - 0x1F are Knox Tones
Tone Values 0x20 - 0x4A are Musical Notes
*/

/* DTMF tone pairs - most common format used in code */
static const int dtmf_tones[16][2] = {
  {697, 1209},  // 1
  {697, 1336},  // 2
  {697, 1477},  // 3
  {770, 1209},  // 4
  {770, 1336},  // 5
  {770, 1477},  // 6
  {852, 1209},  // 7
  {852, 1336},  // 8
  {852, 1477},  // 9
  {941, 1209},  // *
  {941, 1336},  // 0
  {941, 1477},  // #
  {697, 1633},  // A
  {770, 1633},  // B
  {852, 1633},  // C
  {941, 1633}   // D
};

/* Knox-Box / Knox tones - modified DTMF frequency pairs (in Hz)
   Used by some Knox rapid entry systems for keybox release
   Format: [low freq, high freq] per digit/character */
static const int knox_tones[16][2] = {
  { 697, 1633 },    // 0
  { 1209, 697 },    // 1
  { 1336, 697 },    // 2
  { 1477, 697 },    // 3
  { 1209, 770 },    // 4
  { 1336, 770 },    // 5
  { 1477, 770 },    // 6
  { 1209, 852 },    // 7
  { 1336, 852 },    // 8
  { 1477, 852 },    // 9
  { 1209, 941 },    // *
  { 1336, 941 },    // 0
  { 1477, 941 },    // #
  { 1633, 697 },    // A
  { 1633, 770 },    // B
  { 1633, 852 }     // C
};

// Musical note frequencies (A4 = 440 Hz) - equal temperament
// Format: note name + octave (C4 = middle C)
static const float note_frequencies[42] = {
  196.00f,   // G3
  207.65f,   // G#3 / Ab3
  220.00f,   // A3
  233.08f,   // A#3 / Bb3
  246.94f,   // B3

  // Octave 4 (middle octave - most used)
  261.63f,   // C4   ← middle C
  277.18f,   // C#4 / Db4
  293.66f,   // D4
  311.13f,   // D#4 / Eb4
  329.63f,   // E4
  349.23f,   // F4
  369.99f,   // F#4 / Gb4
  392.00f,   // G4
  415.30f,   // G#4 / Ab4
  440.00f,   // A4   ← concert pitch reference
  466.16f,   // A#4 / Bb4
  493.88f,   // B4

  // Octave 5
  523.25f,   // C5
  554.37f,   // C#5 / Db5
  587.33f,   // D5
  622.25f,   // D#5 / Eb5
  659.25f,   // E5
  698.46f,   // F5
  739.99f,   // F#5 / Gb5
  783.99f,   // G5
  830.61f,   // G#5 / Ab5
  880.00f,   // A5
  932.33f,   // A#5 / Bb5
  987.77f,   // B5

  // Octave 6 (high)
  1046.50f,  // C6
  1108.73f,  // C#6 / Db6
  1174.66f,  // D6
  1244.51f,  // D#6 / Eb6
  1318.51f,  // E6
  1396.91f,  // F6
  1479.98f,  // F#6 / Gb6
  1567.98f,  // G6
  1661.22f,  // G#6 / Ab6
  1760.00f,  // A6
  1864.66f,  // A#6 / Bb6
  1975.53f,  // B6
};

#endif //TT_H
