/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkOpenXRInteractorStyle.h

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkOpenXRInteractorStyle
 * @brief   extended from vtkInteractorStyle3D to override command methods
 */

#ifndef vtkOpenXRInteractorStyle_h
#define vtkOpenXRInteractorStyle_h

#include "vtkRenderingOpenXRModule.h" // For export macro

#include "vtkEventData.h" // for enums
#include "vtkInteractorStyle3D.h"
#include "vtkNew.h"                // for ivars
#include "vtkOpenXRRenderWindow.h" // for enums

class vtkCallbackCommand;
class vtkCell;
class vtkPlane;
// class vtkOpenVRControlsHelper;
class vtkVRHardwarePicker;
class vtkVRMenuRepresentation;
class vtkVRMenuWidget;
class vtkTextActor3D;
class vtkSelection;
class vtkSphereSource;

class VTKRENDERINGOPENXR_EXPORT vtkOpenXRInteractorStyle : public vtkInteractorStyle3D
{
public:
  static vtkOpenXRInteractorStyle* New();
  vtkTypeMacro(vtkOpenXRInteractorStyle, vtkInteractorStyle3D);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //@{
  /**
   * Override generic event bindings to call the corresponding action.
   */
  void OnSelect3D(vtkEventData* edata) override;
  void OnNextPose3D(vtkEventData* edata) override;
  void OnViewerMovement3D(vtkEventData* edata) override;
  void OnMove3D(vtkEventData* edata) override;
  void OnMenu3D(vtkEventData* edata) override;
  //@}

  //@{
  /**
   * Interaction mode entry points.
   */
  virtual void StartPick(vtkEventDataDevice3D*);
  virtual void EndPick(vtkEventDataDevice3D*);
  virtual void StartLoadCamPose(vtkEventDataDevice3D*);
  virtual void EndLoadCamPose(vtkEventDataDevice3D*);
  virtual void StartPositionProp(vtkEventDataDevice3D*);
  virtual void EndPositionProp(vtkEventDataDevice3D*);
  virtual void StartClip(vtkEventDataDevice3D*);
  virtual void EndClip(vtkEventDataDevice3D*);
  virtual void StartDolly3D(vtkEventDataDevice3D*);
  virtual void EndDolly3D(vtkEventDataDevice3D*);
  //@}

  //@{
  /**
   * Multitouch events binding.
   */
  void OnPan() override;
  void OnPinch() override;
  void OnRotate() override;
  //@}

  //@{
  /**
   * Methods for interaction.
   */
  void ProbeData(vtkEventDataDevice controller);
  void LoadNextCameraPose();
  virtual void PositionProp(vtkEventData*, double* lwpos = nullptr, double* lwori = nullptr);
  virtual void Clip(vtkEventDataDevice3D*);
  //@}

  //@{
  /**
   * Map controller inputs to actions.
   * Actions are defined by a VTKIS_*STATE*, interaction entry points,
   * and the corresponding method for interaction.
   */
  void MapInputToAction(vtkCommand::EventIds eid, int state);
  void MapInputToAction(vtkCommand::EventIds eid, vtkEventDataAction action, int state);
  //@}

  //@{
  /**
   * Define the helper text that goes with an input,
   * deprecated as open vr mostly provides it
   */
  void AddTooltipForInput(
    vtkEventDataDevice device, vtkEventDataDeviceInput input, const std::string& text);
  //@}

  //@{
  /**
   * Indicates if picking should be updated every frame. If so, the interaction
   * picker will try to pick a prop and rays will be updated accordingly.
   * Default is set to off.
   */
  vtkSetMacro(HoverPick, bool);
  vtkGetMacro(HoverPick, bool);
  vtkBooleanMacro(HoverPick, bool);
  //@}

  //@{
  /**
   * Specify if the grab mode use the ray to grab distant objects
   */
  vtkSetMacro(GrabWithRay, bool);
  vtkGetMacro(GrabWithRay, bool);
  vtkBooleanMacro(GrabWithRay, bool);
  //@}

  //@{
  /**
   * Get the interaction state for a device (dolly, pick, none, etc...)
   */
  int GetInteractionState(vtkEventDataDevice device)
  {
    return this->InteractionState[static_cast<int>(device)];
  }
  //@}

  //@{
  /**
   * Show/hide ray for a given controller.
   */
  void ShowRay(vtkEventDataDevice controller);
  void HideRay(vtkEventDataDevice controller);
  //@}

  //@{
  /**
   * Show billboard with given text, or hide billboard.
   */
  void ShowBillboard(const std::string& text);
  void HideBillboard();
  //@}

  //@{
  /**
   * Make the pick actor a sphere of given radius centered at given position,
   * and show it.
   */
  void ShowPickSphere(double* pos, double radius, vtkProp3D*);
  //@}

  //@{
  /**
   * Make the pick actor a polydata built from the points and edges of the
   * given cell, and show it.
   */
  void ShowPickCell(vtkCell* cell, vtkProp3D*);
  //@}

  //@{
  /**
   * Hide the pick actor (sphere or polydata).
   */
  void HidePickActor();
  //@}

  //@{
  /**
   * Control visibility of descriptive tooltips for control/hmd models
   */
  void ToggleDrawControls();
  void SetDrawControls(bool);
  //@}

  //@{
  /**
   * Set/Get the Interactor wrapper being controlled by this object.
   * (Satisfy superclass API.)
   */
  void SetInteractor(vtkRenderWindowInteractor* iren) override;
  //@}

  //@{
  /**
   * aReturn a menu to allow the user to add options to it.
   */
  vtkVRMenuWidget* GetMenu() { return this->Menu.Get(); }
  //@}

protected:
  vtkOpenXRInteractorStyle();
  ~vtkOpenXRInteractorStyle() override;

  void EndPickCallback(vtkSelection* sel);

  // Ray drawing
  void UpdateRay(vtkEventDataDevice controller);

  static void MenuCallback(
    vtkObject* object, unsigned long event, void* clientdata, void* calldata);

  // Utility routines
  void StartAction(int VTKIS_STATE, vtkEventDataDevice3D* edata);
  void EndAction(int VTKIS_STATE, vtkEventDataDevice3D* edata);

  // Pick using hardware selector
  bool HardwareSelect(vtkEventDataDevice controller, bool actorPassOnly);

  // Controls helpers drawing
  void AddTooltipForInput(vtkEventDataDevice device, vtkEventDataDeviceInput input);

  vtkNew<vtkVRMenuWidget> Menu;
  vtkNew<vtkVRMenuRepresentation> MenuRepresentation;
  vtkNew<vtkCallbackCommand> MenuCommand;

  vtkNew<vtkTextActor3D> TextActor3D;
  vtkNew<vtkActor> PickActor;
  vtkNew<vtkSphereSource> Sphere;

  // device input to interaction state mapping
  std::map<std::tuple<vtkCommand::EventIds, vtkEventDataAction>, int> InputMap;

  bool HoverPick = false;
  bool GrabWithRay = true;

  /**
   * Store required controllers information when performing action
   */
  int InteractionState[vtkEventDataNumberOfDevices];
  vtkProp3D* InteractionProps[vtkEventDataNumberOfDevices];
  vtkPlane* ClippingPlanes[vtkEventDataNumberOfDevices];

  vtkNew<vtkVRHardwarePicker> HardwarePicker;

private:
  vtkOpenXRInteractorStyle(const vtkOpenXRInteractorStyle&) = delete;
  void operator=(const vtkOpenXRInteractorStyle&) = delete;
};

#endif
