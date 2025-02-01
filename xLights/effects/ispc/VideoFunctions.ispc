export void process_pixels(uniform int buffer_width,
                           uniform int buffer_height,
                           uniform int image_width,
                           uniform int image_height,
                           uniform int sample_spacing,
                           uniform int channels,
                           uniform unsigned int8 image_data[],
                           uniform unsigned int8 output_buffer[]) {

    // Outer foreach loop for parallel execution across rows (y)
    foreach (y = 0 ... buffer_height) {
        int cury = y * sample_spacing;
        if (cury >= image_height) continue;  // Skip invalid rows

        // Use a regular `for` loop inside `foreach` for x (columns)
        for (int x = 0; x < buffer_width; ++x) {
            int curx = x * sample_spacing;
            if (curx >= image_width) continue;  // Skip invalid columns

            int pixel_index = (image_height - 1 - cury) * image_width * channels + curx * channels;
            int output_index = (y * buffer_width + x) * 4;

            // Load pixel data (SIMD optimized)
            varying unsigned int8 r = image_data[pixel_index];
            varying unsigned int8 g = image_data[pixel_index + 1];
            varying unsigned int8 b = image_data[pixel_index + 2];
            varying unsigned int8 a = (channels == 4) ? image_data[pixel_index + 3] : 255;

            // Store in output buffer
            output_buffer[output_index] = r;
            output_buffer[output_index + 1] = g;
            output_buffer[output_index + 2] = b;
            output_buffer[output_index + 3] = a;
        }
    }
}
