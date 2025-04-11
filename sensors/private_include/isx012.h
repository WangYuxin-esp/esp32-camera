/*
 *
 * ISX012 DVP driver.
 *
 */
#ifndef __ISX012_H__
#define __ISX012_H__

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
int isx012_detect(int slv_addr, sensor_id_t *id);

/**
 * @brief initialize sensor function pointers
 *
 * @param sensor pointer of sensor
 * @return
 *      Always 0
 */
int isx012_init(sensor_t *sensor);

#endif // __ISX012_H__
