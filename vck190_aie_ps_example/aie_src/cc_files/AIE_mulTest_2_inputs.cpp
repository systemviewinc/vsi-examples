/* Provide Declarations */
#include <string.h>
#include <adf.h>
#include <math.h>
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>
#define PI 3.14159265359f
//copying constants into all the lanes
// used for v4int16, v4int32, v4float
template <typename VT> inline VT vsi_splat_4(VT v) {
    VT result;
    result = upd_elem(result, 0, ext_elem(v,0));
    result = upd_elem(result, 1, ext_elem(v,0));
    result = upd_elem(result, 2, ext_elem(v,0));
    result = upd_elem(result, 3, ext_elem(v,0));
    return result;
}
// used for v8int16, v8int32, v8float
template <typename VT> inline VT vsi_splat_8(VT v) {
    VT result;
    result = upd_elem(result, 0, ext_elem(v,0));
    result = upd_elem(result, 1, ext_elem(v,0));
    result = upd_elem(result, 2, ext_elem(v,0));
    result = upd_elem(result, 3, ext_elem(v,0));
    result = upd_elem(result, 4, ext_elem(v,0));
    result = upd_elem(result, 5, ext_elem(v,0));
    result = upd_elem(result, 6, ext_elem(v,0));
    result = upd_elem(result, 7, ext_elem(v,0));
    return result;
}
//used for v16int16, v16int32, v16float
template <typename VT> inline VT vsi_splat_16(VT v) {
    VT result;
    result = upd_elem(result, 0, ext_elem(v,0));
    result = upd_elem(result, 1, ext_elem(v,0));
    result = upd_elem(result, 2, ext_elem(v,0));
    result = upd_elem(result, 3, ext_elem(v,0));
    result = upd_elem(result, 4, ext_elem(v,0));
    result = upd_elem(result, 5, ext_elem(v,0));
    result = upd_elem(result, 6, ext_elem(v,0));
    result = upd_elem(result, 7, ext_elem(v,0));
    result = upd_elem(result, 8, ext_elem(v,0));
    result = upd_elem(result, 9, ext_elem(v,0));
    result = upd_elem(result, 10, ext_elem(v,0));
    result = upd_elem(result, 11, ext_elem(v,0));
    result = upd_elem(result, 12, ext_elem(v,0));
    result = upd_elem(result, 13, ext_elem(v,0));
    result = upd_elem(result, 14, ext_elem(v,0));
    result = upd_elem(result, 15, ext_elem(v,0));
    return result;
}
// complex upd_elem and splat 
static inline v8int32 vsi_complex_upd_elem(v8int32 a, unsigned int i, uint64_t in){
    unsigned int index = i*2;
    a = upd_elem(a, index, *(uint32_t*)&in);
    return upd_elem(a, index+1, *(uint32_t*)(&in+1));
}
static inline v16int32 vsi_complex_upd_elem(v16int32 a, unsigned int i, uint64_t in){
    unsigned int index = i*2;
    a = upd_elem(a, index, *(uint32_t*)&in);
    return upd_elem(a, index+1, *(uint32_t*)(&in+1));
}
static inline v8float vsi_complex_upd_elem(v8float a, unsigned int i, double in){
    unsigned int index = i*2;
    a = upd_elem(a, index, ext_elem((*(v4float*)&in) , 0));
    return upd_elem(a, index+1, ext_elem((*(v4float*)&in) , 0));
}
static inline v16float vsi_complex_upd_elem(v16float a, unsigned int i, double in){
    unsigned int index = i*2;
    a = upd_elem(a, index, ext_elem((*(v4float*)&in) , 0));
    return upd_elem(a, index+1, ext_elem((*(v4float*)&in) , 0));
}
template <typename VT> inline VT vsi_complex_splat_4(VT v) {
    VT result;
    result = upd_elem(result, 0, ext_elem(v,0));
    result = upd_elem(result, 1, ext_elem(v,1));
    result = upd_elem(result, 2, ext_elem(v,0));
    result = upd_elem(result, 3, ext_elem(v,1));
    result = upd_elem(result, 4, ext_elem(v,0));
    result = upd_elem(result, 5, ext_elem(v,1));
    result = upd_elem(result, 6, ext_elem(v,0));
    result = upd_elem(result, 7, ext_elem(v,1));
    return result;
}
template <typename VT> inline VT vsi_complex_splat_8(VT v) {
    VT result;
    result = upd_elem(result, 0, ext_elem(v,0));
    result = upd_elem(result, 1, ext_elem(v,1));
    result = upd_elem(result, 2, ext_elem(v,0));
    result = upd_elem(result, 3, ext_elem(v,1));
    result = upd_elem(result, 4, ext_elem(v,0));
    result = upd_elem(result, 5, ext_elem(v,1));
    result = upd_elem(result, 6, ext_elem(v,0));
    result = upd_elem(result, 7, ext_elem(v,1));
    result = upd_elem(result, 8, ext_elem(v,0));
    result = upd_elem(result, 9, ext_elem(v,1));
    result = upd_elem(result, 10, ext_elem(v,0));
    result = upd_elem(result, 11, ext_elem(v,1));
    result = upd_elem(result, 12, ext_elem(v,0));
    result = upd_elem(result, 13, ext_elem(v,1));
    result = upd_elem(result, 14, ext_elem(v,0));
    result = upd_elem(result, 15, ext_elem(v,1));
    return result;
}
// select vectors :
static inline v16int32 vsi_select(unsigned int sel_mask, v16int32 a, v16int32 b) {
       return select16(~sel_mask, a, b);
}
static inline v8int32 vsi_select(unsigned int sel_mask, v8int32 a, v8int32 b) {
       return ext_w(select16(~sel_mask,concat(a,null_v8int32()),0,0x76543210,0,
                                      concat(b,null_v8int32()),0,0x76543210,0),0);
}
static inline v4int32 vsi_select(unsigned int sel_mask, v4int32 a, v4int32 b) {
       return ext_v(vsi_select(sel_mask, concat(a,null_v4int32()), concat(b,null_v4int32())), 0);
}
static inline v8int32 vsi_select(unsigned int sel_mask, v16int32 a, unsigned int start_a, unsigned int pmask_a, v8int32 b) {
       return ext_w(select16(~sel_mask,a,start_a,pmask_a,0,
                                      concat(b,null_v8int32()),0,0x76543210,0),0);
}
static inline v8int32 vsi_select(unsigned int sel_mask, v8int32 a, v16int32 b ,unsigned int start_b, unsigned int pmask_b ) {
       return ext_w(select16(~sel_mask,concat(a,null_v8int32()),0,0x76543210,0,
                                      b,start_b,pmask_b,0),0);
}
static inline v8int32 vsi_select(unsigned int sel_mask, v16int32 a, unsigned int start_a, unsigned int pmask_a, 
                                                        v16int32 b ,unsigned int start_b, unsigned int pmask_b ) {
       return ext_w(select16(~sel_mask,a,start_a,pmask_a,0,
                                      b,start_b,pmask_b,0),0);
}
static inline v8int16 vsi_select(unsigned int sel_mask, v8int16 a, v8int16 b) {
       return ext_v(select32(~sel_mask,
                             concat(concat(a,null_v8int16()),null_v16int16()),0,0x76543210,0,0x3210,
                             concat(concat(b,null_v8int16()),null_v16int16()),0,0x76543210,0,0x3210),
                    0);
}
static inline v8int16 vsi_select(unsigned int sel_mask, v16int16 a, unsigned int start_a, unsigned int pmask_a, v8int16 b) {
       return ext_v(select32(~sel_mask,
                             concat(a,null_v16int16()),start_a,pmask_a,0,0x3210,
                             concat(concat(b,null_v8int16()),null_v16int16()),0,0x76543210,0,0x3210),
                    0);
}
static inline v8int16 vsi_select(unsigned int sel_mask, v8int16 a,  v16int16 b, unsigned int start_b, unsigned int pmask_b) {
       return ext_v(select32(~sel_mask,
                             concat(concat(a,null_v8int16()),null_v16int16()),0,0x76543210,0,0x3210,
                             concat(b,null_v16int16()),start_b,pmask_b,0,0x3210),
                    0);
}
static inline v8int16 vsi_select(unsigned int sel_mask, v16int16 a,  unsigned int start_a, unsigned int pmask_a, 
                                                        v16int16 b,  unsigned int start_b, unsigned int pmask_b) {
       return ext_v(select32(~sel_mask,
                             concat(a,null_v16int16()),start_a,pmask_a,0,0x3210,
                             concat(b,null_v16int16()),start_b,pmask_b,0,0x3210),
                    0);
}
static inline v8float vsi_select(unsigned int sel_mask, v8float a, v8float b) {
       return as_v8float(vsi_select(sel_mask,as_v8int32(a),as_v8int32(b)));
}
static inline v4float vsi_select(unsigned int sel_mask, v4float a, v4float b) {
       return as_v4float(vsi_select(sel_mask,as_v4int32(a),as_v4int32(b)));
}
static inline v8float vsi_select(unsigned int sel_mask, v16float a, unsigned int start_a, unsigned int pmask_a, v8float b) {
       return as_v8float(vsi_select(sel_mask,as_v16int32(a),start_a,pmask_a,as_v8int32(b)));
}
static inline v8float vsi_select(unsigned int sel_mask, v8float a,  v16float b, unsigned int start_b, unsigned int pmask_b) {
       return as_v8float(vsi_select(sel_mask,as_v8int32(a),as_v16int32(b),start_b, pmask_b));
}
static inline v8float vsi_select(unsigned int sel_mask, v16float a, unsigned int start_a, unsigned int pmask_a, 
                                                        v16float b, unsigned int start_b, unsigned int pmask_b) {
       return as_v8float(vsi_select(sel_mask,as_v16int32(a),start_a,pmask_a,as_v16int32(b),start_b,pmask_b));
}
static inline v16float vsi_select(unsigned int sel_mask, v16float a,  v16float b) {
       return fpselect16(~sel_mask, a, 0, 0x76543210, 0xfedcba98, b, 0, 0x76543210, 0xfedcba98);
}
// Compare vectors:
static inline uint32 vsi_gt(v8int32 a, v8int32 b) {
       return gt16(concat(a,null_v8int32()),0,0x76543210,0,
                   concat(b,null_v8int32()),0,0x76543210,0);
}
static inline uint32 vsi_gt(v16int32 a, v16int32 b) {
       return vsi_gt(ext_w(a,0), ext_w(b,0));
}
static inline uint32 vsi_gt(v4int32 a, v4int32 b) {
       return vsi_gt(concat(a,null_v4int32()), concat(b,null_v4int32()));
}
static inline uint32 vsi_gt(v8int16 a, v8int16 b) {
       return gt32(concat(concat(a,null_v8int16()),null_v16int16()),0,0x76543210,0,0x3210,
                   concat(concat(b,null_v8int16()),null_v16int16()),0,0x76543210,0,0x3210);
}
static inline uint32 vsi_gt(v8float a, v8float b) {
       return fplt(b,concat(a,null_v8float()),0,0x76543210);
}
static inline uint32 vsi_gt(v4float a, v4float b) {
       return vsi_gt(concat(a,null_v4float()), concat(b,null_v4float()));
}
static inline uint32 vsi_ge(v8int32 a, v8int32 b) {
       return ge16(concat(a,null_v8int32()),0,0x76543210,0,
                   concat(b,null_v8int32()),0,0x76543210,0);
}
static inline uint32 vsi_ge(v16int32 a, v16int32 b) {
       return vsi_ge(ext_w(a,0), ext_w(b,0));
}
static inline uint32 vsi_ge(v4int32 a, v4int32 b) {
       return vsi_ge(concat(a,null_v4int32()), concat(b,null_v4int32()));
}
static inline uint32 vsi_ge(v8int16 a, v8int16 b) {
       return ge32(concat(concat(a,null_v8int16()),null_v16int16()),0,0x76543210,0,0x3210,
                   concat(concat(b,null_v8int16()),null_v16int16()),0,0x76543210,0,0x3210);
}
static inline uint32 vsi_ge(v8float a, v8float b) {
       return fpge(a,concat(b,null_v8float()),0,0x76543210);
}
static inline uint32 vsi_ge(v4float a, v4float b) {
       return vsi_ge(concat(a,null_v4float()), concat(b,null_v4float()));
}
static inline uint32 vsi_eq(v8int32 a, v8int32 b) {
       return (ge16(concat(a,null_v8int32()),0,0x76543210,0,concat(b,null_v8int32()),0,0x76543210,0) & 
                ~gt16(concat(a,null_v8int32()),0,0x76543210,0,concat(b,null_v8int32()),0,0x76543210,0));
}
static inline uint32 vsi_eq(v16int32 a, v16int32 b) {
       return vsi_eq(ext_w(a,0), ext_w(b,0));
}
static inline uint32 vsi_eq(v4int32 a, v4int32 b) {
       return vsi_eq(concat(a,null_v4int32()), concat(b,null_v4int32()));
}
static inline uint32 vsi_eq(v8int16 a, v8int16 b) {
       return (ge32(concat(concat(a,null_v8int16()),null_v16int16()),0,0x76543210,0,0x3210,
                    concat(concat(b,null_v8int16()),null_v16int16()),0,0x76543210,0,0x3210) &
              ~gt32(concat(concat(a,null_v8int16()),null_v16int16()),0,0x76543210,0,0x3210,
                    concat(concat(b,null_v8int16()),null_v16int16()),0,0x76543210,0,0x3210));
}
static inline uint32 vsi_eq(v8float a, v8float b) {
       return (fpge(a,concat(b,null_v8float()),0,0x76543210) & 
                ~fplt(b,concat(a,null_v8float()),0,0x76543210));
}
static inline uint32 vsi_eq(v4float a, v4float b) {
       return vsi_eq(concat(a,null_v4float()), concat(b,null_v4float()));
}
// shuffle vectors 
// v8int32: use add16 with zero to shuffle 
static inline v8int32 vsi_shuffle(v8int32 v,unsigned int mask){
       v16int32 ai = concat(v,null_v8int32());
       v16int32 rv = add16(null_v16int32(), /* xbuff */
                           0, /* xstart */
                           0, /* xoffset */
                           0, /* xoffset_hi */
                           ai,/* ybuff */
                           0, /* ystart */
                           mask, /* yoffset */
                           0); /* yoffset_hi */
       return ext_w(rv,0);
}
// v8int16: use add32 to shuffle
static inline v8int16 vsi_shuffle(v8int16 v,unsigned int mask){
       v32int16 ai = null_v32int16();
       ai = upd_v(ai,0,v);
       ai = add32(ai,  /* xbuff */
                  0,   /* xstart */
                  mask,/* xoffset */
                  0,   /* xoffset_hi */
                  0x3210, /* xsquare */
                  null_v32int16(), /*ybuff*/
                  0,   /* ystart */
                  0,   /* yoffset */
                  0,   /* yoffset_hi */
                  0x3210); /* ysquare */
       return ext_v(ai,0);
}
//  v8float : use fpadd with 0
static inline v8float vsi_shuffle(v8float v,unsigned int mask){
       return fpadd(null_v8float(), /*acc */
                    concat(v,null_v8float()), /* xbuff (v16float) */
                    0, /* xstart 0 */
                    mask); /* xoffsets */
}
// Shuffle concat : concatenate two vectors and shuffle// v16int32 : v8int32, v8int32, permute mask_hi (lanes 15-8), permute mask_lo (lanes 7-0) 
static inline v16int32 vsi_shuffle(v8int32 v1, v8int32 v2, unsigned int mask_hi, unsigned int mask_lo) {
       return add16(concat(v1,v2),     /* xbuff */
                    0,                 /* xstart */
                    mask_lo,           /* xoffset */
                    mask_hi,           /* xoffset_hi */
                    null_v16int32(),   /* ybuff = zero */
                    0,                 /* ystart */
                    0,0);              /* yoffsets = 0 */
}
// v8int32 : v8int32, v8int32 , permute mask
static inline v8int32 vsi_shuffle(v8int32 v1, v8int32 v2, unsigned int mask) {
       return ext_w(vsi_shuffle(v1,v2,0,mask),0);
}
static inline v8int32 vsi_shuffle(v16int32 v, unsigned int mask) {
       return ext_w (add16(v,0,mask,0,null_v16int32(),0,0,0),0); 
}
static inline v8int32 vsi_shuffle (v32int32 v, unsigned int mask) {
       return ext_w(shuffle16(v,0,mask,0),0); 
}
// v16int16 : v8int16, v8int16, permute mask_hi (lanes 15-8), permute mask_lo (lanes 7-0) 
static inline v16int16 vsi_shuffle (v8int16 v1, v8int16 v2, unsigned int mask_hi, unsigned int mask_lo) {
       return ext_w(add32(concat(concat(v1,v2),null_v16int16()), /* xbuff */
                          0,                                     /* xstart */
                          mask_lo,                               /* xoffset */
                          mask_hi,                               /* xoffset_hi */
                          0x3210,                                /* xsquare */
                          null_v32int16(),                       /* ybuff = zero*/
                          0,                                     /* ystart */
                          0,0,                                   /* yoffsets = 0 */
                          0x3210), 0);
}
static inline v8int16 vsi_shuffle (v8int16 v1, v8int16 v2, unsigned int mask) {
       return ext_v(add32(concat(concat(v1,v2),null_v16int16()), /* xbuff */
                          0,                                     /* xstart */
                          mask,                                  /* xoffset */
                          0,                                     /* xoffset_hi */
                          0x3210,                                /* xsquare */
                          null_v32int16(),                       /* ybuff = zero*/
                          0,                                     /* ystart */
                          0,0,                                   /* yoffsets = 0 */
                          0x3210), 0);
}
static inline v8int16 vsi_shuffle (v16int16 v, unsigned int mask) {
       return ext_v (add32(concat(v,null_v16int16()),0,mask,0,0x3210,
                           null_v32int16(),0,0,0,0x3210),0);
}
static inline v32int32 vsi_shuffle (v16int32 v1, v16int32 v2, unsigned int mask2_hi, unsigned int mask_hi, 
                                                            unsigned int mask2_lo, unsigned int mask_lo) {
       v32int32 v32input = concat(v1,v2); 
       return concat(shuffle16(v32input,0,mask_lo,mask2_lo), 
                     shuffle16(v32input,0,mask_hi,mask2_hi));
}
static inline v32float vsi_shuffle (v16float v1, v16float v2, unsigned int mask2_hi, unsigned int mask_hi, 
                                                            unsigned int mask2_lo, unsigned int mask_lo) {
       v32float v32input = concat(v1,v2); 
       return concat(fpshuffle16(v32input,0,mask_lo,mask2_lo), 
                     fpshuffle16(v32input,0,mask_hi,mask2_hi));
}
static inline v16float vsi_shuffle (v8float v1, v8float v2, unsigned int mask_hi, unsigned int mask_lo) {
       return as_v16float(vsi_shuffle(as_v8int32(v1),as_v8int32(v2),mask_hi,mask_lo));
}
static inline v8float vsi_shuffle (v8float v1, v8float v2, unsigned int mask) {
       return ext_w(vsi_shuffle(v1,v2,0,mask),0);
}
static inline v8float vsi_shuffle (v16float v, unsigned int mask) {
       return as_v8float (vsi_shuffle(as_v16int32(v),mask));
}
static inline v8float vsi_shuffle (v32float v, unsigned int mask) {
       return ext_w(fpshuffle16(v,0,mask,0),0);
}
// Unary negate 
static inline v8int32 vsi_neg (v8int32 v) {
       return ext_w (sub16 (null_v16int32(),0,0,0,
                              concat(v,null_v8int32()),0,0x76543210,0),0);
}
static inline v8int16 vsi_neg (v8int16 v) {
       return ext_v (sub32 (null_v32int16(),0,0,0,0x3210,
                            concat(concat(v,null_v8int16()),null_v16int16()),0,0,0,0x3210),0);
}
static inline v8float vsi_neg (v8float v) {
       return fpsub(null_v8float(),concat(v,null_v8float()),0,0);
}
// window pointer value
#define get_wp(dir,type) inline type * restrict get_wp_##type ( dir##_window_##type *w, int count) {\
       type * restrict rv;\
       window_incr(w,count);\
       rv = (type * restrict) w->ptr;\
       window_incr(w,-count);\
       return rv;\
}
get_wp(input,int8)
get_wp(output,int8)
get_wp(input,int16)
get_wp(output,int16)
get_wp(input,int32)
get_wp(output,int32)
get_wp(input,float)
get_wp(output,float)

