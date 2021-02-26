/*
 * @Author: cijliu
 * @Date: 2021-02-02 15:48:13
 * @LastEditTime: 2021-02-24 17:26:52
 */
#include "include/h264.h"
#include "sys/stat.h"
#include <assert.h>
#define RTP_TS_DEFAULT      (3600)
static uint8_t *h264_nal_data_ptr(uint8_t *ptr)
{
    if(ptr[0] == 0x00 && ptr[1] == 0x00 && ptr[2] == 0x01) {
        return &ptr[3];
    }
    if(ptr[0] == 0x00 && ptr[1] == 0x00 && ptr[2] == 0x00 && ptr[3] == 0x01) {
        return &ptr[4];
    }
    return NULL;
}
static int h264_nal_is_header(uint8_t *ptr)
{
    if(ptr[0] == 0x00 && ptr[1] == 0x00 && ptr[2] == 0x01){
        return 0;
    }
    if(ptr[0] == 0x00 && ptr[1] == 0x00 && ptr[2] == 0x00 && ptr[3] == 0x01){
        return 0;
    }
    return -1;
}
static uint8_t *h264_next_nal(uint8_t *ptr, uint8_t *end)
{
    uint8_t *p = ptr;
    while(p < end){
        if(!h264_nal_is_header(p)){
            return p;
        }
        p++;
    }
    return NULL;
}
h264_nalu_t *h264_nal_packet_malloc(unsigned char *buf, int len)
{
    uint8_t *end = buf + len - 1;
    uint8_t *data = h264_nal_data_ptr(buf);
    h264_nalu_t *nalu = (h264_nalu_t *)malloc(sizeof(h264_nalu_t));
    h264_nalu_t *h264_nal = nalu;
    while(data && data < end){
        memset(nalu, 0, sizeof(h264_nalu_t));
        uint8_t *next = h264_next_nal(data, end);
        
        nalu->type = H264_NAL(data[0]);
        nalu->data = data;
        nalu->len = next - data;
        if(next == NULL) {
            nalu->len = end - data + 1;
            return h264_nal;
        }
        nalu->next = (h264_nalu_t *)malloc(sizeof(h264_nalu_t));
        nalu = nalu->next;
        data = h264_nal_data_ptr(next);
    }
    free(h264_nal);
    return NULL;
}

void h264_nal_packet_free(h264_nalu_t *nal)
{
    h264_nalu_t *p = nal;
    while(p){
        h264_nalu_t *next = p->next;
        free(p);
        p = next;
    }
    nal = NULL;
}
static rtp_packet_t* rtp_packet_nalu(rtp_header_t *header, uint8_t *data, uint32_t len)
{
    RTP_ASSERT(header);
    RTP_ASSERT(data);
    uint32_t payload_size = RTP_MAX_PAYLOAD - sizeof(rtp_header_t);
    if(len > payload_size) {
        return NULL;
    }
    rtp_packet_t *ptk = (rtp_packet_t*)malloc(sizeof(rtp_packet_t));
    rtp_header_t *hdr = (rtp_header_t*)ptk->data;
    memset(ptk, 0, sizeof(rtp_packet_t));
    memcpy(hdr, header, sizeof(rtp_header_t));
    memcpy(ptk->data + sizeof(rtp_header_t), data, len);
    hdr->m = 1;
    hdr->seq = htons(hdr->seq);
    hdr->ts  = htonl(hdr->ts);
    ptk->next = NULL;
    ptk->len = len + sizeof(rtp_header_t);
    header->seq++;
    header->ts += RTP_TS_DEFAULT;
    return ptk;
}
static rtp_packet_t* rtp_packet_nalu_with_fu_a(rtp_header_t *header, uint8_t nalu_type, uint8_t *data, uint32_t len)
{
    RTP_ASSERT(header);
    RTP_ASSERT(data);
    uint32_t size = 0;
    uint32_t payload_size = RTP_MAX_PAYLOAD - sizeof(rtp_header_t) - 2;
    if(len < payload_size) {
        return NULL;
    }
    rtp_packet_t *ptk = (rtp_packet_t*)malloc(sizeof(rtp_packet_t));
    rtp_packet_t *cur = ptk;
    memset(ptk, 0, sizeof(rtp_packet_t));
    do{
        memcpy(&cur->nalu.hdr, header, sizeof(rtp_header_t));
        cur->nalu.hdr.seq = htons(cur->nalu.hdr.seq);
        cur->nalu.hdr.ts  = htonl(cur->nalu.hdr.ts);
        cur->nalu.fu_indicator  = (nalu_type & 0xe0) | 28;
        cur->nalu.fu_header     = (nalu_type & 0x1f);   //FU-A
        if(cur == ptk){
            cur->nalu.fu_header |=0x80;//FU-A
        }
        memcpy(cur->nalu.rbsp, data + size, payload_size);
        size += payload_size;
        len  -= payload_size;
        cur->len = RTP_MAX_PAYLOAD;
        rtp_packet_t *next = (rtp_packet_t*)malloc(sizeof(rtp_packet_t));
        memset(next, 0, sizeof(rtp_packet_t));
        cur->next = next;
        cur = next;
        header->seq++;
    } while(len > payload_size);
    
    memcpy(&cur->nalu.hdr, header, sizeof(rtp_header_t));
    cur->nalu.hdr.m = 1;
    cur->nalu.hdr.seq = htons(cur->nalu.hdr.seq);
    cur->nalu.hdr.ts  = htonl(cur->nalu.hdr.ts);
    cur->nalu.fu_indicator  = (nalu_type & 0xe0) | 28;
    cur->nalu.fu_header     = (nalu_type & 0x1f) | 0x40;   //FU-A
    memcpy(cur->nalu.rbsp, data + size, len);
    cur->len = len + sizeof(rtp_header_t) + 2;
    header->seq++;
    header->ts += RTP_TS_DEFAULT;
    return ptk;
}
rtp_packet_t* rtp_h264_packet_malloc(rtp_header_t *header, uint8_t *data, uint32_t len)
{
    uint32_t payload_size = RTP_MAX_PAYLOAD - sizeof(rtp_header_t);
    if(len > payload_size){
        uint8_t nalu_type = data[0];
        uint8_t *nalu_rbsp = &data[1];
        uint32_t rbsp_len = len - 1;
        return rtp_packet_nalu_with_fu_a(header, nalu_type, nalu_rbsp, rbsp_len);
    }
    return rtp_packet_nalu(header, data, len);
}