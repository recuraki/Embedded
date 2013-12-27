/*a
 MPL115A2: Calc Test Code
 Ref: http://garretlab.web.fc2.com/arduino/lab/barometer_sensor/index.html
 */
float read_coefficients2(int total_bits, int fractional_bits, int zero_pad,
unsigned char lsb, unsigned char msb) {
  
  return ((float) ((msb << 8) + lsb) / ((long)1 << 16 - total_bits + fractional_bits + zero_pad));
}
float read_coefficients(int total_bits, int fractional_bits, int zero_pad,
unsigned char msb, unsigned char lsb) {
  
  return ((float) ((msb << 8) + lsb) / ((long)1 << 16 - total_bits + fractional_bits + zero_pad));
}

main()
{
  float a0, b1, b2, c12, c11, c22;
  unsigned int Padc;
  unsigned int Tadc;
  unsigned char msb, lsb;


  msb = 0x52;
  lsb = 0x00;
  Padc =  (((unsigned int)msb << 8) + lsb) >> 6;
  msb = 0x81;
  lsb = 0xc0;
  Tadc =  (((unsigned int)msb << 8) + lsb) >> 6;

  a0 = read_coefficients(16, 3, 0, 0x3a, 0xa0);
  b1 = read_coefficients(16, 13, 0, 0xba, 0xc0);
  b2 = read_coefficients(16, 14, 0, 0xc0, 0x53);
  c12 = read_coefficients(14, 13, 9, 0x2e, 0xc4);
  c11 = read_coefficients(11, 10, 11, 0x00, 0x00);
  c22 = read_coefficients(11, 10, 15, 0x00, 0x00);
  float Pcomp = a0 + (b1 + c11 * Padc + c12 * Tadc) * Padc + (b2 + c22 * Tadc) * Tadc;
  float hpa = Pcomp * 650 / 1023 + 500;
  printf("%f\n", hpa);

  msb = 0x00;
  lsb = 0x52;
  Padc =  (((unsigned int)msb << 8) + lsb) >> 6;
  msb = 0xc0;
  lsb = 0x81;
  Tadc =  (((unsigned int)msb << 8) + lsb) >> 6;
  a0 = read_coefficients2(16, 3, 0, 0x3a, 0xa0);
  b1 = read_coefficients2(16, 13, 0, 0xba, 0xc0);
  b2 = read_coefficients2(16, 14, 0, 0xc0, 0x53);
  c12 = read_coefficients2(14, 13, 9, 0x2e, 0xc4);
  c11 = read_coefficients2(11, 10, 11, 0x00, 0x00);
  c22 = read_coefficients2(11, 10, 15, 0x00, 0x00);
   Pcomp = a0 + (b1 + c11 * Padc + c12 * Tadc) * Padc + (b2 + c22 * Tadc) * Tadc;
   hpa = Pcomp * 650 / 1023 + 500;
  printf("%f\n", hpa);
}

