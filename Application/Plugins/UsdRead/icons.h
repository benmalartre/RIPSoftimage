#ifndef U2X_ICONS_H
#define U2X_ICONS_H
#pragma once

#include <map>
#include <type_traits>
#include "utils.h"
#include "pxr/usd/ar/asset.h"
#include "pxr/usd/ar/resolver.h"
#include <pxr/imaging/hio/image.h>
#include "stb/stb_image.h"
#include "stb/stb_image_resize.h"

enum ICON_SIZE {
  ICON_LOW = 16,
  ICON_MID = 32,
  ICON_HIGH = 64
};

enum ICON_ID {
  ICON_VISIBLE,
  ICON_MAX_ID
};

#define ICON_INTERNAL_FORMAT GL_RGBA
#define ICON_FORMAT GL_RGBA
#define ICON_TYPE GL_UNSIGNED_BYTE

struct Icon {
  ICON_SIZE     _size;
  GLuint        _tex;
  //GLuint  _texHovered;
};

static size_t U2X_NUM_ICONS = 2;
static const char* U2X_ICON_NAMES[] = {
  "visible.png",
  "invisible.png"
};

extern std::map<const char*, Icon> U2X_ICONS;

static void IconHoverDatas(pxr::HioImage::StorageSpec* storage, int nchannels)
{
  uint32_t* pixels = (uint32_t*)storage->data;
  if (nchannels == 4)
  {
    for (int y = 0; y<storage->height; ++y)
    {
      for (int x = 0; x<storage->width; ++x)
      {
        uint32_t index = y * storage->width + x;
        uint32_t pixel = pixels[index];
        uint8_t alpha = (pixel & 0xFF000000) >> 24;
        if (alpha > 0)
        {
          uint8_t blue = (pixel & 0x00FF0000) >> 16;
          uint8_t green = (pixel & 0x0000FF00) >> 8;
          uint8_t red = (pixel & 0x000000FF);

          pixels[index] = 0xFF0000FF;
        }
      }
    }
  }
}

static void CreateIconFromImage(const char* filename,
  const char* name, ICON_SIZE size)
{
  pxr::HioImageSharedPtr img = pxr::HioImage::OpenForReading(filename);

  pxr::HioImage::StorageSpec storage;
  storage.width = size;
  storage.height = size;
  storage.flipped = false;
  storage.format = pxr::HioFormat::HioFormatInt32Vec4;
  storage.data = new char[size * size * img->GetBytesPerPixel()];

  img->Read(storage);

  //IconHoverDatas(&storage, img->GetBytesPerPixel());
  LOG("CREATE ICON FROM IMAGE : " + CString(filename));

  if (U2X_ICONS.find(name) == U2X_ICONS.end())
  {
    LOG("NOT FOUND CREATE IT");
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    LOG("TEXTURE : " + CString((int)tex));

    // setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // upload pixels into texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0,
      ICON_FORMAT, ICON_TYPE, storage.data);

    U2X_ICONS[name] = { size, tex };

    LOG("MAPPED : " + CString((int)U2X_ICONS[name]._tex));
  }
}



static void U2XInitializeIcons()
{
  CString pluginDir = U2XGetInstallationFolder();
  std::string iconDir =std::string( pluginDir.GetAsciiString()) + "\\icons";
  std::vector<std::string> filenames;
  
  for (size_t i = 0; i < U2X_NUM_ICONS; ++i) {
    std::string iconFilename(iconDir + "\\" + std::string(U2X_ICON_NAMES[i]));
    LOG(CString(iconFilename.c_str()));
    if (pxr::HioImage::IsSupportedImageFile(iconFilename))
    {
      LOG("CREATE ICON FORM FILE : " + CString(iconFilename.c_str()));
      CreateIconFromImage(iconFilename.c_str(), U2X_ICON_NAMES[i], ICON_LOW);
    }
  }
}

static void U2XTerminateIcons()
{

}

#endif // U2X_ICONS_H