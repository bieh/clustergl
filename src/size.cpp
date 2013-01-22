/*******************************************************************************
	ClusterGL - Size calculations
*******************************************************************************/

#include "main.h"
#include <iostream>

int getTextureParamSize(GLenum type){
	switch(type)
	{
	case GL_TEXTURE_WIDTH: return 1;    
	case GL_TEXTURE_HEIGHT: return 1;   
	case GL_TEXTURE_DEPTH: return 1;   
	case GL_TEXTURE_INTERNAL_FORMAT: return 1;  
	case GL_TEXTURE_BORDER: return 1;   
	case GL_TEXTURE_RED_SIZE: return 1;
	case GL_TEXTURE_BLUE_SIZE: return 1;
	case GL_TEXTURE_ALPHA_SIZE: return 1;
	case GL_TEXTURE_LUMINANCE_SIZE: return 1;
	case GL_TEXTURE_INTENSITY_SIZE: return 1;
	default: LOG("DEFAULTED getTextureParamSize!\n"); return 1;
	}
}

int getGetSize(GLenum type){
	switch(type)
	{
	case GL_ACCUM_ALPHA_BITS: return 1;
	case GL_ACCUM_BLUE_BITS: return 1;
	case GL_ACCUM_CLEAR_VALUE: return 4;
	case GL_ACCUM_GREEN_BITS: return 1;
	case GL_ACCUM_RED_BITS: return 1;
	case GL_ACTIVE_TEXTURE: return 1;
	case GL_ALIASED_POINT_SIZE_RANGE: return 2;
	case GL_ALIASED_LINE_WIDTH_RANGE: return 2;
	case GL_ALPHA_BIAS: return 1;
	case GL_ALPHA_BITS: return 1;
	case GL_ALPHA_SCALE: return 1;
	case GL_ALPHA_TEST: return 1;
	case GL_ALPHA_TEST_REF: return 1;
	case GL_ARRAY_BUFFER_BINDING: return 1;
	case GL_ATTRIB_STACK_DEPTH: return 1;
	case GL_AUTO_NORMAL: return 1;
	case GL_AUX_BUFFERS: return 1;
	case GL_BLEND: return 1;
	case GL_BLEND_COLOR: return 4;
	case GL_BLEND_DST: return 1;
	case GL_BLEND_DST_ALPHA: return 1;
	case GL_BLEND_DST_RGB: return 1;
	case GL_BLEND_EQUATION_RGB: return 1;
	case GL_BLEND_EQUATION_ALPHA: return 1;
	case GL_BLEND_SRC: return 1;
	case GL_BLEND_SRC_ALPHA: return 1;
	case GL_BLEND_SRC_RGB: return 1;
	case GL_BLUE_BIAS: return 1;
	case GL_BLUE_BITS: return 1;
	case GL_BLUE_SCALE: return 1;
	case GL_CLIENT_ACTIVE_TEXTURE: return 1;
	case GL_CLIENT_ATTRIB_STACK_DEPTH: return 1;
	case GL_CLIP_PLANE0: return 1;
	case GL_CLIP_PLANE1: return 1;
	case GL_CLIP_PLANE2: return 1;		
	case GL_CLIP_PLANE3: return 1;
	case GL_CLIP_PLANE4: return 1;
	case GL_CLIP_PLANE5: return 1;
	case GL_COLOR_ARRAY: return 1;
	case GL_COLOR_ARRAY_BUFFER_BINDING: return 1;
	case GL_COLOR_ARRAY_SIZE: return 1;
	case GL_COLOR_ARRAY_STRIDE: return 1;
	case GL_COLOR_ARRAY_TYPE: return 1;
	case GL_COLOR_CLEAR_VALUE: return 4;
	case GL_COLOR_LOGIC_OP: return 1;
	case GL_COLOR_MATERIAL: return 1;
	case GL_COLOR_MATERIAL_FACE: return 1;
	case GL_COLOR_MATERIAL_PARAMETER: return 1;
	case GL_COLOR_MATRIX: return 16;
	case GL_COLOR_MATRIX_STACK_DEPTH: return 1;
	case GL_COLOR_SUM: return 1;
	case GL_COLOR_TABLE: return 1;
	case GL_COLOR_WRITEMASK: return 4;
//	case GL_COMPRESSED_TEXTURE_FORMATS: return -1;
	case GL_CONVOLUTION_1D: return 1;
	case GL_CONVOLUTION_2D: return 1;
	case GL_CULL_FACE: return 1;
	case GL_CULL_FACE_MODE: return 1;
	case GL_CURRENT_COLOR: return 4;
	case GL_CURRENT_FOG_COORD: return 1;
	case GL_CURRENT_INDEX: return 1;
	case GL_CURRENT_NORMAL: return 3;
	case GL_CURRENT_PROGRAM: return 1;
	case GL_CURRENT_RASTER_COLOR: return 4;
	case GL_CURRENT_RASTER_DISTANCE: return 1;
	case GL_CURRENT_RASTER_INDEX: return 1;
	case GL_CURRENT_RASTER_POSITION: return 4;
	case GL_CURRENT_RASTER_POSITION_VALID: return 1;
	case GL_CURRENT_RASTER_SECONDARY_COLOR: return 4;
	case GL_CURRENT_RASTER_TEXTURE_COORDS: return 4;
	case GL_CURRENT_SECONDARY_COLOR: return 4;
	case GL_CURRENT_TEXTURE_COORDS: return 4;
	case GL_DEPTH_BIAS: return 1;
	case GL_DEPTH_BITS: return 1;
	case GL_DEPTH_CLEAR_VALUE: return 1;
	case GL_DEPTH_FUNC: return 1;
	case GL_DEPTH_RANGE: return 2;
	case GL_DEPTH_SCALE: return 1;
	case GL_DEPTH_TEST: return 1;
	case GL_DEPTH_WRITEMASK: return 1;
	case GL_DITHER: return 1;
	case GL_DOUBLEBUFFER: return 1;
	case GL_DRAW_BUFFER: return 1;
	case GL_DRAW_BUFFER0: return 1;
	case GL_DRAW_BUFFER1: return 1;
	case GL_DRAW_BUFFER2: return 1;
	case GL_DRAW_BUFFER3: return 1;
	case GL_DRAW_BUFFER4: return 1;
	case GL_DRAW_BUFFER5: return 1;
	case GL_DRAW_BUFFER6: return 1;
	case GL_DRAW_BUFFER7: return 1;		
	case GL_DRAW_BUFFER8: return 1;	
	case GL_DRAW_BUFFER9: return 1;	
	case GL_DRAW_BUFFER10: return 1;									
	case GL_EDGE_FLAG: return 1;
	case GL_EDGE_FLAG_ARRAY: return 1;
	case GL_EDGE_FLAG_ARRAY_BUFFER_BINDING: return 1;
	case GL_EDGE_FLAG_ARRAY_STRIDE: return 1;
	case GL_ELEMENT_ARRAY_BUFFER_BINDING: return 1;
	case GL_FEEDBACK_BUFFER_SIZE: return 1;
	case GL_FEEDBACK_BUFFER_TYPE: return 1;
	case GL_FOG: return 1;
	case GL_FOG_COORD_ARRAY: return 1;
	case GL_FOG_COORD_ARRAY_BUFFER_BINDING: return 1;
	case GL_FOG_COORD_ARRAY_STRIDE: return 1;
	case GL_FOG_COORD_ARRAY_TYPE: return 1;
	case GL_FOG_COORD_SRC: return 1;
	case GL_FOG_COLOR: return 4;
	case GL_FOG_DENSITY: return 1;
	case GL_FOG_END: return 1;
	case GL_FOG_HINT: return 1;
	case GL_FOG_INDEX: return 1;
	case GL_FOG_MODE: return 1;
	case GL_FOG_START: return 1;
#ifdef GL_FRAMEBUFFER_BINDING
	case GL_FRAMEBUFFER_BINDING: return 1;
#endif
	case GL_FRAGMENT_SHADER_DERIVATIVE_HINT: return 1;
	case GL_FRONT_FACE: return 1;
	case GL_GENERATE_MIPMAP_HINT: return 1;
	case GL_GREEN_BIAS: return 1;
	case GL_GREEN_BITS: return 1;
	case GL_GREEN_SCALE: return 1;
	case GL_HISTOGRAM: return 1;
	case GL_INDEX_ARRAY: return 1;
	case GL_INDEX_ARRAY_BUFFER_BINDING: return 1;
	case GL_INDEX_ARRAY_STRIDE: return 1;
	case GL_INDEX_ARRAY_TYPE: return 1;
	case GL_INDEX_BITS: return 1;
	case GL_INDEX_CLEAR_VALUE: return 1;
	case GL_INDEX_LOGIC_OP: return 1;
	case GL_INDEX_MODE: return 1;
	case GL_INDEX_OFFSET: return 1;
	case GL_INDEX_SHIFT: return 1;
	case GL_INDEX_WRITEMASK: return 1;
	case GL_LIGHT0: return 1;
	case GL_LIGHT1: return 1;
	case GL_LIGHT2: return 1;
	case GL_LIGHT3: return 1;
	case GL_LIGHT4: return 1;
	case GL_LIGHT5: return 1;
	case GL_LIGHT6: return 1;
	case GL_LIGHT7: return 1;
	case GL_LIGHTING: return 1;
	case GL_LIGHT_MODEL_AMBIENT: return 4;
	case GL_LIGHT_MODEL_COLOR_CONTROL: return 1;
	case GL_LIGHT_MODEL_LOCAL_VIEWER: return 1;
	case GL_LIGHT_MODEL_TWO_SIDE: return 1;
	case GL_LINE_SMOOTH: return 1;
	case GL_LINE_SMOOTH_HINT: return 1;
	case GL_LINE_STIPPLE: return 1;
	case GL_LINE_STIPPLE_PATTERN: return 1;
	case GL_LINE_STIPPLE_REPEAT: return 1;
	case GL_LINE_WIDTH: return 1;
	case GL_LINE_WIDTH_GRANULARITY: return 1;
	case GL_LINE_WIDTH_RANGE: return 2;
	case GL_LIST_BASE: return 1;
	case GL_LIST_INDEX: return 1;
	case GL_LIST_MODE: return 1;
	case GL_LOGIC_OP_MODE: return 1;
	case GL_MAP1_COLOR_4: return 1;
	case GL_MAP1_GRID_DOMAIN: return 2;
	case GL_MAP1_GRID_SEGMENTS: return 1;
	case GL_MAP1_INDEX: return 1;
	case GL_MAP1_NORMAL: return 1;
	case GL_MAP1_TEXTURE_COORD_1: return 1;
	case GL_MAP1_TEXTURE_COORD_2: return 1;
	case GL_MAP1_TEXTURE_COORD_3: return 1;
	case GL_MAP1_TEXTURE_COORD_4: return 1;
	case GL_MAP1_VERTEX_3: return 1;
	case GL_MAP1_VERTEX_4: return 1;
	case GL_MAP2_COLOR_4: return 1;
	case GL_MAP2_GRID_DOMAIN: return 4;
	case GL_MAP2_GRID_SEGMENTS: return 2;
	case GL_MAP2_INDEX: return 1;
	case GL_MAP2_NORMAL: return 1;
	case GL_MAP2_TEXTURE_COORD_1: return 1;
	case GL_MAP2_TEXTURE_COORD_2: return 1;
	case GL_MAP2_TEXTURE_COORD_3: return 1;
	case GL_MAP2_TEXTURE_COORD_4: return 1;
	case GL_MAP2_VERTEX_3: return 1;
	case GL_MAP2_VERTEX_4: return 1;
	case GL_MAP_COLOR: return 1;
	case GL_MAP_STENCIL: return 1;
	case GL_MATRIX_MODE: return 1;
	case GL_MAX_3D_TEXTURE_SIZE: return 1;
	case GL_MAX_CLIENT_ATTRIB_STACK_DEPTH: return 1;
	case GL_MAX_ATTRIB_STACK_DEPTH: return 1;
	case GL_MAX_CLIP_PLANES: return 1;
#ifdef GL_MAX_COLOR_ATTACHMENTS
	case GL_MAX_COLOR_ATTACHMENTS: return 1;
#endif
	case GL_MAX_COLOR_MATRIX_STACK_DEPTH: return 1;
	case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS: return 1;
	case GL_MAX_CUBE_MAP_TEXTURE_SIZE: return 1;
	case GL_MAX_DRAW_BUFFERS: return 1;
	case GL_MAX_ELEMENTS_INDICES: return 1;
	case GL_MAX_ELEMENTS_VERTICES: return 1;
	case GL_MAX_EVAL_ORDER: return 1;
	case GL_MAX_FRAGMENT_UNIFORM_COMPONENTS: return 1;
	case GL_MAX_LIGHTS: return 1;
	case GL_MAX_LIST_NESTING: return 1;
	case GL_MAX_MODELVIEW_STACK_DEPTH: return 1;
	case GL_MAX_NAME_STACK_DEPTH: return 1;
	case GL_MAX_PIXEL_MAP_TABLE: return 1;
	case GL_MAX_PROJECTION_STACK_DEPTH: return 1;
#ifdef GL_MAX_SAMPLES
	case GL_MAX_SAMPLES: return 1;
#endif
	case GL_MAX_TEXTURE_COORDS: return 1;
	case GL_MAX_TEXTURE_IMAGE_UNITS: return 1;
	case GL_MAX_TEXTURE_LOD_BIAS: return 1;
	case GL_MAX_TEXTURE_SIZE: return 1;
	case GL_MAX_TEXTURE_STACK_DEPTH: return 1;
	case GL_MAX_TEXTURE_UNITS: return 1;
	case GL_MAX_VARYING_FLOATS: return 1;
	case GL_MAX_VERTEX_ATTRIBS: return 1;
	case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS: return 1;
	case GL_MAX_VERTEX_UNIFORM_COMPONENTS: return 1;
	case GL_MAX_VIEWPORT_DIMS: return 2;
	case GL_MINMAX: return 1;
	case GL_MODELVIEW_MATRIX: return 16;
	case GL_MODELVIEW_STACK_DEPTH: return 1;
	case GL_NAME_STACK_DEPTH: return 1;
	case GL_NORMAL_ARRAY: return 1;
	case GL_NORMAL_ARRAY_BUFFER_BINDING: return 1;
	case GL_NORMAL_ARRAY_STRIDE: return 1;
	case GL_NORMAL_ARRAY_TYPE: return 1;
	case GL_NORMALIZE: return 1;
	case GL_NUM_COMPRESSED_TEXTURE_FORMATS: return 1;
	case GL_PACK_ALIGNMENT: return 1;
	case GL_PACK_IMAGE_HEIGHT: return 1;
	case GL_PACK_LSB_FIRST: return 1;
	case GL_PACK_ROW_LENGTH: return 1;
	case GL_PACK_SKIP_IMAGES: return 1;
	case GL_PACK_SKIP_PIXELS: return 1;
	case GL_PACK_SKIP_ROWS: return 1;
	case GL_PACK_SWAP_BYTES: return 1;
	case GL_PERSPECTIVE_CORRECTION_HINT: return 1;
	case GL_PIXEL_MAP_A_TO_A_SIZE: return 1;
	case GL_PIXEL_MAP_B_TO_B_SIZE: return 1;
	case GL_PIXEL_MAP_G_TO_G_SIZE: return 1;
	case GL_PIXEL_MAP_I_TO_A_SIZE: return 1;
	case GL_PIXEL_MAP_I_TO_B_SIZE: return 1;
	case GL_PIXEL_MAP_I_TO_G_SIZE: return 1;
	case GL_PIXEL_MAP_I_TO_I_SIZE: return 1;
	case GL_PIXEL_MAP_I_TO_R_SIZE: return 1;
	case GL_PIXEL_MAP_R_TO_R_SIZE: return 1;
	case GL_PIXEL_MAP_S_TO_S_SIZE: return 1;
	case GL_PIXEL_PACK_BUFFER_BINDING: return 1;
	case GL_PIXEL_UNPACK_BUFFER_BINDING: return 1;
	case GL_POINT_DISTANCE_ATTENUATION: return 3;
	case GL_POINT_FADE_THRESHOLD_SIZE: return 1;
	case GL_POINT_SIZE: return 1;
	case GL_POINT_SIZE_GRANULARITY: return 1;
	case GL_POINT_SIZE_MAX: return 1;
	case GL_POINT_SIZE_MIN: return 1;
	case GL_POINT_SIZE_RANGE: return 2;
	case GL_POINT_SMOOTH: return 1;
	case GL_POINT_SMOOTH_HINT: return 1;
	case GL_POINT_SPRITE: return 1;
	case GL_POLYGON_MODE: return 2;
	case GL_POLYGON_OFFSET_FACTOR: return 1;
	case GL_POLYGON_OFFSET_UNITS: return 1;
	case GL_POLYGON_OFFSET_FILL: return 1;
	case GL_POLYGON_OFFSET_LINE: return 1;
	case GL_POLYGON_OFFSET_POINT: return 1;
	case GL_POLYGON_SMOOTH: return 1;
	case GL_POLYGON_SMOOTH_HINT: return 1;
	case GL_POLYGON_STIPPLE: return 1;
	case GL_POST_COLOR_MATRIX_COLOR_TABLE: return 1;
	case GL_POST_COLOR_MATRIX_RED_BIAS: return 1;
	case GL_POST_COLOR_MATRIX_GREEN_BIAS: return 1;
	case GL_POST_COLOR_MATRIX_BLUE_BIAS: return 1;
	case GL_POST_COLOR_MATRIX_ALPHA_BIAS: return 1;
	case GL_POST_COLOR_MATRIX_RED_SCALE: return 1;
	case GL_POST_COLOR_MATRIX_GREEN_SCALE: return 1;
	case GL_POST_COLOR_MATRIX_BLUE_SCALE: return 1;
	case GL_POST_COLOR_MATRIX_ALPHA_SCALE: return 1;
	case GL_POST_CONVOLUTION_COLOR_TABLE: return 1;
	case GL_POST_CONVOLUTION_RED_BIAS: return 1;
	case GL_POST_CONVOLUTION_GREEN_BIAS: return 1;
	case GL_POST_CONVOLUTION_BLUE_BIAS: return 1;
	case GL_POST_CONVOLUTION_ALPHA_BIAS: return 1;
	case GL_POST_CONVOLUTION_RED_SCALE: return 1;
	case GL_POST_CONVOLUTION_GREEN_SCALE: return 1;
	case GL_POST_CONVOLUTION_BLUE_SCALE: return 1;
	case GL_POST_CONVOLUTION_ALPHA_SCALE: return 1;
	case GL_PROJECTION_MATRIX: return 16;
	case GL_PROJECTION_STACK_DEPTH: return 1;
	case GL_READ_BUFFER: return 1;
	case GL_RED_BIAS: return 1;
	case GL_RED_BITS: return 1;
	case GL_RED_SCALE: return 1;
	case GL_RENDER_MODE: return 1;
	case GL_RESCALE_NORMAL: return 1;
	case GL_RGBA_MODE: return 1;
	case GL_SAMPLE_BUFFERS: return 1;
	case GL_SAMPLE_COVERAGE_VALUE: return 1;
	case GL_SAMPLE_COVERAGE_INVERT: return 1;
	case GL_SAMPLES: return 1;
	case GL_SCISSOR_BOX: return 4;
	case GL_SCISSOR_TEST: return 1;
	case GL_SECONDARY_COLOR_ARRAY: return 1;
	case GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING: return 1;
	case GL_SECONDARY_COLOR_ARRAY_SIZE: return 1;
	case GL_SECONDARY_COLOR_ARRAY_STRIDE: return 1;
	case GL_SECONDARY_COLOR_ARRAY_TYPE: return 1;
	case GL_SEPARABLE_2D: return 1;
	case GL_SHADE_MODEL: return 1;
//	case GL_SMOOTH_LINE_WIDTH_RANGE: return 2
//	case GL_SMOOTH_LINE_WIDTH_GRANULARITY: return 1;
//	case GL_SMOOTH_POINT_SIZE_RANGE: return 2;
//	case GL_SMOOTH_POINT_SIZE_GRANULARITY: return 1;
	case GL_STENCIL_BACK_FAIL: return 1;
	case GL_STENCIL_BACK_FUNC: return 1;
	case GL_STENCIL_BACK_PASS_DEPTH_FAIL: return 1;
	case GL_STENCIL_BACK_PASS_DEPTH_PASS: return 1;
	case GL_STENCIL_BACK_REF: return 1;
	case GL_STENCIL_BACK_VALUE_MASK: return 1;
	case GL_STENCIL_BACK_WRITEMASK: return 1;
	case GL_STENCIL_BITS: return 1;
	case GL_STENCIL_CLEAR_VALUE: return 1;
	case GL_STENCIL_FAIL: return 1;
	case GL_STENCIL_FUNC: return 1;
	case GL_STENCIL_PASS_DEPTH_FAIL: return 1;
	case GL_STENCIL_PASS_DEPTH_PASS: return 1;
	case GL_STENCIL_REF: return 1;
	case GL_STENCIL_TEST: return 1;
	case GL_STENCIL_VALUE_MASK: return 1;
	case GL_STENCIL_WRITEMASK: return 1;
	case GL_STEREO: return 1;
	case GL_SUBPIXEL_BITS: return 1;
	case GL_TEXTURE_1D: return 1;
	case GL_TEXTURE_BINDING_1D: return 1;
	case GL_TEXTURE_2D: return 1;
	case GL_TEXTURE_BINDING_2D: return 1;
	case GL_TEXTURE_3D: return 1;
	case GL_TEXTURE_BINDING_3D: return 1;
	case GL_TEXTURE_BINDING_CUBE_MAP: return 1;
	case GL_TEXTURE_COMPRESSION_HINT: return 1;
	case GL_TEXTURE_COORD_ARRAY: return 1;
	case GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING: return 1;
	case GL_TEXTURE_COORD_ARRAY_SIZE: return 1;
	case GL_TEXTURE_COORD_ARRAY_STRIDE: return 1;
	case GL_TEXTURE_COORD_ARRAY_TYPE: return 1;
	case GL_TEXTURE_CUBE_MAP: return 1;
	case GL_TEXTURE_GEN_Q: return 1;
	case GL_TEXTURE_GEN_R: return 1;
	case GL_TEXTURE_GEN_S: return 1;
	case GL_TEXTURE_GEN_T: return 1;
	case GL_TEXTURE_MATRIX: return 16;
	case GL_TEXTURE_STACK_DEPTH: return 1;
	case GL_TRANSPOSE_COLOR_MATRIX: return 16;
	case GL_TRANSPOSE_MODELVIEW_MATRIX: return 16;
	case GL_TRANSPOSE_PROJECTION_MATRIX: return 16;
	case GL_TRANSPOSE_TEXTURE_MATRIX: return 16;
	case GL_UNPACK_ALIGNMENT: return 1;
	case GL_UNPACK_IMAGE_HEIGHT: return 1;
	case GL_UNPACK_LSB_FIRST: return 1;
	case GL_UNPACK_ROW_LENGTH: return 1;
	case GL_UNPACK_SKIP_IMAGES: return 1;
	case GL_UNPACK_SKIP_PIXELS: return 1;
	case GL_UNPACK_SKIP_ROWS: return 1;
	case GL_UNPACK_SWAP_BYTES: return 1;
	case GL_VERTEX_ARRAY: return 1;
	case GL_VERTEX_ARRAY_BUFFER_BINDING: return 1;
	case GL_VERTEX_ARRAY_SIZE: return 1;
	case GL_VERTEX_ARRAY_STRIDE: return 1;
	case GL_VERTEX_ARRAY_TYPE: return 1;
	case GL_VERTEX_PROGRAM_POINT_SIZE: return 1;
	case GL_VERTEX_PROGRAM_TWO_SIDE: return 1;
	case GL_VIEWPORT: return 4;
	case GL_ZOOM_X: return 1;
	case GL_ZOOM_Y: return 1;
//	case GL_CONTEXT_PROFILE_MASK: return 1;
	case GL_NUM_EXTENSIONS: return 1;
	default: LOG("DEFAULTED getGetSize (0x%x, %s)!\n", type, getGLParamName(type)); return 1;
	}
}

