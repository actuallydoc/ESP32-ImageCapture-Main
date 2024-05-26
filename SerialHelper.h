// Pošlji paket preko UART SC16IS752 na kanalu B
void posljiPaket() {
    I2CExpander.write(SC16IS752_CHANNEL_B, String("START\n"));
    ZAJEMI = false;
    renderButton(6, false);
    delay(500);
}
