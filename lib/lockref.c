// SPDX-License-Identifier: GPL-2.0
#include <linux/lockref.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>

/*
 * 递增引用计数
 */
void lockref_get(struct lockref *lr)
{
    spin_lock(&lr->lock);
    lr->count++;
    spin_unlock(&lr->lock);
}

/*
 * 递减引用计数，返回新值
 */
int lockref_put_return(struct lockref *lr)
{
    int ret;
    spin_lock(&lr->lock);
    lr->count--;
    ret = lr->count;
    spin_unlock(&lr->lock);
    return ret;
}

/*
 * 如果 count != 0，则递增，返回1，否则返回0
 */
int lockref_get_not_zero(struct lockref *lr)
{
    int ret = 0;
    spin_lock(&lr->lock);
    if (lr->count != 0) {
        lr->count++;
        ret = 1;
    }
    spin_unlock(&lr->lock);
    return ret;
}

/*
 * 如果 count != 0，递减，返回1，否则返回0
 */
int lockref_put_not_zero(struct lockref *lr)
{
    int ret = 0;
    spin_lock(&lr->lock);
    if (lr->count != 0) {
        lr->count--;
        ret = 1;
    }
    spin_unlock(&lr->lock);
    return ret;
}

/*
 * 如果递减后为0，保持锁定返回1，否则解锁返回0
 */
int lockref_put_or_lock(struct lockref *lr)
{
    int ret = 0;
    spin_lock(&lr->lock);
    lr->count--;
    if (lr->count == 0) {
        ret = 1;  // 不解锁，调用者负责释放
    } else {
        spin_unlock(&lr->lock);
    }
    return ret;
}

/*
 * 标记对象死亡（count < 0）
 */
void lockref_mark_dead(struct lockref *lr)
{
    spin_lock(&lr->lock);
    lr->count = -1;
    spin_unlock(&lr->lock);
}

/*
 * 如果 count >= 0，递增返回1，否则返回0
 */
int lockref_get_not_dead(struct lockref *lr)
{
    int ret = 0;
    spin_lock(&lr->lock);
    if (lr->count >= 0) {
        lr->count++;
        ret = 1;
    }
    spin_unlock(&lr->lock);
    return ret;
}
