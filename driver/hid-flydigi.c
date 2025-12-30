// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * USB driver for Flydigi Vader 5 Pro (2.4G USB)
 * Binds to Interface 0 (vendor-specific Xbox-like protocol)
 */

#include <linux/module.h>
#include <linux/input.h>
#include <linux/usb.h>
#include <linux/usb/input.h>
#include <linux/slab.h>

#define VENDOR_FLYDIGI		0x37d7
#define DEVICE_VADER5_24G	0x2401

#define INTF_CLASS_VENDOR	0xff
#define INTF_SUBCLASS_XBOX	0x5d
#define INTF_PROTOCOL		0x01

#define EP_IN_ADDR		0x81
#define PKT_SIZE		32

#define AXIS_MAX		32767
#define AXIS_MIN		(-32768)
#define TRIGGER_MAX		255

struct flydigi_data {
	struct input_dev *input;
	struct usb_device *udev;
	struct usb_interface *intf;
	struct urb *irq_in;
	unsigned char *idata;
	dma_addr_t idata_dma;
	char phys[64];
};

static void flydigi_parse(struct flydigi_data *fd, unsigned char *data)
{
	struct input_dev *input = fd->input;
	int hat_x = 0, hat_y = 0;

	/* Xbox-like format: data starts at offset 0 */
	u8 misc = data[2];
	u8 buttons = data[3];

	if (misc & 0x01)
		hat_y = -1;
	if (misc & 0x02)
		hat_y = 1;
	if (misc & 0x04)
		hat_x = -1;
	if (misc & 0x08)
		hat_x = 1;
	input_report_abs(input, ABS_HAT0X, hat_x);
	input_report_abs(input, ABS_HAT0Y, hat_y);

	input_report_key(input, BTN_START, misc & 0x10);
	input_report_key(input, BTN_SELECT, misc & 0x20);
	input_report_key(input, BTN_THUMBL, misc & 0x40);
	input_report_key(input, BTN_THUMBR, misc & 0x80);

	input_report_key(input, BTN_TL, buttons & 0x01);
	input_report_key(input, BTN_TR, buttons & 0x02);
	input_report_key(input, BTN_MODE, buttons & 0x04);
	input_report_key(input, BTN_A, buttons & 0x10);
	input_report_key(input, BTN_B, buttons & 0x20);
	input_report_key(input, BTN_X, buttons & 0x40);
	input_report_key(input, BTN_Y, buttons & 0x80);

	input_report_abs(input, ABS_Z, data[4]);
	input_report_abs(input, ABS_RZ, data[5]);

	input_report_abs(input, ABS_X, (s16)(data[6] | (data[7] << 8)));
	input_report_abs(input, ABS_Y, -(s16)(data[8] | (data[9] << 8)));
	input_report_abs(input, ABS_RX, (s16)(data[10] | (data[11] << 8)));
	input_report_abs(input, ABS_RY, -(s16)(data[12] | (data[13] << 8)));

	input_sync(input);
}

static void flydigi_irq_in(struct urb *urb)
{
	struct flydigi_data *fd = urb->context;
	int ret;

	switch (urb->status) {
	case 0:
		break;
	case -ECONNRESET:
	case -ENOENT:
	case -ESHUTDOWN:
		return;
	default:
		goto resubmit;
	}

	if (urb->actual_length >= 14) {
		printk_ratelimited(KERN_INFO "flydigi: len=%d [0]=0x%02x [1]=0x%02x\n",
			urb->actual_length, fd->idata[0], fd->idata[1]);
		flydigi_parse(fd, fd->idata);
	}

resubmit:
	ret = usb_submit_urb(urb, GFP_ATOMIC);
	if (ret)
		dev_err(&fd->intf->dev, "urb resubmit failed: %d\n", ret);
}

static int flydigi_open(struct input_dev *dev)
{
	struct flydigi_data *fd = input_get_drvdata(dev);

	fd->irq_in->dev = fd->udev;
	if (usb_submit_urb(fd->irq_in, GFP_KERNEL))
		return -EIO;
	return 0;
}

static void flydigi_close(struct input_dev *dev)
{
	struct flydigi_data *fd = input_get_drvdata(dev);

	usb_kill_urb(fd->irq_in);
}

