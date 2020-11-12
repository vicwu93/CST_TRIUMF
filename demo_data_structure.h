#ifndef DEMO_DATA_STRUCTURE_H
#define DEMO_DATA_STRUCTURE_H

#include <cstdint>
#include <vector>

#define CLOCK_RATE_HZ 125000000
#define MAX_WF_WORDS 2048

// Data structure that mimics the key points of the VX2740 data
// structure.
// In particular:
// * 64 bit words
// * First word encodes 48-bit timestamp (@125MHz), channel ID, and some user flags
// * Second word encode energy (which we don't use in this dummy data yet)
// * Third word encodes number of 64-bit waveform data words
// * Then the waveform data follows
//
// Helper functions are provided for encoding/decoding from a buffer of uint64_t.
class DemoData {
  public:
    double timestamp_secs; // Max timestamp is ~26 days!
    uint16_t channel_id;   // 0 to 63
    bool is_end_of_slice;
    std::vector<uint64_t> wf_words;

    DemoData() {
      timestamp_secs = 0;
      channel_id = 0;
      is_end_of_slice = false;
    }

    uint32_t get_encoded_size_bytes() {
      return (wf_words.size() + 3) * sizeof(uint64_t);
    }

    // Encode to a buffer of uint64_t.
    void encode(uint64_t* buf) {
      uint64_t timestamp_word = 0;
      uint64_t energy_word = 0;
      uint64_t wf_size_word = 0;

      timestamp_word |= ((uint64_t)(timestamp_secs * CLOCK_RATE_HZ) & 0xFFFFFFFFFFFF);
      timestamp_word |= ((uint64_t)(channel_id & 0x3F) << 56);

      if (is_end_of_slice) {
        timestamp_word |= ((uint64_t)1 << 48);
      }

      if (wf_words.size() > MAX_WF_WORDS) {
        // Resize waveform and set truncated bit
        wf_words.resize(MAX_WF_WORDS);
        wf_size_word |= 0x80;
      }

      wf_size_word |= (wf_words.size() & 0x7F);

      *buf++ = timestamp_word;
      *buf++ = energy_word;
      *buf++ = wf_size_word;

      for (auto it = wf_words.begin(); it != wf_words.end(); it++) {
        *buf++ = *it;
      }
    }

    // Decode from a buffer of uint64_t.
    void decode(uint64_t* buf) {
      uint64_t timestamp_word = *buf++;
      uint64_t energy_word = *buf++;
      uint64_t wf_size_word = *buf++;

      timestamp_secs = (double)(timestamp_word & 0xFFFFFFFFFFFF) / CLOCK_RATE_HZ;
      channel_id = (timestamp_word >> 56) & 0x3F;
      is_end_of_slice = (timestamp_word & ((uint64_t)1 << 48));

      uint16_t wf_words_size = (wf_size_word & 0x7F);
      wf_words.resize(wf_words_size);

      for (uint16_t i = 0; i < wf_words_size; i++) {
        wf_words[i] = *buf++;
      }
    }
};

#endif
