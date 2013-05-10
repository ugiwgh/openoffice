/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/

#ifndef SVX_SIDEBAR_CONTROLLER_ITEM_HXX
#define SVX_SIDEBAR_CONTROLLER_ITEM_HXX

#include <sfx2/ctrlitem.hxx>

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>
#include <unotools/cmdoptions.hxx>
#include <vcl/image.hxx>

#include <com/sun/star/frame/XFrame.hpp>

#include <boost/function.hpp>


namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;

class SfxViewFrame;
class ToolBox;


namespace sfx2 { namespace sidebar {

/** The sfx2::sidebar::ControllerItem is a wrapper around the
    SfxControllerItem that becomes necessary to allow objects (think
    sidebar panels) to receive state changes without having one
    SfxControllerItem per supported item as base class (which is not
    possible in C++ anyway).

    It also gives access to the label and icon of a slot/command.
*/
class SFX2_DLLPUBLIC ControllerItem
    : public SfxControllerItem
{
public:
    class ItemUpdateReceiverInterface
    {
    public:
        virtual void NotifyItemUpdate(
            const sal_uInt16 nSId,
            const SfxItemState eState,
            const SfxPoolItem* pState,
            const bool bIsEnabled) = 0;
    };

    /** This is the preferred constructor that allows the created
        controller item to return non-empty values for GetLable() and
        GetIcon() calls.
    */
    ControllerItem (
        const sal_uInt16 nSlotId,
        SfxBindings &rBindings,
        ItemUpdateReceiverInterface& rItemUpdateReceiver,
        const ::rtl::OUString& rsCommandName,
        const cssu::Reference<css::frame::XFrame>& rxFrame);
    
    /** This is the simpler constructor variant that still exists for
        compatibility resons.  Note that GetLabel() and GetIcon() will
        return empty strings/images.
    */
    ControllerItem (
        const sal_uInt16 nId,
        SfxBindings &rBindings,
        ItemUpdateReceiverInterface& rItemUpdateReceiver);
    
    virtual ~ControllerItem (void);

    /** Returns </TRUE> when the slot/command has not been disabled.
        Changes of this state are notified via the
        ItemUpdateReceiverInterface::NotifyContextChang() method.
    */
    bool IsEnabled (const SfxItemState eState) const;

    /** Force the controller item to call its NotifyItemUpdate
        callback with up-to-date data.
    */
    void RequestUpdate (void);
    
    /** Return the label for the command.  It contains the keyboard
        accelerator when one exists.
    */
    ::rtl::OUString GetLabel (void) const;

    /** Return the icon for the command.  Uses the system high contrast mode state.
    */
    Image GetIcon (void) const;

    /** Return the icon for the command.  Allows the user to override
        the high contrast mode state.
    */
    Image GetIcon (const bool bIsHighContrastMode) const;

    /** Convenience method for setting all relevant properties for the
        slot/command represented by the called object at the given tool
        box.
    */
    void SetupToolBoxItem (ToolBox& rToolBox, const sal_uInt16 nIndex);

    /** Do not call.  Used by local class only.  Should be a member of
        a local and hidden interface.
    */
    void NotifyFrameContextChange (void);
    /** Do not call.  Used by local class only.  Should be a member of
        a local and hidden interface.
    */
    void ResetFrame (void);

protected:
    virtual void StateChanged (sal_uInt16 nSId, SfxItemState eState, const SfxPoolItem* pState);

private:
    ItemUpdateReceiverInterface& mrItemUpdateReceiver;
    cssu::Reference<css::frame::XFrame> mxFrame;
    cssu::Reference<css::lang::XComponent> mxFrameActionListener;
    const ::rtl::OUString msCommandName;
    
    void SetupCommandURL (const sal_Char* sCommandName);
};

} } // end of namespace sfx2::sidebar

#endif