/* Single precision floating point using vector unit */
static inline float float_mul(float a, float b){
        return ext_elem(fpmul(upd_elem(null_v16float(), 0, a, 0), 0, 0x0, upd_elem(null_v8float(), 0, b, 0), 0, 0x0), 0);
}
static inline float float_div(float a, float b){
        return ext_elem(fpmul(upd_elem(null_v16float(), 0, a, 0), 0, 0x0, upd_elem(null_v8float(), 0, inv(b), 0), 0, 0x0), 0);
}
static inline float float_add(float a, float b){
        return ext_elem(fpadd(upd_elem(null_v8float(), 0, a, 0), upd_elem(null_v16float(), 0, b, 0), 0, 0x0), 0);
}
static inline float float_sub(float a, float b){
        return ext_elem(fpsub(upd_elem(null_v8float(), 0, a, 0), upd_elem(null_v16float(), 0, b, 0), 0, 0x0), 0);
}
static inline float float_mac(float a, float b, float c){
        return ext_elem(fpmac (upd_elem(null_v8float(), 0, a, 0),
                               upd_elem(null_v16float(), 0, b, 0), 0, 0x0,
                               upd_elem(null_v8float(), 0, c, 0), 0, 0x0), 0);
}
static inline float float_msc(float a, float b, float c){
        return ext_elem(fpmsc (upd_elem(null_v8float(), 0, a, 0),
                               upd_elem(null_v16float(), 0, b, 0), 0, 0x0,
                               upd_elem(null_v8float(), 0, c, 0), 0, 0x0), 0);
}
static inline float float_neg(float a){
        return ext_elem(fpneg(upd_elem(null_v16float(), 0, a, 0), 0, 0x0), 0);
}
static inline float float_mod(float num, float divisor) {
        return float_sub(num, float_mul(fix2float(float2fix(float_sub(float_mul(num, inv(divisor)), 0.5f), 0), 0), divisor));
}
static inline unsigned int float_fpge(float in_acc, float in_xbuf){
	return fpge(upd_elem(null_v8float(), 0, in_acc), upd_elem(null_v32float(), 0, in_xbuf, 0), 0, 0);
}
/* v8 Complex arithmatic*/
static inline v8cfloat vsi_v8cfadd(v8cfloat x, v8cfloat y){
  	return concat(fpadd(ext_w(x,0),y,0,0x3210),fpadd(ext_w(x,1),y,0,0x7654));
}
static inline v8cfloat vsi_v8cfsub(v8cfloat x, v8cfloat y){
  	return concat(fpsub(ext_w(x,0),y,0,0x3210),fpsub(ext_w(x,1),y,0,0x7654));
}
static inline v8cfloat vsi_v8cfmul(v8cfloat x, v8cfloat y){
  	return concat(fpmul(x,0,0x3210,ext_w(y,0),0,0x3210),fpmul(x,0,0x7654,ext_w(y,1),0,0x3210));
}
static inline v8cfloat vsi_v8cfmul(v8cfloat x, v8float y){
  	return concat(fpmul(x,0,0x3210,y,0,0x3210),fpmul(x,0,0x7654,y,0,0x7654));
}
static inline v8cfloat vsi_v8cfmac(v8cfloat acc, v8cfloat x, v8cfloat y){
  	return concat(fpmac(ext_w(acc,0),x,0,0x3210,ext_w(y,0),0,0x3210), fpmac(ext_w(acc,1),x,0,0x7654,ext_w(y,1),0,0x3210));
}
static inline v8cfloat vsi_v8cfmac(v8cfloat acc, v8cfloat x, v8float y){
  	return concat(fpmac(ext_w(acc,0),x,0,0x3210,y,0,0x3210), fpmac(ext_w(acc,1),x,0,0x7654,y,0,0x7654));
}
static inline v8cfloat vsi_v8cfmsc(v8cfloat acc, v8cfloat x, v8cfloat y){
  	return concat(fpmsc(ext_w(acc,0),x,0,0x3210,ext_w(y,0),0,0x3210), fpmsc(ext_w(acc,1),x,0,0x7654,ext_w(y,1),0,0x3210));
}
static inline v8cfloat vsi_v8cfmsc(v8cfloat acc, v8cfloat x, v8float y){
  	return concat(fpmsc(ext_w(acc,0),x,0,0x3210,y,0,0x3210), fpmsc(ext_w(acc,1),x,0,0x7654,y,0,0x7654));
}
static inline v16float vsi_v8cfnorm(v16float x){
  	v16float mulResult = concat(fpmul(x,0,0x76543210,ext_w(x,0),0,0x76543210),fpmul(x,0,0xfedcba98,ext_w(x,1),0,0x76543210));
  	v16float addResult = concat(fpadd(ext_w(mulResult,0), mulResult, 1,0x76543210), fpadd(ext_w(mulResult,1), mulResult, 1,0xfedcba98)); 
  	return concat(fpshuffle(addResult, 0, 0xeca86420), null_v8float());
}
static inline v16float vsi_v8cfconj(v16float x, v8float conjMusk){
  	return concat(fpmul(x,0,0x76543210,conjMusk,0,0x76543210),fpmul(x,0,0xfedcba98,conjMusk,0,0x76543210));
}
static inline v8cint32 vsi_v8cmac(v8cint32 acc, v8cint32 x, v8cint32 y){
  	return concat(srs(mac(lups(ext_w(acc,0),0), ext_w(x,0), ext_w(y,0)), 0), srs(mac(lups(ext_w(acc,1),0), ext_w(x,1), ext_w(y,1)), 0));
}
static inline v8cint32 vsi_v8cmsc(v8cint32 acc, v8cint32 x, v8cint32 y){
  	return concat(srs(msc(lups(ext_w(acc,0),0), ext_w(x,0), ext_w(y,0)), 0), srs(msc(lups(ext_w(acc,1),0), ext_w(x,1), ext_w(y,1)), 0));
}
static inline v16int32 vsi_v8cmac(v16int32 acc, v16int32 x, v8int32 y){
  	return concat(srs(lmac8(lups(ext_w(acc,0), 0), x, 0, 0x76543210, y, 0, 0x33221100),0), srs(lmac8(lups(ext_w(acc,1), 0), x, 0, 0xfedcba98, y, 0, 0x77665544),0));
}
static inline v16int32 vsi_v8cmsc(v16int32 acc, v16int32 x, v8int32 y){
  	return concat(srs(lmsc8(lups(ext_w(acc,0), 0), x, 0, 0x76543210, y, 0, 0x33221100),0), srs(lmsc8(lups(ext_w(acc,1), 0), x, 0, 0xfedcba98, y, 0, 0x77665544),0));
}
static inline v8cint32 vsi_v8cmul(v8cint32 x, v8cint32 y){
  	return concat(srs(mul(ext_w(x,0),ext_w(y,0)),0), srs(mul(ext_w(x,1),ext_w(y,1)),0));
}
static inline v16int32 vsi_v8cmul(v16int32 x, v8int32 y){
  	return concat(srs(lmul8(x,0,0x76543210,y,0,0x33221100),0), srs(lmul8(x,0,0xfedcba98,y,0,0x77665544),0));
}
static inline v16int32 vsi_v8cnorm(v16int32 x){
  	v16int32 mulResult = concat(srs(mul(ext_w(x,0),ext_w(x,0)),0),srs(mul(ext_w(x,1),ext_w(x,1)),0)) ;
  	return add16(mulResult, 0, 0xeca86420, 0x0, 0, 0xfdb97531, 0x0); 
}
static inline v16int32 vsi_v8cconj(v16int32 x, v8int32 conjMusk){
  	return concat(srs(mul(ext_w(x,0),conjMusk),0), srs(mul(ext_w(x,1),conjMusk),0));
}
/* Scaler Complex arithmatic using vector unit*/
static inline double vsi_cfconstructor(float x, float y){
  	v4float result = upd_elem(null_v4float(), 0 , x);
  	result = upd_elem(result, 1 , y);
 	return *(double*)(&result);
}
static inline double vsi_cfadd(double x, double y){
	v8float result = as_v8float(fpadd(as_v4cfloat(concat(*((v4float*)&x), null_v4float())), 
                                      as_v8cfloat(concat(concat(*((v4float*)&y), null_v4float()), null_v8float())), 0, 0x76543210));
    	return *(double*)(&result);
}
static inline double vsi_cfsub(double x, double y){
    	v8float result = as_v8float(fpsub(as_v4cfloat(concat(*((v4float*)&x), null_v4float())), 
                                      as_v8cfloat(concat(concat(*((v4float*)&y), null_v4float()), null_v8float())), 0, 0x76543210));
    	return *(double*)(&result);
}
static inline double vsi_cfmult(double x, double y){
    	v8float result = as_v8float(fpmul(as_v8cfloat(concat(concat(*((v4float*)&x), null_v4float()), null_v8float())), 0,0x76543210, 
                                      as_v4cfloat(concat(*((v4float*)&y), null_v4float())), 0, 0x76543210));
    	return *(double*)(&result);
}
static inline double vsi_cfmultf(double x, double y){
    	v8float result = as_v8float(fpmul(as_v8cfloat(concat(concat(*((v4float*)&x), null_v4float()), null_v8float())), 0,0x76543210, 
                                      concat(*((v4float*)&y), null_v4float()), 0, 0x76543210));
    	return *(double*)(&result);
}
static inline double vsi_cfconj(double x){
    	v8float y_val = null_v8float();
    	y_val = upd_elem(y_val,0, 1.0f);
    	y_val = upd_elem(y_val,1, -1.0f);
    	v8float result = fpmul(concat(*((v8float*)&x), null_v8float()), 0,0x76543210, 
                                      y_val, 0, 0x76543210);
    	return *(double*)(&result);
}
static inline double vsi_cfnorm(double x){
    	v8float mul_r = fpmul(concat(*((v8float*)&x), null_v8float()), 0,0x76543210, 
                          *((v8float*)&x), 0, 0x76543210);
    	v8float result = fpadd(mul_r, concat(mul_r, null_v8float()), 1, 0x76543210);
    	return *(double*)(&result);
}
static inline uint64_t vsi_cconstructor(uint64_t x, uint64_t y){
  v4int32 result = upd_elem(null_v4int32(), 0 , *(uint32_t*)&x);
  result = upd_elem(result, 1 , *(uint32_t*)&y);
  return *(uint64_t*)(&result);
}
static inline uint64_t vsi_cadd(uint64_t x, uint64_t y){
       v8cint32 result = add8(as_v8cint32(concat(*((v8int32*)&x), null_v8int32())), 0, 0x76543210,
                              as_v8cint32(concat(*((v8int32*)&y), null_v8int32())), 0, 0x76543210);
       return *(uint64_t*)(&result);
}
static inline uint64_t vsi_csub(uint64_t x, uint64_t y){
       v8cint32 result = sub8(as_v8cint32(concat(*((v8int32*)&x), null_v8int32())), 0, 0x76543210,
                              as_v8cint32(concat(*((v8int32*)&y), null_v8int32())), 0, 0x76543210);
       return *(uint64_t*)(&result);
}
static inline uint64_t vsi_cmult(uint64_t x, uint64_t y){
       v2cint32 result = srs(mul(*((v2cint32*)&x), *((v2cint32*)&y)), 0);
       return *(uint64_t*)(&result);
}
static inline uint64_t vsi_cmultf(uint64_t x, uint64_t y){
       v8int32 result = srs(lmul8(*((v16int32*)&x), 0, 0x10101010, *((v8int32*)&y), 0, 0x00000000), 0);
       return *(uint64_t*)(&result);
}
static inline uint64_t vsi_cconj(uint64_t x){
    	v4int32 y_val = null_v4int32();
    	y_val = upd_elem(y_val,0, 1);
    	y_val = upd_elem(y_val,1, -1);
    	v4int32 result = srs(mul(*((v4int32*)&x), y_val), 0);
    	return *(uint64_t*)(&result);
}
static inline uint64_t vsi_cnorm(uint64_t x){
    	v4int32 mul_r = srs(mul(*((v4int32*)&x), *((v4int32*)&x)), 0);
    	v16int32 result = add16(concat(concat(mul_r, null_v4int32()), null_v8int32()), 0, 0x0, 0x0, 0, 0x1, 0);
    	return *(uint64_t*)(&result);
}
static inline v8int32 float_to_fix(v8float in) {
   	v8int32 out = null_v8int32();
    	out = upd_elem(out,0,float2fix(ext_elem(in,0),0));
    	out = upd_elem(out,1,float2fix(ext_elem(in,1),0));
    	out = upd_elem(out,2,float2fix(ext_elem(in,2),0));
    	out = upd_elem(out,3,float2fix(ext_elem(in,3),0));
    	out = upd_elem(out,4,float2fix(ext_elem(in,4),0));
    	out = upd_elem(out,5,float2fix(ext_elem(in,5),0));
    	out = upd_elem(out,6,float2fix(ext_elem(in,6),0));
    	out = upd_elem(out,7,float2fix(ext_elem(in,7),0));
    	return out;
}
static inline v8float fix_to_float(v8int32 in) {
    v8float out = null_v8float();
    out = upd_elem(out,0,fix2float(ext_elem(in,0),0));
    out = upd_elem(out,1,fix2float(ext_elem(in,1),0));
    out = upd_elem(out,2,fix2float(ext_elem(in,2),0));
    out = upd_elem(out,3,fix2float(ext_elem(in,3),0));
    out = upd_elem(out,4,fix2float(ext_elem(in,4),0));
    out = upd_elem(out,5,fix2float(ext_elem(in,5),0));
    out = upd_elem(out,6,fix2float(ext_elem(in,6),0));
    out = upd_elem(out,7,fix2float(ext_elem(in,7),0));
    return out;
}
static inline v8float vsi_inv(v8float a) {
    v8float r = null_v8float();
    r = upd_elem(r,0,inv(ext_elem(a,0)));
    r = upd_elem(r,1,inv(ext_elem(a,1)));
    r = upd_elem(r,2,inv(ext_elem(a,2)));
    r = upd_elem(r,3,inv(ext_elem(a,3)));
    r = upd_elem(r,4,inv(ext_elem(a,4)));
    r = upd_elem(r,5,inv(ext_elem(a,5)));
    r = upd_elem(r,6,inv(ext_elem(a,6)));
    r = upd_elem(r,7,inv(ext_elem(a,7)));
    return r;
}
static inline v8float pre_processing(v8float input){
    return( fpmul(xset_w(0,
				fpsub(input, xset_w(0,
					fpmul(xset_w(0,
						fix_to_float(
							float_to_fix(
								fpsub(fpmul(xset_w(0, input), 0, 0x76543210,
									(upd_elem(null_v8float(), 0u, 0.318471337579618f)), 0, 0x0),
									(upd_elem(null_v16float(), 0u, 0.5f)), 0, 0x0)
								)
							)
						), 0, 0x76543210,
					(upd_elem(null_v8float(), 0u, 3.14f)), 0, 0x0)
				), 0, 0x76543210
			)
		), 0, 0x76543210, (upd_elem(null_v8float(), 0u, 0.318471337579618f)), 0, 0x0)
	);
}
static inline v4float pre_processing(v4float input){
	return( ext_v(fpmul(xset_w(0,
				fpsub(concat(input,null_v4float()), xset_w(0,
					fpmul(xset_w(0,
						fix_to_float(
							float_to_fix(
								fpsub(fpmul(xset_v(0, input), 0, 0x76543210,
									(upd_elem(null_v8float(), 0u, 0.318471337579618f)), 0, 0x0),
									(upd_elem(null_v16float(), 0u, 0.5f)), 0, 0x0)
								)
							)
						), 0, 0x76543210,
					(upd_elem(null_v8float(), 0u, 3.14f)), 0, 0x0)
				), 0, 0x76543210
			)
		), 0, 0x76543210, (upd_elem(null_v8float(), 0u, 0.318471337579618f)), 0, 0x0)
	, 0)
	);
}
static inline float float_sin(float input){
    cint16 sincos_val = sincos(float2fix(input, 31));
    return fix2float(sincos_val.imag, 15);
}
static inline float float_cos(float input){
    cint16 sincos_val = sincos(float2fix(input, 31));
    return fix2float(sincos_val.real, 15);
}
static inline v8float pos_processing(v8float input, v8float sincos_output){
    return (vsi_select(
			fpge(
				fpsub(input, xset_w(0,
					fpmul(xset_w(0,
						fix_to_float(
							float_to_fix(
								fpsub(fpmul(xset_w(0, input), 0, 0x76543210,
									(upd_elem(null_v8float(), 0u, 0.159f)), 0, 0x0),
										(upd_elem(null_v16float(), 0u, 0.5f)), 0, 0x0)
									))
								), 0, 0x76543210, (upd_elem(null_v8float(), 0u, 6.283f)), 0, 0x0)
							), 0, 0x76543210),
						upd_elem(null_v16float(), 0u, 3.14f), 0, 0x0),
					fpmul(xset_w(0, sincos_output), 0, 0x76543210,
				(upd_elem(null_v8float(), 0u, -1.0f)), 0, 0x0),
			sincos_output
		)
	);
}
static inline v4float pos_processing(v4float input, v4float sincos_output){
	return (ext_v(vsi_select(
			fpge(
				fpsub(concat(input,null_v4float()), xset_w(0,
					fpmul(xset_w(0,
						fix_to_float(
							float_to_fix(
								fpsub(fpmul(xset_v(0, input), 0, 0x76543210,
									(upd_elem(null_v8float(), 0u, 0.159f)), 0, 0x0),
									(upd_elem(null_v16float(), 0u, 0.5f)), 0, 0x0)
								))
							), 0, 0x76543210, (upd_elem(null_v8float(), 0u, 6.283f)), 0, 0x0)
						), 0, 0x76543210),
					upd_elem(null_v16float(), 0u, 3.14f), 0, 0x0),
				fpmul(xset_v(0, sincos_output), 0, 0x76543210,
					(upd_elem(null_v8float(), 0u, -1.0f)), 0, 0x0),
				concat(sincos_output,null_v4float())
			)
		, 0)
	);
}
static inline v8float pre_processing_fast(v8float input, float divisor){
    return fpsub(input, xset_w(0,
                             fpmul(xset_w(0,
                                          fix_to_float(
                                                       float_to_fix(
                                                                    fpsub(fpmul(xset_w(0, input), 0, 0x76543210,
                                                                                (upd_elem(null_v8float(), 0u, inv(divisor))), 0, 0x0),
                                                                          (upd_elem(null_v16float(), 0u, 0.5f)), 0, 0x0)
                                                                    )
                                                       )
                                          ), 0, 0x76543210,
                                   (upd_elem(null_v8float(), 0u, divisor)), 0, 0x0)
                             ), 0, 0x76543210);
}
static inline v8float pos_processing_sin_fast(v8float input_rad, v8float result){
    	v8float div_pi2 = fpmul(concat(input_rad, null_v8float()), 0, 0x76543210, upd_elem(null_v8float(), 0, 0.6366197724f), 0, 0x00000000);
    	v8float mod_4 = pre_processing_fast(fpabs(concat(div_pi2,null_v8float()), 0, 0x76543210), 4.0f); 
    	uint32 range = vsi_ge(mod_4, null_v8float()) & (~vsi_gt(mod_4, vsi_splat_8(upd_elem(null_v8float(), 0, 2.0f)))); 
    	uint32 select = (range & vsi_gt(input_rad,null_v8float())) | (~range & ~vsi_ge(input_rad,null_v8float()));
    	return vsi_select(select, result, fpneg(concat(result,null_v8float()), 0, 0x76543210));
}
static inline v8float sinBaskara(v8float x) {  // (16 * x * (PI - x) / ( 49.348022005453289f - 4 * x * (PI - x)));
 	v8float pi_x, pi_xx, pi_xx4, numerator, denumerator;
  	pi_x        = fpsub(vsi_splat_8(upd_elem(null_v8float(), 0u, PI)), concat(x,null_v8float()), 0, 0x76543210);
  	pi_xx       = fpmul(concat(x,null_v8float()), 0, 0x76543210, pi_x, 0, 0x76543210);
  	pi_xx4      = fpmul(concat(pi_xx, null_v8float()), 0, 0x76543210, upd_elem(null_v8float(), 0, 4.0f), 0, 0x00000000);
  	numerator   = fpmul(concat(pi_xx4, null_v8float()), 0, 0x76543210, upd_elem(null_v8float(), 0, 4.0f), 0, 0x00000000);
  	denumerator = fpsub(vsi_splat_8(upd_elem(null_v8float(), 0, 49.348022005453289f)), concat(pi_xx4, null_v8float()), 0, 0x76543210);
         return fpmul(concat(numerator, null_v8float()), 0, 0x76543210, aie::inv(aie::vector<float,8>(denumerator)), 0, 0x76543210);
}
static inline v8float cosBaskara(v8float x) { // (1- (20 * x * x / (4 * x * x + 39.478417604362631f)));
  	v8float sq_x, numerator, denumerator, divResult;
  	sq_x        = fpmul(concat(x,null_v8float()), 0, 0x76543210, x, 0, 0x76543210);
  	numerator   = fpmul(concat(sq_x,null_v8float()), 0, 0x76543210, upd_elem(null_v8float(), 0, 20.0f), 0, 0x00000000);
  	denumerator = fpmac(vsi_splat_8(upd_elem(null_v8float(), 0u, 39.478417604362631f)), concat(sq_x,null_v8float()), 0, 0x76543210,
                      upd_elem(null_v8float(), 0, 4.0f), 0, 0x00000000);
  	divResult   = fpneg_mul(concat(numerator, null_v8float()), 0, 0x76543210, aie::inv(aie::vector<float,8>(denumerator)), 0, 0x76543210);
  	return fpadd(vsi_splat_8(upd_elem(null_v8float(), 0, 1.0f)), concat(divResult,null_v8float()), 0, 0x76543210);
}
static inline v8float sinBaskaraUndefRange(v8float x) {  // (16 * x * (PI - x) / ( 49.348022005453289f - 4 * x * (PI - x)));
  	v8float rad_in = pre_processing_fast(fpabs(concat(x,null_v8float()), 0, 0x76543210), 3.141592654f);
  	v8float result = sinBaskara(rad_in);
  	return pos_processing_sin_fast(x, result);
}
static inline v8float cosBaskaraUndefRange(v8float x) { // (1- (20 * x * x / (4 * x * x + 39.478417604362631f)));
    	v8float xmod_2pi, xmod_pi, sq_x, numerator, denumerator, divResult, result;
	// Pre-processing
  	xmod_pi = pre_processing_fast(fpabs(concat(x,null_v8float()), 0, 0x76543210), 3.141592654f);
  	xmod_2pi = pre_processing_fast(fpabs(concat(x,null_v8float()), 0, 0x76543210), 6.283185307f);
  	uint32 gt_pi = vsi_gt(xmod_2pi, vsi_splat_8(upd_elem(null_v8float(), 0u, 3.141592654f)));
  	uint32 gt_pi2 = vsi_gt(xmod_2pi, vsi_splat_8(upd_elem(null_v8float(), 0u, 1.570796327f)));
  	uint32 gt_pi3_2 = vsi_gt(xmod_2pi, vsi_splat_8(upd_elem(null_v8float(), 0u, 4.71238898f)));
  	v8float rad_in = vsi_select(gt_pi, vsi_select(gt_pi3_2, fpsub(vsi_splat_8(upd_elem(null_v8float(), 0u, 6.283185307f)), concat(xmod_2pi, null_v8float()), 0, 0x76543210), xmod_pi),
                              vsi_select(gt_pi2, fpsub(vsi_splat_8(upd_elem(null_v8float(), 0u, 3.141592654f)), concat(xmod_2pi, null_v8float()), 0, 0x76543210), xmod_2pi));
  	// Cos Baskara function
  	result = cosBaskara(rad_in);
  	// Pos-processing
  	uint32 select = gt_pi2 & ~gt_pi3_2;
  	return vsi_select(select, fpneg(concat(result,null_v8float()), 0, 0x76543210), result);
}
static inline uint32 vector_reduction(v8int32 v) {
       return (ext_elem(v,0)+ ext_elem(v,1)+ ext_elem(v,2)+ ext_elem(v,3)+
               ext_elem(v,4)+ ext_elem(v,5)+ ext_elem(v,6)+ ext_elem(v,7));
}
static inline v16float vsi_v16float_add(v16float a, v16float b) {
       return concat(fpadd(ext_w(a,0),b,0,0x76543210),
                     fpadd(ext_w(a,1),b,8,0x76543210)); 
} 
static inline v16float vsi_v16float_mul(v16float a, v16float b) {
       return concat(fpmul(a,0,0x76543210,ext_w(b,0),0,0x76543210),
                     fpmul(a,8,0x76543210,ext_w(b,1),0,0x76543210));
}
static inline v16float vsi_v16float_sub(v16float a, v16float b) {
       return concat(fpsub(ext_w(a,0),b,0,0x76543210),
                     fpsub(ext_w(a,1),b,8,0x76543210));
}
static inline v4float vsi_v4float_add(v8float a, v8float b, int offset = 0,unsigned int mask = 0x76543210) {
        return ext_v(fpadd(a,concat(b,null_v8float()),offset,mask),0);
}
static inline v4float vsi_v4float_add(v4float a, v8float b, int offset = 0, unsigned int mask = 0x76543210) {
        return vsi_v4float_add(concat(a,null_v4float()),b,offset,mask);
}
static inline v4float vsi_v4float_add(v4float a, v4float b,int offset = 0,unsigned int mask = 0x76543210) {
        return vsi_v4float_add(concat(a,null_v4float()),concat(b,null_v4float()),offset,mask);
}
static inline v4float vsi_v4float_add(v8float a, v4float b,int offset = 0, unsigned int mask = 0x76543210) {
        return vsi_v4float_add(a,concat(b,null_v4float()),offset, mask);
}
static inline v4float vsi_v4float_add(v8float a, v8float b,unsigned int offsetb, unsigned int maskb) {
        return ext_v(fpadd(a,concat(b,null_v8float()),offsetb,maskb),0);
}
static inline v4float vsi_v4float_sub(v8float a, v8float b,int offset = 0,unsigned int mask = 0x76543210) {
        return ext_v(fpsub(a,concat(b,null_v8float()),offset,mask),0);
}
static inline v4float vsi_v4float_sub(v4float a, v8float b,int offset = 0, unsigned int mask = 0x76543210) {
        return vsi_v4float_sub(concat(a,null_v4float()),b,offset,mask);
}
static inline v4float vsi_v4float_sub(v4float a, v4float b , int offset = 0, unsigned int mask = 0x76543210) {
        return vsi_v4float_sub(concat(a,null_v4float()),concat(b,null_v4float()),offset,mask);
}
static inline v4float vsi_v4float_sub(v8float a, v4float b, int offset = 0, unsigned int mask = 0x76543210) {
        return vsi_v4float_sub(a,concat(b,null_v4float()),offset,mask);
}
static inline v4float vsi_v4float_sub(v8float a, v8float b, unsigned int offsetb, unsigned int maskb) {
        return ext_v(fpsub(a,concat(b,null_v8float()),offsetb,maskb),0);
}
static inline v4float vsi_v4float_mul(v8float a, v8float b) {
        return ext_v(fpmul(concat(a,null_v8float()),0,0x76543210,b,0,0x76543210),0);
}
static inline v4float vsi_v4float_mul(v4float a, v4float b) {
        return vsi_v4float_mul(concat(a,null_v4float()),concat(b,null_v4float()));
}
static inline v4float vsi_v4float_mul(v8float a, unsigned int offseta, unsigned int maska, v8float b) {
        return ext_v(fpmul(concat(a,null_v8float()),offseta,maska,b,0,0x76543210),0);
}
static inline v4float vsi_v4float_mul(v4float a, unsigned int offseta, unsigned int maska, v4float b) {
        return vsi_v4float_mul(concat(a,null_v4float()), offseta, maska, concat(b,null_v4float()));
}
static inline v4float vsi_v4float_mul(v8float a, unsigned int offseta, unsigned int maska, v8float b, unsigned int offsetb, unsigned int maskb) {
        return ext_v(fpmul(concat(a,null_v8float()),offseta,maska,b,offsetb,maskb),0);
}
static inline v8float vsi_v8float_mul(v8float a, unsigned int offseta, unsigned int maska, v8float b) {
        return fpmul(concat(a,null_v8float()),offseta,maska,b,0,0x76543210);
}
static inline v8float vsi_v8float_sub(v8float a, v8float b, unsigned int offsetb, unsigned int maskb) {
        return fpsub(a,concat(b,null_v8float()),offsetb,maskb);
}
static inline v16int32 vsi_v16int32_mul(v16int32 a, v16int32 b) {
        return concat(srs(mul(ext_w(a,0), ext_w(b,0)),0) ,null_v8int32());
}
static inline v8int32 vsi_v8SExt16to32(v8int16 a) {
   v8int32 r= null_v8int32(); 
   r = upd_elem(r, 0, ext_elem(a,0));
   r = upd_elem(r, 1, ext_elem(a,1));
   r = upd_elem(r, 2, ext_elem(a,2));
   r = upd_elem(r, 3, ext_elem(a,3));
   r = upd_elem(r, 4, ext_elem(a,4));
   r = upd_elem(r, 5, ext_elem(a,5));
   r = upd_elem(r, 6, ext_elem(a,6));
   r = upd_elem(r, 7, ext_elem(a,7));
   return r;
}

