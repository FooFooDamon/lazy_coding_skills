// SPDX-License-Identifier: GPL-2.0

/*
 * TODO: Brief description of this file.
 *
 * Copyright (c) ${YEAR} ${LCS_USER} <${LCS_EMAIL}>
 * All rights reserved.
*/

// set shiftwidth=8 | set tabstop=8 | set softtabstop=8 | set noexpandtab

//#include "${SELF_HEADER}.h"

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
#define __DRVNAME__			"${TITLE}"
#endif
#endif

typedef struct ${TITLE}
{
	struct i2c_client *client;
	// TODO: More fields according to your need.
} ${TITLE}_t;

static int ${TITLE}_probe(struct i2c_client *client
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 3, 0)
	, const struct i2c_device_id *id
#endif
)
{
	struct device *dev = &client->dev;
	${TITLE}_t *info = devm_kzalloc(dev, sizeof(${TITLE}_t), GFP_KERNEL);
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

static void ${TITLE}_remove(struct i2c_client *client)
{
	//${TITLE}_t *${TITLE} = i2c_get_clientdata(client);

	// TODO: Add your own stuff.

	dev_notice(&client->dev, "Removed %s\n", client->name);
}

#if IS_ENABLED(CONFIG_OF)
/*
 * Match by devicetree (Open Firmware).
 */
static const struct of_device_id ${TITLE}_of_match[] = {
	{ .compatible = "${TITLE}" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, ${TITLE}_of_match);
#endif

/*
 * Traditional match.
 */
static const struct i2c_device_id ${TITLE}_ids[] = {
	{ .name = "${TITLE}" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(i2c, ${TITLE}_ids);

static struct i2c_driver ${TITLE}_driver = {
	.probe = ${TITLE}_probe,
	.remove = ${TITLE}_remove,
	.driver = {
		.name = "${TITLE}",
#if IS_ENABLED(CONFIG_OF)
		.of_match_table = of_match_ptr(${TITLE}_of_match),
#endif
	},
	.id_table = ${TITLE}_ids,
};

module_i2c_driver(${TITLE}_driver);

MODULE_DESCRIPTION("Driver for ${TITLE}");
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
