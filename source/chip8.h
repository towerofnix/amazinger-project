namespace chip8 {
  void test();
  void initialize();
  void loadProgram(unsigned short* buffer, int size);
  void gfx_buffer(bool* gfxBuffer);
  int emulateCycle();
}