int getTypeSize(GLenum type) {

	switch (type) 
	{
	case GL_BYTE: 			    		return sizeof(GLbyte);
	case GL_UNSIGNED_BYTE:  	    	return sizeof(GLubyte);
	case GL_SHORT: 			    		return sizeof(GLshort);
	case GL_UNSIGNED_SHORT: 	    	return sizeof(GLushort);
	case GL_INT: 			    		return sizeof(GLint);
	case GL_UNSIGNED_INT: 		    	return sizeof(GLuint);
	case GL_FLOAT: 			    		return sizeof(GLfloat);
	case GL_DOUBLE:			    		return sizeof(GLdouble);
	case GL_BITMAP: 		    		return sizeof(GLubyte);
	case GL_UNSIGNED_BYTE_3_3_2:  	    return sizeof(GLubyte);	//3bits + 3bits + 2bits
	case GL_UNSIGNED_BYTE_2_3_3_REV:    return sizeof(GLubyte);	//etc
	case GL_UNSIGNED_SHORT_5_6_5: 	    return sizeof(GLushort);
	case GL_UNSIGNED_SHORT_5_6_5_REV:   return sizeof(GLushort);
	case GL_UNSIGNED_SHORT_4_4_4_4:     return sizeof(GLushort);
	case GL_UNSIGNED_SHORT_4_4_4_4_REV: return sizeof(GLushort);
	case GL_UNSIGNED_SHORT_5_5_5_1:	    return sizeof(GLushort);
	case GL_UNSIGNED_SHORT_1_5_5_5_REV: return sizeof(GLushort);
	case GL_UNSIGNED_INT_8_8_8_8: 	    return sizeof(GLuint);
	case GL_UNSIGNED_INT_8_8_8_8_REV:   return sizeof(GLuint);
	case GL_UNSIGNED_INT_10_10_10_2:    return sizeof(GLuint);
	case GL_UNSIGNED_INT_2_10_10_10_REV:return sizeof(GLuint);
	default: LOG("DEFAULTED getTypeSize!\n"); return 4;
	}
}

