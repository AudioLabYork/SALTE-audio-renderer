#pragma once

/* hrir measurements from SADIE database
 * Non-Commercial Licence
 * 1. This software ("Software") is owned by the University of York (the "University").
 * 2. The Software is provided to you free of charge subject to the conditions contained in this Licence.
 * 3. You may only use the Software for non-commercial purposes.
 * 4. You may only distribute this Software if you (i) do so without charge or other financial benefit and (ii) include a copy of this licence.
 * 5. You must not remove any copyright notices from the Software.
 * 6. This Software is provided by the University on an "as-is" and "as available" basis. The University excludes all representations, warranties, and conditions (whether express or implied), including, those relating to quality, fitness for any purpose, and non-infringement of third party rights.
 * 7. To the fullest extent permitted by law, in no event shall the University be liable under this licence for (i) any indirect, incidental, special, exemplary, punitive, consequential damages, or (ii) any loss of data, loss of use, or loss of profits.
 * 8. This licence is governed by the laws of England and Wales.


 * Armstrong, C.; Thresh, L.; Murphy, D.; Kearney, G. A Perceptual Evaluation of Individual and Non-Individual HRTFs: A Case Study of the SADIE II Database. Appl. Sci. 2018, 8, 2029.
 */

// octo virtual loudspeaker positions
extern const float octo[2][6];

// icos virtual loudspeaker positions
extern const float icos[2][12];

// leb26 virtual loudspeaker positions
extern const float leb26[2][26];

// leb50 virtual loudspeaker positions
extern const float leb50[2][50];

extern const int sHrirIndexOrder1[6];
extern const int sHrirIndexOrder2[12];
extern const int sHrirIndexOrder3[26];
extern const int sHrirIndexOrder5[50];

extern const float sDefaultHrirs[60][2][256];

// octo low band, high band matrices
extern const float octo_lo_dec_mat[24];
extern const float octo_hi_dec_mat[24];

// icos low band, high band matrices
extern const float icos_lo_dec_mat[108];
extern const float icos_hi_dec_mat[108];

// leb26 low band, high band matrices
extern const float leb26_lo_dec_mat[416];
extern const float leb26_hi_dec_mat[416];

// leb50 low band, high band matrices
extern const float leb50_lo_dec_mat[1800];
extern const float leb50_hi_dec_mat[1800];

// order 1 low band, high band filters Fc = 757.93Hz
extern const float order_1_lo_band_48[257];
extern const float order_1_hi_band_48[257];

// order 2 low band, high band filters Fc = 1429.17Hz
extern const float order_2_lo_band_48[257];
extern const float order_2_hi_band_48[257];

// order 3 low band, high band filters Fc = 2100.71Hz
extern const float order_3_lo_band_48[257];
extern const float order_3_hi_band_48[257];

// order 5 low band, high band filters Fc = 3451.17Hz
extern const float order_5_lo_band_48[257];
extern const float order_5_hi_band_48[257];
