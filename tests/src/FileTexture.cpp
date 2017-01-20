/**
* Copyright 2017 BitTorrent Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include "RenderOnce.h"

#include <okui/TextureInterface.h>

#include <gtest/gtest.h>

#if ONAIR_OKUI_HAS_NATIVE_APPLICATION && !OPENGL_ES // TODO: fix for OpenGL ES

using namespace okui;

struct Pixel {
    uint8_t r{0}, g{0}, b{0}, a{255};
    bool operator==(const Pixel& other) const {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }
};

#include "TexturePixels.h"

static void TextureTest(const unsigned char* imageData, size_t imageDataSize, int width, int height, const Pixel* expectedPixels, int tolerance = 0) {
    std::shared_ptr<TextureInterface> texture;

    RenderOnce([&](View* view) {
        texture = view->loadTextureFromMemory(std::make_shared<std::string>((const char*)imageData, imageDataSize));

        EXPECT_NE(texture, nullptr);

        EXPECT_FALSE(texture->isLoaded());

        EXPECT_EQ(texture->width(), width);
        EXPECT_EQ(texture->height(), height);
        EXPECT_FLOAT_EQ(texture->aspectRatio(), (double)width / height);
    }, [&](View* view) {
        ASSERT_TRUE(texture->isLoaded());

        std::vector<Pixel> pixels;
        pixels.resize(texture->width() * texture->height());

        glBindTexture(GL_TEXTURE_2D, texture->id());
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

        for (int y = 0; y < texture->height(); ++y) {
            for (int x = 0; x < texture->width(); ++x) {
                auto& pixel = pixels[x + texture->width() * y];
                auto& expected = expectedPixels[x + texture->width() * y];
                EXPECT_GE(pixel.r, expected.r - tolerance);
                EXPECT_LE(pixel.r, expected.r + tolerance);
                EXPECT_GE(pixel.g, expected.g - tolerance);
                EXPECT_LE(pixel.g, expected.g + tolerance);
                EXPECT_GE(pixel.b, expected.b - tolerance);
                EXPECT_LE(pixel.b, expected.b + tolerance);
                EXPECT_GE(pixel.a, expected.a - tolerance);
                EXPECT_LE(pixel.a, expected.a + tolerance);
            }
        }
    });
}

TEST(FileTexture, png16BitRGB) {
    // basn2c16 from http://www.schaik.com/pngsuite/pngsuite_bas_png.html
    static const unsigned char imageData[] = {
        0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
        0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x10, 0x02, 0x00, 0x00, 0x00, 0xAC, 0x88, 0x31,
        0xE0, 0x00, 0x00, 0x00, 0x04, 0x67, 0x41, 0x4D, 0x41, 0x00, 0x01, 0x86, 0xA0, 0x31, 0xE8, 0x96,
        0x5F, 0x00, 0x00, 0x00, 0xE5, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9C, 0xD5, 0x96, 0xC1, 0x0A, 0x83,
        0x30, 0x10, 0x44, 0xA7, 0xE0, 0x41, 0x7F, 0xCB, 0x7E, 0xB7, 0xFD, 0xAD, 0xF6, 0x96, 0x1E, 0x06,
        0x03, 0x92, 0x86, 0x26, 0x66, 0x93, 0xCC, 0x7A, 0x18, 0x86, 0x45, 0xE4, 0x3D, 0xD6, 0xA0, 0x8F,
        0x10, 0x42, 0x00, 0x3E, 0x2F, 0xE0, 0x9A, 0xEF, 0x64, 0x72, 0x73, 0x7E, 0x18, 0x3D, 0x27, 0x33,
        0x5F, 0xCE, 0xE2, 0xF3, 0x5A, 0x77, 0xB7, 0x02, 0xEB, 0xCE, 0x74, 0x28, 0x70, 0xA2, 0x33, 0x97,
        0xF3, 0xED, 0xF2, 0x70, 0x5D, 0xD1, 0x01, 0x60, 0xF3, 0xB2, 0x81, 0x5F, 0xE8, 0xEC, 0xF2, 0x02,
        0x79, 0x74, 0xA6, 0xB0, 0xC0, 0x3F, 0x74, 0xA6, 0xE4, 0x19, 0x28, 0x43, 0xE7, 0x5C, 0x6C, 0x03,
        0x35, 0xE8, 0xEC, 0x32, 0x02, 0xF5, 0xE8, 0x4C, 0x01, 0x81, 0xBB, 0xE8, 0xCC, 0xA9, 0x67, 0xA0,
        0x0D, 0x9D, 0xF3, 0x49, 0x1B, 0xB0, 0x40, 0x67, 0x1F, 0x2E, 0x60, 0x87, 0xCE, 0x1C, 0x28, 0x60,
        0x8D, 0x1E, 0x05, 0xF8, 0xC7, 0xEE, 0x0F, 0x1D, 0x00, 0xB6, 0x67, 0xE7, 0x0D, 0xF4, 0x44, 0x67,
        0xEF, 0x26, 0xD0, 0x1F, 0xBD, 0x9B, 0xC0, 0x28, 0xF4, 0x28, 0x60, 0xF7, 0x1D, 0x18, 0x8B, 0xCE,
        0xFB, 0x8D, 0x36, 0x30, 0x03, 0x9D, 0xBD, 0x59, 0x60, 0x1E, 0x7A, 0xB3, 0xC0, 0x6C, 0xF4, 0x28,
        0x50, 0x7F, 0x06, 0x34, 0xD0, 0x39, 0xAF, 0xDC, 0x80, 0x12, 0x3A, 0x7B, 0xB1, 0x80, 0x1E, 0x7A,
        0xB1, 0x80, 0x2A, 0x7A, 0x14, 0xC8, 0x9F, 0x01, 0x6D, 0x74, 0xCE, 0x33, 0x1B, 0xF0, 0x80, 0xCE,
        0x9E, 0x08, 0xF8, 0x41, 0x4F, 0x04, 0xBC, 0xA1, 0x33, 0xBF, 0xE6, 0x42, 0xFE, 0x5E, 0x07, 0xBB,
        0xC4, 0xEC, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82,
    };

    TextureTest(imageData, sizeof(imageData), 32, 32, png16BitRGBPixels);
}

TEST(FileTexture, png8BitRGBA) {
    // basn6a08 from http://www.schaik.com/pngsuite/pngsuite_bas_png.html
    static const unsigned char imageData[] = {
        0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
        0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x08, 0x06, 0x00, 0x00, 0x00, 0x73, 0x7A, 0x7A,
        0xF4, 0x00, 0x00, 0x00, 0x04, 0x67, 0x41, 0x4D, 0x41, 0x00, 0x01, 0x86, 0xA0, 0x31, 0xE8, 0x96,
        0x5F, 0x00, 0x00, 0x00, 0x6F, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9C, 0xED, 0xD6, 0x31, 0x0A, 0x80,
        0x30, 0x0C, 0x46, 0xE1, 0x27, 0x64, 0x68, 0x4F, 0xA1, 0xF7, 0x3F, 0x55, 0x04, 0x8F, 0x21, 0xC4,
        0xDD, 0xC5, 0x45, 0x78, 0x1D, 0x52, 0xE8, 0x50, 0x28, 0xFC, 0x1F, 0x4D, 0x28, 0xD9, 0x8A, 0x01,
        0x30, 0x5E, 0x7B, 0x7E, 0x9C, 0xFF, 0xBA, 0x33, 0x83, 0x1D, 0x75, 0x05, 0x47, 0x03, 0xCA, 0x06,
        0xA8, 0xF9, 0x0D, 0x58, 0xA0, 0x07, 0x4E, 0x35, 0x1E, 0x22, 0x7D, 0x80, 0x5C, 0x82, 0x54, 0xE3,
        0x1B, 0xB0, 0x42, 0x0F, 0x5C, 0xDC, 0x2E, 0x00, 0x79, 0x20, 0x88, 0x92, 0xFF, 0xE2, 0xA0, 0x01,
        0x36, 0xA0, 0x7B, 0x40, 0x07, 0x94, 0x3C, 0x10, 0x04, 0xD9, 0x00, 0x19, 0x50, 0x36, 0x40, 0x7F,
        0x01, 0x1B, 0xF0, 0x00, 0x52, 0x20, 0x1A, 0x9C, 0x16, 0x0F, 0xB8, 0x4C, 0x00, 0x00, 0x00, 0x00,
        0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82,
    };

    TextureTest(imageData, sizeof(imageData), 32, 32, png8BitRGBAPixels);
}

TEST(FileTexture, jpgWater) {
    // cropped from http://www.imagemagick.org/Usage/images/tile_water.jpg
    static const unsigned char imageData[] = {
        0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46, 0x00, 0x01, 0x01, 0x00, 0x00, 0x48,
        0x00, 0x48, 0x00, 0x00, 0xFF, 0xE1, 0x00, 0x8C, 0x45, 0x78, 0x69, 0x66, 0x00, 0x00, 0x4D, 0x4D,
        0x00, 0x2A, 0x00, 0x00, 0x00, 0x08, 0x00, 0x05, 0x01, 0x12, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x01, 0x00, 0x00, 0x01, 0x1A, 0x00, 0x05, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x4A,
        0x01, 0x1B, 0x00, 0x05, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x52, 0x01, 0x28, 0x00, 0x03,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x00, 0x87, 0x69, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0xA0, 0x01, 0x00, 0x03, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0xA0, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
        0x00, 0x20, 0xA0, 0x03, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00,
        0x00, 0x00, 0xFF, 0xED, 0x00, 0x38, 0x50, 0x68, 0x6F, 0x74, 0x6F, 0x73, 0x68, 0x6F, 0x70, 0x20,
        0x33, 0x2E, 0x30, 0x00, 0x38, 0x42, 0x49, 0x4D, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x38, 0x42, 0x49, 0x4D, 0x04, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0xD4, 0x1D, 0x8C, 0xD9,
        0x8F, 0x00, 0xB2, 0x04, 0xE9, 0x80, 0x09, 0x98, 0xEC, 0xF8, 0x42, 0x7E, 0xFF, 0xC0, 0x00, 0x11,
        0x08, 0x00, 0x20, 0x00, 0x20, 0x03, 0x01, 0x12, 0x00, 0x02, 0x11, 0x01, 0x03, 0x11, 0x01, 0xFF,
        0xC4, 0x00, 0x1F, 0x00, 0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
        0xFF, 0xC4, 0x00, 0xB5, 0x10, 0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05, 0x05, 0x04,
        0x04, 0x00, 0x00, 0x01, 0x7D, 0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31, 0x41,
        0x06, 0x13, 0x51, 0x61, 0x07, 0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xA1, 0x08, 0x23, 0x42, 0xB1,
        0xC1, 0x15, 0x52, 0xD1, 0xF0, 0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0A, 0x16, 0x17, 0x18, 0x19,
        0x1A, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44,
        0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x63, 0x64,
        0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x83, 0x84,
        0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0xA2,
        0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9,
        0xBA, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
        0xD8, 0xD9, 0xDA, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xF1, 0xF2, 0xF3,
        0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFF, 0xC4, 0x00, 0x1F, 0x01, 0x00, 0x03, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0xFF, 0xC4, 0x00, 0xB5, 0x11, 0x00, 0x02, 0x01,
        0x02, 0x04, 0x04, 0x03, 0x04, 0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02, 0x77, 0x00, 0x01, 0x02,
        0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71, 0x13, 0x22, 0x32,
        0x81, 0x08, 0x14, 0x42, 0x91, 0xA1, 0xB1, 0xC1, 0x09, 0x23, 0x33, 0x52, 0xF0, 0x15, 0x62, 0x72,
        0xD1, 0x0A, 0x16, 0x24, 0x34, 0xE1, 0x25, 0xF1, 0x17, 0x18, 0x19, 0x1A, 0x26, 0x27, 0x28, 0x29,
        0x2A, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x53,
        0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x73,
        0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A,
        0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8,
        0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6,
        0xC7, 0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xE2, 0xE3, 0xE4,
        0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFF,
        0xDB, 0x00, 0x43, 0x00, 0x08, 0x06, 0x06, 0x07, 0x06, 0x05, 0x08, 0x07, 0x07, 0x07, 0x09, 0x09,
        0x08, 0x0A, 0x0C, 0x14, 0x0D, 0x0C, 0x0B, 0x0B, 0x0C, 0x19, 0x12, 0x13, 0x0F, 0x14, 0x1D, 0x1A,
        0x1F, 0x1E, 0x1D, 0x1A, 0x1C, 0x1C, 0x20, 0x24, 0x2E, 0x27, 0x20, 0x22, 0x2C, 0x23, 0x1C, 0x1C,
        0x28, 0x37, 0x29, 0x2C, 0x30, 0x31, 0x34, 0x34, 0x34, 0x1F, 0x27, 0x39, 0x3D, 0x38, 0x32, 0x3C,
        0x2E, 0x33, 0x34, 0x32, 0xFF, 0xDB, 0x00, 0x43, 0x01, 0x09, 0x09, 0x09, 0x0C, 0x0B, 0x0C, 0x18,
        0x0D, 0x0D, 0x18, 0x32, 0x21, 0x1C, 0x21, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32,
        0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32,
        0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32,
        0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0xFF, 0xDD, 0x00, 0x04, 0x00, 0x04, 0xFF,
        0xDA, 0x00, 0x0C, 0x03, 0x01, 0x00, 0x02, 0x11, 0x03, 0x11, 0x00, 0x3F, 0x00, 0xCE, 0xCB, 0xA1,
        0xE6, 0x91, 0xE7, 0x1B, 0xBE, 0x6A, 0xFD, 0x15, 0xA6, 0xCF, 0xCF, 0x2C, 0xFB, 0x11, 0xCA, 0x59,
        0xCE, 0x14, 0x62, 0x87, 0xBA, 0x5F, 0xE0, 0x19, 0x35, 0x51, 0x4D, 0x1A, 0x45, 0x3E, 0xC2, 0x2D,
        0xA1, 0xE1, 0x8B, 0x53, 0x53, 0xCF, 0x95, 0xF8, 0x07, 0x14, 0xDD, 0xD0, 0x36, 0xD7, 0x51, 0xF2,
        0xC7, 0x21, 0x5D, 0xAB, 0x57, 0xE0, 0x85, 0x94, 0x65, 0xEB, 0x37, 0x52, 0xC4, 0x7B, 0x56, 0xBA,
        0x9F, 0xFF, 0xD0, 0xC8, 0xBB, 0x84, 0x91, 0xF2, 0xD3, 0x56, 0xE8, 0x31, 0x0B, 0xD4, 0xD7, 0xE8,
        0xEB, 0x9A, 0x27, 0xC0, 0x2E, 0x78, 0x92, 0x5A, 0xDA, 0x05, 0x5D, 0xCF, 0x4B, 0x35, 0xD6, 0xD8,
        0xB0, 0x3A, 0xD0, 0xF9, 0xA4, 0x0F, 0x9E, 0x45, 0x91, 0x7B, 0x14, 0x3F, 0x28, 0x03, 0x35, 0x95,
        0x6F, 0x6B, 0x2C, 0xF2, 0x6E, 0x39, 0xC5, 0x0E, 0x9C, 0x17, 0xC4, 0x0E, 0x95, 0x34, 0xBD, 0xE2,
        0xE5, 0xC5, 0xEC, 0x92, 0xF0, 0xBC, 0x0A, 0x74, 0x96, 0x65, 0x63, 0xE3, 0xAD, 0x25, 0xC8, 0xBE,
        0x12, 0x69, 0xBA, 0x69, 0xFB, 0xA7, 0xFF, 0xD9,
    };

    TextureTest(imageData, sizeof(imageData), 32, 32, jpegRGBPixels, 8);
}

#endif // ONAIR_OKUI_HAS_NATIVE_APPLICATION
