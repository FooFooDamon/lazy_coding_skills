// SPDX-License-Identifier: GPL-2.0

/*
 * TODO: Brief description of this file.
 *
 * Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
 * All rights reserved.
 */

// set shiftwidth=8 | set tabstop=8 | set softtabstop=8 | set noexpandtab

//#include "${BASENAME}.h"

#include <linux/version.h>
#include <linux/module.h>
#include <linux/i2c.h>

#ifndef __VER__
#define __VER__				"<none>"
#endif

#ifndef DRV_VER_MAJOR
#define DRV_VER_MAJOR			0
#endif

#ifndef DRV_VER_MINOR
#define DRV_VER_MINOR			1
#endif

#ifndef DRV_VER_RELEASE
#define DRV_VER_RELEASE			0
#endif

#ifndef DRIVER_VERSION
#define DRIVER_VERSION			__stringify(DRV_VER_MAJOR) "." __stringify(DRV_VER_MINOR) "." __stringify(DRV_VER_RELEASE)
#endif

#ifndef __DRVNAME__
#ifdef KBUILD_MODNAME
#define __DRVNAME__			KBUILD_MODNAME
#else
#define __DRVNAME__			"${BASENAME}"
#endif
#endif

typedef struct ${BASENAME}
{
	struct i2c_client *client;
	// TODO: More fields according to your need.
} ${BASENAME}_t;

static int ${BASENAME}_probe(struct i2c_client *client
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 3, 0)
	, const struct i2c_device_id *id
#endif
)
{
	struct device *dev = &client->dev;
	${BASENAME}_t *info = devm_kzalloc(dev, sizeof(${BASENAME}_t), GFP_KERNEL);
	int err = info ? 0 : -ENOMEM;

	if (err)
		return err;

	info->client = client;
	i2c_set_clientdata(client, info);

	// TODO: Add your own stuff.

	dev_notice(dev, "Probed %s successfully with driver %s:%s-%s on Linux-%#x\n",
		client->name, __DRVNAME__, DRIVER_VERSION, __VER__, LINUX_VERSION_CODE);

	return 0;
}

static void ${BASENAME}_remove(struct i2c_client *client)
{
	//${BASENAME}_t *${BASENAME} = i2c_get_clientdata(client);

	// TODO: Add your own stuff.

	dev_notice(&client->dev, "Removed %s\n", client->name);
}

#if IS_ENABLED(CONFIG_OF)
/*
 * Match by devicetree (Open Firmware).
 */
static const struct of_device_id ${BASENAME}_of_match[] = {
	{ .compatible = "${BASENAME}" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, ${BASENAME}_of_match);
#endif

/*
 * Traditional match.
 */
static const struct i2c_device_id ${BASENAME}_ids[] = {
	{ .name = "${BASENAME}" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(i2c, ${BASENAME}_ids);

static struct i2c_driver ${BASENAME}_driver = {
	.probe = ${BASENAME}_probe,
	.remove = ${BASENAME}_remove,
	.driver = {
		.name = "${BASENAME}",
#if IS_ENABLED(CONFIG_OF)
		.of_match_table = of_match_ptr(${BASENAME}_of_match),
#endif
	},
	.id_table = ${BASENAME}_ids,
};

module_i2c_driver(${BASENAME}_driver);

MODULE_DESCRIPTION("Driver for ${BASENAME}");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION "-" __VER__);
MODULE_AUTHOR("${LCS_USER} <${LCS_EMAIL}>");

/*
 * ================
 *   CHANGE LOG
 * ================
 *
 * >>> ${DATE}, ${LCS_USER} <${LCS_EMAIL}>:
 *  01. Initial commit.
 */
