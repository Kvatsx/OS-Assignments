/*
* OHCI HCD (Host Controller Driver) for USB.
*
* Copyright (C) 2010 ST Microelectronics.
* Deepak Sikri<deepak.sikri@st.com>
*
* Based on various ohci-*.c drivers
*
* This file is licensed under the terms of the GNU General Public
* License version 2. This program is licensed "as is" without any
* warranty of any kind, whether express or implied.
*/

#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/signal.h>
#include <linux/usb.h>
#include <linux/usb/hcd.h>

#include "ohci.h"

#define DRIVER_DESC "OHCI SPEAr driver"

static const char hcd_name[] = "SPEAr-ohci";
struct spear_ohci {
	struct clk *clk;
};

#define to_spear_ohci(hcd)     (struct spear_ohci *)(hcd_to_ohci(hcd)->priv)

static struct hc_driver __read_mostly ohci_spear_hc_driver;

static int spear_ohci_hcd_drv_probe(struct platform_device *pdev)
{
	const struct hc_driver *driver = &ohci_spear_hc_driver;
	struct ohci_hcd *ohci;
	struct usb_hcd *hcd = NULL;
	struct clk *usbh_clk;
	struct spear_ohci *sohci_p;
	struct resource *res;
	int retval, irq;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		retval = irq;
		goto fail;
	}

	/*
	 * Right now device-tree probed devices don't get dma_mask set.
	 * Since shared usb code relies on it, set it here for now.
	 * Once we have dma capability bindings this can go away.
	 */
	retval = dma_coerce_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(32));
	if (retval)
		goto fail;

	usbh_clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(usbh_clk)) {
		dev_err(&pdev->dev, "Error getting interface clock\n");
		retval = PTR_ERR(usbh_clk);
		goto fail;
	}

	hcd = usb_create_hcd(driver, &pdev->dev, dev_name(&pdev->dev));
	if (!hcd) {
		retval = -ENOMEM;
		goto fail;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		retval = -ENODEV;
		goto err_put_hcd;
	}

	hcd->rsrc_start = pdev->resource[0].start;
	hcd->rsrc_len = resource_size(res);
	if (!devm_request_mem_region(&pdev->dev, hcd->rsrc_start, hcd->rsrc_len,
				hcd_name)) {
		dev_dbg(&pdev->dev, "request_mem_region failed\n");
		retval = -EBUSY;
		goto err_put_hcd;
	}

	hcd->regs = devm_ioremap(&pdev->dev, hcd->rsrc_start, hcd->rsrc_len);
	if (!hcd->regs) {
		dev_dbg(&pdev->dev, "ioremap failed\n");
		retval = -ENOMEM;
		goto err_put_hcd;
	}

	sohci_p = to_spear_ohci(hcd);
	sohci_p->clk = usbh_clk;

	clk_prepare_enable(sohci_p->clk);

	ohci = hcd_to_ohci(hcd);

	retval = usb_add_hcd(hcd, platform_get_irq(pdev, 0), 0);
	if (retval == 0)
		return retval;

	clk_disable_unprepare(sohci_p->clk);
err_put_hcd:
	usb_put_hcd(hcd);
fail:
	dev_err(&pdev->dev, "init fail, %d\n", retval);

	return retval;
}

static int spear_ohci_hcd_drv_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);
	struct spear_ohci *sohci_p = to_spear_ohci(hcd);

	usb_remove_hcd(hcd);
	if (sohci_p->clk)
		clk_disable_unprepare(sohci_p->clk);

	usb_put_hcd(hcd);
	return 0;
}

#if defined(CONFIG_PM)
static int spear_ohci_hcd_drv_suspend(struct platform_device *dev,
		pm_message_t message)
{
	struct usb_hcd *hcd = platform_get_drvdata(dev);
	struct ohci_hcd	*ohci = hcd_to_ohci(hcd);
	struct spear_ohci *sohci_p = to_spear_ohci(hcd);

	if (time_before(jiffies, ohci->next_statechange))
		msleep(5);
	ohci->next_statechange = jiffies;

	clk_disable_unprepare(sohci_p->clk);

	return 0;
}

static int spear_ohci_hcd_drv_resume(struct platform_device *dev)
{
	struct usb_hcd *hcd = platform_get_drvdata(dev);
	struct ohci_hcd	*ohci = hcd_to_ohci(hcd);
	struct spear_ohci *sohci_p = to_spear_ohci(hcd);

	if (time_before(jiffies, ohci->next_statechange))
		msleep(5);
	ohci->next_statechange = jiffies;

	clk_prepare_enable(sohci_p->clk);
	ohci_resume(hcd, false);
	return 0;
}
#endif

static struct of_device_id spear_ohci_id_table[] = {
	{ .compatible = "st,spear600-ohci", },
	{ },
};

/* Driver definition to register with the platform bus */
static struct platform_driver spear_ohci_hcd_driver = {
	.probe =	spear_ohci_hcd_drv_probe,
	.remove =	spear_ohci_hcd_drv_remove,
#ifdef CONFIG_PM
	.suspend =	spear_ohci_hcd_drv_suspend,
	.resume =	spear_ohci_hcd_drv_resume,
#endif
	.driver = {
		.owner = THIS_MODULE,
		.name = "spear-ohci",
		.of_match_table = spear_ohci_id_table,
	},
};

static const struct ohci_driver_overrides spear_overrides __initconst = {
	.extra_priv_size = sizeof(struct spear_ohci),
};
static int __init ohci_spear_init(void)
{
	if (usb_disabled())
		return -ENODEV;

	pr_info("%s: " DRIVER_DESC "\n", hcd_name);

	ohci_init_driver(&ohci_spear_hc_driver, &spear_overrides);
	return platform_driver_register(&spear_ohci_hcd_driver);
}
module_init(ohci_spear_init);

static void __exit ohci_spear_cleanup(void)
{
	platform_driver_unregister(&spear_ohci_hcd_driver);
}
module_exit(ohci_spear_cleanup);

MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_AUTHOR("Deepak Sikri");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:spear-ohci");