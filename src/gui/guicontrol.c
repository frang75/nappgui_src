/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: guicontrol.c
 *
 */

/* Generic-virtual gui control */

#include "guicontrol.h"
#include "component.inl"
#include "view.inl"
#include <core/strings.h>
#include <sewer/cassert.h>

/*---------------------------------------------------------------------------*/

Label *guicontrol_label(GuiControl *control)
{
    GuiComponent *component = cast(control, GuiComponent);
    if (component != NULL && component->type == ekGUI_TYPE_LABEL)
        return cast(component, Label);
    return NULL;
}

/*---------------------------------------------------------------------------*/

Button *guicontrol_button(GuiControl *control)
{
    GuiComponent *component = cast(control, GuiComponent);
    if (component != NULL && component->type == ekGUI_TYPE_BUTTON)
        return cast(component, Button);
    return NULL;
}

/*---------------------------------------------------------------------------*/

PopUp *guicontrol_popup(GuiControl *control)
{
    GuiComponent *component = cast(control, GuiComponent);
    if (component != NULL && component->type == ekGUI_TYPE_POPUP)
        return cast(component, PopUp);
    return NULL;
}

/*---------------------------------------------------------------------------*/

Edit *guicontrol_edit(GuiControl *control)
{
    GuiComponent *component = cast(control, GuiComponent);
    if (component != NULL && component->type == ekGUI_TYPE_EDITBOX)
        return cast(component, Edit);
    return NULL;
}

/*---------------------------------------------------------------------------*/

Combo *guicontrol_combo(GuiControl *control)
{
    GuiComponent *component = cast(control, GuiComponent);
    if (component != NULL && component->type == ekGUI_TYPE_COMBOBOX)
        return cast(component, Combo);
    return NULL;
}

/*---------------------------------------------------------------------------*/

ListBox *guicontrol_listbox(GuiControl *control)
{
    GuiComponent *component = cast(control, GuiComponent);
    if (component != NULL && component->type == ekGUI_TYPE_CUSTOMVIEW)
    {
        if (str_equ_c(_view_subtype((View *)component), "ListBox") == TRUE)
            return cast(component, ListBox);
    }
    return NULL;
}

/*---------------------------------------------------------------------------*/

UpDown *guicontrol_updown(GuiControl *control)
{
    GuiComponent *component = cast(control, GuiComponent);
    if (component != NULL && component->type == ekGUI_TYPE_UPDOWN)
        return cast(component, UpDown);
    return NULL;
}

/*---------------------------------------------------------------------------*/

Slider *guicontrol_slider(GuiControl *control)
{
    GuiComponent *component = cast(control, GuiComponent);
    if (component != NULL && component->type == ekGUI_TYPE_SLIDER)
        return cast(component, Slider);
    return NULL;
}

/*---------------------------------------------------------------------------*/

Progress *guicontrol_progress(GuiControl *control)
{
    GuiComponent *component = cast(control, GuiComponent);
    if (component != NULL && component->type == ekGUI_TYPE_PROGRESS)
        return cast(component, Progress);
    return NULL;
}

/*---------------------------------------------------------------------------*/

View *guicontrol_view(GuiControl *control)
{
    GuiComponent *component = cast(control, GuiComponent);
    if (component != NULL && component->type == ekGUI_TYPE_CUSTOMVIEW)
    {
        if (str_equ_c(_view_subtype(cast(component, View)), "View") == TRUE)
            return cast(component, View);
    }
    return NULL;
}

/*---------------------------------------------------------------------------*/

TextView *guicontrol_textview(GuiControl *control)
{
    GuiComponent *component = cast(control, GuiComponent);
    if (component != NULL && component->type == ekGUI_TYPE_TEXTVIEW)
        return cast(component, TextView);
    return NULL;
}

/*---------------------------------------------------------------------------*/

WebView *guicontrol_webview(GuiControl *control)
{
    GuiComponent *component = cast(control, GuiComponent);
    if (component != NULL && component->type == ekGUI_TYPE_WEBVIEW)
        return cast(component, WebView);
    return NULL;
}

/*---------------------------------------------------------------------------*/

ImageView *guicontrol_imageview(GuiControl *control)
{
    GuiComponent *component = cast(control, GuiComponent);
    if (component != NULL && component->type == ekGUI_TYPE_CUSTOMVIEW)
    {
        if (str_equ_c(_view_subtype(cast(component, View)), "ImageView") == TRUE)
            return cast(component, ImageView);
    }
    return NULL;
}

/*---------------------------------------------------------------------------*/

TableView *guicontrol_tableview(GuiControl *control)
{
    GuiComponent *component = cast(control, GuiComponent);
    if (component != NULL && component->type == ekGUI_TYPE_CUSTOMVIEW)
    {
        if (str_equ_c(_view_subtype(cast(component, View)), "TableView") == TRUE)
            return cast(component, TableView);
    }
    return NULL;
}

/*---------------------------------------------------------------------------*/

SplitView *guicontrol_splitview(GuiControl *control)
{
    GuiComponent *component = cast(control, GuiComponent);
    if (component != NULL && component->type == ekGUI_TYPE_SPLITVIEW)
        return cast(component, SplitView);
    return NULL;
}

/*---------------------------------------------------------------------------*/

Panel *guicontrol_panel(GuiControl *control)
{
    GuiComponent *component = cast(control, GuiComponent);
    if (component != NULL && component->type == ekGUI_TYPE_PANEL)
        return cast(component, Panel);
    return NULL;
}

/*---------------------------------------------------------------------------*/

void guicontrol_tag(GuiControl *control, const uint32_t tag)
{
    GuiComponent *component = cast(control, GuiComponent);
    cassert_no_null(component);
    component->tag.tag_uint32 = tag;
}

/*---------------------------------------------------------------------------*/

uint32_t guicontrol_get_tag(const GuiControl *control)
{
    GuiComponent *component = cast(control, GuiComponent);
    cassert_no_null(component);
    return component->tag.tag_uint32;
}
