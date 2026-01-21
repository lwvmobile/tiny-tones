/*-------------------------------------------------------------------------------
 * tt.c
 * Tiny Tones Encoder and Decoder for Codec2 1600/3200 Voice Frames
 *
 * LWVMOBILE
 * 2026-01 Tiny Tones
 *-----------------------------------------------------------------------------*/

#include "tt.h"

// #define DEBUG_TT_ENCODE
// #define DEBUG_TT_DECODE
// #define TT_UNIT_TEST

//silence_frame is the defined silence_frame for decoder type - SILENCE_3200 or SILENCE_1600
//input is uint8_t byte array of M17 / Codec2 1600 or 3200 single payload sample
//tone_phase is an rolling integer value held by calling function that should be retained between frames
//len is the amount of audio samples below to produce - LEN_3200 or LEN_1600
//audio is short audio samples of S16LE audio samples at 8k/1 at len value
//return value is flag to signal this is a tone frame (tone_phase) or not (-1, -2, -3, -4)
int tiny_tone_decoder (uint64_t silence_frame, uint8_t * input, int tone_phase, int len, short * audio)
{

  //zero fill audio
  for (int i = 0; i < len; i++)
    audio[i] = 0;

  //check to see if first 40-bits passed here matches the silence frame passed here
  uint64_t header = 0;
  header <<= 8; header |= input[0];
  header <<= 8; header |= input[1];
  header <<= 8; header |= input[2];
  header <<= 8; header |= input[3];
  header <<= 8; header |= input[4];

  //compare silence and header, if not equal, this is not a silence or tone frame
  if (header != (silence_frame >> 24))
    return -1;

  //calculate and check vs embedded checksum
  uint8_t checksum = 0;
  for (int i = 0; i < 7; i++)
    checksum += input[i];

  //invert checksum
  checksum = ~checksum;
  checksum &= 0xFF;

  //checksum error or not tone frame
  if (checksum != input[7])
    return -2;

  //payload indicator
  uint8_t indicator = (input[5] >> 4) & 0xF;

  //this is not a tone frame, incorrect indicator
  if (indicator != 0xF)
    return -3;

  //gain step values corresponding to gain application
  uint8_t gain_step = (input[5] >> 0) & 0xF;

  //byte values corresponding to frequency look ups
  uint8_t index = input[6];

  //gain value is a stepping of 6.25, where as 15 = 100%, and 0 = 6.25%
  int gain = (gain_step + 1) * 6.25;

  //lookup frequencies based on value on the index
  int freqhigh = 0, freqlow = 0;
  if (index <= 0x20)
  {
    freqhigh = tt_frequencies[index][0];
    freqlow  = tt_frequencies[index][1];
  }
  else if (index <= MAX_TT_FRAMES)
  {
    freqhigh = tt_frequencies[index][0] / 100;
    freqlow  = tt_frequencies[index][1] / 100;
  }
  else
  {
    //unknown, or non indexed values
    return -4;
  }

  #ifdef DEBUG_TT_DECODE
  //debug loading values
  if (index < 0x20)
    fprintf (stderr, " Index: %02X; Gain Step: %X (%03d%%); Freq: %d / %d ", index, gain_step, gain, freqhigh, freqlow);
  else fprintf (stderr, " Index: %02X; Gain Step: %X (%03d%%); Freq: %d ", index, gain_step, gain, freqhigh);
  #endif

  //produce audio samples based on frequency, and gain
  for (int i = 0; i < len; i++)
  {
    audio[i] = FTOSGAIN * gain 
      * (sin((tone_phase) * (2 * M_PI * freqhigh / 8000))/2 
      +  sin((tone_phase) * (2 * M_PI * freqlow  / 8000))/2);
    tone_phase++;
  }

  //valid tone frame
  return tone_phase;

}

