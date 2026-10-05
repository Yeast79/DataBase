// Copyright (c) 2023-2027 Renmin University of China
// SPDX-License-Identifier: MulanPSL-2.0

#include "rm_scan.h"

#include "rm_file_handle.h"

/**
 * @brief 初始化file_handle和rid
 * @param file_handle
 */
RmScan::RmScan(const RmFileHandle* file_handle) : file_handle_(file_handle) {
    rid_ = Rid{RM_FIRST_RECORD_PAGE, -1};
    next();
}

/**
 * @brief 找到文件中下一个存放了记录的位置
 */
void RmScan::next() {
    while (rid_.page_no < file_handle_->file_hdr_.num_pages) {
        // 获取当前页面
        RmPageHandle page_handle = file_handle_->fetch_page_handle(rid_.page_no);
        // 从下一个槽位开始查找
        int slot_no = rid_.slot_no + 1;
        for (; slot_no < file_handle_->file_hdr_.num_records_per_page; slot_no++) {
            if (Bitmap::is_set(page_handle.bitmap, slot_no)) {
                rid_.slot_no = slot_no;
                file_handle_->buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), false);
                return;
            }
        }
        file_handle_->buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), false);
        // 当前页没有更多记录，换下一页，从 slot 0 开始
        rid_.page_no++;
        rid_.slot_no = -1;
    }
    // 扫描结束，设置一个无效的 rid
    rid_ = Rid{RM_NO_PAGE, -1};
}

/**
 * @brief ​ 判断是否到达文件末尾
 */
bool RmScan::is_end() const { return rid_.page_no == RM_NO_PAGE; }

/**
 * @brief RmScan内部存放的rid
 */
Rid RmScan::rid() const { return rid_; }