static int flydigi_probe(struct usb_interface *intf,
			 const struct usb_device_id *id)
{
	struct usb_device *udev = interface_to_usbdev(intf);
	struct usb_endpoint_descriptor *ep_in = NULL;
	struct flydigi_data *fd;
	struct input_dev *input;
	struct usb_endpoint_descriptor *ep;
	int i, ret;

	dev_info(&intf->dev, "probe: interface %d\n",
		 intf->cur_altsetting->desc.bInterfaceNumber);

	/* Find EP1 IN */
	for (i = 0; i < intf->cur_altsetting->desc.bNumEndpoints; i++) {
		ep = &intf->cur_altsetting->endpoint[i].desc;
		if (usb_endpoint_is_int_in(ep) &&
		    usb_endpoint_num(ep) == 1) {
			ep_in = ep;
			break;
		}
	}
	if (!ep_in) {
		dev_err(&intf->dev, "EP1 IN not found\n");
		return -ENODEV;
	}

	fd = kzalloc(sizeof(*fd), GFP_KERNEL);
	if (!fd)
		return -ENOMEM;

	fd->idata = usb_alloc_coherent(udev, PKT_SIZE, GFP_KERNEL, &fd->idata_dma);
	if (!fd->idata) {
		ret = -ENOMEM;
		goto err_free_fd;
	}

	fd->irq_in = usb_alloc_urb(0, GFP_KERNEL);
	if (!fd->irq_in) {
		ret = -ENOMEM;
		goto err_free_idata;
	}

	input = input_allocate_device();
	if (!input) {
		ret = -ENOMEM;
		goto err_free_urb;
	}

	fd->udev = udev;
	fd->intf = intf;
	fd->input = input;

	usb_make_path(udev, fd->phys, sizeof(fd->phys));
	strlcat(fd->phys, "/input0", sizeof(fd->phys));

	input->name = "Flydigi Vader 5 Pro";
	input->phys = fd->phys;
	usb_to_input_id(udev, &input->id);
	input->dev.parent = &intf->dev;

	input->open = flydigi_open;
	input->close = flydigi_close;

	input_set_drvdata(input, fd);

	set_bit(EV_KEY, input->evbit);
	set_bit(EV_ABS, input->evbit);

	set_bit(BTN_A, input->keybit);
	set_bit(BTN_B, input->keybit);
	set_bit(BTN_X, input->keybit);
	set_bit(BTN_Y, input->keybit);
	set_bit(BTN_TL, input->keybit);
	set_bit(BTN_TR, input->keybit);
	set_bit(BTN_SELECT, input->keybit);
	set_bit(BTN_START, input->keybit);
	set_bit(BTN_MODE, input->keybit);
	set_bit(BTN_THUMBL, input->keybit);
	set_bit(BTN_THUMBR, input->keybit);

	input_set_abs_params(input, ABS_X, AXIS_MIN, AXIS_MAX, 16, 128);
	input_set_abs_params(input, ABS_Y, AXIS_MIN, AXIS_MAX, 16, 128);
	input_set_abs_params(input, ABS_RX, AXIS_MIN, AXIS_MAX, 16, 128);
	input_set_abs_params(input, ABS_RY, AXIS_MIN, AXIS_MAX, 16, 128);
	input_set_abs_params(input, ABS_Z, 0, TRIGGER_MAX, 0, 0);
	input_set_abs_params(input, ABS_RZ, 0, TRIGGER_MAX, 0, 0);
	input_set_abs_params(input, ABS_HAT0X, -1, 1, 0, 0);
	input_set_abs_params(input, ABS_HAT0Y, -1, 1, 0, 0);

	usb_fill_int_urb(fd->irq_in, udev,
			 usb_rcvintpipe(udev, ep_in->bEndpointAddress),
			 fd->idata, PKT_SIZE, flydigi_irq_in, fd,
			 ep_in->bInterval);
	fd->irq_in->transfer_dma = fd->idata_dma;
	fd->irq_in->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	ret = input_register_device(input);
	if (ret) {
		dev_err(&intf->dev, "input_register failed: %d\n", ret);
		goto err_free_input;
	}

	usb_set_intfdata(intf, fd);

	dev_info(&intf->dev, "Vader 5 Pro initialized\n");
	return 0;

err_free_input:
	input_free_device(input);
err_free_urb:
	usb_free_urb(fd->irq_in);
err_free_idata:
	usb_free_coherent(udev, PKT_SIZE, fd->idata, fd->idata_dma);
err_free_fd:
	kfree(fd);
	return ret;
}

static void flydigi_disconnect(struct usb_interface *intf)
{
	struct flydigi_data *fd = usb_get_intfdata(intf);

	dev_info(&intf->dev, "disconnecting\n");

	usb_set_intfdata(intf, NULL);
	if (fd) {
		input_unregister_device(fd->input);
		usb_free_urb(fd->irq_in);
		usb_free_coherent(fd->udev, PKT_SIZE, fd->idata, fd->idata_dma);
		kfree(fd);
	}
}

static const struct usb_device_id flydigi_devices[] = {
	{ USB_DEVICE_INTERFACE_PROTOCOL(VENDOR_FLYDIGI, DEVICE_VADER5_24G, INTF_PROTOCOL) },
	{ }
};
MODULE_DEVICE_TABLE(usb, flydigi_devices);

static struct usb_driver flydigi_driver = {
	.name = "flydigi",
	.id_table = flydigi_devices,
	.probe = flydigi_probe,
	.disconnect = flydigi_disconnect,
};
module_usb_driver(flydigi_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("jim_z");
MODULE_DESCRIPTION("Flydigi Vader 5 Pro USB driver");