// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_OSRENDERER_H_
#define CEF_TESTS_CEFCLIENT_OSRENDERER_H_
#pragma once


#define WEB_CONTENT_TEXTURE_PRIORITY 99

#include "include/cef_browser.h"
#include "include/cef_render_handler.h"

struct TexLayer {
  TexLayer():id(0), width(0), height(0), bFlippedh(false), bRepeatableS(false), bRepeatableT(false) {

  }
  unsigned int id;
  int width;
  int height;
  bool bRepeatableS;
  bool bRepeatableT;  
  bool bFlippedh;
};

class ClientOSRenderer {
 public:
  // The context must outlive this object.
  explicit ClientOSRenderer(bool transparent);
  virtual ~ClientOSRenderer();

  // Initialize the OpenGL environment.
  void Initialize();

  // Clean up the OpenGL environment.
  void Cleanup();

  // Render to the screen.
  void Render();

  // Forwarded from CefRenderHandler callbacks.
  void OnPopupShow(CefRefPtr<CefBrowser> browser,
                   bool show);
  void OnPopupSize(CefRefPtr<CefBrowser> browser,
                   const CefRect& rect);
  void OnPaint(CefRefPtr<CefBrowser> browser,
               CefRenderHandler::PaintElementType type,
               const CefRenderHandler::RectList& dirtyRects,
               const void* buffer, int width, int height);

  // Apply spin.
  void SetSpin(float spinX, float spinY);
  void IncrementSpin(float spinDX, float spinDY);

  bool IsTransparent() { return transparent_; }

  int GetViewWidth() { return view_width_; }
  int GetViewHeight() { return view_height_; }

  const CefRect& popup_rect() const { return popup_rect_; }
  const CefRect& original_popup_rect() const { return original_popup_rect_; }

  CefRect GetPopupRectInWebView(const CefRect& original_rect);
  void ClearPopupRects();

  void addTextureWithPrio(int priority, TexLayer tex);
  void removeTextureWithPrio(int priority);

 private:
  bool transparent_;
  bool initialized_;
  std::map<int, TexLayer> texture_ids_;
  int view_width_;
  int view_height_;
  CefRect popup_rect_;
  CefRect original_popup_rect_;
  float spin_x_;
  float spin_y_;
};

#endif  // CEF_TESTS_CEFCLIENT_OSRENDERER_H_

