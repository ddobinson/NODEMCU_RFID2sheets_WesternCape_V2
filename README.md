# NODEMCU_RFID2sheets_WesternCape_V2
NodeMCU upload RFID data to sheets


//https://www.youtube.com/watch?v=ON7neIqPC2A
/*
              RFID    NODEMCU   BLUEBOARD   RED/GREEN BOARD    5110 LCD(can't use with Nodemcu)
                        3.3v        raw       raw
              3.3v      vin      vcc      vcc
              RST       D0        D9        A1                 RST D7
              GND       GND       GND       GND                DIN D5
              MISO      D6        D12       D12                DC D6
              MOSI      D7        D11       D11                CE D8
              SCK       D5        D13       D13                CLK D4
              SDA       D4        D10       D10
  topled                D3        D3        D3
  bottom led            D2        A0        A0
  buzzer                D8        D2        D2
  config                D?        A3        A3
  status led            D1        D1        D
*/
