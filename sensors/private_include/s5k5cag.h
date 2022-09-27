/*
 *
 * S5K5CAG DVP driver.
 *
 */
#ifndef __S5K5CAG_H__
#define __S5K5CAG_H__

#include "sensor.h"

/**
 * @brief Detect sensor pid
 *
 * @param slv_addr SCCB address
 * @param id Detection result
 * @return
 *     0:       Can't detect this sensor
 *     Nonzero: This sensor has been detected
 */
int s5k5cag_detect(int slv_addr, sensor_id_t *id);

/**
 * @brief initialize sensor function pointers
 *
 * @param sensor pointer of sensor
 * @return
 *      Always 0
 */
int s5k5cag_init(sensor_t *sensor);

#endif // __S5K5CAG_H__