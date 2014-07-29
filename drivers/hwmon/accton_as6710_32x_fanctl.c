/*
 * A hwmon driver for the Accton as6710 32x fan contrl
 *
 * Copyright (C) 2013 Accton Technology Corporation.
 * Brandon Chuang <brandon_chuang@accton.com.tw>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/string.h>

static void accton_as6710_32x_fan_update_device(struct device *dev);
static ssize_t fan_show_value(struct device *dev, struct device_attribute *da, char *buf);
static ssize_t fan_set_value(struct device *dev, struct device_attribute *da,
                                    const char *buf, size_t count);

extern int accton_i2c_cpld_read(unsigned short cpld_addr, u8 reg);
extern int accton_i2c_cpld_write(unsigned short cpld_addr, u8 reg, u8 value);

#define FAN_SPEED_CTRL_INTERVAL 3000

#define PSU1_AIRFLOW_PATH       "/sys/bus/i2c/devices/5-0058/psu_fan_dir"
#define PSU2_AIRFLOW_PATH       "/sys/bus/i2c/devices/6-005b/psu_fan_dir"
#define THERMAL_SENSOR1_PATH    "/sys/bus/i2c/devices/8-0048/temp1_input"
#define THERMAL_SENSOR4_PATH    "/sys/bus/i2c/devices/11-004b/temp1_input"

static struct accton_as6710_32x_fan  *fan_data = NULL;

/* Fan speed control related data
 */
enum fan_duty_cycle {
    FAN_DUTY_CYCLE_MIN = 40,
	FAN_DUTY_CYCLE_65  = 65,
	FAN_DUTY_CYCLE_80  = 80,
	FAN_DUTY_CYCLE_MAX = 100
};

typedef struct fan_ctrl_policy {
   enum fan_duty_cycle duty_cycle;
   int cpld_val;
   int temp_down_adjust; /* The boundary temperature to down adjust fan speed */
   int temp_up_adjust;   /* The boundary temperature to up adjust fan speed */
} fan_ctrl_policy_t;

enum fan_direction {
    FAN_DIR_F2B,
    FAN_DIR_B2F
};

fan_ctrl_policy_t  fan_ctrl_policy_f2b[] = {
{FAN_DUTY_CYCLE_MIN, 0x08,     0,  47950},
{FAN_DUTY_CYCLE_65,  0x0d, 42050,  49650},
{FAN_DUTY_CYCLE_80,  0x10, 47050,  54050},
{FAN_DUTY_CYCLE_MAX, 0x14, 52050,      0}
};

fan_ctrl_policy_t  fan_ctrl_policy_b2f[] = {
{FAN_DUTY_CYCLE_MIN, 0x08,     0,  40850},
{FAN_DUTY_CYCLE_65,  0x0d, 35400,  44850},
{FAN_DUTY_CYCLE_80,  0x10, 40400,  47400},
{FAN_DUTY_CYCLE_MAX, 0x14, 45400,      0}
};

struct accton_as6710_32x_fan {
    struct device   *dev;
	struct mutex     update_lock;
    char             valid;           /* != 0 if registers are valid */
    unsigned long    last_updated;    /* In jiffies */
    u8               reg_val[2];     /* Register value, 0 = fan fault status
                                                        1 = fan PWM duty cycle */
    struct task_struct *fanctrl_tsk;
	struct completion	fanctrl_update_stop;
	unsigned int		fanctrl_update_interval;
    enum fan_direction  fandir;
    u8  fanctrl_disabled;
    int temp_input[2];  /* The temperature read from thermal sensor(lm75) */
};

/* fan related data
 */
static const u8 fan_reg[] = {
    0xC,        /* fan fault */
    0xD,        /* fan PWM duty cycle */
};

enum sysfs_fan_attributes {
    FAN_FAULT = 0,
    FAN_DUTY_CYCLE,
    FAN_DISABLE_FANCTRL
};
                    
static SENSOR_DEVICE_ATTR(fan_fault, S_IRUGO, fan_show_value, NULL, FAN_FAULT);
static SENSOR_DEVICE_ATTR(fan_duty_cycle, S_IWUSR | S_IRUGO, fan_show_value, 
                                           fan_set_value, FAN_DUTY_CYCLE);
