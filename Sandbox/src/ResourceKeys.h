#pragma once

#define RES_DEFAULT "default"


#define RTV_MAIN "main"



// scene
#define RTV_SCENE_MS "scene_ms"
#define DSV_SCENE_MS "scene_ms"
#define SRV_SCENE "scene"

// variance shadow map
#define RTV_VSM_MS   "vsm_ms"
#define DSV_VSM_MS   "vsm_ms"
#define SRV_VSM_MS   "vsm_ms"
#define RTV_VSM_TEMP "vsm_temp"
#define SRV_VSM_TEMP "vsm_temp"
#define RTV_VSM		 "vsm"
#define SRV_VSM		 "vsm"

// normap shadow map
#define DSV_BASIC_SMAP	"basic_smap"
#define SRV_BASIC_SMAP	"basic_smap"

// shaders
#define NULL_SHADER "null"
#define VS_PHONG "phong"
#define PS_PHONG "phong"
#define IL_PHONG "phong"
#define VS_FS_OUT_TC_POS "fullscreen_out_tc_pos"
#define PS_GAMMA_CORRECTION "gamma_correction"
#define IL_FS_OUT_TC_POS "fullscreen_out_tc_pos"
#define VS_BASIC "basic"
#define PS_BASIC "basic"
#define PS_VSM   "vsm"
#define IL_BASIC "basic"
#define PS_BLUR   "blur"

// cbuf
#define CB_VS_PHONG_SYSTEM "phong.vs.SystemCBuf"
#define CB_VS_PHONG_ENTITY "phong.vs.EntityCBuf"
#define CB_PS_PHONG_SYSTEM "phong.ps.SystemCBuf"
#define CB_PS_PHONG_ENTITY "phong.ps.EntityCBuf"
#define CB_PS_GAMMA_CORRECTION_SYSTEM "gamma_correction.ps.SystemCBuf"
#define CB_VS_BASIC_SYSTEM "basic.vs.SystemCBuf"
#define CB_VS_BASIC_ENTITY "basic.vs.EntityCBuf"
#define CB_PS_BASIC_ENTITY "basic.ps.EntityCBuf"
#define CB_PS_BLUR_KERNEL  "blur.ps.KernelCBuf"
#define CB_PS_BLUR_CONTROL "blur.ps.ControlCBuf"

// mesh
#define VB_CUBE "cube.vb"
#define IB_CUBE "cube.ib"
#define VB_PLANE "plane.vb"
#define IB_PLANE "plane.ib"
#define VB_FS_QUAD "fs_quad.vb"
#define IB_FS_QUAD "fs_quad.ib"

// States
#define SS_ANISO_WRAP "anisotropic_wrap"
#define SS_ANISO_CLAMP "anisotropic_clamp"
#define RS_DEPTH_SLOPE_SCALED_BIAS "depth_slope_scaled_bias"
#define SS_CMP_LESS_EQUAL_LINEAR_CLAMP "cmp_less_equal_linear_clamp"
#define SS_LINEAR_CLAMP "linear_clamp"
#define SS_POINT_CLAMP "point_clamp"