#ifndef _MSC_VER
#define __forceinline __attribute__((always_inline)) inline
#endif



/* Global Declarations */

/* Types Declarations */

/* Types used in Compare instructions */

/* Function definitions */

/* Types Definitions */

/* Function Declarations */


/* LLVM Intrinsic Builtin Function Bodies */


/* Function Bodies */

void AIE_mulTest(input_window_float *  restrict  A, input_window_float *  restrict  B, output_window_float *  restrict  C) {
  uint64_t index;
  uint64_t index__PHI_TEMPORARY;

#line 1 "/net/shared/kafi/AIE_mulTest/cc_files/AIE_mulTest_2_inputs.cc"
  ;
  ;
  ;
#line 4 "/net/shared/kafi/AIE_mulTest/cc_files/AIE_mulTest_2_inputs.cc"
  ;
  index__PHI_TEMPORARY = 0;   /* for PHI node */

 for (;;)      /* Syntactic loop 'vector.body prevent chess from unrolling */
 chess_unroll_loop(1) 
 { 
vector_body:
  index = index__PHI_TEMPORARY;
#line 5 "/net/shared/kafi/AIE_mulTest/cc_files/AIE_mulTest_2_inputs.cc"
  v8float wide_load = *(v8float*)((float*)get_wp_float(A,index));
  v8float wide_load11 = *(v8float*)((float*)get_wp_float(B,index));
  
{v8float  *store_temp_0 = (v8float*)((float*)get_wp_float(C,index));
   *store_temp_0 = as_v8float(aie::mul(aie::vector<float,8>(wide_load11),aie::vector<float,8>(wide_load)));}
;
#line 4 "/net/shared/kafi/AIE_mulTest/cc_files/AIE_mulTest_2_inputs.cc"
  if (chess_copy(index) == 0) {
    break;
  } else {
    index__PHI_TEMPORARY = ((uint64_t)(((uint64_t)index) + ((uint64_t)8)));   /* for PHI node */
    continue;
  }

  } /* end of syntactic loop 'vector.body' */
for_cond_cleanup:
  return;
}

