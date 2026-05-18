/* Tabcontrols with associated panels */

#include "tabpanel.h"
#include "buttons.h"
#include "labels.h"
#include "listboxes.h"
#include "form.h"
#include "sliders.h"
#include "res_guihello.h"
#include <gui/guiall.h>

typedef struct _tabspaneldata_t TabsPanelData;

struct _tabspaneldata_t
{
    Layout *layout;
    Window *window;
};

/*---------------------------------------------------------------------------*/

static void i_destroy_data(TabsPanelData **data)
{
    heap_delete(data, TabsPanelData);
}

/*---------------------------------------------------------------------------*/

static Panel *i_panel(const uint32_t index, Window *window)
{
    switch (index)
    {
    case 0:
        return labels_single_line();
    case 1:
        return buttons_basics(NULL);
    case 2:
        return listboxes();
    case 3:
        return form_basic(window);
    case 4:
        return sliders();
    default:
        cassert_default(index);
    }

    return NULL;
}

/*---------------------------------------------------------------------------*/

static void i_OnTabSelect(TabsPanelData *data, Event *e)
{
    const EvButton *p = event_params(e, EvButton);
    Panel *panel = NULL;
    cassert_no_null(p);
    cassert_no_null(data);
    panel = i_panel(p->index, data->window);
    layout_panel_replace(data->layout, panel, 0, 1);
    layout_update(data->layout);
}

/*---------------------------------------------------------------------------*/

static Tabs *i_tabs_create(void)
{
    Tabs *tabs = tabs_create(ekGUI_POS_TOP);
    tabs_length(tabs, 300);
    tabs_add_elem(tabs, "Labels", NULL);
    tabs_add_elem(tabs, "Buttons", NULL);
    tabs_add_elem(tabs, "ListBoxes", NULL);
    tabs_add_elem(tabs, "Form", NULL);
    tabs_add_elem(tabs, "Sliders", NULL);
    tabs_tooltip(tabs, "TabControl Tooltip");
    return tabs;
}

/*---------------------------------------------------------------------------*/

static Layout *i_layout(TabsPanelData *data, Window *window)
{
    Layout *layout = layout_create(1, 2);
    Tabs *tabs = i_tabs_create();
    Panel *panel = i_panel(0, window);
    cassert_no_null(data);
    layout_tabs(layout, tabs, 0, 0);
    layout_panel(layout, panel, 0, 1);
    tabs_OnSelect(tabs, listener(data, i_OnTabSelect, TabsPanelData));
    data->layout = layout;
    data->window = window;
    return layout;
}

/*---------------------------------------------------------------------------*/

Panel *tabpanel(Window *window)
{
    TabsPanelData *data = heap_new0(TabsPanelData);
    Panel *panel = panel_create();
    Layout *layout = i_layout(data, window);
    panel_layout(panel, layout);
    panel_data(panel, &data, i_destroy_data, TabsPanelData);
    return panel;
}
