#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <nuttx/analog/adc.h>

struct adc_lookup_s
{
  uint8_t phase;   /* the number */
  bool high;       /* true if hss, false otherwise */
};

int32_t adc0_channel_lookup(struct adc_msg_s msgs[], 
                            size_t channels,
                            struct adc_lookup_s look);


int32_t adc1_channel_lookup(struct adc_msg_s msgs[], 
                             size_t channels,
                             struct adc_lookup_s look);

int afec0_example(void);

