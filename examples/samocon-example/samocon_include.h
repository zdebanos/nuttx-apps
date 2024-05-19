/* samocon_include.h */

#include "adcs.h"

/* calibrate_adcs.c */
int calibrate_adcs(int adc_n);

/* bldc_commutate.c */
int commutate_b_example(void);

/* halls.c */
int hall_example(void);

/* i2c.c */
int i2c_eeprom_example(void);

/* pwms.c */
int pwm0_example(void);
int pwm1_example(void);

/* rt_task.c */
void *rt_task(void *p);
void rt_task_endme(int n);

