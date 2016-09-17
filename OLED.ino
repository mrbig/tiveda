
void initDisplay()  {
  Wire.begin(0, 2);
  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextWrap(false);
  display.setRotation(0);
  display.setCursor(30, 20);
  display.setTextSize(2);
  display.print("TIVEDA");
  display.setTextSize(1);
  display.display();
  delay(5000);
  display.clearDisplay();
  display.display();
}

