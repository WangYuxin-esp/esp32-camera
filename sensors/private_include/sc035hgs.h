/*
 *
 * SC035HGS DVP driver.
 *
 */
#ifndef __SC035HGS_H__
#define __SC035HGS_H__

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
int sc035hgs_detect(int slv_addr, sensor_id_t *id);

/**
 * @brief initialize sensor function pointers
 *
 * @param sensor pointer of sensor
 * @return
 *      Always 0
 */
int sc035hgs_init(sensor_t *sensor);

#endif // __SC035HGS_H__