static SENSOR_DEVICE_ATTR(fan_disable_fanctrl, S_IWUSR | S_IRUGO, fan_show_value, 
                                           fan_set_value, FAN_DISABLE_FANCTRL);

static struct attribute *accton_as6710_32x_fan_attributes[] = {
    /* fan related attributes */
    &sensor_dev_attr_fan_fault.dev_attr.attr,
    &sensor_dev_attr_fan_duty_cycle.dev_attr.attr,
    &sensor_dev_attr_fan_disable_fanctrl.dev_attr.attr,
    NULL
};

static int accton_as6710_32x_fan_read_value(u8 reg)
{
    return accton_i2c_cpld_read(0x60, reg);
}

static int accton_as6710_32x_fan_write_value(u8 reg, u8 value)
{
    return accton_i2c_cpld_write(0x60, reg, value);
}

/* fan related functions
 */
static int fan_reg_val_to_duty_cycle(u8 cpld_val) 
{
    int i, arr_size;
    fan_ctrl_policy_t *policy;

    if (FAN_DIR_F2B == fan_data->fandir) {
        policy   = fan_ctrl_policy_f2b;
        arr_size = ARRAY_SIZE(fan_ctrl_policy_f2b);
    }
    else {
        policy   = fan_ctrl_policy_b2f;
        arr_size = ARRAY_SIZE(fan_ctrl_policy_b2f);
    }
    
    for (i = 0; i < arr_size; i++) {
        if (policy[i].cpld_val == cpld_val)
            return policy[i].duty_cycle;
    }
    
    return policy[0].duty_cycle;
}

static int fan_duty_cycle_to_reg_val(u8 duty_cycle) 
{
    int i, arr_size;
    fan_ctrl_policy_t *policy;

    if (FAN_DIR_F2B == fan_data->fandir) {
        policy   = fan_ctrl_policy_f2b;
        arr_size = ARRAY_SIZE(fan_ctrl_policy_f2b);
    }
    else {
        policy   = fan_ctrl_policy_b2f;
        arr_size = ARRAY_SIZE(fan_ctrl_policy_b2f);
    }
    
    for (i = 0; i < arr_size; i++) {
        if (policy[i].duty_cycle == duty_cycle)
            return policy[i].cpld_val;
    }
    
    return policy[0].cpld_val;
}

static ssize_t fan_show_value(struct device *dev, struct device_attribute *da,
             char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    ssize_t ret = 0;
    
    accton_as6710_32x_fan_update_device(dev);
    
	if (fan_data->valid) {
		switch (attr->index) {
		case FAN_FAULT:
			ret = sprintf(buf, "%d\n", fan_data->reg_val[FAN_FAULT] ? 1 : 0);
			break;
		case FAN_DUTY_CYCLE:
			ret = sprintf(buf, "%d\n", fan_reg_val_to_duty_cycle(fan_data->reg_val[FAN_DUTY_CYCLE]));
			break;
        case FAN_DISABLE_FANCTRL:
            ret = sprintf(buf, "%d\n", fan_data->fanctrl_disabled);
            break;
		default:
			break;
		}
	}
    
    return ret;
}

static ssize_t fan_set_value(struct device *dev, struct device_attribute *da,
            const char *buf, size_t count) {
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    int error, value;
    
    error = kstrtoint(buf, 10, &value);
    if (error)
        return error;

    switch (attr->index) {
    case FAN_DUTY_CYCLE:
    {
        int reg_val = fan_duty_cycle_to_reg_val(value);

        if (value < 0 || value > FAN_DUTY_CYCLE_MAX)
    	    return -EINVAL;

        mutex_lock(&fan_data->update_lock);
        fan_data->reg_val[FAN_DUTY_CYCLE] = reg_val;
        accton_as6710_32x_fan_write_value(fan_reg[FAN_DUTY_CYCLE], reg_val);
        mutex_unlock(&fan_data->update_lock);
        break;
    }
    case FAN_DISABLE_FANCTRL:
        fan_data->fanctrl_disabled = value ? 1 : 0;
        break;
    default:
        break;
    }
    
    return count;
}

static const struct attribute_group accton_as6710_32x_fan_group = {
    .attrs = accton_as6710_32x_fan_attributes,
};