//silence frame is the mode specific silence frame to load - SILENCE_3200 or SILENCE_1600
//idx is the tone id index value to encode
//gain_step is the preferred gain_step value
//output is a bytewise array of payload frames based on num_frames
//return value is success load on 1, or not success (i.e., unknown idx value or bad gain_step)
int tiny_tone_encoder(uint64_t silence_frame, uint8_t idx, uint8_t gain_step, uint8_t * output)
{

  //load silence first
  for (int i = 0; i < 8; i++)
    output[i] = (silence_frame >> (56-(i*8))) & 0xFF;

  //make sure index is within number of frames encodable
  if (idx > MAX_TT_FRAMES)
    return -1;

  //check gain value
  if (gain_step > 0xF)
    return -2;

  //load indicator
  output[5] = 0xF0;

  //load gain_step value
  output[5] |= gain_step;

  //load idx
  output[6] = idx;

  //calculate checksum and load
  uint8_t checksum = 0;
  for (int i = 0; i < 7; i++)
    checksum += output[i];

  checksum = ~checksum;
  checksum &= 0xFF;

  output[7] = checksum;

  #ifdef DEBUG_TT_ENCODE
  //debug loading values
  if (idx < 0x20)
    fprintf (stderr, " Index: %02X; Gain Step: %X (%03d%%); Freq: %d / %d ", idx, gain_step, (int)((gain_step+1) * 6.25), tt_frequencies[idx][0], tt_frequencies[idx][1]);
  else fprintf (stderr, " Index: %02X; Gain Step: %X (%03d%%); Freq: %d ", idx, gain_step, (int)((gain_step+1) * 6.25), tt_frequencies[idx][0] / 100);
  #endif

  return 1;

}

//initialize TINY_TONES struct
void init_tt_struct(TINY_TONES * tt)
{
  tt->tone_phase = 0;
  tt->tone_frames_to_send = 0;
  tt->tone_idx = 0;
  tt->tone_gain = 0xF;
  tt->tone_pitch = 0;
}

//unit test build with gcc or clang
//gcc -o test tt.c -lm -DTT_UNIT_TEST -DDEBUG_TT_DECODE -DDEBUG_TT_ENCODE -Wall -Wextra -Wpedantic
//clang -o test tt.c -lm -DTT_UNIT_TEST -DDEBUG_TT_DECODE -DDEBUG_TT_ENCODE -Wall -Wextra -Wpedantic
#ifdef TT_UNIT_TEST
int main (void)
{

  uint8_t bytes[8];
  short audio[320];

  TINY_TONES tt;

  init_tt_struct(&tt);

  //unit test encode, decode and dump all possible tone frames
  fprintf (stderr, "3200 Frames: \n");
  for (uint8_t i = 0; i <= MAX_TT_FRAMES; i++)
  {
    for (uint8_t j = 0; j < 16; j++)
    {
      memset (bytes, 0, sizeof(bytes));
      int ret = tiny_tone_encoder(SILENCE_3200, i, j, bytes);
      if (ret > 0)
        fprintf (stderr, "OK; ");
      else fprintf (stderr, "FAIL (%i); ", ret);

      for (int k = 0; k < 8; k++)
        fprintf (stderr, "%02X", bytes[k]);

      //decode
      fprintf (stderr, " --");
      memset (audio, 0, sizeof(audio));
      tt.tone_phase = 0;
      tt.tone_phase = tiny_tone_decoder(SILENCE_3200, bytes, tt.tone_phase, LEN_3200, audio);

      if (tt.tone_phase > 0)
        fprintf (stderr, "OK; ");
      else fprintf (stderr, "FAIL (%i); ", tt.tone_phase);

      //dump audio samples
      // fprintf (stderr, "\n");
      // for (int k = 0; k < LEN_3200; k++)
      // {
      //   if (k != 0 && k % 16 == 0)
      //     fprintf (stderr, "\n");
      //   fprintf (stderr, "(%d)", audio[k]);
      // }

      fprintf (stderr, "\n");
    }
    
  }

  fprintf (stderr, "1600 Frames: \n");
  for (uint8_t i = 0; i <= MAX_TT_FRAMES; i++)
  {
    for (uint8_t j = 0; j < 16; j++)
    {
      memset (bytes, 0, sizeof(bytes));
      int ret = tiny_tone_encoder(SILENCE_1600, i, j, bytes);
      if (ret > 0)
        fprintf (stderr, "OK; ");
      else fprintf (stderr, "FAIL (%i); ", ret);

      for (int k = 0; k < 8; k++)
        fprintf (stderr, "%02X", bytes[k]);

      //decode
      fprintf (stderr, " --");
      memset (audio, 0, sizeof(audio));
      tt.tone_phase = 0;
      tt.tone_phase = tiny_tone_decoder(SILENCE_1600, bytes, tt.tone_phase, LEN_1600, audio);

      if (tt.tone_phase > 0)
        fprintf (stderr, "OK; ");
      else fprintf (stderr, "FAIL (%i); ", tt.tone_phase);

      //dump audio samples
      // fprintf (stderr, "\n");
      // for (int k = 0; k < LEN_1600; k++)
      // {
      //   if (k != 0 && k % 16 == 0)
      //     fprintf (stderr, "\n");
      //   fprintf (stderr, "(%d)", audio[k]);
      // }

      fprintf (stderr, "\n");
    }
    
  }

  return tt.tone_phase;
}
#endif
