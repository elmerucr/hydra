#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <hydra.h>

uint8_t *ram;

uint8_t  read8(uint32_t address);
void     write8(uint32_t address, uint8_t value);
uint32_t read32(uint32_t address);
void     write32(uint32_t address, uint32_t value);

int main()
{
	ram = (uint8_t *)malloc(0x4000000); // 64mb

	hydra_t *cpu = hydra_new(read8, write8, read32, write32);

    write32(0x0, 0xe51f2008);
	write32(0x4, 0xe1a01002);
	write32(0x8, 0xeafffffc);
	write32(0xc, 0xdeadbeef);
	hydra_reset(cpu);

    for (int i=0; i<13; i++) {
        printf("cycle %2i:  ", i+1);
	    hydra_tick(cpu);
    }

	hydra_destroy(cpu);
	free(ram);
	return 0;
}

uint8_t read8(uint32_t address)
{
	return ram[address & 0x3ffffff];
}

void write8(uint32_t address, uint8_t value)
{
	ram[address & 0x3ffffff] = value;
}

uint32_t read32(uint32_t address)
{
	return
		(ram[(address + 3) & 0x3ffffff] << 24) |
		(ram[(address + 2) & 0x3ffffff] << 16) |
		(ram[(address + 1) & 0x3ffffff] <<  8) |
		(ram[(address + 0) & 0x3ffffff] <<  0) ;
}

void write32(uint32_t address, uint32_t value)
{
	ram[(address + 0) & 0x3ffffff] = (value >>  0) & 0xff;
	ram[(address + 1) & 0x3ffffff] = (value >>  8) & 0xff;
	ram[(address + 2) & 0x3ffffff] = (value >> 16) & 0xff;
	ram[(address + 3) & 0x3ffffff] = (value >> 24) & 0xff;
}
