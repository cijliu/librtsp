/*
 * @Author: cijliu
 * @Date: 2021-02-08 19:51:36
 * @LastEditTime: 2021-02-26 13:55:05
 */
#ifndef __H264_H__
#define __H264_H__
#include "stdint.h"
#include "stdio.h"
#include "include/rtp.h"
#define H264_NAL(v)	(v & 0x1F)
typedef enum { 
    H264_NAL_PFRAME   = 1,
    H264_NAL_IDR = 5, 
    H264_NAL_SEI = 6, 
    H264_NAL_SPS = 7, 
    H264_NAL_PPS = 8 
} h264_nalu_enum_t;
typedef struct h264_nalu{
    h264_nalu_enum_t type;
    uint8_t *data;
    uint32_t len;
    struct h264_nalu *next;
} h264_nalu_t;

uint8_t* h264_nalu_find(uint8_t* ptr, uint8_t* end);
uint8_t* h264_search_start_code(uint8_t* ptr, uint8_t* end);
int h264_nal_type(unsigned char* ptr);
int h264_nal_new_access(unsigned char* ptr, uint8_t* end);
rtp_packet_t* rtp_h264_packet_malloc(rtp_header_t *header, uint8_t *data, uint32_t len);

h264_nalu_t *h264_nal_packet_malloc(unsigned char *buf, int len);
void h264_nal_packet_free(h264_nalu_t *nal);
#endif