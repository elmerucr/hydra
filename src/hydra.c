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

hydra_t *hydra_new(hydra_read8 r8, hydra_write8 w8, hydra_read32 r32, hydra_write32 w32)
{
	hydra_t *t = (hydra_t *)malloc(sizeof(hydra_t));
	t->read8 = r8;
	t->write8 = w8;
	t->read32 = r32;
	t->write32 = w32;

	return t;
}

void hydra_destroy(hydra_t *t)
{
	free(t);
}

void hydra_reset(hydra_t *t)
{
	t->reg[R15] = 0x0c000003;
    // 0b000011'000000000000000000000000'11 in binary, NZCVIF'ProgramCounter'M1:M0
	// I and F bits set to prevent interrupts, processor in SVC mode (supervisor)
    // all other registers are undefined

	t->execute_valid = false;
	t->decode_valid = false;
	t->fetch_valid = false;

	//t->idle = 4;
}

void hydra_tick(hydra_t *t)
{
	// if (t->idle) {
	// 	t->idle--;
	// 	return;
	// }

	t->pc = t->reg[15] & 0x03fffffc;

	// Fetch
	t->fetch = t->read32(t->pc);
    t->fetch_valid = true;
	printf("pc:%08x  fetched:%08x  ", t->pc, t->fetch);

	// Decode
	if (t->decode_valid) {
        // yes, decode... does this actually do something in an emulator?
        printf("decode:%08x  ", t->decode);
    } else {
        printf("decode:--------  ");
    }

    // Always reset branch flag
    t->branch = false;

	// Execute
	if (t->execute_valid) {
        printf("execute: %08x\n", t->execute);
		// if condition is right... then
        hydra_execute(t);
	} else {
        printf("execute: --------\n");
    }

    // Only increase pc if branch flag not set
    if (t->branch == false) {
	    t->reg[R15] = (t->reg[R15] & 0xfc000003) | ((t->pc + 4) & 0x03fffffc);
    }

    // Move everything in the pipeline
	t->execute = t->decode;
	t->execute_valid = t->decode_valid;

    t->decode = t->fetch;
    t->decode_valid = t->fetch_valid;
}

void hydra_execute(hydra_t *t)
{
    if (((t->execute & 0x0e000000) >> 25) == 0b101) {
        t->branch = true;
		if (t->execute & 0x01000000) {
			// link register, fixme; need to subtract 4 before writing into R14
			t->reg[reg_index[14][t->reg[15] & 0b11]] = t->reg[R15];
		}
        t->pc += ((t->execute & 0xffffff) << 2);
	    t->reg[R15] = (t->reg[R15] & 0xfc000003) | (t->pc & 0x03fffffc);
        t->fetch_valid = false;
        t->decode_valid = false;
    }
}
