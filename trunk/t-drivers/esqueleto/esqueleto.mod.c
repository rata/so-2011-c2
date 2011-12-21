#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xf7165d8a, "module_layout" },
	{ 0x8ecd3d8b, "class_destroy" },
	{ 0x5c2a02d5, "device_destroy" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0x11d2568d, "cdev_del" },
	{ 0xd7459479, "device_create" },
	{ 0xa51dd63b, "__class_create" },
	{ 0x5d73d5a5, "cdev_add" },
	{ 0x215781ec, "cdev_init" },
	{ 0x29537c9e, "alloc_chrdev_region" },
	{ 0x50eedeb8, "printk" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

