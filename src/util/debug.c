

void debugPrintFloat(int x, int y, char* format, float value) {
  char conbuf[100];
  nuDebConTextPos(0, x, y);
  sprintf(conbuf, format, value);
  nuDebConCPuts(0, conbuf);
}