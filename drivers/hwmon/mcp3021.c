/*
 * mcp3021.c - driver for the Microchip MCP3021 chip
 *
 * Copyright (C) 2008-2009, 2011 Freescale Semiconductor, Inc.
 * Author: Mingkai Hu <Mingkai.hu@freescale.com>
 *
 * This driver export the value of analog input voltage to sysfs, the
 * voltage unit is mV. Through the sysfs interface, lm-sensors tool
 * can also display the input voltage.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/device.h>

/*
 * MCP3021 macros
 */
/* Vdd info */
#define MCP3021_VDD_MAX		5500
#define MCP3021_VDD_MIN		2700
#define MCP3021_VDD_REF		3300

/* output format */
#define MCP3021_SAR_SHIFT	2
#define MCP3021_SAR_MASK	0x3ff

#define MCP3021_OUTPUT_RES	10	/* 10-bit resolution */
#define MCP3021_OUTPUT_SCALE	4
#define MCP3021_OUTPUT_MAX_VAL	((1 << MCP3021_OUTPUT_RES) - 1)

/*
 * Client data (each client gets its own)
 */
struct mcp3021_data {
	struct device *hwmon_dev;
	struct mutex update_lock;	/* protect register access */

	/* Register values */
	u16 in_input;
};

/*
 * Driver data (common to all clients)
 */
static int mcp3021_probe(struct i2c_client *client,
				const struct i2c_device_id *id);
static int mcp3021_remove(struct i2c_client *client);

enum chips { mcp3021 };

static int mcp3021_read16(struct i2c_client *client, u16 *data)
{
	int ret;
	int reg;
	uint8_t buf[2];

	ret = i2c_master_recv(client, buf, 2);
	if (ret != 2)
		return ret;

	/* The output code of the MCP3021 is transmitted with MSB first. */
	reg = (buf[0] << 8) | buf[1];

	/* The ten-bit output code is composed of the lower 4-bit of the
	 * first byte and the upper 6-bit of the second byte.
	 */
	reg = (reg >> MCP3021_SAR_SHIFT) & MCP3021_SAR_MASK;
	*data = reg;

	return 0;
}

/*
 * The basic transfer function is volts = vdd * val / 1024, except
 * that at val's limits (0 and 1023), val should have 0.25 added to it.
 * We scale val by a factor of four so we can add one instead of 0.25.
 */
static inline u16 volts_from_reg(u16 vdd, u16 val)
{
	val = val * MCP3021_OUTPUT_SCALE;

	if ((val == 0) ||
	    (val == MCP3021_OUTPUT_MAX_VAL * MCP3021_OUTPUT_SCALE))
		val++;

	return  val * vdd / ((1 << MCP3021_OUTPUT_RES)
			* MCP3021_OUTPUT_SCALE);
}

static struct mcp3021_data *mcp3021_update_device(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mcp3021_data *data = i2c_get_clientdata(client);
	u16 vdd = MCP3021_VDD_REF;

	mutex_lock(&data->update_lock);
	mcp3021_read16(client, &data->in_input);
	data->in_input = volts_from_reg(vdd, data->in_input);
	mutex_unlock(&data->update_lock);

	return data;
}

static ssize_t show_in_input(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct mcp3021_data *data = mcp3021_update_device(dev);
	return sprintf(buf, "%d\n", data->in_input);
}

static DEVICE_ATTR(in0_input, S_IRUGO, show_in_input, NULL);

static struct attribute *mcp3021_attributes[] = {
	&dev_attr_in0_input.attr,
	NULL
};

static const struct attribute_group mcp3021_group = {
	.attrs = mcp3021_attributes,
};

static int mcp3021_check_status(struct i2c_client *client)
{
	u16 reg;
	return  mcp3021_read16(client, &reg);
}

static int mcp3021_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	int err;
	struct mcp3021_data *data = NULL;

	if (mcp3021_check_status(client))
		return -ENXIO;
	if (!i2c_check_functionality(client->adapter,
			I2C_FUNC_SMBUS_WORD_DATA))
		return -EIO;

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	i2c_set_clientdata(client, data);

	mutex_init(&data->update_lock);

	err = sysfs_create_group(&client->dev.kobj, &mcp3021_group);
	if (err)
		goto exit_free;

	data->hwmon_dev = hwmon_device_register(&client->dev);
	if (IS_ERR(data->hwmon_dev)) {
		err = PTR_ERR(data->hwmon_dev);
		goto exit_remove;
	}

	return 0;

exit_remove:
	sysfs_remove_group(&client->dev.kobj, &mcp3021_group);
exit_free:
	kfree(data);
	i2c_set_clientdata(client, NULL);
	return err;
}

static int mcp3021_remove(struct i2c_client *client)
{
	struct mcp3021_data *data = i2c_get_clientdata(client);

	hwmon_device_unregister(data->hwmon_dev);
	sysfs_remove_group(&client->dev.kobj, &mcp3021_group);
	kfree(data);
	i2c_set_clientdata(client, NULL);

	return 0;
}

static const struct i2c_device_id mcp3021_id[] = {
	{ "mcp3021", mcp3021 },
	{ }
};

static struct i2c_driver mcp3021_driver = {
	.driver = {
		.name = "mcp3021",
	},
	.probe = mcp3021_probe,
	.remove = mcp3021_remove,
	.id_table = mcp3021_id,
};

static int __init mcp3021_init(void)
{
	return i2c_add_driver(&mcp3021_driver);
}

static void __exit mcp3021_exit(void)
{
	i2c_del_driver(&mcp3021_driver);
}

MODULE_AUTHOR("Mingkai Hu <Mingkai.hu@freescale.com>");
MODULE_DESCRIPTION("Microchip MCP3021 driver");
MODULE_LICENSE("GPL");

module_init(mcp3021_init);
module_exit(mcp3021_exit);
