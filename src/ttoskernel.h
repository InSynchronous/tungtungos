#pragma once

/*
TTOS Kernel. Userspace begins in `main.cpp`
*/

extern "C" void kernel_start();

namespace ttos {

/* *
 * @brief Set's the color of the VGA cursor in the text buffer
 *
 * @param color The color to set the cursor to
 */
void VGA_SetColor(unsigned char color);

/**
 * @brief Writes a single character to the VGA text buffer.
 *
 * @param character The character to display.
 * @param x The horizontal position, from 0 to 79.
 * @param y The vertical position, from 0 to 24.
 */
void VGA_Write(const char character, int x, int y);

/**
 * @brief Prints a null-terminated string to the VGA text buffer. Automatically
 * wraps text by character.
 *
 * @param string The string to print.
 * @param x The horizontal starting position, from 0 to 79.
 * @param y The vertical starting position, from 0 to 24.
 */
void VGA_Print(const char *string, int x, int y);

/**
 * @brief Clears the entire VGA text screen.
 */
void VGA_Clear();

/**
 * @brief Halts the CPU indefinitely.
 */
void CPU_halt();
} // namespace ttos
