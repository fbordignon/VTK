/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOpenXRManagerOpenGLGraphics.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkOpenXRManagerOpenGLGraphics.h"

#include "vtkObjectFactory.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkOpenXRManager.h"

// include what we need for the helper window
#ifdef VTK_USE_X
// We need to cast to a XOpenGLRenderWindow for vtkXVisualInfo->visualid
#include "vtkXOpenGLRenderWindow.h"

// From vtkXOpenGLRenderWindow.cxx :
// Work-around to get forward declarations of C typedef of anonymous
// structs working. We do not want to include XUtil.h in the header as
// it populates the global namespace.
#include "GL/glx.h"
#include <X11/Xutil.h>
struct vtkXVisualInfo : public XVisualInfo
{
};
#endif // VTK_USE_X

vtkStandardNewMacro(vtkOpenXRManagerOpenGLGraphics);

//------------------------------------------------------------------------------
void vtkOpenXRManagerOpenGLGraphics::EnumerateSwapchainImages(
  XrSwapchain swapchain, SwapchainImagesOpenGL& swapchainImages)
{
  uint32_t chainLength = this->GetChainLength(swapchain);

  swapchainImages.Images.resize(chainLength, { XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR });

  vtkOpenXRManager::GetInstance().XrCheckError(
    xrEnumerateSwapchainImages(swapchain, (uint32_t)swapchainImages.Images.size(), &chainLength,
      reinterpret_cast<XrSwapchainImageBaseHeader*>(swapchainImages.Images.data())),
    "Failed to enumerate swapchain images");
}

//------------------------------------------------------------------------------
bool vtkOpenXRManagerOpenGLGraphics::CreateGraphicsBinding(vtkOpenGLRenderWindow* helperWindow)
{
#ifdef VTK_USE_X
  // Create the XrGraphicsBindingOpenGLXlibKHR structure
  // That will be in the next chain of the XrSessionCreateInfo
  // We need to fill xDisplay, visualId, glxFBConfig, glxDrawable and glxContext

  auto graphicsBindingGLX =
    std::shared_ptr<XrGraphicsBindingOpenGLXlibKHR>(new XrGraphicsBindingOpenGLXlibKHR{
      XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR, // .type
      nullptr,                                  // .next
      nullptr,                                  // .xDisplay. a valid X11 display
      0,                                        // .visualid. a valid X11 visual id
      0,                                        // .glxFBConfig. a valid X11 OpenGL GLX GLXFBConfig
      0,                                        // .glxDrawable. a valid X11 OpenGL GLX GLXDrawable
      0                                         // .glxContext. a valid X11 OpenGL GLX GLXContext
    });
  this->GraphicsBinding = graphicsBindingGLX;

  vtkNew<vtkXOpenGLRenderWindow> xoglRenWin;
  vtkXOpenGLRenderWindow* glxHelperWindow = vtkXOpenGLRenderWindow::SafeDownCast(helperWindow);

  if (glxHelperWindow == nullptr)
  {
    xoglRenWin->InitializeFromCurrentContext();
    glxHelperWindow = xoglRenWin;
  }

  vtkXVisualInfo* v = glxHelperWindow->GetDesiredVisualInfo();
  GLXFBConfig* fbConfig = reinterpret_cast<GLXFBConfig*>(glxHelperWindow->GetGenericFBConfig());

  graphicsBindingGLX->xDisplay = glxHelperWindow->GetDisplayId();
  graphicsBindingGLX->glxDrawable = glxHelperWindow->GetWindowId();
  graphicsBindingGLX->glxContext = glXGetCurrentContext();
  graphicsBindingGLX->visualid = v->visualid;
  graphicsBindingGLX->glxFBConfig = *fbConfig;

#elif defined(_WIN32)
  auto graphicsBindingGLWin32 =
    std::shared_ptr<XrGraphicsBindingOpenGLWin32KHR>(new XrGraphicsBindingOpenGLWin32KHR{
      XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR, // .type
      nullptr,                                   // .next
      0,                                         // .hdC : a valid Windows HW device context handle.
      0 // .hGLRChandle : a valid Windows OpenGL rendering context
    });
  this->GraphicsBinding = graphicsBindingGLWin32;

  graphicsBindingGLWin32->hDC = wglGetCurrentDC();
  graphicsBindingGLWin32->hGLRC = wglGetCurrentContext();

#else
  vtkErrorMacro(<< "Only X11 and Win32 are supported at the moment.");
  return false;
#endif

  return true;
}

//------------------------------------------------------------------------------
bool vtkOpenXRManagerOpenGLGraphics::CheckGraphicsRequirements(
  XrInstance instance, XrSystemId id, xr::ExtensionDispatchTable extensions)
{
  XrGraphicsRequirementsOpenGLKHR openGLReqs = {
    XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR, // .type
    nullptr,                                  // .next
    0,                                        // .minApiVersionSupported
    0                                         // .maxApiVersionSupported
  };

  // this function pointer was loaded with xrGetInstanceProcAddr (see XrExtensions.h)
  if (!vtkOpenXRManager::GetInstance().XrCheckError(
        extensions.xrGetOpenGLGraphicsRequirementsKHR(instance, id, &openGLReqs),
        "Failed to get OpenGL graphics requirements!"))
  {
    return false;
  }

  return true;
}
