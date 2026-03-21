#include <hydra.h>
#include <stdlib.h>
#include <stdio.h>

// See page 2-4
const unsigned int reg_index[16][4] = {
	{ R0,  R0,      R0,      R0      },
	{ R1,  R1,      R1,      R1      },
	{ R2,  R2,      R2,      R2      },
	{ R3,  R3,      R3,      R3      },
	{ R4,  R4,      R4,      R4      },
	{ R5,  R5,      R5,      R5      },
	{ R6,  R6,      R6,      R6      },
	{ R7,  R7,      R7,      R7      },
	{ R8,  R8_FIQ,  R8,      R8      },
	{ R9,  R9_FIQ,  R9,      R9      },
	{ R10, R10_FIQ, R10,     R10     },
	{ R11, R11_FIQ, R11,     R11     },
	{ R12, R12_FIQ, R12,     R12     },
	{ R13, R13_FIQ, R13_IRQ, R13_SVC },
	{ R14, R14_FIQ, R14_IRQ, R14_SVC },
	{ R15, R15,     R15,     R15     }
};

struct hydra_t {
	uint32_t reg[27];

    uint32_t pc;    // for convenience
    bool branch;

	//uint8_t idle;

	bool fetch_valid;
	bool decode_valid;
	bool execute_valid;

	uint32_t fetch;
	uint32_t decode;
	uint32_t execute;

	hydra_read8 read8;
	hydra_write8 write8;
	hydra_read32 read32;
	hydra_write32 write32;

};

hydra *hydra_new(hydra_read8 r8, hydra_write8 w8, hydra_read32 r32, hydra_write32 w32)
{
	hydra *h = (hydra *)malloc(sizeof(hydra));
	h->read8 = r8;
	h->write8 = w8;
	h->read32 = r32;
	h->write32 = w32;

	return h;
}

void hydra_destroy(hydra *h)
{
	free(h);
}

void hydra_reset(hydra *h)
{
	h->reg[R15] = 0x0c000003;
    // 0b000011'000000000000000000000000'11 in binary, NZCVIF'ProgramCounter'M1:M0
	// I and F bits set to prevent interrupts, processor in SVC mode (supervisor)
    // all other registers are undefined

	h->execute_valid = false;
	h->decode_valid = false;
	h->fetch_valid = false;

	//h->idle = 4;
}

void hydra_tick(hydra *h)
{
	// if (h->idle) {
	// 	h->idle--;
	// 	return;
	// }

	h->pc = h->reg[15] & 0x03fffffc;

	// Fetch
	h->fetch = h->read32(h->pc);
    h->fetch_valid = true;
	printf("pc:%08x  fetched:%08x  ", h->pc, h->fetch);

	// Decode
	if (h->decode_valid) {
        // yes, decode... does this actually do something in an emulator?
        printf("decode:%08x  ", h->decode);
    } else {
        printf("decode:--------  ");
    }

    // Always reset branch flag
    h->branch = false;

	// Execute
	if (h->execute_valid) {
        printf("execute: %08x\n", h->execute);
		// if condition is right... then
        hydra_execute(h);
	} else {
        printf("execute: --------\n");
    }

    // Only increase pc if branch flag not set
    if (h->branch == false) {
	    h->reg[R15] = (h->reg[R15] & 0xfc000003) | ((h->pc + 4) & 0x03fffffc);
    }

    // Move everything in the pipeline
	h->execute = h->decode;
	h->execute_valid = h->decode_valid;

    h->decode = h->fetch;
    h->decode_valid = h->fetch_valid;
}

void hydra_execute(hydra *h)
{
    if (((h->execute & 0x0e000000) >> 25) == 0b101) {
        h->branch = true;
		if (h->execute & 0x01000000) {
			// link register, fixme; need to subtract 4 before writing into R14
			h->reg[reg_index[14][h->reg[15] & 0b11]] = h->reg[R15];
		}
        h->pc += ((h->execute & 0xffffff) << 2);
	    h->reg[R15] = (h->reg[R15] & 0xfc000003) | (h->pc & 0x03fffffc);
        h->fetch_valid = false;
        h->decode_valid = false;
    }
}
