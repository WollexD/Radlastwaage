#ifndef CONFIG_H
#define CONFIG_H

uint8_t MasterAddress[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t LVAddress[]     = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t LHAddress[]     = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t RVAddress[]     = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t RHAddress[]     = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// Array aus Pointern auf die bestehenden Arrays
uint8_t* deviceAddresses[] = {LVAddress, LHAddress, RVAddress, RHAddress, MasterAddress};

// Optionale Namen für besseren Zugriff
enum DeviceIndex {
    LV     = 0,
    LH     = 1,
    RV     = 2,
    RH     = 3,
    MASTER = 4
};

#endif