static int read_file_contents(char *path, char *buf, long data_len)
{
    int ret = 0;
    struct file *fp = NULL;

    fp = filp_open(path, O_RDONLY, 0);

    if (IS_ERR(fp)) {
        ret = PTR_ERR(fp);
        dev_dbg(fan_data->dev, "Failed to open file(%s)\n", path);
        goto exit;
    }

    if (kernel_read(fp, 0, buf, data_len) != data_len) {
        ret = -EIO;
        dev_dbg(fan_data->dev, "Failed to read file(%s)\n", path);
		goto exit_close;
	}

exit_close:
    filp_close(fp, NULL);
exit:
    return ret;
}

static void accton_as6710_32x_fan_update_device(struct device *dev)
{
    mutex_lock(&fan_data->update_lock);

    if (time_after(jiffies, fan_data->last_updated + HZ + HZ / 2) || 
        !fan_data->valid) {
        int i;
        char airflow[5];
        char temp[2][6];

        dev_dbg(fan_data->dev, "Starting accton_as6710_32x_fan update\n");
        fan_data->valid = 0;

        /* Update fan duty cycle and failure state
         */
        for (i = 0; i < ARRAY_SIZE(fan_data->reg_val); i++) {
            int status = accton_as6710_32x_fan_read_value(fan_reg[i]);
			
            if (status < 0) {
                dev_dbg(fan_data->dev, "Failed to read fan reg %d, err %d\n", fan_reg[i], status);
				goto exit;
            }
            else {
                fan_data->reg_val[i] = status;
            }
        }

        /* Update fan airflow direction
         */
        if (read_file_contents(PSU1_AIRFLOW_PATH, airflow, sizeof(airflow)-1) == 0 || 
            read_file_contents(PSU2_AIRFLOW_PATH, airflow, sizeof(airflow)-1) == 0  )
        {
            airflow[sizeof(airflow)-1] = '\0';
            fan_data->fandir = (strncmp(&airflow[1], "F2B", strlen("F2B")) == 0) ? 
                               FAN_DIR_F2B : FAN_DIR_B2F;
        }
        else
        {
            dev_dbg(fan_data->dev, "Failed to read fan airflow direction\n");
            goto exit;
        }

        /* Update temperature
         */
        if (read_file_contents(THERMAL_SENSOR1_PATH, temp[0], sizeof(temp[0])) == 0 && 
            read_file_contents(THERMAL_SENSOR4_PATH, temp[1], sizeof(temp[1])) == 0  )
        {
            temp[0][sizeof(temp[0])-1] = '\0';
            temp[1][sizeof(temp[1])-1] = '\0';
            
            if (kstrtoint(temp[0], 10, &fan_data->temp_input[0]) != 0 ||
                kstrtoint(temp[1], 10, &fan_data->temp_input[1]) != 0  ) 
            {
                dev_dbg(fan_data->dev, "Failed to convert temperature read from lm75\n");
                goto exit;
            }
        }
        else
        {
            dev_dbg(fan_data->dev, "Failed to read temperature from lm75\n");
            goto exit;
        }

        fan_data->last_updated = jiffies;
        fan_data->valid = 1;
    }

exit:
	mutex_unlock(&fan_data->update_lock);
}

static void set_fan_speed_by_temp(int duty_cycle, int temperature) {  
    int  i, arr_size, new_duty_cycle, reg_val;
    fan_ctrl_policy_t *policy;

    if (FAN_DIR_F2B == fan_data->fandir) {
        policy   = fan_ctrl_policy_f2b;
        arr_size = ARRAY_SIZE(fan_ctrl_policy_f2b);
    }
    else {
        policy   = fan_ctrl_policy_b2f;
        arr_size = ARRAY_SIZE(fan_ctrl_policy_b2f);
    }

	for (i = 0; i < arr_size; i++) {
	    if (policy[i].duty_cycle != duty_cycle)
		    continue;
			
		break;
	}

	if (i == arr_size) {
        dev_dbg(fan_data->dev, "no matched duty cycle (%d)\n", duty_cycle);
		return;
	}
	
	/* Adjust new duty cycle
	 */
    new_duty_cycle = duty_cycle;
    
    if ((temperature >= policy[i].temp_up_adjust) && (duty_cycle != FAN_DUTY_CYCLE_MAX)) {
	    new_duty_cycle = policy[i+1].duty_cycle;
	}
	else if ((temperature <= policy[i].temp_down_adjust) && (duty_cycle != FAN_DUTY_CYCLE_MIN)) {
	    new_duty_cycle = policy[i-1].duty_cycle;
	}
	
	if (new_duty_cycle == duty_cycle) {
        /* Duty cycle does not change, just return */
	    return;
	}
	
	/* Update duty cycle
	 */
	reg_val = fan_duty_cycle_to_reg_val(new_duty_cycle);
    
    mutex_lock(&fan_data->update_lock);
    fan_data->reg_val[FAN_DUTY_CYCLE] = reg_val;
    accton_as6710_32x_fan_write_value(fan_reg[FAN_DUTY_CYCLE], reg_val);
    mutex_unlock(&fan_data->update_lock);
}

