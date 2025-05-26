#ifndef __LINUX_OF_H
#define __LINUX_OF_H

#include <linux/types.h>
#include <linux/libfdt.h>
#include <linux/string.h>
#include <linux/errno.h>

struct device_node {
    const void *fdt;
    int offset;
};

static const void *of_flat_dtb = __dtb_file_start_address; // 全局 dtb 指针，移植前先赋值

/* 查找 compatible 节点 */
static inline struct device_node *of_find_compatible_node(const struct device_node *from, const char *type, const char *compatible)
{
    static struct device_node node;
    int offset = -1;
    const void *fdt = of_flat_dtb;

    if (!fdt)
        return 0;

    if (from)
        offset = from->offset;

    while (1) {
        offset = fdt_next_node(fdt, offset, 0);
        if (offset < 0)
            return 0;

        const char *prop = fdt_getprop(fdt, offset, "compatible", 0);
        if (prop && strcmp(prop, compatible) == 0) {
            node.fdt = fdt;
            node.offset = offset;
            return &node;
        }
    }
}

/* 获取属性指针 */
static inline const void *of_get_property(const struct device_node *np, const char *propname, int *len)
{
    if (!np)
        return 0;
    return fdt_getprop(np->fdt, np->offset, propname, len);
}

/* 读 u32 属性 */
static inline int of_property_read_u32(const struct device_node *np, const char *propname, u32 *out_value)
{
    int len;
    const u32 *val;

    if (!np || !out_value)
        return -EINVAL;

    val = fdt_getprop(np->fdt, np->offset, propname, &len);
    if (!val || len < 4)
        return -EINVAL;

    *out_value = fdt32_to_cpu(*val);
    return 0;
}

/* 读 u32 数组属性 */
static inline int of_property_read_u32_array(const struct device_node *np, const char *propname, u32 *out_values, int count)
{
    int len, i;
    const u32 *val;

    if (!np || !out_values || count <= 0)
        return -EINVAL;

    val = fdt_getprop(np->fdt, np->offset, propname, &len);
    if (!val || len < count * 4)
        return -EINVAL;

    for (i = 0; i < count; i++)
        out_values[i] = fdt32_to_cpu(val[i]);

    return 0;
}

/* 读字符串属性 */
static inline int of_property_read_string(const struct device_node *np, const char *propname, const char **out_string)
{
    int len;
    const char *val;

    if (!np || !out_string)
        return -EINVAL;

    val = fdt_getprop(np->fdt, np->offset, propname, &len);
    if (!val)
        return -EINVAL;

    *out_string = val;
    return 0;
}

/* 判断节点名称是否匹配 */
static inline int of_node_name_eq(const struct device_node *np, const char *name)
{
    const char *node_name;
    if (!np)
        return 0;

    node_name = fdt_get_name(np->fdt, np->offset, 0);
    return (strcmp(node_name, name) == 0);
}

/* 查找路径节点 */
static inline struct device_node *of_find_node_by_path(const char *path)
{
    static struct device_node node;
    int offset;
    const void *fdt = of_flat_dtb;

    if (!fdt)
        return 0;

    offset = fdt_path_offset(fdt, path);
    if (offset < 0)
        return 0;

    node.fdt = fdt;
    node.offset = offset;
    return &node;
}

/* 解析 phandle 属性（获取引用节点） */
static inline struct device_node *of_parse_phandle(const struct device_node *np, const char *phandle_name, int index)
{
    static struct device_node node;
    const fdt32_t *phandle_list;
    int len;
    u32 phandle;
    int offset;

    if (!np)
        return 0;

    phandle_list = fdt_getprop(np->fdt, np->offset, phandle_name, &len);
    if (!phandle_list)
        return 0;

    if (len < (index + 1) * sizeof(u32))
        return 0;

    phandle = fdt32_to_cpu(phandle_list[index]);
    offset = fdt_node_offset_by_phandle(np->fdt, phandle);
    if (offset < 0)
        return 0;

    node.fdt = np->fdt;
    node.offset = offset;
    return &node;
}

#endif
