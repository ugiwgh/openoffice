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

#include <sfx2/sidebar/propertypanel.hrc>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sidebar/ControlFactory.hxx>
#include "PosSizePropertyPanel.hxx"
#include "PosSizePropertyPanel.hrc"
#include <svx/sidebar/SidebarDialControl.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include <svx/dlgutil.hxx>
#include <unotools/viewoptions.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/toolbox.hxx>
#include <svx/svdview.hxx>
#include <svl/aeitem.hxx>

using namespace css;
using namespace cssu;
using ::sfx2::sidebar::Theme;

#define A2S(pString) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(pString)))
#define USERITEM_NAME rtl::OUString::createFromAscii("FitItem")
#define NO_SELECT       (65535)



namespace svx { namespace sidebar {



PosSizePropertyPanel::PosSizePropertyPanel(
    Window* pParent,
    const cssu::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings,
    const cssu::Reference<css::ui::XSidebar>& rxSidebar)
:   Control(
        pParent, 
        SVX_RES(RID_SIDEBAR_POSSIZE_PANEL)),
    mpFtPosX(new FixedText(this, SVX_RES(FT_SBSHAPE_HORIZONTAL))),
    mpMtrPosX(new MetricField(this, SVX_RES(MF_SBSHAPE_HORIZONTAL))),
    mpFtPosY(new FixedText(this, SVX_RES(FT_SBSHAPE_VERTICAL))),
    mpMtrPosY(new MetricField(this, SVX_RES(MF_SBSHAPE_VERTICAL))),
    mpFtWidth(new FixedText(this, SVX_RES(FT_WIDTH))),
    mpMtrWidth(new MetricField(this, SVX_RES(MTR_FLD_WIDTH))),
    mpFtHeight(new FixedText(this, SVX_RES(FT_HEIGHT))),
    mpMtrHeight(new MetricField(this, SVX_RES(MTR_FLD_HEIGHT))),
    mpCbxScale(new CheckBox(this, SVX_RES(CBX_SCALE))),
    mpFtAngle(new FixedText(this, SVX_RES(FT_ANGLE))),
    mpMtrAngle(new MetricBox(this, SVX_RES(MTR_FLD_ANGLE))),
    mpDial(new SidebarDialControl(this, SVX_RES(DIAL_CONTROL))),
    mpFtFlip(new FixedText(this, SVX_RES(FT_FLIP))),
    mpFlipTbxBackground(sfx2::sidebar::ControlFactory::CreateToolBoxBackground(this)),
    mpFlipTbx(sfx2::sidebar::ControlFactory::CreateToolBox(mpFlipTbxBackground.get(), SVX_RES(TBX_FLIP))),
    maRect(),
    mpView(0),
    mlOldWidth(1),
    mlOldHeight(1),
    meRP(RP_LT),
    maAnchorPos(),
    mlRotX(0),
    mlRotY(0),
    maUIScale(),
    mePoolUnit(),
    meDlgUnit(),
    maTransfPosXControl(SID_ATTR_TRANSFORM_POS_X, *pBindings, *this),
    maTransfPosYControl(SID_ATTR_TRANSFORM_POS_Y, *pBindings, *this),
    maTransfWidthControl(SID_ATTR_TRANSFORM_WIDTH, *pBindings, *this),
    maTransfHeightControl(SID_ATTR_TRANSFORM_HEIGHT, *pBindings, *this),
    maSvxAngleControl( SID_ATTR_TRANSFORM_ANGLE, *pBindings, *this),
    maRotXControl(SID_ATTR_TRANSFORM_ROT_X, *pBindings, *this),
    maRotYControl(SID_ATTR_TRANSFORM_ROT_Y, *pBindings, *this),
    maProPosControl(SID_ATTR_TRANSFORM_PROTECT_POS, *pBindings, *this),
    maProSizeControl(SID_ATTR_TRANSFORM_PROTECT_SIZE, *pBindings, *this),
    maAutoWidthControl(SID_ATTR_TRANSFORM_AUTOWIDTH, *pBindings, *this),
    maAutoHeightControl(SID_ATTR_TRANSFORM_AUTOHEIGHT, *pBindings, *this),
    m_aMetricCtl(SID_ATTR_METRIC, *pBindings, *this),
    maImgFlipHori(SVX_RES(IMG_HORI_FLIP)),
    maImgFlipVert(SVX_RES(IMG_VERT_FLIP)),
    mxFrame(rxFrame),
    maContext(),
    mpBindings(pBindings),
    mbMtrPosXMirror(false),
    mbSizeProtected(false),
    mbPositionProtected(false),
    mbAutoWidth(false),
    mbAutoHeight(false),
    mbAdjustEnabled(false),
    mbIsFlip(false),
    mbInDestructor(false),
    mxSidebar(rxSidebar)
{
	Initialize();
	FreeResource();

    mpBindings->Update( SID_ATTR_TRANSFORM_WIDTH );
    mpBindings->Update( SID_ATTR_TRANSFORM_HEIGHT );
    mpBindings->Update( SID_ATTR_TRANSFORM_PROTECT_SIZE );
    mpBindings->Update( SID_ATTR_METRIC );
}



PosSizePropertyPanel::~PosSizePropertyPanel()
{
    mbInDestructor = true;

    // Destroy the background windows of the toolboxes.
    mpFlipTbx.reset();
    mpFlipTbxBackground.reset();
}



void PosSizePropertyPanel::ShowMenu (void)
{
    if (mpBindings != NULL)
    {
        SfxDispatcher* pDispatcher = mpBindings->GetDispatcher();
        if (pDispatcher != NULL)
            pDispatcher->Execute(SID_ATTR_TRANSFORM, SFX_CALLMODE_ASYNCHRON);
    }
}



void PosSizePropertyPanel::Initialize()
{
    mpFtPosX->SetBackground(Wallpaper());
    mpFtPosY->SetBackground(Wallpaper());
    mpFtWidth->SetBackground(Wallpaper());
    mpFtHeight->SetBackground(Wallpaper());
    mpFtAngle->SetBackground(Wallpaper());
    mpFtFlip->SetBackground(Wallpaper());

        //Position : Horizontal / Vertical
	mpMtrPosX->SetModifyHdl( LINK( this, PosSizePropertyPanel, ChangePosXHdl ) );
	mpMtrPosY->SetModifyHdl( LINK( this, PosSizePropertyPanel, ChangePosYHdl ) );
	mpMtrPosX->SetAccessibleName(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Horizontal")));	//wj acc
	mpMtrPosY->SetAccessibleName(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Vertical")));		//wj acc
	
    //Size : Width / Height
	mpMtrWidth->SetModifyHdl( LINK( this, PosSizePropertyPanel, ChangeWidthHdl ) );
	mpMtrHeight->SetModifyHdl( LINK( this, PosSizePropertyPanel, ChangeHeightHdl ) );
	mpMtrWidth->SetAccessibleName(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Width")));	//wj acc
	mpMtrHeight->SetAccessibleName(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Height")));	//wj acc
	
    //Size : Keep ratio
	mpCbxScale->SetClickHdl( LINK( this, PosSizePropertyPanel, ClickAutoHdl ) );
	
    //rotation:
	mpMtrAngle->SetModifyHdl(LINK( this, PosSizePropertyPanel, AngleModifiedHdl));
	mpMtrAngle->EnableAutocomplete( false );
	mpMtrAngle->SetAccessibleName(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Rotation")));	//wj acc
	
    //rotation control
	mpDial->SetModifyHdl(LINK( this, PosSizePropertyPanel, RotationHdl));
	
    //flip:
	mpFlipTbx->SetSelectHdl( LINK( this, PosSizePropertyPanel, FlipHdl) );
	mpFlipTbx->SetItemImage(FLIP_HORIZONTAL,maImgFlipHori);
    mpFlipTbx->SetItemImage(FLIP_VERTICAL,maImgFlipVert);
	mpFlipTbx->SetQuickHelpText(FLIP_HORIZONTAL,String(SVX_RES(STR_QH_HORI_FLIP))); //Add
	mpFlipTbx->SetQuickHelpText(FLIP_VERTICAL,String(SVX_RES(STR_QH_VERT_FLIP))); //Add
	
	mpMtrPosX->SetAccessibleRelationLabeledBy(mpFtPosX.get());	
	mpMtrPosY->SetAccessibleRelationLabeledBy(mpFtPosY.get());	
	mpMtrWidth->SetAccessibleRelationLabeledBy(mpFtWidth.get());	
	mpMtrHeight->SetAccessibleRelationLabeledBy(mpFtHeight.get());	
	mpMtrAngle->SetAccessibleRelationLabeledBy(mpFtAngle.get());
#ifdef HAS_IA2
	mpMtrAngle->SetMpSubEditAccLableBy(mpFtAngle.get());
#endif
	mpFlipTbx->SetAccessibleRelationLabeledBy(mpFtFlip.get());

    mpMtrAngle->InsertValue(0, FUNIT_CUSTOM);
	mpMtrAngle->InsertValue(4500, FUNIT_CUSTOM);
	mpMtrAngle->InsertValue(9000, FUNIT_CUSTOM);
	mpMtrAngle->InsertValue(13500, FUNIT_CUSTOM);
	mpMtrAngle->InsertValue(18000, FUNIT_CUSTOM);
	mpMtrAngle->InsertValue(22500, FUNIT_CUSTOM);
	mpMtrAngle->InsertValue(27000, FUNIT_CUSTOM);
	mpMtrAngle->InsertValue(31500, FUNIT_CUSTOM);
    mpMtrAngle->SetDropDownLineCount(mpMtrAngle->GetEntryCount());

    SfxViewShell* pCurSh = SfxViewShell::Current();
	if ( pCurSh )
		mpView = pCurSh->GetDrawView();
	else
		mpView = NULL;

	if ( mpView != NULL )
	{
		maUIScale = mpView->GetModel()->GetUIScale();

		const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
		if(1 == rMarkList.GetMarkCount())
		{
			const SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
			const SdrObjKind eKind((SdrObjKind)pObj->GetObjIdentifier());

			if((pObj->GetObjInventor() == SdrInventor) && (OBJ_TEXT == eKind || OBJ_TITLETEXT == eKind || OBJ_OUTLINETEXT == eKind) && ((SdrTextObj*)pObj)->HasText())
			{
				mbAdjustEnabled = true;
			}
		}
	}
	
    mePoolUnit = maTransfWidthControl.GetCoreMetric();
	meDlgUnit = GetModuleFieldUnit();
	SetFieldUnit( *mpMtrPosX, meDlgUnit, true );
	SetFieldUnit( *mpMtrPosY, meDlgUnit, true );
	SetFieldUnit( *mpMtrWidth, meDlgUnit, true );
	SetFieldUnit( *mpMtrHeight, meDlgUnit, true );
}



void PosSizePropertyPanel::SetupIcons(void)
{
    if(Theme::GetBoolean(Theme::Bool_UseSymphonyIcons))
    {
        // todo
    }
    else
    {
        // todo
    }
}



PosSizePropertyPanel* PosSizePropertyPanel::Create (
    Window* pParent,
    const cssu::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings,
    const cssu::Reference<css::ui::XSidebar>& rxSidebar)
{
    if (pParent == NULL)
        throw lang::IllegalArgumentException(A2S("no parent Window given to PosSizePropertyPanel::Create"), NULL, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException(A2S("no XFrame given to PosSizePropertyPanel::Create"), NULL, 1);
    if (pBindings == NULL)
        throw lang::IllegalArgumentException(A2S("no SfxBindings given to PosSizePropertyPanel::Create"), NULL, 2);
    
    return new PosSizePropertyPanel(
        pParent,
        rxFrame,
        pBindings,
        rxSidebar);
}



void PosSizePropertyPanel::DataChanged(
    const DataChangedEvent& rEvent)
{
    (void)rEvent;
    
    SetupIcons();
}



void PosSizePropertyPanel::HandleContextChange(
    const ::sfx2::sidebar::EnumContext aContext)
{
    if(maContext == aContext)
    {
        // Nothing to do.
        return;
    }

    maContext = aContext;

    sal_Int32 nLayoutMode (0);
    switch (maContext.GetCombinedContext_DI())
    {
        case CombinedEnumContext(Application_WriterAndWeb, Context_Draw):
            nLayoutMode = 0;
            break;

        case CombinedEnumContext(Application_WriterAndWeb, Context_Graphic):
        case CombinedEnumContext(Application_WriterAndWeb, Context_Media):
        case CombinedEnumContext(Application_WriterAndWeb, Context_Frame):
        case CombinedEnumContext(Application_WriterAndWeb, Context_OLE):
        case CombinedEnumContext(Application_WriterAndWeb, Context_Form):
            nLayoutMode = 1;
            break;

        case CombinedEnumContext(Application_Calc, Context_Draw):
        case CombinedEnumContext(Application_Calc, Context_Graphic):
        case CombinedEnumContext(Application_DrawImpress, Context_Draw):
        case CombinedEnumContext(Application_DrawImpress, Context_TextObject):
        case CombinedEnumContext(Application_DrawImpress, Context_Graphic):
            nLayoutMode = 2;
            break;

        case CombinedEnumContext(Application_Calc, Context_Chart):
        case CombinedEnumContext(Application_Calc, Context_Form):
        case CombinedEnumContext(Application_Calc, Context_Media):
        case CombinedEnumContext(Application_Calc, Context_OLE):
        case CombinedEnumContext(Application_Calc, Context_MultiObject):
        case CombinedEnumContext(Application_DrawImpress, Context_Media):
        case CombinedEnumContext(Application_DrawImpress, Context_Form):
        case CombinedEnumContext(Application_DrawImpress, Context_OLE):
        case CombinedEnumContext(Application_DrawImpress, Context_3DObject):
        case CombinedEnumContext(Application_DrawImpress, Context_MultiObject):
            nLayoutMode = 3;
            break;
    }    
    switch (nLayoutMode)
    {
        case 0:
		{
			mpMtrWidth->SetMin( 2 );
			mpMtrHeight->SetMin( 2 );
			mpFtPosX->Hide();
			mpMtrPosX->Hide();
			mpFtPosY->Hide();
			mpMtrPosY->Hide();
			
            //rotation
			mpFtAngle->Show();
			mpMtrAngle->Show();
			mpDial->Show();
			
            //flip
			mpFtFlip->Show();
			mpFlipTbx->Show();
			Size aTbxSize = mpFlipTbx->CalcWindowSizePixel();
			mpFlipTbx->SetOutputSizePixel( aTbxSize );
			mbIsFlip = true;

			mpFtWidth->SetPosPixel(Point(LogicToPixel(Point(FT_POSITION_X_X,FT_POSITION_X_Y), MAP_APPFONT)));
			mpMtrWidth->SetPosPixel(Point(LogicToPixel(Point(MF_POSITION_X_X,MF_POSITION_X_Y), MAP_APPFONT)));
			mpFtHeight->SetPosPixel(Point(LogicToPixel(Point(FT_POSITION_Y_X,FT_POSITION_Y_Y), MAP_APPFONT)));
			mpMtrHeight->SetPosPixel(Point(LogicToPixel(Point(MF_POSITION_Y_X,MF_POSITION_Y_Y), MAP_APPFONT)));
			mpCbxScale->SetPosPixel(Point(LogicToPixel(Point(FT_WIDTH_X,FT_WIDTH_Y), MAP_APPFONT)));

			mpFtAngle->SetPosPixel(Point(LogicToPixel(Point(FT_ANGLE_X,FT_ANGLE_Y), MAP_APPFONT)));
			mpMtrAngle->SetPosPixel(Point(LogicToPixel(Point(MF_ANGLE_X2,MF_ANGLE_Y2), MAP_APPFONT)));
			mpFlipTbx->SetPosPixel(Point(LogicToPixel(Point(FLIP_HORI_X2,FLIP_HORI_Y2), MAP_APPFONT)));
			mpDial->SetPosPixel(Point(LogicToPixel(Point(ROTATE_CONTROL_X2,ROTATE_CONTROL_Y2), MAP_APPFONT)));
			mpFtFlip->SetPosPixel(Point(LogicToPixel(Point(FT_FLIP_X2,FT_FLIP_Y2), MAP_APPFONT)));

			Size aSize(GetOutputSizePixel().Width(),PS_SECTIONPAGE_HEIGHT2);
			aSize = LogicToPixel( aSize, MapMode(MAP_APPFONT) ); 
			SetSizePixel(aSize);
            if (mxSidebar.is())
                mxSidebar->requestLayout();
		}
		break;

        case 1:		
		{
			mpMtrWidth->SetMin( 2 );
			mpMtrHeight->SetMin( 2 );
			mpFtPosX->Hide();
			mpMtrPosX->Hide();
			mpFtPosY->Hide();
			mpMtrPosY->Hide();
        
			//rotation
			mpFtAngle->Hide();
			mpMtrAngle->Hide();
			mpDial->Hide();
        
			//flip
			mpFlipTbx->Hide();
			mpFtFlip->Hide();
			mbIsFlip = false;
        
			mpFtWidth->SetPosPixel(Point(LogicToPixel(Point(FT_POSITION_X_X,FT_POSITION_X_Y), MAP_APPFONT)));
			mpMtrWidth->SetPosPixel(Point(LogicToPixel(Point(MF_POSITION_X_X,MF_POSITION_X_Y), MAP_APPFONT)));
			mpFtHeight->SetPosPixel(Point(LogicToPixel(Point(FT_POSITION_Y_X,FT_POSITION_Y_Y), MAP_APPFONT)));
			mpMtrHeight->SetPosPixel(Point(LogicToPixel(Point(MF_POSITION_Y_X,MF_POSITION_Y_Y), MAP_APPFONT)));
			mpCbxScale->SetPosPixel(Point(LogicToPixel(Point(FT_WIDTH_X,FT_WIDTH_Y), MAP_APPFONT)));
        
			Size aSize(GetOutputSizePixel().Width(),PS_SECTIONPAGE_HEIGHT3);
			aSize = LogicToPixel( aSize, MapMode(MAP_APPFONT) ); 
			SetSizePixel(aSize);
            if (mxSidebar.is())
                mxSidebar->requestLayout();
		}
		break;

        case 2:
		{
			mpMtrWidth->SetMin( 1 );
			mpMtrHeight->SetMin( 1 );
			mpFtPosX->Show();
			mpMtrPosX->Show();
			mpFtPosY->Show();
			mpMtrPosY->Show();
        
			//rotation
			mpFtAngle->Show();
			mpMtrAngle->Show();
			mpDial->Show();
        
            //flip
			mpFlipTbx->Show();
			mpFtFlip->Show();
			Size aTbxSize = mpFlipTbx->CalcWindowSizePixel();
			mpFlipTbx->SetOutputSizePixel( aTbxSize );
			mbIsFlip = true;
        
			Size aSize(GetOutputSizePixel().Width(),PS_SECTIONPAGE_HEIGHT);
			aSize = LogicToPixel( aSize, MapMode(MAP_APPFONT) ); 
			SetSizePixel(aSize);
            if (mxSidebar.is())
                mxSidebar->requestLayout();
		}
		break;
        
		case 3:
		{
			mpMtrWidth->SetMin( 1 );
			mpMtrHeight->SetMin( 1 );
			mpFtPosX->Show();
			mpMtrPosX->Show();
			mpFtPosY->Show();
			mpMtrPosY->Show();
        
			//rotation
			mpFtAngle->Hide();
			mpMtrAngle->Hide();
			mpDial->Hide();
        
			//flip
			mpFlipTbx->Hide();
			mpFtFlip->Hide();
			mbIsFlip = false;
        
			Size aSize(GetOutputSizePixel().Width(),PS_SECTIONPAGE_HEIGHT4);
			aSize = LogicToPixel( aSize, MapMode(MAP_APPFONT) ); 
			SetSizePixel(aSize);
            if (mxSidebar.is())
                mxSidebar->requestLayout();
		}
		break;
	}

    //Added for windows classic theme
	mpFlipTbx->SetBackground(Wallpaper());
	mpFlipTbx->SetPaintTransparent(true);
}



IMPL_LINK( PosSizePropertyPanel, ChangeWidthHdl, void*, /*pBox*/ )
{
	if( mpCbxScale->IsChecked() &&
		mpCbxScale->IsEnabled() )
	{
		long nHeight = (long) ( ((double) mlOldHeight * (double) mpMtrWidth->GetValue()) / (double) mlOldWidth );
		if( nHeight <= mpMtrHeight->GetMax( FUNIT_NONE ) )
		{
			mpMtrHeight->SetUserValue( nHeight, FUNIT_NONE );
		}
		else
		{
			nHeight = (long)mpMtrHeight->GetMax( FUNIT_NONE );
			mpMtrHeight->SetUserValue( nHeight );
			const long nWidth = (long) ( ((double) mlOldWidth * (double) nHeight) / (double) mlOldHeight );
			mpMtrWidth->SetUserValue( nWidth, FUNIT_NONE );
		}
	}
	executeSize();
	return 0;
}



IMPL_LINK( PosSizePropertyPanel, ChangeHeightHdl, void *, EMPTYARG )
{
	if( mpCbxScale->IsChecked() &&
		mpCbxScale->IsEnabled() )
	{
		long nWidth = (long) ( ((double)mlOldWidth * (double)mpMtrHeight->GetValue()) / (double)mlOldHeight );
		if( nWidth <= mpMtrWidth->GetMax( FUNIT_NONE ) )
		{
			mpMtrWidth->SetUserValue( nWidth, FUNIT_NONE );
		}
		else
		{
			nWidth = (long)mpMtrWidth->GetMax( FUNIT_NONE );
			mpMtrWidth->SetUserValue( nWidth );
			const long nHeight = (long) ( ((double)mlOldHeight * (double)nWidth) / (double)mlOldWidth );
			mpMtrHeight->SetUserValue( nHeight, FUNIT_NONE );
		}
	}
	executeSize();
	return 0;
}



IMPL_LINK( PosSizePropertyPanel, ChangePosXHdl, void *, EMPTYARG )
{
	executePosX();
	return 0;
}



IMPL_LINK( PosSizePropertyPanel, ChangePosYHdl, void *, EMPTYARG )
{
	executePosY();
	return 0;
}



IMPL_LINK( PosSizePropertyPanel, ClickAutoHdl, void *, EMPTYARG )
{
	if ( mpCbxScale->IsChecked() )
	{
		mlOldWidth  = Max( GetCoreValue( *mpMtrWidth,  mePoolUnit ), 1L );
		mlOldHeight = Max( GetCoreValue( *mpMtrHeight, mePoolUnit ), 1L );
	}

	// mpCbxScale must synchronized with that on Position and Size tabpage on Shape Properties dialog
	SvtViewOptions	aPageOpt( E_TABPAGE, String::CreateFromInt32( RID_SVXPAGE_POSITION_SIZE ) );
	aPageOpt.SetUserItem( USERITEM_NAME, ::com::sun::star::uno::makeAny( ::rtl::OUString( String::CreateFromInt32( mpCbxScale->IsChecked() ) ) ) );

	return 0;
}



IMPL_LINK( PosSizePropertyPanel, AngleModifiedHdl, void *, EMPTYARG )
{
	String sTmp = mpMtrAngle->GetText();
	bool    bNegative = 0;
	sal_Unicode nChar = sTmp.GetChar( 0 );

	if( nChar == '-' )
	{
		bNegative = 1;
		nChar = sTmp.GetChar( 1 );
	}

	if( (nChar < '0') || (nChar > '9') )
		return 0;
	double dTmp = sTmp.ToDouble();
	if(bNegative)
	{
		while(dTmp<0)
			dTmp += 360;
	}
	sal_Int64 nTmp = dTmp*100;

	SfxInt32Item aAngleItem( SID_ATTR_TRANSFORM_ANGLE,(sal_uInt32) nTmp);
	SfxInt32Item aRotXItem( SID_ATTR_TRANSFORM_ROT_X,(sal_uInt32) mlRotX);
	SfxInt32Item aRotYItem( SID_ATTR_TRANSFORM_ROT_Y,(sal_uInt32) mlRotY);
	
	GetBindings()->GetDispatcher()->Execute(
		SID_ATTR_TRANSFORM, SFX_CALLMODE_RECORD, &aAngleItem, &aRotXItem, &aRotYItem, 0L );

	return 0;
}



IMPL_LINK( PosSizePropertyPanel, RotationHdl, void *, EMPTYARG )
{
	sal_Int32 nTmp = mpDial->GetRotation();

	SfxInt32Item aAngleItem( SID_ATTR_TRANSFORM_ANGLE,(sal_uInt32) nTmp);
	SfxInt32Item aRotXItem( SID_ATTR_TRANSFORM_ROT_X,(sal_uInt32) mlRotX);
	SfxInt32Item aRotYItem( SID_ATTR_TRANSFORM_ROT_Y,(sal_uInt32) mlRotY);
	
	GetBindings()->GetDispatcher()->Execute(
		SID_ATTR_TRANSFORM, SFX_CALLMODE_RECORD, &aAngleItem, &aRotXItem, &aRotYItem, 0L );

	return 0;
}



IMPL_LINK( PosSizePropertyPanel, FlipHdl, ToolBox*, pBox )
{
	switch (pBox->GetCurItemId())
	{
		case FLIP_HORIZONTAL:
		{
			SfxVoidItem aHoriItem (SID_FLIP_HORIZONTAL);
			GetBindings()->GetDispatcher()->Execute(
				SID_FLIP_HORIZONTAL, SFX_CALLMODE_RECORD, &aHoriItem, 0L );
		}
		break;
		case FLIP_VERTICAL:
		{
			SfxVoidItem aVertItem (SID_FLIP_VERTICAL );
			GetBindings()->GetDispatcher()->Execute(
				SID_FLIP_VERTICAL, SFX_CALLMODE_RECORD, &aVertItem, 0L );
		}
		break;
	}
	return 0;
}



void PosSizePropertyPanel::NotifyItemUpdate( 
    sal_uInt16 nSID, 
    SfxItemState eState, 
    const SfxPoolItem* pState)
{  
	mpFtAngle->Enable();
	mpMtrAngle->Enable();
	mpDial->Enable();
	mpFtFlip->Enable();
	mpFlipTbx->Enable();

    const SfxUInt32Item*	pWidthItem;
	const SfxUInt32Item*	pHeightItem;		
	
	SfxViewShell* pCurSh = SfxViewShell::Current();
	if ( pCurSh )
		mpView = pCurSh->GetDrawView();
	else
		mpView = NULL;

	if ( mpView == NULL )
		return;

	const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

	if(1 == rMarkList.GetMarkCount())
	{
		const SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
		const SdrObjKind eKind((SdrObjKind)pObj->GetObjIdentifier());

		if((pObj->GetObjInventor() == SdrInventor) && (OBJ_TEXT == eKind || OBJ_TITLETEXT == eKind || OBJ_OUTLINETEXT == eKind) && ((SdrTextObj*)pObj)->HasText())
			mbAdjustEnabled = true;
		else
			mbAdjustEnabled = false;
	}
	else
		mbAdjustEnabled = false;

	switch (nSID)
    {
        case SID_ATTR_TRANSFORM_WIDTH:
            if ( SFX_ITEM_AVAILABLE == eState )
            {
                pWidthItem = dynamic_cast< const SfxUInt32Item* >(pState);

                if(pWidthItem)
                {
                    long mlOldWidth1 = pWidthItem->GetValue();

                    mlOldWidth1 = Fraction( mlOldWidth1 ) / maUIScale;
                    SetMetricValue( *mpMtrWidth, mlOldWidth1, mePoolUnit );
                    mlOldWidth = mlOldWidth1;
                    break;
                }
            }

            mpMtrWidth->SetText( String());
            break;
            
        case SID_ATTR_TRANSFORM_HEIGHT:
            if ( SFX_ITEM_AVAILABLE == eState )
            {
                pHeightItem = dynamic_cast< const SfxUInt32Item* >(pState);

                if(pHeightItem)
                {
                    long mlOldHeight1 = pHeightItem->GetValue();

                    mlOldHeight1 = Fraction( mlOldHeight1 ) / maUIScale;
                    SetMetricValue( *mpMtrHeight, mlOldHeight1, mePoolUnit );
                    mlOldHeight = mlOldHeight1;
                    break;
                }
            }

            mpMtrHeight->SetText( String());
            break;

        case SID_ATTR_TRANSFORM_POS_X:
            if(SFX_ITEM_AVAILABLE == eState)
            {
                const SfxInt32Item* pItem = dynamic_cast< const SfxInt32Item* >(pState);

                if(pItem)
                {
                    long nTmp = pItem->GetValue(); 
                    nTmp = Fraction( nTmp ) / maUIScale;
                    SetMetricValue( *mpMtrPosX, nTmp, mePoolUnit );
                    break;
                }
            }

            mpMtrPosX->SetText( String());
            break;

        case SID_ATTR_TRANSFORM_POS_Y:
            if(SFX_ITEM_AVAILABLE == eState)
            {
                const SfxInt32Item* pItem = dynamic_cast< const SfxInt32Item* >(pState);

                if(pItem)
                {
                    long nTmp = pItem->GetValue(); 
                    nTmp = Fraction( nTmp ) / maUIScale;
                    SetMetricValue( *mpMtrPosY, nTmp, mePoolUnit );
                    break;
                }
            }

            mpMtrPosY->SetText( String());
            break;

        case SID_ATTR_TRANSFORM_ROT_X:
            if (SFX_ITEM_AVAILABLE == eState)
            {
                const SfxInt32Item* pItem = dynamic_cast< const SfxInt32Item* >(pState);

                if(pItem)
                {
                    mlRotX = pItem->GetValue(); 
                    mlRotX = Fraction( mlRotX ) / maUIScale;
                }
            }
            break;

        case SID_ATTR_TRANSFORM_ROT_Y:
            if (SFX_ITEM_AVAILABLE == eState)
            {
                const SfxInt32Item* pItem = dynamic_cast< const SfxInt32Item* >(pState);

                if(pItem)
                {
                    mlRotY = pItem->GetValue(); 
                    mlRotY = Fraction( mlRotY ) / maUIScale;
                }
            }
            break;

        case SID_ATTR_TRANSFORM_PROTECT_POS:
            if(SFX_ITEM_AVAILABLE == eState)
            {
                const SfxBoolItem* pItem = dynamic_cast< const SfxBoolItem* >(pState);

                if(pItem)
                {
                    // record the state of position protect
                    mbPositionProtected = pItem->GetValue();
                    break;
                }
            }

            mbPositionProtected = false;
            break;

        case SID_ATTR_TRANSFORM_PROTECT_SIZE:
            if(SFX_ITEM_AVAILABLE == eState)
            {
                const SfxBoolItem* pItem = dynamic_cast< const SfxBoolItem* >(pState);

                if(pItem)
                {
                    // record the state of size protect
                    mbSizeProtected = pItem->GetValue();
                    break;
                }
            }

            mbSizeProtected = false;
            break;

        case SID_ATTR_TRANSFORM_AUTOWIDTH:
            if(SFX_ITEM_AVAILABLE == eState)
            {
                const SfxBoolItem* pItem = dynamic_cast< const SfxBoolItem* >(pState);

                if(pItem)
                {
                    mbAutoWidth = pItem->GetValue();
                }
            }
            break;

        case SID_ATTR_TRANSFORM_AUTOHEIGHT:
            if(SFX_ITEM_AVAILABLE == eState)
            {
                const SfxBoolItem* pItem = dynamic_cast< const SfxBoolItem* >(pState);

                if(pItem)
                {
                    mbAutoHeight = pItem->GetValue();
                }
            }
            break;

        case SID_ATTR_TRANSFORM_ANGLE:
            if (eState >= SFX_ITEM_AVAILABLE)
            {
                const SfxInt32Item* pItem = dynamic_cast< const SfxInt32Item* >(pState);

                if(pItem)
                {
                    long nTmp = pItem->GetValue(); 

                    mpMtrAngle->SetValue( nTmp );
                    mpDial->SetRotation( nTmp );

                    switch(nTmp)
                    {
                        case 0:
                            mpMtrAngle->SelectEntryPos(0);
                            break;
                        case 4500:
                            mpMtrAngle->SelectEntryPos(1);
                            break;
                        case 9000:
                            mpMtrAngle->SelectEntryPos(2);
                            break;
                        case 13500:
                            mpMtrAngle->SelectEntryPos(3);
                            break;
                        case 18000:
                            mpMtrAngle->SelectEntryPos(4);
                            break;
                        case 22500:
                            mpMtrAngle->SelectEntryPos(5);
                            break;
                        case 27000:
                            mpMtrAngle->SelectEntryPos(6);
                            break;
                        case 315000:
                            mpMtrAngle->SelectEntryPos(7);
                    }

                    break;
                }
            }

            mpMtrAngle->SetText( String() );
            mpDial->SetRotation( 0 );
            break;
            
        case SID_ATTR_METRIC:
            MetricState( eState, pState );
            break;

        default:
            break;
    }

    const sal_Int32 nCombinedContext(maContext.GetCombinedContext_DI());

    switch (rMarkList.GetMarkCount())
    {
        case 0:
            break;

        case 1:
        {
            const SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            const SdrObjKind eKind((SdrObjKind)pObj->GetObjIdentifier());

            if(((nCombinedContext == CombinedEnumContext(Application_DrawImpress, Context_Draw) 
               || nCombinedContext == CombinedEnumContext(Application_DrawImpress, Context_TextObject)
                 ) && OBJ_EDGE == eKind)
               || OBJ_CAPTION == eKind)
            {
                mpFtAngle->Disable();
                mpMtrAngle->Disable();
                mpDial->Disable();
                mpFlipTbx->Disable();
                mpFtFlip->Disable();
            }
            break;
        }
        
        default:
        {
            sal_uInt16 nMarkObj = 0;
            bool isNoEdge = true;
            while(rMarkList.GetMark(nMarkObj))
            {
                const SdrObject* pObj = rMarkList.GetMark(nMarkObj)->GetMarkedSdrObj();
                const SdrObjKind eKind((SdrObjKind)pObj->GetObjIdentifier());

                if(((nCombinedContext == CombinedEnumContext(Application_DrawImpress, Context_Draw)
                  || nCombinedContext == CombinedEnumContext(Application_DrawImpress, Context_TextObject)
                     ) && OBJ_EDGE == eKind)
                  || OBJ_CAPTION == eKind)
                {
                    isNoEdge = false;
                    break;
                }
                nMarkObj++;
            }
            if(!isNoEdge)
            {
                mpFtAngle->Disable();
                mpMtrAngle->Disable();
                mpDial->Disable();
                mpFlipTbx->Disable();
                mpFtFlip->Disable();
            }
            break;
        }
    }

    if(nCombinedContext == CombinedEnumContext(Application_DrawImpress, Context_TextObject))
	{
		mpFlipTbx->Disable();
		mpFtFlip->Disable();
	}

    DisableControls();

	// mpCbxScale must synchronized with that on Position and Size tabpage on Shape Properties dialog
	SvtViewOptions	aPageOpt( E_TABPAGE, String::CreateFromInt32( RID_SVXPAGE_POSITION_SIZE ) );
	String	sUserData;
	::com::sun::star::uno::Any	aUserItem = aPageOpt.GetUserItem( USERITEM_NAME );
	::rtl::OUString	aTemp;
	if ( aUserItem >>= aTemp )
		sUserData = String( aTemp );
	mpCbxScale->Check( (bool)sUserData.ToInt32() );
}



SfxBindings* PosSizePropertyPanel::GetBindings() 
{ 
    return mpBindings; 
}



void PosSizePropertyPanel::executeSize()
{
	if ( mpMtrWidth->IsValueModified() || mpMtrHeight->IsValueModified())
	{
		Fraction aUIScale = mpView->GetModel()->GetUIScale();

		// get Width
		double nWidth = (double)mpMtrWidth->GetValue( meDlgUnit );
		nWidth = MetricField::ConvertDoubleValue( nWidth, mpMtrWidth->GetBaseValue(), mpMtrWidth->GetDecimalDigits(), meDlgUnit, FUNIT_100TH_MM );
		long lWidth = (long)(nWidth * (double)aUIScale);
		lWidth = OutputDevice::LogicToLogic( lWidth, MAP_100TH_MM, (MapUnit)mePoolUnit );
		lWidth = (long)mpMtrWidth->Denormalize( lWidth );

		// get Height
		double nHeight = (double)mpMtrHeight->GetValue( meDlgUnit );
		nHeight = MetricField::ConvertDoubleValue( nHeight, mpMtrHeight->GetBaseValue(), mpMtrHeight->GetDecimalDigits(), meDlgUnit, FUNIT_100TH_MM );
		long lHeight = (long)(nHeight * (double)aUIScale);
		lHeight = OutputDevice::LogicToLogic( lHeight, MAP_100TH_MM, (MapUnit)mePoolUnit );
		lHeight = (long)mpMtrWidth->Denormalize( lHeight );

		// put Width & Height to itemset
		SfxUInt32Item aWidthItem( SID_ATTR_TRANSFORM_WIDTH, (sal_uInt32) lWidth);
		SfxUInt32Item aHeightItem( SID_ATTR_TRANSFORM_HEIGHT, (sal_uInt32) lHeight);
		SfxAllEnumItem aPointItem (SID_ATTR_TRANSFORM_SIZE_POINT, (sal_uInt16)meRP);
        const sal_Int32 nCombinedContext(maContext.GetCombinedContext());

        if( nCombinedContext == CombinedEnumContext(Application_Writer, Context_Graphic) // mnContextId == PROPERTY_CONTEXT_SW_GRAPHIC 
            || nCombinedContext == CombinedEnumContext(Application_Writer, Context_OLE) //mnContextId == PROPERTY_CONTEXT_SW_OLE
            )
        // if( mnContextId == PROPERTY_CONTEXT_SW_GRAPHIC || mnContextId == PROPERTY_CONTEXT_SW_OLE )
        {
			GetBindings()->GetDispatcher()->Execute(SID_ATTR_TRANSFORM, SFX_CALLMODE_RECORD, &aWidthItem, &aHeightItem, &aPointItem, 0L );
        }
		else
		{
			if ( (mpMtrWidth->IsValueModified()) && (mpMtrHeight->IsValueModified()))
				GetBindings()->GetDispatcher()->Execute(SID_ATTR_TRANSFORM, SFX_CALLMODE_RECORD, &aWidthItem, &aHeightItem, &aPointItem, 0L );
			else if( mpMtrWidth->IsValueModified())
				GetBindings()->GetDispatcher()->Execute(SID_ATTR_TRANSFORM, SFX_CALLMODE_RECORD, &aWidthItem, &aPointItem, 0L );
			else if ( mpMtrHeight->IsValueModified())
				GetBindings()->GetDispatcher()->Execute(SID_ATTR_TRANSFORM, SFX_CALLMODE_RECORD, &aHeightItem, &aPointItem, 0L );
		}		
	}
}



void PosSizePropertyPanel::executePosX()
{
	if ( mpMtrPosX->IsValueModified())
	{
		long lX = GetCoreValue( *mpMtrPosX, mePoolUnit );
		if( mbMtrPosXMirror )
			lX = -lX;
		long lY = GetCoreValue( *mpMtrPosY, mePoolUnit );

		Size aPageSize;
		Rectangle aRect;
		maRect = mpView->GetAllMarkedRect();	
		aRect = mpView->GetAllMarkedRect();

		Fraction aUIScale = mpView->GetModel()->GetUIScale();
		lX += maAnchorPos.X();
		lX = Fraction( lX ) * aUIScale;
		lY += maAnchorPos.Y();
		lY = Fraction( lY ) * aUIScale;
		
		SfxInt32Item aPosXItem( SID_ATTR_TRANSFORM_POS_X,(sal_uInt32) lX);
		SfxInt32Item aPosYItem( SID_ATTR_TRANSFORM_POS_Y,(sal_uInt32) lY);
	
		GetBindings()->GetDispatcher()->Execute(
			SID_ATTR_TRANSFORM, SFX_CALLMODE_RECORD, &aPosXItem, 0L );	
	}
}



void PosSizePropertyPanel::executePosY()
{
	if ( mpMtrPosY->IsValueModified() )
	{
		long lX = GetCoreValue( *mpMtrPosX, mePoolUnit );
		long lY = GetCoreValue( *mpMtrPosY, mePoolUnit );

		Size aPageSize;
		Rectangle aRect;
		maRect = mpView->GetAllMarkedRect();	
		aRect = mpView->GetAllMarkedRect();

		Fraction aUIScale = mpView->GetModel()->GetUIScale();
		lX += maAnchorPos.X();
		lX = Fraction( lX ) * aUIScale;
		lY += maAnchorPos.Y();
		lY = Fraction( lY ) * aUIScale;
		
		SfxInt32Item aPosXItem( SID_ATTR_TRANSFORM_POS_X,(sal_uInt32) lX);
		SfxInt32Item aPosYItem( SID_ATTR_TRANSFORM_POS_Y,(sal_uInt32) lY);
	
		GetBindings()->GetDispatcher()->Execute(
			SID_ATTR_TRANSFORM, SFX_CALLMODE_RECORD, &aPosYItem, 0L );	
	}
}



void PosSizePropertyPanel::MetricState( SfxItemState eState, const SfxPoolItem* pState )
{
	bool bPosXBlank = false;
	bool bPosYBlank = false;
	bool bWidthBlank = false;
	bool bHeightBlank = false;
	String sNull = String::CreateFromAscii("");
	meDlgUnit = GetCurrentUnit(eState,pState);

	if( mpMtrPosX->GetText() == sNull )
		bPosXBlank = true;
	SetFieldUnit( *mpMtrPosX, meDlgUnit, true );
	if(bPosXBlank)
		mpMtrPosX->SetText(String());

	if( mpMtrPosY->GetText() == sNull )
		bPosYBlank = true;
	SetFieldUnit( *mpMtrPosY, meDlgUnit, true );
	if(bPosYBlank)
		mpMtrPosY->SetText(String());

	if( mpMtrWidth->GetText() == sNull )
		bWidthBlank = true;
	SetFieldUnit( *mpMtrWidth, meDlgUnit, true );
	if(bWidthBlank)
		mpMtrWidth->SetText(String());

	if( mpMtrHeight->GetText() == sNull )
		bHeightBlank = true;
	SetFieldUnit( *mpMtrHeight, meDlgUnit, true );
	if(bHeightBlank)
		mpMtrHeight->SetText(String());
}



FieldUnit PosSizePropertyPanel::GetCurrentUnit( SfxItemState eState, const SfxPoolItem* pState )
{
	FieldUnit eUnit = FUNIT_NONE;

	if ( pState && eState >= SFX_ITEM_DEFAULT )
    {
		eUnit = (FieldUnit)( (const SfxUInt16Item*)pState )->GetValue();
    }
	else
	{
		SfxViewFrame* pFrame = SfxViewFrame::Current();
		SfxObjectShell* pSh = NULL;
		if ( pFrame )
			pSh = pFrame->GetObjectShell();
		if ( pSh )
		{
			SfxModule* pModule = pSh->GetModule();
			if ( pModule )
			{
				const SfxPoolItem* pItem = pModule->GetItem( SID_ATTR_METRIC );
				if ( pItem )
					eUnit = (FieldUnit)( (SfxUInt16Item*)pItem )->GetValue();
			}
			else
			{
				DBG_ERRORFILE( "GetModuleFieldUnit(): no module found" );
			}
		}
	}
	
	return eUnit;
}



void PosSizePropertyPanel::DisableControls()
{
    if( mbPositionProtected )
	{	
		// the position is protected("Position protect" option in modal dialog is checked),
		// disable all the Position controls in sidebar
		mpFtPosX->Disable();
		mpMtrPosX->Disable();
		mpFtPosY->Disable();
		mpMtrPosY->Disable();
		mpFtAngle->Disable();
		mpMtrAngle->Disable();
		mpDial->Disable();
		mpFtFlip->Disable();
		mpFlipTbx->Disable();

		mpFtWidth->Disable();
		mpMtrWidth->Disable();
		mpFtHeight->Disable();
		mpMtrHeight->Disable();
        mpCbxScale->Disable();
	}
	else
	{
		mpFtPosX->Enable();
		mpMtrPosX->Enable();
		mpFtPosY->Enable();
		mpMtrPosY->Enable();

        //mpFtAngle->Enable();
		//mpMtrAngle->Enable();
		//mpDial->Enable();
		//mpFtFlip->Enable();
		//mpFlipTbx->Enable();

		if( mbSizeProtected )
		{
			mpFtWidth->Disable();
			mpMtrWidth->Disable();
			mpFtHeight->Disable();
			mpMtrHeight->Disable();
			mpCbxScale->Disable();
		}
		else
		{
			if(	mbAdjustEnabled )
			{
				if( mbAutoWidth )
				{
					mpFtWidth->Disable();
					mpMtrWidth->Disable();
					mpCbxScale->Disable();
				}
				else
				{
					mpFtWidth->Enable();
					mpMtrWidth->Enable();
				}
				if( mbAutoHeight )
				{
					mpFtHeight->Disable();
					mpMtrHeight->Disable();
					mpCbxScale->Disable();
				}
				else
				{
					mpFtHeight->Enable();
					mpMtrHeight->Enable();
				}
				if( !mbAutoWidth && !mbAutoHeight )
					mpCbxScale->Enable();
			}
			else
			{
				mpFtWidth->Enable();
				mpMtrWidth->Enable();
				mpFtHeight->Enable();
				mpMtrHeight->Enable();
				mpCbxScale->Enable();
			}
		}
	}
}


} } // end of namespace svx::sidebar

// eof