static int fan_speed_ctrl_routine(void *arg)
{
    while (!kthread_should_stop()) 
    {
		int duty_cycle, temp = 0;

        msleep(fan_data->fanctrl_update_interval);

        if (fan_data->fanctrl_disabled) {
            continue;
        }
        
        accton_as6710_32x_fan_update_device(fan_data->dev);

		/* Set fan speed as max if one of the following state occurs:
		 * 1. Invalid fan data
		 * 2. Any fan is in failed state
		 */
		if (!fan_data->valid || fan_data->reg_val[FAN_FAULT]) {
            int reg_val = fan_duty_cycle_to_reg_val(FAN_DUTY_CYCLE_MAX);

            mutex_lock(&fan_data->update_lock);
            fan_data->reg_val[FAN_DUTY_CYCLE] = reg_val;
			accton_as6710_32x_fan_write_value(fan_reg[FAN_DUTY_CYCLE], reg_val);
            mutex_unlock(&fan_data->update_lock);
			continue;
		}
		
		/* Set fan speed by current duty cycle and temperature
		 */
		temp = (fan_data->temp_input[0] + fan_data->temp_input[1]) / 2;
		duty_cycle = fan_reg_val_to_duty_cycle(fan_data->reg_val[FAN_DUTY_CYCLE]);
		set_fan_speed_by_temp(duty_cycle, temp);
	}

    complete_all(&fan_data->fanctrl_update_stop);
	
	return 0;
}

static int __init accton_as6710_32x_fan_init(void)
{
    int status = 0;

    fan_data = kzalloc(sizeof(struct accton_as6710_32x_fan), GFP_KERNEL);
    if (!fan_data) {
        status = -ENOMEM;
        goto exit;
    }
	
	mutex_init(&fan_data->update_lock);

    /* Register sysfs hooks */
    fan_data->dev = hwmon_device_register(NULL);
    if (IS_ERR(fan_data->dev)) {
        goto exit_free;
    }
    
    status = sysfs_create_group(&fan_data->dev->kobj, &accton_as6710_32x_fan_group);
    if (status) {
        goto exit_unregister;
    }

    dev_info(fan_data->dev, "accton_as6710_32x_fan\n");
    
	/* initialize fan speed control routine */
    init_completion(&fan_data->fanctrl_update_stop);
    fan_data->fanctrl_update_interval = FAN_SPEED_CTRL_INTERVAL;
    fan_data->fanctrl_tsk = kthread_run(fan_speed_ctrl_routine, NULL, "accton_as6710_32x_fanctl");
    
	if (IS_ERR(fan_data->fanctrl_tsk)) {
        status = PTR_ERR(fan_data->fanctrl_tsk);
        goto exit_removegroup;
    }
	
    return 0;

exit_removegroup:
    sysfs_remove_group(&fan_data->dev->kobj, &accton_as6710_32x_fan_group);
exit_unregister:
    hwmon_device_unregister(fan_data->dev);
exit_free:
    kfree(fan_data);	
exit:
    return status;
}

static void __exit accton_as6710_32x_fan_exit(void)
{
	kthread_stop(fan_data->fanctrl_tsk);
	wait_for_completion(&fan_data->fanctrl_update_stop);
    sysfs_remove_group(&fan_data->dev->kobj, &accton_as6710_32x_fan_group);
	hwmon_device_unregister(fan_data->dev);
    kfree(fan_data);
}

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("accton_as6710_32x_fan driver");
MODULE_LICENSE("GPL");

module_init(accton_as6710_32x_fan_init);
module_exit(accton_as6710_32x_fan_exit);

