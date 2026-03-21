#pragma once

#ifndef EHJ_EXIT_ON_GL_ERR
#define EHJ_EXIT_ON_GL_ERR 1
#endif

/**
 * @brief automatic callback func on ogl error
 */
void ehj_gl_err_callback();

/**
 * @brief manual callback func on ogl error, breaks on error
 * deprecated, use ehj_gl_err_callback instead
 */
uint32_t ehj_gl_err();

/**
 * @brief manual callback func on ogl error, continues on error
 * deprecated, use ehj_gl_err_callback instead
 */
uint32_t ehj_gl_err_continue();
