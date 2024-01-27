#ifndef TEXTURELOADER_H_INCLUDED
#define TEXTURELOADER_H_INCLUDED

#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glew.h>
#include <SOIL2/SOIL2.h>

class TextureLoader {
public:
    static GLuint loadTexture(const char* path, GLuint texture) {
        glBindTexture(GL_TEXTURE_2D, texture);

        // Set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // Set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Load image using SOIL2
        int width, height, channels;
        unsigned char* image = SOIL_load_image(path, &width, &height, &channels, SOIL_LOAD_RGBA);

        if (!image)
        {
            std::cerr << "Failed to load texture: " << path << std::endl;
            return 0;
        }

        // Upload image data to OpenGL
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

        // Generate mipmaps
        glGenerateMipmap(GL_TEXTURE_2D);

        // Free image data
        SOIL_free_image_data(image);

        glBindTexture(GL_TEXTURE_2D, 0);

        return texture;
    }
};

#endif // TEXTURELOADER_H_INCLUDED
