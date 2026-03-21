#ifndef HYDRA_H
#define HYDRA_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HYDRA_MAJOR_VERSION  0
#define HYDRA_MINOR_VERSION  0
#define HYDRA_RELEASE_DATE   20260321

typedef uint8_t (*hydra_read8)(uint32_t addr);
typedef uint32_t (*hydra_read32)(uint32_t addr);
typedef void(*hydra_write8)(uint32_t address, uint8_t value);
typedef void(*hydra_write32)(uint32_t address, uint32_t value);

// Page 5-2
enum processor_modes {
	USR_MODE = 0b00,
	FIQ_MODE = 0b01,
	IRQ_MODE = 0b10,
	SVC_MODE = 0b11
};

enum register_names {
	R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, R15,
	R8_FIQ, R9_FIQ, R10_FIQ, R11_FIQ, R12_FIQ, R13_FIQ, R14_FIQ,
	R13_IRQ, R14_IRQ,
	R13_SVC, R14_SVC
};

struct hydra_t;
typedef struct hydra_t hydra;

hydra *hydra_new(hydra_read8 r8, hydra_write8 w8, hydra_read32 r32, hydra_write32 w32);
void hydra_destroy(hydra *h);
void hydra_reset(hydra *h);
void hydra_tick(hydra *h);
void hydra_execute(hydra *h);

#ifdef __cplusplus
}
#endif

#endif
