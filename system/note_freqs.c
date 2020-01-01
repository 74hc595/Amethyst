#include "note_freqs.h"

const note_def note_freqs[128] PROGMEM = {
  { .tccrb=_BV(WGM02), .ocra=0 }, /* 0 (C-2) */
  { .tccrb=_BV(WGM02), .ocra=0 }, /* 1 (C#-2) */
  { .tccrb=_BV(WGM02), .ocra=0 }, /* 2 (D-2) */
  { .tccrb=_BV(WGM02), .ocra=0 }, /* 3 (D#-2) */
  { .tccrb=_BV(WGM02), .ocra=0 }, /* 4 (E-2) */
  { .tccrb=_BV(WGM02), .ocra=0 }, /* 5 (F-2) */
  { .tccrb=_BV(WGM02), .ocra=0 }, /* 6 (F#-2) */
  { .tccrb=_BV(WGM02), .ocra=0 }, /* 7 (G-2) */
  { .tccrb=_BV(WGM02), .ocra=0 }, /* 8 (G#-2) */
  { .tccrb=_BV(WGM02), .ocra=0 }, /* 9 (A-2) */
  { .tccrb=_BV(WGM02), .ocra=0 }, /* 10 (A#-2) */
  { .tccrb=_BV(WGM02), .ocra=0 }, /* 11 (B-2) */
  { .tccrb=_BV(WGM02), .ocra=0 }, /* 12 (C-1) */
  { .tccrb=_BV(WGM02), .ocra=0 }, /* 13 (C#-1) */
  { .tccrb=_BV(WGM02), .ocra=0 }, /* 14 (D-1) */
  { .tccrb=_BV(WGM02), .ocra=0 }, /* 15 (D#-1) */
  { .tccrb=_BV(WGM02), .ocra=0 }, /* 16 (E-1) */
  { .tccrb=_BV(WGM02), .ocra=0 }, /* 17 (F-1) */
  { .tccrb=_BV(WGM02), .ocra=0 }, /* 18 (F#-1) */
  { .tccrb=_BV(WGM02), .ocra=0 }, /* 19 (G-1) */
  { .tccrb=_BV(WGM02), .ocra=0 }, /* 20 (G#-1) */
  { .tccrb=_BV(WGM02)|_BV(CS02)|_BV(CS00), .ocra=254 }, /* 21 (A-1), approx 27.525 Hz (0.09% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02)|_BV(CS00), .ocra=240 }, /* 22 (A#-1), approx 29.130 Hz (0.02% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02)|_BV(CS00), .ocra=226 }, /* 23 (B-1), approx 30.935 Hz (0.22% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02)|_BV(CS00), .ocra=214 }, /* 24 (C0), approx 32.670 Hz (0.10% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02)|_BV(CS00), .ocra=202 }, /* 25 (C#0), approx 34.610 Hz (0.11% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02)|_BV(CS00), .ocra=190 }, /* 26 (D0), approx 36.796 Hz (0.24% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02)|_BV(CS00), .ocra=180 }, /* 27 (D#0), approx 38.841 Hz (0.13% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02)|_BV(CS00), .ocra=170 }, /* 28 (E0), approx 41.125 Hz (0.19% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02)|_BV(CS00), .ocra=160 }, /* 29 (F0), approx 43.696 Hz (0.10% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02)|_BV(CS00), .ocra=151 }, /* 30 (F#0), approx 46.300 Hz (0.11% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02)|_BV(CS00), .ocra=143 }, /* 31 (G0), approx 48.890 Hz (0.22% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02)|_BV(CS00), .ocra=135 }, /* 32 (G#0), approx 51.787 Hz (0.24% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02)|_BV(CS00), .ocra=127 }, /* 33 (A0), approx 55.050 Hz (0.09% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02)|_BV(CS00), .ocra=120 }, /* 34 (A#0), approx 58.261 Hz (0.02% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02)|_BV(CS00), .ocra=113 }, /* 35 (B0), approx 61.870 Hz (0.22% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02)|_BV(CS00), .ocra=107 }, /* 36 (C1), approx 65.339 Hz (0.10% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02)|_BV(CS00), .ocra=101 }, /* 37 (C#1), approx 69.221 Hz (0.11% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02)|_BV(CS00), .ocra=95 }, /* 38 (D1), approx 73.593 Hz (0.24% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02)|_BV(CS00), .ocra=90 }, /* 39 (D#1), approx 77.681 Hz (0.13% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02)|_BV(CS00), .ocra=85 }, /* 40 (E1), approx 82.251 Hz (0.19% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02)|_BV(CS00), .ocra=80 }, /* 41 (F1), approx 87.391 Hz (0.10% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02)|_BV(CS00), .ocra=76 }, /* 42 (F#1), approx 91.991 Hz (0.55% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02)|_BV(CS00), .ocra=71 }, /* 43 (G1), approx 98.469 Hz (0.48% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02)|_BV(CS00), .ocra=67 }, /* 44 (G#1), approx 104.348 Hz (0.50% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02), .ocra=254 }, /* 45 (A1), approx 110.099 Hz (0.09% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02), .ocra=240 }, /* 46 (A#1), approx 116.522 Hz (0.02% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02), .ocra=226 }, /* 47 (B1), approx 123.740 Hz (0.22% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02), .ocra=214 }, /* 48 (C2), approx 130.679 Hz (0.10% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02), .ocra=202 }, /* 49 (C#2), approx 138.442 Hz (0.11% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02), .ocra=190 }, /* 50 (D2), approx 147.185 Hz (0.24% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02), .ocra=180 }, /* 51 (D#2), approx 155.362 Hz (0.13% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02), .ocra=170 }, /* 52 (E2), approx 164.501 Hz (0.19% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02), .ocra=160 }, /* 53 (F2), approx 174.782 Hz (0.10% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02), .ocra=151 }, /* 54 (F#2), approx 185.200 Hz (0.11% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02), .ocra=143 }, /* 55 (G2), approx 195.561 Hz (0.22% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02), .ocra=135 }, /* 56 (G#2), approx 207.150 Hz (0.24% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02), .ocra=127 }, /* 57 (A2), approx 220.198 Hz (0.09% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02), .ocra=120 }, /* 58 (A#2), approx 233.043 Hz (0.02% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02), .ocra=113 }, /* 59 (B2), approx 247.480 Hz (0.22% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02), .ocra=107 }, /* 60 (C3), approx 261.357 Hz (0.10% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02), .ocra=101 }, /* 61 (C#3), approx 276.883 Hz (0.11% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02), .ocra=95 }, /* 62 (D3), approx 294.371 Hz (0.24% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02), .ocra=90 }, /* 63 (D#3), approx 310.724 Hz (0.13% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02), .ocra=85 }, /* 64 (E3), approx 329.002 Hz (0.19% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02), .ocra=80 }, /* 65 (F3), approx 349.565 Hz (0.10% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02), .ocra=76 }, /* 66 (F#3), approx 367.963 Hz (0.55% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02), .ocra=71 }, /* 67 (G3), approx 393.876 Hz (0.48% error) */
  { .tccrb=_BV(WGM02)|_BV(CS02), .ocra=67 }, /* 68 (G#3), approx 417.391 Hz (0.50% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=254 }, /* 69 (A3), approx 440.397 Hz (0.09% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=240 }, /* 70 (A#3), approx 466.087 Hz (0.02% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=226 }, /* 71 (B3), approx 494.959 Hz (0.22% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=214 }, /* 72 (C4), approx 522.714 Hz (0.10% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=202 }, /* 73 (C#4), approx 553.766 Hz (0.11% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=190 }, /* 74 (D4), approx 588.741 Hz (0.24% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=180 }, /* 75 (D#4), approx 621.449 Hz (0.13% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=170 }, /* 76 (E4), approx 658.005 Hz (0.19% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=160 }, /* 77 (F4), approx 699.130 Hz (0.10% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=151 }, /* 78 (F#4), approx 740.800 Hz (0.11% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=143 }, /* 79 (G4), approx 782.243 Hz (0.22% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=135 }, /* 80 (G#4), approx 828.598 Hz (0.24% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=127 }, /* 81 (A4), approx 880.794 Hz (0.09% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=120 }, /* 82 (A#4), approx 932.173 Hz (0.02% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=113 }, /* 83 (B4), approx 989.919 Hz (0.22% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=107 }, /* 84 (C5), approx 1045.428 Hz (0.10% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=101 }, /* 85 (C#5), approx 1107.533 Hz (0.11% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=95 }, /* 86 (D5), approx 1177.482 Hz (0.24% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=90 }, /* 87 (D#5), approx 1242.898 Hz (0.13% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=85 }, /* 88 (E5), approx 1316.009 Hz (0.19% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=80 }, /* 89 (F5), approx 1398.260 Hz (0.10% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=76 }, /* 90 (F#5), approx 1471.853 Hz (0.55% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=71 }, /* 91 (G5), approx 1575.504 Hz (0.48% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=67 }, /* 92 (G#5), approx 1669.564 Hz (0.50% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=64 }, /* 93 (A5), approx 1747.825 Hz (0.69% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=60 }, /* 94 (A#5), approx 1864.347 Hz (0.02% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=57 }, /* 95 (B5), approx 1962.470 Hz (0.66% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=53 }, /* 96 (C6), approx 2110.581 Hz (0.84% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=50 }, /* 97 (C#6), approx 2237.216 Hz (0.89% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=48 }, /* 98 (D6), approx 2330.433 Hz (0.80% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=45 }, /* 99 (D#6), approx 2485.795 Hz (0.13% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=42 }, /* 100 (E6), approx 2663.352 Hz (1.00% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=40 }, /* 101 (F6), approx 2796.520 Hz (0.10% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=38 }, /* 102 (F#6), approx 2943.705 Hz (0.55% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=36 }, /* 103 (G6), approx 3107.244 Hz (0.92% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01)|_BV(CS00), .ocra=34 }, /* 104 (G#6), approx 3290.023 Hz (0.98% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01), .ocra=254 }, /* 105 (A6), approx 3523.175 Hz (0.09% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01), .ocra=240 }, /* 106 (A#6), approx 3728.693 Hz (0.02% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01), .ocra=226 }, /* 107 (B6), approx 3959.674 Hz (0.22% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01), .ocra=214 }, /* 108 (C7), approx 4181.712 Hz (0.10% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01), .ocra=202 }, /* 109 (C#7), approx 4430.131 Hz (0.11% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01), .ocra=190 }, /* 110 (D7), approx 4709.928 Hz (0.24% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01), .ocra=180 }, /* 111 (D#7), approx 4971.591 Hz (0.13% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01), .ocra=170 }, /* 112 (E7), approx 5264.038 Hz (0.19% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01), .ocra=160 }, /* 113 (F7), approx 5593.040 Hz (0.10% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01), .ocra=151 }, /* 114 (F#7), approx 5926.400 Hz (0.11% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01), .ocra=143 }, /* 115 (G7), approx 6257.947 Hz (0.22% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01), .ocra=135 }, /* 116 (G#7), approx 6628.788 Hz (0.24% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01), .ocra=127 }, /* 117 (A7), approx 7046.349 Hz (0.09% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01), .ocra=120 }, /* 118 (A#7), approx 7457.386 Hz (0.02% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01), .ocra=113 }, /* 119 (B7), approx 7919.348 Hz (0.22% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01), .ocra=107 }, /* 120 (C8), approx 8363.424 Hz (0.10% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01), .ocra=101 }, /* 121 (C#8), approx 8860.261 Hz (0.11% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01), .ocra=95 }, /* 122 (D8), approx 9419.857 Hz (0.24% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01), .ocra=90 }, /* 123 (D#8), approx 9943.182 Hz (0.13% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01), .ocra=85 }, /* 124 (E8), approx 10528.075 Hz (0.19% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01), .ocra=80 }, /* 125 (F8), approx 11186.080 Hz (0.10% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01), .ocra=76 }, /* 126 (F#8), approx 11774.821 Hz (0.55% error) */
  { .tccrb=_BV(WGM02)|_BV(CS01), .ocra=71 }, /* 127 (G8), approx 12604.033 Hz (0.48% error) */
};