int getLightParamSize(GLenum type) {

	//TODO: fill in missing types
	switch (type) 
	{
	case GL_AMBIENT: 					return 4;
	case GL_DIFFUSE:  					return 4;
	case GL_SPECULAR: 					return 4;
	case GL_POSITION: 					return 4;
	case GL_SPOT_DIRECTION: 			return 3;
	case GL_SPOT_EXPONENT: 				return 1;
	case GL_SPOT_CUTOFF: 				return 1;
	case GL_CONSTANT_ATTENUATION:		return 1;
	case GL_LINEAR_ATTENUATION:			return 1;
	case GL_QUADRATIC_ATTENUATION:		return 1;

	case GL_LIGHT_MODEL_AMBIENT:		return 4;
	case GL_LIGHT_MODEL_COLOR_CONTROL:	return 1;
	case GL_LIGHT_MODEL_LOCAL_VIEWER:	return 1;
	case GL_LIGHT_MODEL_TWO_SIDE:		return 1;
	case GL_AMBIENT_AND_DIFFUSE:		return 4;

	case GL_EMISSION:					return 4;
	case GL_SHININESS:					return 1;
	case GL_COLOR_INDEXES:				return 3;

	default:LOG("DEFAULTED getLightParamSize!\n"); return 4;
	}
}

int getFormatSize(GLenum format) {

	//LOG("getFormatSize(%d)\n", format);
	
	//passed in the number of objects
	if(format == 1 || format == 2 || format == 3 || format == 4){
		return format;
	}

/*
	int bpp = 1;
	    
	if(format == GL_BGR || format == GL_RGB) 
		bpp = 3;
	else if(format == GL_RGBA || format == GL_BGRA) 
		bpp = 4;
	else
		LOG("DEFAULTED getFormatSize\n");
*/
	switch(format)
	{
	case GL_COLOR_INDEX: return 1;
	case GL_RED: return 1;
	case GL_GREEN: return 1;
	case GL_BLUE: return 1;
	case GL_ALPHA: return 1;
	case GL_RGB: return 3;
	case GL_BGR: return 3;
	case GL_RGBA: return 4;
	case GL_BGRA: return 4;
	case GL_LUMINANCE: return 1;
	case GL_LUMINANCE_ALPHA: return 1;
	case GL_DEPTH_COMPONENT: return 1;
	
	default:LOG("DEFAULTED getFormatSize\n"); return 4;
	}                

	return 4;
}
